#include "colors.h"
#include "gridglobals.h"


int	GRID_addColorIP( GRID_ColorIPEntry* entries, GRID_ColorIPEntry* newEntry ) {
	GRID_ColorIPEntry* ip = entries;
	int c = 0;
	while( ip->energy.color != 0 ) {
		ip = entries + ++c;
	}
	*ip = *newEntry;
	return c;
}
	
int	GRID_addColorIPX( GRID_ColorIPEntry* entries, struct sockaddr_in* host ) {

	Uint32 colors[] = { 0xff0000ff, 0x00ff00ff, 0x0000ffff, 0xeeeeeeff, 0xaabbccff, 0x00ffffff, 0xffff00ff, 0xff00ffff };
	int c = GRID_countColorIPEntries( entries );
	Uint32 color = colors[ c ];

	GRID_Energy e;
	e.energy = STARTING_ENERGY;
	e.color = color;

	GRID_ColorIPEntry ip = {*host, e, 0.0f, 0};
	return GRID_addColorIP( entries, &ip );
}

int	GRID_countColorIPEntries( GRID_ColorIPEntry* entries ) {
	GRID_ColorIPEntry* ip = entries;
	int c = 0;
	while( ip->energy.color != 0 )
		ip = entries + ++c;
	return c;
}

GRID_Color	GRID_findColorEntry( GRID_ColorIPEntry* entries, struct sockaddr_in* host ) {

	int count = GRID_countColorIPEntries( entries );
	
	GRID_ColorIPEntry* ip = entries;
	int c = 0;
	while( memcmp( &(ip->host), host, sizeof( struct sockaddr_in ) ) != 0  ) {
		ip = entries + ++c;
		if( c >= count )
			return 0;
	}
	return ip->energy.color;
}

GRID_ColorIPEntry*	GRID_findEntry( GRID_Color c, GRID_ColorIPEntry* entries ) {
	GRID_ColorIPEntry* ip = entries;
	while( ip->energy.color != 0 ) {
		if( ip->energy.color == c )
			return ip;
		ip = ip + 1;
	}
	return NULL;
}

