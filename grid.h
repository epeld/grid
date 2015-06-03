#pragma once

#include "polygon.h"

GRID_Tile*	GRID_findTile( GRID_Vector v, GRID_Tile* tiles );
GRID_Vector	GRID_screenToOpenGL( double x, double y );
void	GRID_generateTiles( GRID_Tile* tiles, int n );
void	GRID_coupleNeighbors( GRID_Tile* tiles, int count );
