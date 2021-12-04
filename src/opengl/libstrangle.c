/*
Copyright (C) 2016-2020 Björn Spindel

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
#include "real_dlsym.h"
#include "config.h"

#include <dlfcn.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <stdio.h>

static StrangleConfig config;

__attribute__ ((constructor))
void strangle_init() {
// 	extern const char* __progname;

	config = strangle_createConfig();
}


void* getStrangleFunc( const char *symbol ) {
	// kill me

    if ( !strcmp( symbol, "dlsym" ) ) {
		return real_dlsym;

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
	} else if ( !strcmp( symbol, "glXCreateContextAttribsARB" ) ) {
		return (void*)glXCreateContextAttribsARB;
	}

	return NULL;
}

int getInterval( int interval ) {
	if ( config.vsync != NULL ) {
		return *config.vsync;
	}
	return interval;
}

char* strToLower( const char* str ) {
	char* result = strdup( str );
	for ( char* p = result; *p; ++p ) {
		*p = tolower( *p );
	}
	return result;
}

#ifdef HOOK_DLSYM
EXPORT
void* dlsym( void* handle, const char* name )
{
	void* func = getStrangleFunc( name );
	if ( func != NULL ) {
		return func;
	}

	return real_dlsym( handle, name );
}
#endif

EXPORT
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

int* getGlFinish() {
	return config.glfinish;
}

StrangleConfig* getConfig() {
	return &config;
}
