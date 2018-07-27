/*
 * Copyright (C) 2018 Björn Spindel
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

// THIS IS ALL GARBAGE

// #define _GNU_SOURCE
//
// #include "gl.h"
// #include "libstrangle.h"
// #include "real_dlsym.h"
//
// #include <stddef.h>
//
// float textureHijackf( int pname, float param ) {
// 	switch ( pname ) {
// 		case GL_TEXTURE_LOD_BIAS: {
// 			float* picmip = getMipLodBias();
// 			if ( picmip ) return *picmip;
// 			break;
// 		}
// 		case GL_TEXTURE_MIN_FILTER: {
// 			int* retro = getRetro();
// 			if ( retro && *retro ) return (float)GL_NEAREST_MIPMAP_NEAREST;
// 			break;
// 		}
// 		case GL_TEXTURE_MAG_FILTER: {
// 			int* retro = getRetro();
// 			if ( retro && *retro ) return (float)GL_NEAREST;
// 			break;
// 		}
// 		default:
// 			break;
// 	}
//
// 	return param;
// }
//
// int textureHijacki( int pname, int param ) {
// 	float result = textureHijackf( pname, (float) param );
// 	if ( result == (float) param ) {
// 		return param;
// 	}
//
// 	return result;
// }
//
// EXPORTED
// void glTexEnvf( int target, int pname, float param ) {
// 	static void (*realFunction)( int, int, float );
// 	if (realFunction == NULL) {
// 		realFunction = strangle_requireFunction( __func__ );
// 	}
//
// 	if ( pname == GL_TEXTURE_FILTER_CONTROL && param == GL_TEXTURE_LOD_BIAS ) {
// 		float* picmip = getMipLodBias();
// 		if ( picmip ) {
// 			param = *picmip;
// 		}
// 	}
//
// 	return realFunction( target, pname, param );
// }
//
// EXPORTED
// void glTexEnvi( int target, int pname, int param ) {
// 	static void (*realFunction)( int, int, int );
// 	if (realFunction == NULL) {
// 		realFunction = strangle_requireFunction( __func__ );
// 	}
//
// 	if ( pname == GL_TEXTURE_FILTER_CONTROL && param == GL_TEXTURE_LOD_BIAS ) {
// 		float* picmip = getMipLodBias();
// 		if ( picmip ) {
// 			param = (int)*picmip;
// 		}
// 	}
//
// 	return realFunction( target, pname, param );
// }
//
// EXPORTED
// void glTexEnvfv( int target, int pname, float* param ) {
// 	static void (*realFunction)( int, int, float* );
// 	if (realFunction == NULL) {
// 		realFunction = strangle_requireFunction( __func__ );
// 	}
//
// 	if ( pname == GL_TEXTURE_FILTER_CONTROL && *param == GL_TEXTURE_LOD_BIAS ) {
// 		float* picmip = getMipLodBias();
// 		if ( picmip ) {
// 			param = picmip;
// 		}
// 	}
//
// 	return realFunction( target, pname, param );
// }
//
// EXPORTED
// void glTexEnviv( int target, int pname, int* param ) {
// 	static void (*realFunction)( int, int, int* );
// 	if (realFunction == NULL) {
// 		realFunction = strangle_requireFunction( __func__ );
// 	}
//
// 	if ( pname == GL_TEXTURE_FILTER_CONTROL && *param == GL_TEXTURE_LOD_BIAS ) {
// 		float* picmip = getMipLodBias();
// 		if ( picmip ) {
// 			*param = (int)*picmip;
// 		}
// 	}
//
// 	return realFunction( target, pname, param );
// }
//
// EXPORTED
// void glTexParameterf( void* target, int pname, float param ) {
// 	static void (*realFunction)( void*, int, float );
// 	if (realFunction == NULL) {
// 		realFunction = strangle_requireFunction( __func__ );
// 	}
//
// 	param = textureHijackf( pname, param );
//
// 	return realFunction( target, pname, param );
// }
//
// EXPORTED
// void glTexParameteri( void* target, int pname, int param ) {
// 	static void (*realFunction)( void*, int, int );
// 	if (realFunction == NULL) {
// 		realFunction = strangle_requireFunction( __func__ );
// 	}
//
// 	param = textureHijacki( pname, param );
//
// 	return realFunction( target, pname, param );
// }
//
// EXPORTED
// void glTexParameterfv( void* target, int pname, float* param ) {
// 	static void (*realFunction)( void*, int, float* );
// 	if (realFunction == NULL) {
// 		realFunction = strangle_requireFunction( __func__ );
// 	}
//
// 	*param = textureHijackf( pname, *param );
//
// 	return realFunction( target, pname, param );
// }
//
// EXPORTED
// void glTexParameteriv( void* target, int pname, int* param ) {
// 	static void (*realFunction)( void*, int, int* );
// 	if (realFunction == NULL) {
// 		realFunction = strangle_requireFunction( __func__ );
// 	}
//
// 	*param = textureHijacki( pname, *param );
//
// 	return realFunction( target, pname, param );
// }
//
// EXPORTED
// void glTexParameterIiv( void* target, int pname, int* param ) {
// 	static void (*realFunction)( void*, int, int* );
// 	if (realFunction == NULL) {
// 		realFunction = strangle_requireFunction( __func__ );
// 	}
//
// 	*param = textureHijacki( pname, *param );
//
// 	return realFunction( target, pname, param );
// }
//
// EXPORTED
// void glTexParameterIuiv( void* target, int pname, unsigned int* param ) {
// 	static void (*realFunction)( void*, int, unsigned int* );
// 	if (realFunction == NULL) {
// 		realFunction = strangle_requireFunction( __func__ );
// 	}
//
// 	*param = textureHijacki( pname, *param );
//
// 	return realFunction( target, pname, param );
// }
//
// EXPORTED
// void glSamplerParameterf( void* sampler, int pname, float param ) {
// 	static void (*realFunction)( void*, int, float );
// 	if (realFunction == NULL) {
// 		realFunction = strangle_requireFunction( __func__ );
// 	}
//
// 	param = textureHijackf( pname, param );
//
// 	return realFunction( sampler, pname, param );
// }
//
// EXPORTED
// void glSamplerParameteri( void* sampler, int pname, int param ) {
// 	static void (*realFunction)( void*, int, int );
// 	if (realFunction == NULL) {
// 		realFunction = strangle_requireFunction( __func__ );
// 	}
//
// 	param = textureHijacki( pname, param );
//
// 	return realFunction( sampler, pname, param );
// }
//
// EXPORTED
// void glSamplerParameterfv( void* sampler, int pname, float* param ) {
// 	static void (*realFunction)( void*, int, float* );
// 	if (realFunction == NULL) {
// 		realFunction = strangle_requireFunction( __func__ );
// 	}
//
// 	*param = textureHijackf( pname, *param );
//
// 	return realFunction( sampler, pname, param );
// }
//
// EXPORTED
// void glSamplerParameteriv( void* sampler, int pname, int* param ) {
// 	static void (*realFunction)( void*, int, int* );
// 	if (realFunction == NULL) {
// 		realFunction = strangle_requireFunction( __func__ );
// 	}
//
// 	*param = textureHijacki( pname, *param );
//
// 	return realFunction( sampler, pname, param );
// }
//
// EXPORTED
// void glSamplerParameterIiv( void* sampler, int pname, int* param ) {
// 	static void (*realFunction)( void*, int, int* );
// 	if (realFunction == NULL) {
// 		realFunction = strangle_requireFunction( __func__ );
// 	}
//
// 	*param = textureHijacki( pname, *param );
//
// 	return realFunction( sampler, pname, param );
// }
//
// EXPORTED
// void glSamplerParameterIuiv( void* sampler, int pname, unsigned int* param ) {
// 	static void (*realFunction)( void*, int, unsigned int* );
// 	if (realFunction == NULL) {
// 		realFunction = strangle_requireFunction( __func__ );
// 	}
//
// 	*param = textureHijacki( pname, *param );
//
// 	return realFunction( sampler, pname, param );
// }
//
// EXPORTED
// void glTextureParameterf( void* texture, int pname, float param ) {
// 	static void (*realFunction)( void*, int, float );
// 	if (realFunction == NULL) {
// 		realFunction = strangle_requireFunction( __func__ );
// 	}
//
// 	param = textureHijackf( pname, param );
//
// 	return realFunction( texture, pname, param );
// }
//
// EXPORTED
// void glTextureParameteri( void* texture, int pname, int param ) {
// 	static void (*realFunction)( void*, int, int );
// 	if (realFunction == NULL) {
// 		realFunction = strangle_requireFunction( __func__ );
// 	}
//
// 	param = textureHijacki( pname, param );
//
// 	return realFunction( texture, pname, param );
// }
//
// EXPORTED
// void glTextureParameterfv( void* texture, int pname, float* param ) {
// 	static void (*realFunction)( void*, int, float* );
// 	if (realFunction == NULL) {
// 		realFunction = strangle_requireFunction( __func__ );
// 	}
//
// 	*param = textureHijackf( pname, *param );
//
// 	return realFunction( texture, pname, param );
// }
//
// EXPORTED
// void glTextureParameteriv( void* texture, int pname, int* param ) {
// 	static void (*realFunction)( void*, int, int* );
// 	if (realFunction == NULL) {
// 		realFunction = strangle_requireFunction( __func__ );
// 	}
//
// 	*param = textureHijacki( pname, *param );
//
// 	return realFunction( texture, pname, param );
// }
//
// EXPORTED
// void glTextureParameterIiv( void* texture, int pname, int* param ) {
// 	static void (*realFunction)( void*, int, int* );
// 	if (realFunction == NULL) {
// 		realFunction = strangle_requireFunction( __func__ );
// 	}
//
// 	*param = textureHijacki( pname, *param );
//
// 	return realFunction( texture, pname, param );
// }
//
// EXPORTED
// void glTextureParameterIuiv( void* texture, int pname, unsigned int* param ) {
// 	static void (*realFunction)( void*, int, unsigned int* );
// 	if (realFunction == NULL) {
// 		realFunction = strangle_requireFunction( __func__ );
// 	}
//
// 	*param = textureHijacki( pname, *param );
//
// 	return realFunction( texture, pname, param );
// }
