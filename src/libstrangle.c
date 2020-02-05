/*
Copyright (C) 2016-2018 Björn Spindel

This file is part of libstrangle.

libstrangle is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

libstrangle is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libstrangle.  If not, see <http://www.gnu.org/licenses/>.
*/

#define _GNU_SOURCE

#include "libstrangle.h"
#include "glx.h"
#include "egl.h"
#include "vulkan.h"
#include "real_dlsym.h"

#include <dlfcn.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <stdio.h>

static struct config {
	long       targetFrameTime;
	int*       vsync;
	int*       glfinish;
	int*       retro;
	float*     anisotropy;
	float*     mipLodBias;
} config;

__attribute__ ((constructor))
void init() {
	char* env;

// 	extern const char* __progname;

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

struct timespec nanotimeToTimespec( nanotime_t time ) {
	struct timespec ts;
	ts.tv_nsec = time % ONE_BILLION;
	ts.tv_sec = time / ONE_BILLION;

	return ts;
}

nanotime_t timespecToNanotime( const struct timespec* ts ) {
	return (nanotime_t)ts->tv_sec * (nanotime_t)ONE_BILLION + ts->tv_nsec;
}

nanotime_t getNanoTime() {
	struct timespec ts;
	clock_gettime( CLOCK_MONOTONIC_RAW, &ts );

	return timespecToNanotime( &ts );
}

nanotime_t getElapsedTime( nanotime_t oldTime ) {
	return getNanoTime() - oldTime;
}

nanotime_t getSleepTime( nanotime_t oldTime, nanotime_t target ) {
	return target - getElapsedTime( oldTime );
}

int strangle_nanosleep( nanotime_t sleepTime ) {
	if ( sleepTime <= 0 ) {
		return 0;
	}

	struct timespec ts = nanotimeToTimespec( sleepTime );
	return nanosleep( &ts, NULL );
}

void limiter() {
	static nanotime_t oldTime = 0,
	                  overhead = 0;

	if ( config.glfinish != NULL && *config.glfinish == true ) {
		glFinish();
	}

	if ( config.targetFrameTime <= 0 ) {
		return;
	}

	nanotime_t start = getNanoTime();
	nanotime_t sleepTime = getSleepTime( oldTime, config.targetFrameTime );
	if ( sleepTime > overhead ) {
		nanotime_t adjustedSleepTime = sleepTime - overhead;
		strangle_nanosleep( adjustedSleepTime );
		overhead = (getElapsedTime( start ) - adjustedSleepTime + overhead * 99) / 100;
	}

	oldTime = getNanoTime();
}

void* getStrangleFunc( const char *symbol ) {
	// kill me

    if ( !strcmp( symbol, "dlsym" ) ) {
		return dlsym;

	} else if ( !strcmp( symbol, "eglSwapBuffers" ) ) {
		return (void*)eglSwapBuffers;

	} else if ( !strcmp( symbol, "glXSwapBuffers" ) ) {
		return (void*)glXSwapBuffers;
	} else if ( !strcmp( symbol, "glXGetProcAddress" ) ) {
		return (void*)glXGetProcAddress;
	} else if ( !strcmp( symbol, "glXGetProcAddressARB" ) ) {
		return (void*)glXGetProcAddressARB;
	} else if ( !strcmp( symbol, "glXSwapIntervalEXT" ) ) {
		return (void*)glXSwapIntervalEXT;
	} else if ( !strcmp( symbol, "glXSwapIntervalSGI" ) ) {
		return (void*)glXSwapIntervalSGI;
	} else if ( !strcmp( symbol, "glXSwapIntervalMESA" ) ) {
		return (void*)glXSwapIntervalMESA;
	} else if ( !strcmp( symbol, "glXMakeCurrent" ) ) {
		return (void*)glXMakeCurrent;

	} else if ( !strcmp( symbol, "vkQueuePresentKHR" ) ) {
		return (void*)vkQueuePresentKHR;
	} else if ( !strcmp( symbol, "vkGetInstanceProcAddr" ) ) {
		return (void*)vkGetInstanceProcAddr;
	} else if ( !strcmp( symbol, "vkGetDeviceProcAddr" ) ) {
		return (void*)vkGetDeviceProcAddr;
	} else if ( !strcmp( symbol, "vkCreateSwapchainKHR" ) ) {
		return (void*)vkCreateSwapchainKHR;
	} else if ( !strcmp( symbol, "vkCreateSampler" ) ) {
		return (void*)vkCreateSampler;
	}

	return NULL;
}

int getInterval( int interval ) {
	if ( config.vsync != NULL ) {
		return *config.vsync;
	}
	return interval;
}

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

char* strToLower( const char* str ) {
	char* result = strdup( str );
	for ( char* p = result; *p; ++p ) {
		*p = tolower( *p );
	}
	return result;
}

#ifdef HOOK_DLSYM
EXPORTED
void* dlsym( void* handle, const char* name )
{
	void* func = getStrangleFunc( name );
	if ( func != NULL ) {
		return func;
	}

	return real_dlsym( handle, name );
}
#endif

EXPORTED
void glFinish() {
	if ( config.glfinish != NULL && *config.glfinish == false ) {
		return;
	}
	void (*realFunction)() = real_dlsym( RTLD_NEXT, "glFinish" );
	realFunction();
}


void* strangle_requireFunction( const char* name ) {
	void (*func)() = real_dlsym( RTLD_NEXT, name );

	if (func == NULL) {
		printf( "Strangle: Failed to get function %s\n", name );
		exit( 1 );
	}

	return func;
}

void setVsync() {
	if ( config.vsync != NULL ) {
		glXSwapIntervalSGI( *config.vsync );
	}
}

int* getVsync() {
	return config.vsync;
}

float* getMipLodBias() {
	return config.mipLodBias;
}

float* getAnisotropy() {
	return config.anisotropy;
}

int* getRetro() {
	return config.retro;
}

struct config* getConfig() {
	return &config;
}
