#pragma once

#define MAX_OPTION_STRING 64

typedef struct {
	const char* fullName;
	const char* shortName;

	int takesValue;
	
	char value[ MAX_OPTION_STRING ] ;
} GRID_cmdLineOption;

void	GRID_parseOptions( GRID_cmdLineOption* options, int argc, char** argv );

GRID_cmdLineOption*	GRID_findOption( const char* name, GRID_cmdLineOption* options );
