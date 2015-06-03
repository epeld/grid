#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "grid.h"
#include "vector.h"
#include "polygon.h"
#include "queue.h"
#include "energy.h"
#include "gridglobals.h"
#include "network.h"
#include "options.h"

void	GRID_displayEnergyBar( GRID_Energy* energy ) {

	#define HEIGHT 20
	#define INSET 10

	float totalLength = 800 - 2 * INSET;

	float ratio = energy->energy / MAX_COLOR_ENERGY;
	
	glBegin( GL_QUADS );
	GRID_glColor( energy->color );
	glVertex2f( INSET,INSET );
	glVertex2f( INSET, INSET + HEIGHT );
	glVertex2f( INSET + ratio * totalLength, INSET + HEIGHT );
	glVertex2f( INSET + ratio * totalLength, INSET );
	glEnd();
}

void	GRID_displayEnergyRate( GRID_Energy* energy, float rate ) {

	int offset = INSET * 2 + HEIGHT;

	float ratio = rate;
	float totalLength = 800 - 2 * INSET;
	
	glBegin( GL_QUADS );
	GRID_glColor( GRID_meanColor( 0, energy->color ) );
	glVertex2f( INSET,INSET + offset );
	glVertex2f( INSET, INSET + HEIGHT + offset );
	glVertex2f( INSET + ratio * totalLength, INSET + HEIGHT + offset );
	glVertex2f( INSET + ratio * totalLength, INSET + offset );
	glEnd();

}

Uint32	GRID_networkTimer( Uint32 interval, void* p ) {

	SDL_Event user_event;
	user_event.type = SDL_USEREVENT;

	SDL_PushEvent( &user_event );
	return interval;
}

int main( int argc,char** argv ) {
	printf("The Grid\n");
	printf("written by Erik Peldan\n");

	GRID_cmdLineOption options[] = { 
		{ "--help", "-h", 0 },
		{ "--port", "-p", 1 },
		{ "--ip", "-i", 1 },
		{NULL, NULL, 0 }
	};

	GRID_parseOptions( options, argc -1, argv + 1 );
	if( GRID_findOption("--help", options )->value[0] ) {
		printf("HELP:\n");
		printf("--port or -p specifies port (default %s)\n", PORT_NUMBER_STRING);
		printf("--ip or -i specifies Server IP (default 127.0.0.1)\n");
		return 1;
	}
	const char* portString;
	if( strlen( portString = GRID_findOption("--port", options )->value) ) {
		printf("port %s chosen\n", portString );
	}
	else {
		portString = PORT_NUMBER_STRING;
	}
	const char* ipString;
	if( strlen( ipString = GRID_findOption("--ip", options )->value) ) {
		printf("IP %s chosen\n", ipString );
	}
	else {
		ipString = "127.0.0.1";
	}


	/*
	 *	Setup socket
	 */
	int mySocket = socket( AF_INET, SOCK_DGRAM, 0 );
	if( mySocket == -1 ) {
		fprintf(stderr, "Error: could not set up socket (%s)\n", strerror(errno) );
		return 1;
	}
	int optval = 1;
	setsockopt(mySocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

	/*
	 *	Fetch destination address
	 */
	struct addrinfo *res;
	//res = getlocaladdrinfo( portString );
	printf("Requesting %s:%s\n", ipString, portString );
	res = GRID_getaddrinfo( ipString, portString );

	/*
	 *	Before we can play we need to request a color from the server
	 */
	GRID_Color myColor = GRID_requestColorAndWait( mySocket, res );
	if( myColor == 0 ) {
		fprintf(stderr, "Failed to get color from server. Terminating\n");
		return 1;
	}
	printf("Color received: %x\n", myColor );

	/*
	 *	Init SDL
	 */
	SDL_Init( SDL_INIT_EVERYTHING );
	SDL_SetVideoMode( 800, 600, 0, SDL_SWSURFACE | SDL_DOUBLEBUF | SDL_OPENGL );

	/*
	 *	Setup opengl
	 */
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LEQUAL );
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GRID_Tile* tiles = malloc( sizeof(GRID_Tile) * NUM_TILES );
	memset( tiles, 0, sizeof( GRID_Tile ) * NUM_TILES );

	GRID_generateTiles( tiles, NUM_TILES );
	GRID_coupleNeighbors( tiles, NUM_TILES );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 800.0f/600.0f, 150.0f, 450.0f );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode( GL_MODELVIEW );

	glLoadIdentity();

	/*
	 *	Setup game
	 */
	float theta = M_PI / 4.0, phi = 0;

	int timeToQuit = 0;
	GRID_Energy myEnergy; //the current player's energy
	myEnergy.color = myColor;
	myEnergy.energy = STARTING_ENERGY;
	float energyRate = DEFAULT_ENERGY_RATE;

	int owned = 0;

	SDL_AddTimer( 100, GRID_networkTimer, NULL );

	Uint32 delta = 100;

	while( ! timeToQuit ) {

		Uint32 startTime = SDL_GetTicks();

		/*
		 *	Query state of input
		 */
		Uint8* keystate = SDL_GetKeyState( NULL );
		if( keystate[ SDLK_LEFT ] ) {
			phi -= PHI_SPEED;
		}
		else if( keystate[ SDLK_RIGHT ] ) {
			phi += PHI_SPEED;
		}
		if( keystate[ SDLK_UP ] ) {
			theta -= THETA_SPEED;
			if( theta <= THETA_MIN ) {
				theta = THETA_MIN;
			}
		}
		else if( keystate[ SDLK_DOWN ] ) {
			theta += THETA_SPEED;
			if( theta >= THETA_MAX ) {
				theta = THETA_MAX;
			}
		}
		if( keystate[ SDLK_ESCAPE ] )
			timeToQuit = 1;


		/*
		 *	Drawing code (and some logic)
		 */
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		
		//position camera
		float x = (-TILE_ZPOS)*sin(theta)*cos(phi), y = (-TILE_ZPOS)*sin(theta)*sin(phi), z = (-TILE_ZPOS)*cos(theta) + TILE_ZPOS;
		glPushMatrix();
		gluLookAt( x, y, z, 0, 0, TILE_ZPOS, 0, 0, acos( z / (-TILE_ZPOS) ) );

		for( int i = 0; i < NUM_TILES; ++i ) {
			//calculate energy distribution to neighbors
			if( !GRID_isNeutral( &( tiles[i].energy ) ) ) {
				tiles[i].pos.z = TILE_ZPOS + GRID_energyElevation( &( tiles[i].energy ) );
				for( int j = 0; j < GRID_TILE_VERTICES; j++ ) {
					GRID_Tile* nb = tiles[i].neighbors[ j ];
					if( !nb )
						continue;
					GRID_energyInfluence( &( tiles[i].energy ), &( nb->pending) );
				}
			}
			//is this tile ours?
			if( memcmp( &( tiles[i].energy.color ), &( myEnergy.color ), sizeof( GRID_Color ) ) == 0 ) {
				owned++;
			}


			GRID_energyTransfer( &( tiles[i].pending ), &( tiles[i].energy ), 1000000000.0f );//moves all pending energy into normal energy

			float z = tiles[i].pos.z;

			glTranslatef( 0, 0, z - TILE_ZPOS );

			glBegin( GL_TRIANGLE_FAN );

				GRID_Color c = GRID_energyColor( &(tiles[i].energy));
				GRID_glColor( c );
				GRID_glTile( tiles + i );

			glEnd();

			if( tiles[i].pos.z != TILE_ZPOS ) {
				glBegin( GL_TRIANGLE_STRIP );

					GRID_Color c = GRID_energyColor( &(tiles[i].energy));
					GRID_glColor( c );
					GRID_glTilePillar( tiles + i, TILE_ZPOS );

				glEnd();
			}



			glTranslatef( 0, 0,  1);

			glBegin( GL_LINE_STRIP );

			
				glColor3f( 1, 1 ,1 );
				GRID_glTile( tiles + i );


			glEnd();

			glTranslatef(0,0, -1);
			glTranslatef( 0, 0, -z + TILE_ZPOS );

			tiles[i].highlighted = 0;

		}

		/*
		 *	Process events
		 */
		SDL_Event e = {0};
		char request = GSR_TILEENERGIES;
		while( SDL_PollEvent( &e ) ) {
			switch( e.type ) {
				case SDL_QUIT:
					timeToQuit = 1;
					break;

				case SDL_USEREVENT://network timer has fired
					//printf("Requesting tile update\n");
					sendto( mySocket, &request, 1, 0, res->ai_addr, res->ai_addrlen );
					break;

				case SDL_MOUSEBUTTONDOWN:
					if( e.button.button == SDL_BUTTON_WHEELUP ) {
						energyRate += 0.1f;
						if( energyRate > 1.0f )
							energyRate = 1.0f;
					}
					if( e.button.button == SDL_BUTTON_WHEELDOWN ) {
						energyRate -= 0.1f;
						if( energyRate < 0.0f )
							energyRate = 0.0f;
					}
					break;

				case SDL_KEYDOWN:
					switch( e.key.keysym.sym ) {
						case SDLK_1:
							energyRate = 0.1f;
							break;

						case SDLK_2:
							energyRate = 0.2f;
							break;

						case SDLK_3:
							energyRate = 0.3f;
							break;

						case SDLK_4:
							energyRate = 0.4f;
							break;

						case SDLK_5:
							energyRate = 0.5f;
							break;

						case SDLK_6:
							energyRate = 0.6f;
							break;

						case SDLK_7:
							energyRate = 0.7f;
							break;

						case SDLK_8:
							energyRate = 0.8f;
							break;

						case SDLK_9:
							energyRate = 0.9f;
							break;

						case SDLK_0:
							energyRate = 1.0f;
							break;
						default:
							break;
						}
						break;

				default:
				break;
			}
		}

		/*
		 *	Mouse state
		 */
		int mouseX, mouseY;
		Uint8 mouse = SDL_GetMouseState( &mouseX, &mouseY );
		if( SDL_BUTTON(1) & mouse ) { //mouse is pressed
			//give energy to the tile
			GRID_Vector v = GRID_screenToOpenGL( mouseX, mouseY );
			GRID_Tile* t = GRID_findTile( v, tiles );
			if( t ) {

				//tell the server to transfer energy
				GRID_requestEnergyTransfer( mySocket, res, ( t - tiles), energyRate, myEnergy.color );

				float amount = energyRate * myEnergy.energy * delta / 1000.0f;
				GRID_energyTransfer( &myEnergy, &(t->energy), amount );

				float elevation = GRID_energyElevation( &(t->energy) );
				t->pos.z = TILE_ZPOS + elevation;
				
			}
			else {
				//tell the server to stop transferring energy
				GRID_requestEnergyTransfer( mySocket, res, 0, 0, myEnergy.color ) ;
			}
		}
		else {
			//tell the server to stop transferring energy
			GRID_requestEnergyTransfer( mySocket, res, 0, 0, myEnergy.color ) ;

			GRID_Vector v = GRID_screenToOpenGL( mouseX, mouseY );
			GRID_Tile* t = GRID_findTile( v, tiles );
			if( t ) {
				t->highlighted = 1;
			}
		}

		myEnergy.energy += owned * RECHARGE_COEFFICIENT * 1.0f/ FPS;
		if( myEnergy.energy > MAX_COLOR_ENERGY )
			myEnergy.energy = MAX_COLOR_ENERGY;

		owned = 0;


		glLoadIdentity();
		glMatrixMode( GL_PROJECTION );
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0,800,600,0);
		glMatrixMode( GL_MODELVIEW );

		//TODO draw energy bars
		GRID_displayEnergyBar( &myEnergy );
		GRID_displayEnergyRate( &myEnergy, energyRate );

		glMatrixMode( GL_PROJECTION );
		glPopMatrix();
		glMatrixMode( GL_MODELVIEW );

		glPopMatrix();


		SDL_GL_SwapBuffers();

		/*
		 *	Communicate with server
		 */
		while( 1 ) {
			char buf[ PACKET_SIZE ];
			int read = recv( mySocket, buf, PACKET_SIZE, MSG_DONTWAIT);
			if( read == -1 && (errno != EWOULDBLOCK && errno != EAGAIN ) ) {
				fprintf(stderr, "Error: failed to recv from server (%s)\n", strerror(errno) );
				break;
			}
			else if( read != -1 && read != 0 ){
				switch( buf[0] ) {
					case GSR_TILEENERGIES:
						GRID_unpackTileUpdate( buf, tiles );
						break;

					case GSR_COLOR:
						//just ignore this
						break;

					case GSR_COLORENERGY:
						myEnergy.energy = GRID_unpackColorEnergy( buf );
						break;

					case GSR_ENERGY:
						if( buf[1] != 123 ) {
							fprintf(stderr, "Warning: strange packet reply from server\n");
						}
						break;
					default:
						fprintf(stderr, "Warning: unrecognized server packet: %u\n", buf[0] );
						break;
				}
			}
			else
				break;
		}

		Uint32 endTime = SDL_GetTicks();
		delta = endTime - startTime;
		//printf("delta %u\n",delta);
		if( 1000/FPS > delta )
			SDL_Delay( 1000/FPS - delta );

	}

	SDL_Quit();
	freeaddrinfo(res);

	return 0;
}

