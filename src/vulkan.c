/*
Copyright (C) 2018 Björn Spindel

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

#include "vulkan.h"
#include "libstrangle.h"
#include "real_dlsym.h"

#include <dlfcn.h>
#include <stdlib.h>

EXPORTED
void* vkQueuePresentKHR( void* queue, void* pPresentInfo) {
	static void* (*realFunction)( void*, void* );
	if (realFunction == NULL) {
		realFunction = strangle_requireFunction( __func__ );
	}

	void* ret = realFunction( queue, pPresentInfo );
	limiter();

	return ret;
}

EXPORTED
void* vkGetInstanceProcAddr( void* instance, void* pName) {
	static void* (*realFunction)( void*, void* );
	if (realFunction == NULL) {
		realFunction = strangle_requireFunction( __func__ );
	}

	void* func = getStrangleFunc( (const char*)pName );
	if ( func != NULL ) {
		return func;
	}

	return realFunction( instance, pName );
}

EXPORTED
void* vkGetDeviceProcAddr( void* instance, void* pName) {
	static void* (*realFunction)( void*, void* );
	if (realFunction == NULL) {
		realFunction = strangle_requireFunction( __func__ );
	}

	void* func = getStrangleFunc( (const char*)pName );
	if ( func != NULL ) {
		return func;
	}

	return realFunction( instance, pName );
}

EXPORTED
void* vkCreateSwapchainKHR( void* device, VkSwapchainCreateInfoKHR* pCreateInfo, void* pAllocator, void* pSwapchain ) {
	static void* (*realFunction)( void*, VkSwapchainCreateInfoKHR*, void*, void* );
	if (realFunction == NULL) {
		realFunction = strangle_requireFunction( __func__ );
	}

	int* vsync = getVsync();
	if ( vsync != NULL && *vsync >= 0 && *vsync <= 3) {
		pCreateInfo->presentMode = *vsync;
	}

	return realFunction( device, pCreateInfo, pAllocator, pSwapchain );
}

EXPORTED
void* vkCreateSampler(void* device, VkSamplerCreateInfo* pCreateInfo, void* pAllocator, void*                                  pSampler) {
	static void* (*realFunction)( void*, VkSamplerCreateInfo*, void*, void* );
	if (realFunction == NULL) {
		realFunction = strangle_requireFunction( __func__ );
	}

	float* picmip = getMipLodBias();
	float* af = getAnisotropy();
	bool retro = getRetro();

	if ( picmip != NULL ) {
		pCreateInfo->mipLodBias = *picmip;
	}

	if ( af != NULL && *af >= 1 && *af <= 16 ) {
		pCreateInfo->anisotropyEnable = 1;
		pCreateInfo->maxAnisotropy = *af;
	}

	if ( retro ) {
		pCreateInfo->magFilter = 0; // VK_FILTER_NEAREST
		pCreateInfo->minFilter = 0;
		pCreateInfo->mipmapMode = 0; // VK_SAMPLER_MIPMAP_MODE_NEAREST
	}

	return realFunction( device, pCreateInfo, pAllocator, pSampler );
}
