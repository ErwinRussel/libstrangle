#pragma once

#define ONE_BILLION 1000000000 // yeah I know, haha

#include <stdint.h>

typedef int64_t nanotime_t;

void limiter( long targetFrameTime );
