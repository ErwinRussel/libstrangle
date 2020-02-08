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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "limiter.h"

#include <time.h>

struct timespec nanotimeToTimespec( nanotime_t time ) {
	struct timespec ts;
	ts.tv_nsec = time % ONE_BILLION;
	ts.tv_sec = time / ONE_BILLION;

	return ts;
}

nanotime_t timespecToNanotime( const struct timespec* ts ) {
	return (nanotime_t)ts->tv_sec * (nanotime_t)ONE_BILLION + ts->tv_nsec;
}

nanotime_t getNanoTime() {
	struct timespec ts;
	clock_gettime( CLOCK_MONOTONIC_RAW, &ts );

	return timespecToNanotime( &ts );
}

nanotime_t getElapsedTime( nanotime_t oldTime ) {
	return getNanoTime() - oldTime;
}

nanotime_t getSleepTime( nanotime_t oldTime, nanotime_t target ) {
	return target - getElapsedTime( oldTime );
}

int strangle_nanosleep( nanotime_t sleepTime ) {
	if ( sleepTime <= 0 ) {
		return 0;
	}

	struct timespec ts = nanotimeToTimespec( sleepTime );
	return nanosleep( &ts, NULL );
}

void limiter( long targetFrameTime ) {
	static nanotime_t oldTime = 0,
	overhead = 0;

	if ( targetFrameTime <= 0 ) {
		return;
	}

	nanotime_t start = getNanoTime();
	nanotime_t sleepTime = getSleepTime( oldTime, targetFrameTime );
	if ( sleepTime > overhead ) {
		nanotime_t adjustedSleepTime = sleepTime - overhead;
		strangle_nanosleep( adjustedSleepTime );
		overhead = (getElapsedTime( start ) - adjustedSleepTime + overhead * 99) / 100;
	}

	oldTime = getNanoTime();
}
