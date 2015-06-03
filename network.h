#pragma once

#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "polygon.h"
#include "energy.h"
#include "colors.h"

#define PORT_NUMBER_STRING "6112"
#define PACKET_SIZE 64

/*
 *	Grid server requests - requests that the client can make to the server
 */
enum {
	GSR_BOGUS = 0,
	GSR_COLOR,
	GSR_TILEENERGIES, //request an update of all energies
	GSR_ENERGY, //request that energy be transferred to a tile
	GSR_COLORENERGY, //not really a request. server sends this back after a GSR_TILEENERGIES request to update the color's total energy
	//GSR_HIGHLIGHT, //client tells server which tile it has highlighted
} ;

struct addrinfo	*getlocaladdrinfo( const char* portString );
struct addrinfo*	GRID_getaddrinfo( const char*ip, const char* ps );

//sends a request to the server at dest and waits for a reply
//error indicated with the color 0
GRID_Color	GRID_requestColorAndWait( int socket, struct addrinfo* dest );

void	GRID_handleColorRequest( int socket,struct sockaddr_in* addr, GRID_ColorIPEntry* entries );

int	GRID_countColorIPEntries( GRID_ColorIPEntry* entries );

void	GRID_handleTilesRequest( int socket, struct sockaddr_in* addr, GRID_Tile* tiles );

void	GRID_unpackTileUpdate( const char* packet, GRID_Tile* tiles );

void	GRID_requestEnergyTransfer( int socket, struct addrinfo* dest, Uint32 index, float rate, GRID_Color color );

void	GRID_handleEnergyTransferRequest( int socket, struct sockaddr_in* addr, const char* packet, GRID_ColorIPEntry* entries );

float	GRID_unpackColorEnergy( const char* packet );

void	GRID_sendColorEnergy( int s, struct sockaddr_in* addr, float energy );
