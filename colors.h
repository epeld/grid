#pragma once

#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "energy.h"
#include "polygon.h"

typedef struct {
	
	struct sockaddr_in host;
	GRID_Energy energy;

	//for energy transfer
	float energyTransferRate;
	Uint32	destinationTileIndex;

	Uint32 owned;

} GRID_ColorIPEntry;

//adds the new entry at the first non-zero color entry in the list
int	GRID_addColorIP( GRID_ColorIPEntry* enties, GRID_ColorIPEntry* newEntry );
int	GRID_addColorIPX( GRID_ColorIPEntry* entries, struct sockaddr_in* host);

GRID_Color	GRID_findColorEntry( GRID_ColorIPEntry* entries, struct sockaddr_in* host );

int	GRID_countColorIPEntries( GRID_ColorIPEntry* entries );

GRID_ColorIPEntry*	GRID_findEntry( GRID_Color c, GRID_ColorIPEntry* entries );
