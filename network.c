#include "network.h"
#include "string.h"
#include "stdio.h"
#include "unistd.h"
#include "gridglobals.h"
#include "assert.h"
#include "energy.h"

#define GN_VERSION_NR 1

/*
 *	Communications formats
 *
 *	Color request - server gives each client a unique color
 *	- Client sends one byte (color request). Server replies with the first byte = color request. followed by a uint32 (the color) followed by a byte indicating version nr
 *
 *	Tile energy request - client requests an update on how energies are distributed among the tiles. split up into multiple packets if necessary
 *	client format: one byte: tile request
 *	server format: first byte: tile request. second byte: starting tile index. <Tile data>
 *		Tile data format: Uint32 color, Uint32 energyLevel (in 1/100ths)
 *	server will also send back a color energy update. format:
 *		first byte: GSR_COLORENERGY, followed by Uint32 currentEnergy
 *
 *	GSR_ENERGY request: request that some of the current color's energy be transferred to a specific tile. format:
 *	Client format:
 *		first byte: GSR_ENERGY, second byte: number of 1/100ths of total energy, Uint32 color, Uint32 tileIndex
 *	Server replies:
 *		first byte: GSR_ENERGY, second byte: 123
 *	
 *
 */

struct addrinfo*	getlocaladdrinfo( const char* ps ) {
	struct addrinfo *res = NULL;
	struct addrinfo hints;
	memset( &hints, 0, sizeof( struct addrinfo ) );
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	int error = getaddrinfo( NULL, ps, &hints, &res );
	if( error ) {
		fprintf(stderr, "Error getting local address (%s)\n", strerror(error) );
	}
	return res;
}

struct addrinfo*	GRID_getaddrinfo( const char*ip, const char* ps ) {
	struct addrinfo *res = NULL;
	struct addrinfo hints;
	memset( &hints, 0, sizeof( struct addrinfo ) );
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	int error = getaddrinfo( ip, ps, &hints, &res );
	if( error ) {
		fprintf(stderr, "Error getting local address (%s)\n", strerror(error) );
	}
	return res;
}

GRID_Color GRID_requestColorAndWait( int s, struct addrinfo* d) {
	int reply = 0;
	char request = GSR_COLOR;

	while( !reply ) {
		int sent = sendto( s, &request, 1, 0, d->ai_addr, d->ai_addrlen );
		if( sent == -1 ) {
			fprintf(stderr, "Error: failed to communicate with server (%s)\n", strerror(errno) );
			return 0;
		}
		else if( sent == 1 ) {
			//wait for a reply
			char buf[ PACKET_SIZE ];
			int r = recv( s, buf, PACKET_SIZE, MSG_DONTWAIT );
			if( r == -1 && (errno == EWOULDBLOCK || errno == EAGAIN ) ) {
				//
			}
			else if( r > 0 ) {
				//reply received!
				if( request != buf[0] ) {
					printf("First byte of reply does not match\n");
					return 0;
				}
				Uint32* ptr = (Uint32*)(buf + 1 );
				Uint32 color = ntohl( *ptr );
				unsigned char v = buf[5];
				if( v != GN_VERSION_NR ) {
					unsigned char myv = GN_VERSION_NR;
					fprintf(stderr, "Warning: version numbers do not match; client has %u, server has %u\n", myv, v );
				}
				return color;
			}
			printf("No reply yet..\n");
			usleep( 1000 * 100 );
		}
		else {
			fprintf(stderr, "Error: no bytes sent\n");
			return 0;
		}
	}
	return 0;
}

void	GRID_handleColorRequest( int socket,struct sockaddr_in* addr, GRID_ColorIPEntry* entries ) {
	Uint32 color = GRID_findColorEntry( entries, addr );
	if( color == 0 ) {
		//add it
		GRID_addColorIPX( entries, addr );
		color = GRID_findColorEntry( entries, addr );
		if( color == 0 ) {
			fprintf(stderr, "Error: the programmer fucked up\n");
			return;
		}
	}

	printf("Will assign color %x to %s\n", color, inet_ntoa( addr->sin_addr ) );

	//construct a reply to the client
	char buf[ PACKET_SIZE ];
	buf[0] = GSR_COLOR;

	Uint32 *clr = (Uint32*)(buf + 1);
	*clr = htonl( color );

	buf[5] = GN_VERSION_NR;

	if( -1 == sendto( socket, buf, 10, 0, addr, sizeof( struct sockaddr_in ) ) ) {
		fprintf(stderr, "Error: failed to send packet to client\n");
	}
}

void	GRID_handleTilesRequest( int socket, struct sockaddr_in* addr, GRID_Tile* tiles ) {
	//each tile takes up 8 bytes
	unsigned int i = 0;
	while( 1 ) {
		char buf2[ PACKET_SIZE ];
		char* buf = buf2;
		buf[0] = GSR_TILEENERGIES;
		unsigned char myindex = (unsigned char)i;
		buf[1] = myindex;
		//buf = buf + 2;
		Uint32 byteCounter = 2;
		while( byteCounter + 8 < PACKET_SIZE && i < NUM_TILES){
			GRID_Tile* t = tiles + i;
			Uint32 color = t->energy.color;
			Uint32 energy = (Uint32)(t->energy.energy * 100.0f);

			Uint32* colorPtr = (Uint32*)(buf + 2);
			Uint32* energyPtr = colorPtr + 1;
			*colorPtr = htonl( color );
			*energyPtr = htonl( energy );
			//printf("%u\n", energy );

			byteCounter += 8;
			buf = buf + 8;
			++i;
			
		} 
		//LOL!1!1one

		int sent = sendto( socket, buf2, byteCounter, 0, addr, sizeof( struct sockaddr_in ) );
		if( sent == -1 ) {
			fprintf(stderr, "Error: couldn't send reply to client (%s)\n", strerror(errno) );
		}
		else {
			//printf("%i bytes sent to client in response to tile request. Equals %i tiles\n", sent, (sent-2)/8 );
			if( i == NUM_TILES ) {
				//printf("All tiles sent\n");
				return;
			}
			else {
				//printf("Last index: %i\n", i );
			}
		}
	}
}

void	GRID_unpackTileUpdate( const char* packet, GRID_Tile* tiles ) {
	assert( packet[0] == GSR_TILEENERGIES );
	unsigned int index = (unsigned char)packet[1];
	unsigned int i = index;

	const char* buf = packet;
	
	int byteCounter = 2;
	while( byteCounter + 8 < PACKET_SIZE && i < NUM_TILES ) {
		GRID_Tile*	tile = tiles + i;
		Uint32* color = (Uint32*)(buf + byteCounter);
		Uint32* energy = color + 1;
//		printf("%u\n", ntohl(*energy) );
		tile->energy.color = ntohl(*color);
		float e = ntohl(*energy)/100.0f;
		tile->energy.energy = e;
		float elevation = GRID_energyElevation( &(tile->energy) );
		tile->pos.z = TILE_ZPOS + elevation;
		//if( e != 5.0f )
		//	printf("%f\n", e );
		byteCounter += 4 + 4;
		++i;
	}
	//printf("Updating tiles %u through %u\n", index, i );
}

void	GRID_requestEnergyTransfer( int socket, struct addrinfo* dest, Uint32 index, float rate, GRID_Color color ) {
	char buf[PACKET_SIZE];
	buf[0] = GSR_ENERGY;
	unsigned char crate = (unsigned char)(rate * 100.0f);
	buf[1] = crate;
	Uint32* colorPtr = (Uint32*)(buf + 2);
	*colorPtr = color;
	Uint32* indexPtr = colorPtr + 1;
	*indexPtr = index;

	if(-1 == sendto( socket, buf, 2 + 4 +4, 0, dest->ai_addr, dest->ai_addrlen ) ) {
		fprintf(stderr, "Failed to request energy transfer (%s)\n", strerror(errno) );
	}
}

void	GRID_handleEnergyTransferRequest( int socket, struct sockaddr_in* addr, const char* packet, GRID_ColorIPEntry* entries ) {
	assert( packet[0] == GSR_ENERGY );
	Uint32 crate = packet[1];
	float rate = crate/100.0f;

	const char* buf = packet;

	Uint32* colorPtr = (Uint32*)(buf + 2);
	Uint32 color = *colorPtr;

	Uint32* tileIndexPtr = colorPtr + 1;
	Uint32 tileIndex = *tileIndexPtr;

	GRID_ColorIPEntry* entry = entries;
	while( entry->energy.color != 0 ) {
		if( entry->energy.color == color )
			break;
		entry = entry + 1;
	}
	if( entry == NULL ) {
		fprintf(stderr, "Error: invalid energy transfer. No such color: %x\n", color );
	}

	//printf("Transfer rate changed to %f\n", rate );
	entry->energyTransferRate = rate;
	if( rate == 0.0f ) {
		entry->destinationTileIndex = 0;
	}
	else {
		//printf("crate is %u and rate is %f\n", crate, rate );
		entry->destinationTileIndex = tileIndex;
		//printf("Tile index is %u\n", tileIndex );
	}

	char reply[] = {GSR_ENERGY, 123};
	if( -1 == sendto( socket, reply, 2, 0, addr, sizeof( struct sockaddr_in ) ) ) {
		fprintf(stderr, "Error: failed to reply to server (%s)\n", strerror(errno) );
	}
	GRID_sendColorEnergy( socket, addr, entry->energy.energy );
}

void	GRID_sendColorEnergy( int s, struct sockaddr_in* addr, float energy ) {
	char buf[10];
	buf[0] = GSR_COLORENERGY;
	Uint32* ptr = (Uint32*)(buf + 1);
	*ptr = (Uint32)(energy*100.0f);

	sendto( s, buf, 5, 0, addr, sizeof( struct sockaddr_in ));
}

float	GRID_unpackColorEnergy( const char* packet ) {
	assert( packet[0] == GSR_COLORENERGY );

	Uint32* ptr = (Uint32*)(packet + 1);
	float v = *ptr/100.0f;
	//printf(" new energy %f\n", v );
	return v;
}
