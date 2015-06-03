#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "vector.h"
#include "polygon.h"
#include "queue.h"
#include "energy.h"
#include "grid.h"
#include "network.h"
#include "gridglobals.h"
#include "colors.h"
#include "options.h"

#include "SDL.h"

int main( int argc,char** argv ) {
	printf( "The Grid - server\n");
	printf( "written by Erik Peldan\n");

	GRID_cmdLineOption options[] = { 
		{ "--help", "-h", 0 },
		{ "--port", "-p", 1 },
		{NULL, NULL, 0 }
	};

	GRID_parseOptions( options, argc -1, argv + 1 );
	if( GRID_findOption("--help", options )->value[0] ) {
		printf("HELP:\n");
		printf("--port or -p specifies port (default %s)\n", PORT_NUMBER_STRING);
		return 1;
	}
	const char* portString;
	if( strlen( portString = GRID_findOption("--port", options )->value) ) {
		printf("port %s chosen\n", portString );
	}
	else {
		portString = PORT_NUMBER_STRING;
	}


	int numTiles = NUM_TILES, perPacket = PACKET_SIZE / sizeof( GRID_Tile );
	printf( "%i tiles. Each packet holds a maximum of %i tiles\n", numTiles, perPacket );

	GRID_ColorIPEntry *entries = malloc( sizeof( GRID_ColorIPEntry ) * 10 );
	memset( entries, 0, sizeof( GRID_ColorIPEntry) * 10 );

	/*
	 *	Socket stuff
	 */
	struct addrinfo *res = getlocaladdrinfo( portString );

	/*
	 *	Create the socket
	 */
	int mySocket = socket( res->ai_family, res->ai_socktype, res->ai_protocol );
	if( mySocket == -1 ) {
		fprintf(stderr, "Error: could not set up socket (%s)\n", strerror(errno) );
		return 1;
	}
	
	int optval = 1;
	setsockopt(mySocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
	if( bind( mySocket, res->ai_addr, res->ai_addrlen ) == -1 ) {
		fprintf(stderr, "Error: could not bind to socket %s as intended (%s)\n", PORT_NUMBER_STRING, strerror(errno) );
		return 1;
	}

	GRID_Tile* tiles = malloc( sizeof(GRID_Tile) * NUM_TILES );
	memset( tiles, 0, sizeof( GRID_Tile ) * NUM_TILES );

	GRID_generateTiles( tiles, NUM_TILES );
	GRID_coupleNeighbors( tiles, NUM_TILES );

	int timeToQuit = 0;

	SDL_Init( SDL_INIT_TIMER );

	while( ! timeToQuit ) {

		Uint32 start = SDL_GetTicks();

		/*
		 *	Drawing code (and some logic)
		 */
		for( int i = 0; i < NUM_TILES; ++i ) {
			//calculate energy distribution to neighbors
			if( !GRID_isNeutral( &( tiles[i].energy ) ) ) {
				//tiles[i].pos.z = TILE_ZPOS + GRID_energyElevation( &( tiles[i].energy ) );
				for( int j = 0; j < GRID_TILE_VERTICES; j++ ) {
					GRID_Tile* nb = tiles[i].neighbors[ j ];
					if( !nb )
						continue;
					GRID_energyInfluence( &( tiles[i].energy ), &( nb->pending) );
				}
				GRID_ColorIPEntry* entry = GRID_findEntry( tiles[i].energy.color, entries );
				if( !entry ) {
					fprintf(stderr, "Warning: couldn't find matching entry for color %x\n", tiles[i].energy.color );
				}
				entry->owned++;
			}



			GRID_energyTransfer( &( tiles[i].pending ), &( tiles[i].energy ), 1000000000.0f );//moves all pending energy into normal energy
			/*if( tiles[i].energy.energy != 5.0f ) {
				printf(" energy %f\n", tiles[i].energy.energy );
			}*/

			tiles[i].highlighted = 0;
		}

		//now go over all color entries and transfer their energies
		GRID_ColorIPEntry* entry = entries;
		while( entry->energy.color != 0 ) {
			
			Uint32 index2 = entry->destinationTileIndex;
			if( index2 >= NUM_TILES ) {
				fprintf(stderr, "WARNING: invalid energy transfer tile index: %u\n", index2);
			}
			
			float amount = entry->energyTransferRate * entry->energy.energy / FPS;
			if( amount >= 0.001f || amount <= -0.001f ) {
				//printf("Transferring amount: %f\n", amount );
				GRID_energyTransfer( &( entry->energy ), &( tiles[ index2 ].energy ), amount );
			}
			entry->energy.energy += entry->owned * RECHARGE_COEFFICIENT / FPS; 
			if( entry->energy.energy > MAX_COLOR_ENERGY )
				entry->energy.energy = MAX_COLOR_ENERGY;
			entry->owned = 0;
			entry = entry + 1;
		}


		while( 1 ) {
			//printf("Attempting read..\n");
			char buf[ PACKET_SIZE ];
			struct sockaddr_in sockAddr;
			//struct addrinfo sockAddr;
			memset( &sockAddr, 0, sizeof(sockAddr) );
			socklen_t sockAddrLen = sizeof( struct sockaddr_storage );
			int read = recvfrom( mySocket, buf, PACKET_SIZE, MSG_DONTWAIT, (struct sockaddr*)&sockAddr, &sockAddrLen );
			if( -1 == read && (errno == EWOULDBLOCK || errno == EAGAIN ) ) {
				//printf("No packets available right now\n");
				break;
			}
			else if( read == -1 || read == 0 ) {
				fprintf( stderr, "Error: could not receive packet (reason: %s)\n", strerror(errno) );
				break;
			}
			else {
				//printf("Received %i bytes from %s : %u\n", read, inet_ntoa( sockAddr.sin_addr ), sockAddr.sin_port);
				char request = buf[0];
				switch( request ) {
					case GSR_COLOR:
						GRID_handleColorRequest( mySocket, &sockAddr, entries );
						printf("Generating new color entry for %s:%u\n", inet_ntoa( sockAddr.sin_addr ), sockAddr.sin_port );
						//generate a color/ip pair
						break;

					case GSR_TILEENERGIES:
						//printf("Tile update requested\n");
						GRID_handleTilesRequest( mySocket, &sockAddr, tiles );
						break;

					case GSR_ENERGY:
						//printf("Energy transfer in progress\n");
						GRID_handleEnergyTransferRequest( mySocket, &sockAddr, buf, entries );
						break;
						

					default:
						printf("Unknown packet from %s:%u\n", inet_ntoa( sockAddr.sin_addr), sockAddr.sin_port );
						break;

				}
			}
		}

		Uint32 end = SDL_GetTicks();
		Uint32 delta = end - start;
		if( 1000/FPS > delta )
			SDL_Delay( 1000/FPS - delta );
	}

	close( mySocket );

	return 0;
}

