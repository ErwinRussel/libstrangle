/*
 * Copyright (C) 2016-2020 Björn Spindel
 *
 * This file is part of libstrangle.
 *
 * libstrangle is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libstrangle is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libstrangle.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "util.h"

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
		result = (int*)malloc( sizeof(*result) );
		*result = (int)tmp;
	}

	return result;
}

float* strangle_strtof( const char* str ) {
	char* endptr = NULL;
	float* result = NULL;
	float tmp = strtof( str, &endptr );

	if ( str != endptr ) {
		result = (float*)malloc( sizeof(*result) );
		*result = (float)tmp;
	}

	return result;
}

StrangleConfig strangle_createConfig() {
	StrangleConfig config;
	char* env;

	config.targetFrameTime = 0;
	config.targetFrameTimeBattery = 0;

	config.vsync      = NULL;
	config.glfinish   = NULL;
	config.noError    = NULL;
	config.retro      = NULL;
	config.anisotropy = NULL;
	config.mipLodBias = NULL;
	config.trilinear  = NULL;
	config.cubic_filter = NULL;

	if (( env = getenv( "STRANGLE_FPS" ))) {
		double tmp = strtod( env, NULL );
		if ( tmp ) {
			config.targetFrameTime = 1000000000.0 / tmp;
			config.targetFrameTimeBattery = config.targetFrameTime;
		}
	}
	if (( env = getenv( "STRANGLE_FPS_BATTERY" ))) {
		double tmp = strtod( env, NULL );
		if ( tmp ) {
			config.targetFrameTimeBattery = 1000000000.0 / tmp;
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

	if (( env = getenv( "STRANGLE_TRILINEAR" ))) {
		config.trilinear = strangle_strtoi( env );
	}

	if (( env = getenv( "STRANGLE_BICUBIC" ))) {
		config.cubic_filter = strangle_strtoi( env );
	}

	if (( env = getenv( "STRANGLE_NO_ERROR" ))) {
		config.noError = strangle_strtoi( env );
	}

	return config;
}
