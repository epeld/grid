#include "polygon.h"
#include "vector.h"
#include <math.h>

#include "SDL/SDL_opengl.h"

void	GRID_initTile( GRID_Tile* tile ) {
	double TOTAL = 2.0 * M_PI;
	double PER_VERTEX = TOTAL / GRID_TILE_VERTICES;

	//set default color and energy levels
	tile->energy.color = GRID_defaultEnergyColor();
	tile->energy.energy = 5.0f;

	tile->pending.energy = 0;

	for( int i = 0; i < GRID_TILE_VERTICES; ++i ) {
		float x = tile->radius * cos( PER_VERTEX * i );
		float y = tile->radius * sin( PER_VERTEX * i );

		GRID_Vector v = {tile->pos.x + x,tile->pos.y + y, tile->pos.z };
		tile->vertices[i] = v;
	}
}

void	GRID_glTile( GRID_Tile* tile ) {
	for( int i = 0; i < GRID_TILE_VERTICES; ++i ) {
		GRID_Vector v = tile->vertices[ i ];

		//glNormal3f( 0,0, 1 );
		GRID_glVector( v );
	}

	GRID_glVector( tile->vertices[0] );
}

void	GRID_glTilePillar( GRID_Tile* tile, float zbase ) {
	
	float delta = tile->pos.z - zbase;

	for( int i = 0; i < GRID_TILE_VERTICES; ++i ) {
		GRID_Vector v = tile->vertices[ i ];
		GRID_Vector v2 = v;
		v2.z -= delta;

		//printf(" %f %f %f %f %f %f\n", v.x, v.y, v.z, v2.x, v2.y, v2.z );

		GRID_glVector( v );
		GRID_glVector( v2 );
	}

	GRID_Vector v = tile->vertices[0];
	v.z -= delta;

	GRID_glVector( tile->vertices[0] );
	GRID_glVector( v );
}


void	GRID_tileNeighbor( GRID_Vector* v, GRID_Tile* tile, int n ) {

	GRID_Vector	r1 = tile->vertices[ n ], r2 = tile->vertices[ (n == GRID_TILE_VERTICES - 1) ? 0 : n + 1 ];

	GRID_Vector	diff = GRID_vectorAdd( r2, GRID_vectorNegate( r1 ) );

	GRID_Vector	edge = GRID_vectorAdd( r1 , GRID_vectorScale( diff, 0.5 ) );

	GRID_Vector	diff2 = GRID_vectorAdd( edge , GRID_vectorNegate( tile->pos ) );
	
	GRID_Vector v2 = GRID_vectorAdd( tile->pos, GRID_vectorScale( diff2, 2.0 ) );
	*v = v2;
}

#define MIN(x,y) (x < y ? x : y)
#define MAX(x,y) (x > y ? x : y)

#define ABS( x ) ( x < 0 ? -x : x )

int InsidePolygon(GRID_Vector *vertices,int N,GRID_Vector p)
{
	int counter = 0;

	for( int i = 1; i <= N; ++i ) {
		GRID_Vector v = vertices[  i - 1 ], v2 = vertices[ i != N ? i : 0 ];

		//if( ABS( p.y   - (p.x * k + m) ) <= 0.5f ) {
		//	printf("MAYBE\n");
		//printf(" min %f, max %f, y1 %f y2 %f, x %f, y %f\n", MIN( v.x, v2.x ), MAX( v.x, v2.x ),  v.y, v2.y , p.x, p.y );
			if( p.x >= MIN( v.x, v2.x ) && p.x < MAX( v.x, v2.x )  ) {
				double k = ( v2.y - v.y ) / (v2.x - v.x );//makes use of the fact that vertices are never horizontally aligned (cannot divide by zero)
				double m = v2.y - k * v2.x;
				if( p.y < k * p.x + m ) {
					++counter;
				}
			}
		//}
	}

	return counter % 2;
}

int	GRID_insideTile( GRID_Tile* tile, GRID_Vector* v ) {
	if( ABS( tile->pos.z - v->z ) < 1.0f ) {
		return InsidePolygon( tile->vertices, GRID_TILE_VERTICES, *v );
	}
	else {
		//printf("Z-value not matching: %f %f\n", tile->pos.z, v->z );
	}
	return 0;
}
