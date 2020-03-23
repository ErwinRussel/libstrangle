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

#define ONE_BILLION 1000000000 // yeah I know, haha

#include <stdint.h>
#include <stdlib.h>

#include <config.h>

typedef int64_t nanotime_t;

typedef struct {
	const size_t size;
	size_t       pos;
	nanotime_t   sum;
	nanotime_t*  items;
} TimeArray;

void limiter( const StrangleConfig* );

