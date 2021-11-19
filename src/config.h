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

#pragma once

typedef struct {
	long       targetFrameTime;
	long       targetFrameTimeBattery;
	int*       vsync;
	int*       glfinish;
	int*       noError;
	int*       retro;
	float*     anisotropy;
	float*     mipLodBias;
	int*       trilinear;
	int*       cubic_filter;
} StrangleConfig;

// char *getenv_array( int count, const char** );
StrangleConfig strangle_createConfig();
int* strangle_strtoi( const char* );
float* strangle_strtof( const char* );
