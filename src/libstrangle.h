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

#ifndef LIBSTRANGLE_H_
#define LIBSTRANGLE_H_

#define EXPORTED __attribute__((__visibility__("default")))

#include <stdbool.h>
#include <stdint.h>

#define ONE_BILLION 1000000000

typedef int64_t nanotime_t;

// Private
void* getStrangleFunc( const char* );
int getInterval( int interval );
void setVsync( void );
int* getVsync( void );
float* getMipLodBias();
float* getAnisotropy();
bool getRetro();
void limiter( void );
char* strToLower( const char* );
char *getenv_array( int count, const char** );
void *strangle_requireFunction( const char* );
int* strangle_strtoi( const char* );
float* strangle_strtof( const char* );
nanotime_t findSleepOverhead();

// Exported
void* dlsym( void*, const char* );
void glFinish( void );

#endif
