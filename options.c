#include "options.h"
#include <stdio.h>
#include <string.h>

void	GRID_parseOptions( GRID_cmdLineOption* options, int argc, char** argv ) {
	GRID_cmdLineOption* it = options;
	while( it->fullName || it->shortName ) {
		memset( it->value, 0, MAX_OPTION_STRING );
		it = it + 1;
	}
	for( int i = 0; i < argc; ++i ) {
		const char* option = argv[i];

		//search options
		GRID_cmdLineOption* it = GRID_findOption( option, options );
		if( it != NULL ) {	
			//we have a match!
			if( it->takesValue ) {
				i += 1;
				if( i >= argc ) {
					fprintf(stderr, "Error: option %s missing its value.\n", option );
					return;
				}
				strcpy( it->value, argv[i] );
			}
			else {
				it->value[0] = 1;
			}
		}
		else {
			fprintf(stderr, "Warning: unrecognized option: %s. Ignoring\n", option );
		}
	}
}

GRID_cmdLineOption*	GRID_findOption( const char* name, GRID_cmdLineOption* options ) {
	GRID_cmdLineOption* it = options;
	while( it->fullName || it->shortName ) {
		//printf("%s %s\n", it->fullName, name );
		if( strcmp( it->fullName, name ) == 0 || strcmp( it->shortName, name) == 0 ) {
			//printf("BINGO\n");
			return it;
		}
		it = it + 1;
	}
	return NULL;
}
