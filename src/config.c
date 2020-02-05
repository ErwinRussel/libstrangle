#include "config.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

char* getenv_array( int count, const char** names ) {
	char* env = NULL;
	for ( int i = 0; i < count; ++i ) {
		env = getenv( names[i] );
		if ( env != NULL && strcmp( env, "" ) ) {
			break;
		}
	}
	return env;
}

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

	if (( env = getenv_array( 2, (const char*[]){ "FPS", "fps" } ) )) {
		double tmp = strtod( env, NULL );
		if ( tmp ) {
			config.targetFrameTime = 1000000000.0 / tmp;
		}
	}

	if (( env = getenv_array( 2, (const char*[]){ "VSYNC", "vsync" } ) )) {
		config.vsync = strangle_strtoi( env );
	}

	if (( env = getenv_array( 2, (const char*[]){ "GLFINISH", "glfinish" } ) )) {
		config.glfinish = strangle_strtoi( env );
	}

	if (( env = getenv_array( 4, (const char*[]){ "MIPLODBIAS", "miplodbias", "PICMIP", "picmip" } ) )) {
		config.mipLodBias = strangle_strtof( env );
	}

	if (( env = getenv_array( 2, (const char*[]){ "AF", "af" } ) )) {
		config.anisotropy = strangle_strtof( env );
	}

	if (( env = getenv_array( 2, (const char*[]){ "RETRO", "retro" } ) )) {
		config.retro = strangle_strtoi( env );
	}

	return config;
}
