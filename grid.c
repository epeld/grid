#include <stdio.h>
#include <math.h>

#include "SDL.h"
#include "SDL/SDL_opengl.h"

#include "vector.h"
#include "polygon.h"
#include "queue.h"
#include "energy.h"

#include "gridglobals.h"

GRID_Tile*	GRID_findTile( GRID_Vector v, GRID_Tile* tiles ) {
	for( int i = 0; i < NUM_TILES; ++i ) {

		//second, more precise one
		if( GRID_insideTile( tiles + i, &v ) ) {
			return tiles + i;
		}

	}
	return NULL;
}

GRID_Vector	GRID_screenToOpenGL( double x, double y ) {
	GLint viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );

	GLdouble modelview[16];
	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );

	GLdouble projection[16];
	glGetDoublev( GL_PROJECTION_MATRIX, projection );

	//flip y-coordinate
	y = viewport[3] - y;

	float z;
	glReadPixels(x,y,1,1, GL_DEPTH_COMPONENT, GL_FLOAT, &z );

	GRID_Vector v;

	GLdouble x2, y2, z2;
	if( GL_FALSE == gluUnProject( x, y ,z , modelview, projection, viewport, &x2, &y2, &z2 ) )
		fprintf(stderr, "gluUnproject failed :(\n");
		

	v.x = x2;
	v.y = y2;
	v.z = z2;

	return v;
}

//TODO make sure we free all the vectors left in the queue when we're done
void	GRID_generateTiles( GRID_Tile* tiles, int n ) {

	#define printf(...)

	GRID_Queue _q = {0};
	GRID_Queue* q = &_q;


	GRID_Vector *v = malloc( sizeof(GRID_Vector) );
	v->x = 0;
	v->y = 0;
	v->z = TILE_ZPOS;

	GRID_queuePut( q, v);

	int counter = 0;
	while( ! GRID_queueIsEmpty(q) && counter < n ) {
		v = (GRID_Vector*)GRID_queueGet(q);

		//did we already add this one?
		int alreadyAdded = 0;
		for( int j = 0; j < counter; ++j ) {
			GRID_Vector* v2 = &( tiles[j].pos );
			//if( v->x  == v2->x && v->y == v2->y && v->z == v2->z ) {
			GRID_Vector v3 = GRID_vectorAdd( *v, GRID_vectorNegate( *v2 ) );
			if( GRID_vectorMagnitude( v3 ) < 0.1f ) {
				alreadyAdded = 1;
				break;
			}
		}
		if( alreadyAdded ) {
			printf("Skipping one\n");

			free( v );
			continue;
		}

		//create the new tile
		tiles[ counter ].radius = TILE_RADIUS;
		tiles[ counter ].pos = *v;

		//printf(" Tile at %f %f %f\n", v->x, v->y, v->z );

		GRID_initTile( tiles + counter );

		//generate all neighbors
		for( int i = 0; i < GRID_TILE_VERTICES; ++i ) {

			GRID_Vector* v2 = malloc( sizeof(GRID_Vector) );
			GRID_tileNeighbor( v2, tiles + counter, i );

			//printf("Neighbor at %f %f %f\n", v2->x, v2->y, v2->z );

			GRID_queuePut( q, v2 );
		}

		++counter;

		free( v );
	}
	printf("Generated %i tiles\n", counter );

	while( ! GRID_queueIsEmpty( q ) ) {
		GRID_Vector* v = GRID_queueGet( q );
		free( v );
	}

	#undef printf
}

void	GRID_coupleNeighbors( GRID_Tile* tiles, int count ) {
	int nbs = 0;
	for( int i = 0; i < count; i++ ) {
		GRID_Tile* t = tiles + i;

		for( int j = 0; j < GRID_TILE_VERTICES; ++j ) {
			GRID_Vector v;
			GRID_tileNeighbor( &v, t, j );

			//see if we can find this neighbor
			for( int k = 0; k < count; k++ ) {
				if( k == i )
					continue;
				GRID_Vector dist = GRID_vectorSubtract( v, tiles[k].pos );
				if( GRID_vectorMagnitude( dist ) < 1.0f ) {
					t->neighbors[ j ] = tiles + k;
					nbs++;
					break;
				}
			}
		}
	}

	printf("A total of %i neighbors coupled\n", nbs );
}

