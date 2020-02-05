#pragma once

typedef struct {
	long       targetFrameTime;
	int*       vsync;
	int*       glfinish;
	int*       retro;
	float*     anisotropy;
	float*     mipLodBias;
} StrangleConfig;

// char *getenv_array( int count, const char** );
StrangleConfig strangle_createConfig();
int* strangle_strtoi( const char* );
float* strangle_strtof( const char* );
