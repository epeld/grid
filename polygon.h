#pragma once

#include "vector.h"
#include "energy.h"

#define GRID_TILE_VERTICES ( 6 )

//tiles are polygons whose normals point along the z-axis
typedef struct GRID_Tile_struct {
	
	struct GRID_Tile_struct* neighbors[ GRID_TILE_VERTICES ];

	GRID_Vector vertices[ GRID_TILE_VERTICES ];

	GRID_Vector pos;
	gridfloat radius;

	GRID_Energy	energy;
	GRID_Energy	pending;

	int	highlighted;

} GRID_Tile;

//calculates vertex positions based on the other data in the tile
void	GRID_initTile( GRID_Tile* tile );

//puts down the vertices of the tile using glVertex3f
void	GRID_glTile( GRID_Tile* tile );

void	GRID_glTilePillar( GRID_Tile* tile, gridfloat zbase );

//computes the position of the tiles nth neighbor
void	GRID_tileNeighbor( GRID_Vector* v, GRID_Tile* tile, int n );

int	GRID_insideTile( GRID_Tile* tile, GRID_Vector* v );
