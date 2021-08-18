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

#include "real_dlsym.h"

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

extern void* __libc_dlsym( void* handle, const char* name );

void* real_dlsym( void* handle, const char* name )
{
	static void *(*the_real_dlsym)( void*, const char* );
	if ( the_real_dlsym == NULL ) {
		void* libdl = dlopen( "libdl.so.2", RTLD_NOW | RTLD_LOCAL );

		if ( libdl == NULL ) {
			fprintf( stderr, "Strangle: fatal error: cannot load libdl: %s\n", dlerror() );
			abort();
		}

		the_real_dlsym = __libc_dlsym( libdl, "dlsym" );

		if ( the_real_dlsym == NULL ) {
			fprintf( stderr, "Strangle: fatal error: cannot find real dlsym: %s\n", dlerror() );
			abort();
		}
	}

	return the_real_dlsym( handle, name );
}
