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

#include <dlfcn.h>
#include <stdlib.h>

void *strangle_requireGlxFunction( const char * name ) {
	static void *(*real_glXGetProcAddress)( const GLubyte * );
	static void *(*real_glXGetProcAddressARB)( const GLubyte * );
	if (real_glXGetProcAddress == NULL) {
		real_glXGetProcAddress = strangle_requireFunction( "glXGetProcAddress" );
	}
	if (real_glXGetProcAddressARB == NULL) {
		real_glXGetProcAddress = strangle_requireFunction( "glXGetProcAddressARB" );
	}

	void (*func)()
	= real_glXGetProcAddress( (const GLubyte *) name );

	if (func == NULL) {
		func = real_glXGetProcAddressARB( (const GLubyte *) name );
	}

	if (func == NULL) {
		func = strangle_requireFunction( name );
	}

	return func;
}

EXPORTED
void glXSwapBuffers( Display *dpy, GLXDrawable drawable ) {
	static void (*realFunction)( Display *, GLXDrawable );
	if (realFunction == NULL) {
		realFunction = strangle_requireGlxFunction( __func__ );
	}

	// The buffer swap is called before the wait in hope that it will reduce perceived input lag
	realFunction( dpy, drawable );
	limiter();
}

EXPORTED
void glXSwapIntervalEXT( Display *dpy, GLXDrawable drawable, int interval ) {
	static void (*realFunction)( Display *, GLXDrawable, int );
	if (realFunction == NULL) {
		realFunction = strangle_requireGlxFunction( __func__ );
	}

	realFunction( dpy, drawable, getInterval( interval ) );
}

EXPORTED
int glXSwapIntervalSGI( int interval ) {
	static int (*realFunction)( int );
	if (realFunction == NULL) {
		realFunction = strangle_requireGlxFunction( __func__ );
	}

	return realFunction( getInterval( interval ) );
}

EXPORTED
int glXSwapIntervalMESA( unsigned int interval ) {
	static int (*realFunction)( int );
	if (realFunction == NULL) {
		realFunction = strangle_requireGlxFunction( __func__ );
	}

	return realFunction( getInterval( interval ) );
}

EXPORTED
void *glXGetProcAddress( const GLubyte * procName ) {
	static void *(*realFunction)( const GLubyte * );
	if (realFunction == NULL) {
		realFunction = strangle_requireFunction( __func__ );
	}

	void *func = getStrangleFunc( (const char*)procName );
	if ( func != NULL ) {
		return func;
	}

	return realFunction( procName );
}

EXPORTED
void *glXGetProcAddressARB( const GLubyte * procName ) {
	static void *(*realFunction)( const GLubyte * );
	if (realFunction == NULL) {
		realFunction = strangle_requireFunction( __func__ );
	}

	void *func = getStrangleFunc( (const char*)procName );
	if ( func != NULL ) {
		return func;
	}

	return realFunction( procName );
}

EXPORTED
Bool glXMakeCurrent( Display * dpy, GLXDrawable drawable, GLXContext ctx ) {
	static Bool (*realFunction)( Display *, GLXDrawable, GLXContext );
	if (realFunction == NULL) {
		realFunction = strangle_requireGlxFunction( __func__ );
	}

	Bool ret = realFunction( dpy, drawable, ctx );
	setVsync();
	return ret;
}
