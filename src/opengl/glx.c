/*
Copyright (C) 2016-2017 Björn Spindel

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

#include "glx.h"
#include "libstrangle.h"
#include "real_dlsym.h"
#include "limiter.h"
#include "config.h"

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glxtokens.h>

void* strangle_requireGlxFunction( const char* name ) {
	static void *(*real_glXGetProcAddress)( const unsigned char* );
	static void *(*real_glXGetProcAddressARB)( const unsigned char* );
	if (real_glXGetProcAddress == NULL) {
		real_glXGetProcAddress = strangle_requireFunction( "glXGetProcAddress" );
	}
	if (real_glXGetProcAddressARB == NULL) {
		real_glXGetProcAddress = strangle_requireFunction( "glXGetProcAddressARB" );
	}

	void (*func)()
	= real_glXGetProcAddress( (const unsigned char*) name );

	if (func == NULL) {
		func = real_glXGetProcAddressARB( (const unsigned char*) name );
	}

	if (func == NULL) {
		func = strangle_requireFunction( name );
	}

	return func;
}

EXPORT
void glXSwapBuffers( void* dpy, void* drawable ) {
	static void (*realFunction)( void*, void* );
	if (realFunction == NULL) {
		realFunction = strangle_requireGlxFunction( __func__ );
	}

	StrangleConfig *config = getConfig();

	// There is probably a better place for this but where???
	if ( config->mipLodBias ) {
		void (*glTexEnvf)( int, int, float ) = strangle_requireFunction("glTexEnvf");
		glTexEnvf( GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, *config->mipLodBias );
	}

	if ( config->glfinish != NULL && *config->glfinish == true ) {
		glFinish();
	}

	// The buffer swap is called before the wait in hope that it will reduce perceived input lag
	realFunction( dpy, drawable );
	limiter( config );
}

EXPORT
void glXSwapIntervalEXT( void* dpy, void* drawable, int interval ) {
	static void (*realFunction)( void*, void*, int );
	if (realFunction == NULL) {
		realFunction = strangle_requireGlxFunction( __func__ );
	}

	realFunction( dpy, drawable, getInterval( interval ) );
}

EXPORT
int glXSwapIntervalSGI( int interval ) {
	static int (*realFunction)( int );
	if (realFunction == NULL) {
		realFunction = strangle_requireGlxFunction( __func__ );
	}

	return realFunction( getInterval( interval ) );
}

EXPORT
int glXSwapIntervalMESA( unsigned int interval ) {
	static int (*realFunction)( int );
	if (realFunction == NULL) {
		realFunction = strangle_requireGlxFunction( __func__ );
	}

	return realFunction( getInterval( interval ) );
}

EXPORT
void* glXGetProcAddress( const unsigned char* procName ) {
	static void* (*realFunction)( const unsigned char* );
	if (realFunction == NULL) {
		realFunction = strangle_requireFunction( __func__ );
	}

	void* func = getStrangleFunc( (const char*)procName );
	if ( func != NULL ) {
		return func;
	}

	return realFunction( procName );
}

EXPORT
void* glXGetProcAddressARB( const unsigned char* procName ) {
	static void *(*realFunction)( const unsigned char* );
	if (realFunction == NULL) {
		realFunction = strangle_requireFunction( __func__ );
	}

	void* func = getStrangleFunc( (const char*)procName );
	if ( func != NULL ) {
		return func;
	}

	return realFunction( procName );
}

EXPORT
bool glXMakeCurrent( void* dpy, void* drawable, void* ctx ) {
	static bool (*realFunction)( void*, void*, void* );
	if (realFunction == NULL) {
		realFunction = strangle_requireGlxFunction( __func__ );
	}

	bool ret = realFunction( dpy, drawable, ctx );
	setVsync();
	return ret;
}

EXPORT
int glXCreateContextAttribsARB( void* dpy, void* config, void* share_context, bool direct, const int* attrib_list ) {
	static int (*realFunction)( void*, void*, void*, bool, const int* );
	if (realFunction == NULL) {
		realFunction = strangle_requireGlxFunction( __func__ );
	}

	StrangleConfig *sconfig = getConfig();

	if (sconfig->noError == NULL || *sconfig->noError == false) {
		return realFunction( dpy, config, share_context, direct, attrib_list );
	}

	// Length of attrib_list in ints, excluding terminator
	int attrib_list_length = 0;
	// Index of GLX_CONTEXT_FLAGS_ARB *value* in attrib_list; -1 if not found
	int context_flags_idx = -1;

	// According to spec, it may be null
	if (attrib_list) {
		for ( ; attrib_list[attrib_list_length] != 0; attrib_list_length += 2) {
			if (attrib_list[attrib_list_length] == GLX_CONTEXT_FLAGS_ARB) {
				context_flags_idx = attrib_list_length + 1;
			}
		}
	}

	// Create our own copy of attrib_list for messing with
	// Allocate two extra slots for writing flags if there's no slot for them

	// Once again, does not include terminator
	int attrib_list_copy_length = attrib_list_length + ((context_flags_idx == -1) ? 2 : 0);

	int* attrib_list_copy = malloc( sizeof(int)*(attrib_list_copy_length + 1) );

	if (!attrib_list_copy) {
		// malloc failure, just call function with original arguments
		return realFunction( dpy, config, share_context, direct, attrib_list );
	}

	memcpy(attrib_list_copy, attrib_list, sizeof(int)*attrib_list_length);
	attrib_list_copy[attrib_list_copy_length+1] = 0;

	if (context_flags_idx == -1) {
		context_flags_idx = attrib_list_length + 3;
		attrib_list_copy[attrib_list_length+2] = GLX_CONTEXT_FLAGS_ARB;
		attrib_list_copy[context_flags_idx] = 0;
	}

	attrib_list_copy[context_flags_idx] |= GL_CONTEXT_FLAG_NO_ERROR_BIT_KHR;

	int res = realFunction( dpy, config, share_context, direct, attrib_list_copy );
	free(attrib_list_copy);

	return res;
}
