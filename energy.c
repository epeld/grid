#include "energy.h"

#define MIN(x,y) ( x < y ? x : y )
#define MAX(x,y) ( x > y ? x : y )

#define GRID_ENERGY_INFLUENCE 0.001f

GRID_Color	GRID_defaultEnergyColor() {
	GRID_Color c = 0x0b0b0eff;

	return c;
}

GRID_Color	GRID_energyColor( GRID_Energy* e ) {
	float ratio = e->energy * 2.0f/ MAXIMAL_ENERGY;
	ratio = MIN( ratio, 1.0f );
	ratio = MAX( ratio, 0.2f );

	/*
	GRID_Color c = e->color;
	c.r *= ratio;
	c.g *= ratio;
	c.b *= ratio;
	*/
	Uint32 r = (Uint32)(e->color & 0xff000000);
	Uint32 g = (Uint32)(e->color & 0x00ff0000);
	Uint32 b = (Uint32)(e->color & 0x0000ff00);


	r >>= (8+8+8);
	g >>= (8+8);
	b >>= 8;


	r = (Uint8)(r * ratio);
	g = (Uint8)(g * ratio);
	b = (Uint8)(b * ratio);


	Uint32 color = r << 24 | g << 16 | b << 8 | 0x000000ff;

	//if( e->color!= GRID_defaultEnergyColor() )
	//printf("rgb %x %x %x %x\n", r,g,b, color);
	
	return color;
}

float	GRID_energyElevation( GRID_Energy* e ) {

	GRID_Color defaultColor = GRID_defaultEnergyColor();
	if( defaultColor == e->color )
		return 0;

	float ratio = e->energy / MAXIMAL_ENERGY;
	ratio = MIN( ratio, 1.0f );

	return ratio * MAXIMAL_ELEVATION;
}

void	GRID_energyTransfer( GRID_Energy* from, GRID_Energy* to, float amount ) {
	if( amount > from->energy )
		amount = from->energy;

	from->energy -= amount;
	
	GRID_Color c1 = from->color, c2 = to->color;
	if( c1 == c2 ) {
		to->energy += amount;
		if( to->energy > MAXIMAL_ENERGY ) {
			to->energy = MAXIMAL_ENERGY;
		}
	}
	else {
		to->energy -= amount;
	}


	//printf("energy remaining: %f\n", to->energy );

	if( to->energy < 0 ) {
		to->color = from->color;
		to->energy = - to->energy;
		//printf("color change\n");
	}
		
}

int		GRID_isNeutral( GRID_Energy* e ) {
	GRID_Color defaultColor = GRID_defaultEnergyColor();
	return ( defaultColor == e->color );
}

void	GRID_energyInfluence( GRID_Energy* from, GRID_Energy* to ) {
	float amount = GRID_ENERGY_INFLUENCE * from->energy;

	GRID_Color c1 = from->color, c2 = to->color;
	if( c1 == c2 ) {
		to->energy += amount;
		if( to->energy > MAXIMAL_ENERGY ) {
			to->energy = MAXIMAL_ENERGY;
		}
	}
	else {
		to->energy -= amount;
	}



	if( to->energy < 0 ) {
		to->color = from->color;
		to->energy = -to->energy;
	}
		
}

GRID_Color	GRID_meanColor( GRID_Color c1, GRID_Color c2 ) {
	Uint32 r = (Uint32)(c1 & 0xff000000);
	Uint32 g = (Uint32)(c1 & 0x00ff0000);
	Uint32 b = (Uint32)(c1 & 0x0000ff00);
	Uint32 r2 = (Uint32)(c2 & 0xff000000);
	Uint32 g2 = (Uint32)(c2 & 0x00ff0000);
	Uint32 b2 = (Uint32)(c2 & 0x0000ff00);


	r >>= (8+8+8);
	g >>= (8+8);
	b >>= 8;
	r2 >>= (8+8+8);
	g2 >>= (8+8);
	b2 >>= 8;


	r += r2;
	g += g2;
	b += b2;
	r /= 2;
	g /= 2;
	b /= 2;

	Uint32 color = r << 24 | g << 16 | b << 8 | 0x000000ff;
	return color;
}
