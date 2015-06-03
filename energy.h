#pragma once

#include "SDL.h"
#include "SDL/SDL_opengl.h"

#define MAXIMAL_ENERGY 200.0f
#define MAXIMAL_ELEVATION 80.0f

typedef Uint32 GRID_Color;

typedef struct {
	GRID_Color color;

	float energy;
} GRID_Energy;

static inline void GRID_glColorAlpha( GRID_Color c ) { glColor4ub( (c & 0xff000000) >> 24, (c & 0x00ff0000) >> 16, (c & 0x0000ff00) >> 8, c & 0x000000ff);}

void		GRID_energyTransfer( GRID_Energy* from, GRID_Energy* to, float amount );
void		GRID_energyInfluence( GRID_Energy* from, GRID_Energy* to );

GRID_Color	GRID_energyColor( GRID_Energy* e);
float		GRID_energyElevation( GRID_Energy* e );

int		GRID_isNeutral( GRID_Energy* e );

GRID_Color	GRID_defaultEnergyColor();

GRID_Color	GRID_meanColor( GRID_Color c1, GRID_Color c2 );

static inline void GRID_glColor( GRID_Color c ) { 
	glColor3ub( (c & 0xff000000) >> 24, (c & 0x00ff0000) >> 16, (c & 0x0000ff00) >> 8);
}
