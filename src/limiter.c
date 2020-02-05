#define _GNU_SOURCE

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
