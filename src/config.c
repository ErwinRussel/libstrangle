#include "config.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// char* getenv_array( int count, const char** names ) {
// 	char* env = NULL;
// 	for ( int i = 0; i < count; ++i ) {
// 		env = getenv( names[i] );
// 		if ( env != NULL && strcmp( env, "" ) ) {
// 			break;
// 		}
// 	}
// 	return env;
// }

int* strangle_strtoi( const char* str ) {
	char* endptr = NULL;
	int* result = NULL;
	long tmp = strtol( str, &endptr, 10 );

	if ( str != endptr ) {
		result = malloc( sizeof(*result) );
		*result = (int)tmp;
	}

	return result;
}

float* strangle_strtof( const char* str ) {
	char* endptr = NULL;
	float* result = NULL;
	float tmp = strtof( str, &endptr );

	if ( str != endptr ) {
		result = malloc( sizeof(*result) );
		*result = (float)tmp;
	}

	return result;
}

StrangleConfig strangle_createConfig() {
	StrangleConfig config;
	char* env;

	config.targetFrameTime = 0;

	if (( env = getenv( "STRANGLE_FPS" ))) {
		double tmp = strtod( env, NULL );
		if ( tmp ) {
			config.targetFrameTime = 1000000000.0 / tmp;
		}
	}

	if (( env = getenv( "STRANGLE_VSYNC" ))) {
		config.vsync = strangle_strtoi( env );
	}

	if (( env = getenv( "STRANGLE_GLFINISH" ))) {
		config.glfinish = strangle_strtoi( env );
	}

	if (( env = getenv( "STRANGLE_PICMIP" ))) {
		config.mipLodBias = strangle_strtof( env );
	}

	if (( env = getenv( "STRANGLE_AF" ))) {
		config.anisotropy = strangle_strtof( env );
	}

	if (( env = getenv( "STRANGLE_RETRO" ))) {
		config.retro = strangle_strtoi( env );
	}

	return config;
}
