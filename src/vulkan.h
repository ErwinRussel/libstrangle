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

#ifndef VULKAN_H_
#define VULKAN_H_

#include <stdint.h>

typedef struct VkSwapchainCreateInfoKHR {
	int                              sType;
	void*                            pNext;
	uint32_t                         flags;
	void*                            surface;
	uint32_t                         minImageCount;
	int                              imageFormat;
	int                              imageColorSpace;
	uint64_t                         imageExtent;
	uint32_t                         imageArrayLayers;
	int                              imageUsage;
	int                              imageSharingMode;
	uint32_t                         queueFamilyIndexCount;
	uint32_t*                        pQueueFamilyIndices;
	int                              preTransform;
	int                              compositeAlpha;
	int                              presentMode;
	uint32_t                         clipped;
	void*                            oldSwapchain;
} VkSwapchainCreateInfoKHR;

void* vkQueuePresentKHR( void* queue, void* pPresentInfo);
void* vkGetInstanceProcAddr( void* instance, void* pName);
void* vkGetDeviceProcAddr( void* instance, void* pName);
void* vkCreateSwapchainKHR( void* device, VkSwapchainCreateInfoKHR* pCreateInfo, void* pAllocator, void* pSwapchain );

#endif
