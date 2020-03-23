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

#include "egl.h"
#include "real_dlsym.h"
#include "libstrangle.h"
#include "limiter.h"
#include "config.h"

#include <stddef.h>
#include <dlfcn.h>
#include <stdbool.h>

#include <GL/gl.h>

EXPORT
unsigned int eglSwapBuffers( void* dpy, void* drawable ) {
	unsigned int ret;
	static unsigned int (*realFunction)( void*, void* );
	if (realFunction == NULL) {
		realFunction = strangle_requireFunction( __func__ );
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
	ret = realFunction( dpy, drawable );
	limiter( config );
	return ret;
}
