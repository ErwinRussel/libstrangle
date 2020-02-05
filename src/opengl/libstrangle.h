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

#pragma once

#define EXPORT __attribute__((__visibility__("default")))

#include "config.h"

// Private
void* getStrangleFunc( const char* );
int getInterval( int interval );
void setVsync( void );
int* getVsync( void );
float* getMipLodBias();
float* getAnisotropy();
int* getRetro();
int* getGlFinish();
StrangleConfig* getConfig();
char* strToLower( const char* );
void *strangle_requireFunction( const char* );

// Exported
void* dlsym( void*, const char* );
void glFinish( void );
