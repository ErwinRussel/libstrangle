/*
 * Copyright © 2019 Intel Corporation
 * Copyright 2020 Björn Spindel
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <mutex>
#include <map>

#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>
#include <vulkan/vk_util.h>
#include "vk_enum_to_str.h"

#include "mesa/util/macros.h"

#include "config.h"
#include "limiter.h"

typedef void* hkey_t;

static StrangleConfig config;
std::mutex mutex_lock;

/* Mapped from VkInstace/VkPhysicalDevice */
struct instance_data {
	struct vk_instance_dispatch_table vtable;
	VkInstance instance;
};

/* Mapped from VkDevice */
struct queue_data;
struct device_data {
	struct instance_data *instance;

	PFN_vkSetDeviceLoaderData set_device_loader_data;

	struct vk_device_dispatch_table vtable;
	VkPhysicalDevice physical_device;
	VkDevice device;

	VkPhysicalDeviceProperties properties;

	struct queue_data *graphic_queue;

	struct queue_data **queues;
	uint32_t n_queues;
};

/* Mapped from VkQueue */
struct queue_data {
	struct device_data *device;

	VkQueue queue;
	VkQueueFlags flags;
	uint32_t family_index;
	uint64_t timestamp_mask;

	VkFence queries_fence;
};

std::map<hkey_t, void*> vk_object_to_data;

#define HKEY(obj) ((hkey_t)(obj))
#define FIND(type, obj) ((type *)find_object_data(HKEY(obj)))

static void *find_object_data(hkey_t obj)
{
	std::scoped_lock lock(mutex_lock);
	return vk_object_to_data[obj];
}

static void map_object(hkey_t obj, void *data)
{
	std::scoped_lock lock(mutex_lock);
	vk_object_to_data[obj] = data;
}

static void unmap_object(hkey_t obj)
{
	std::scoped_lock lock(mutex_lock);
	vk_object_to_data.erase(obj);
}

/**/

#define VK_CHECK(expr) \
	do { \
		VkResult __result = (expr); \
		if (__result != VK_SUCCESS) { \
			fprintf(stderr, "'%s' line %i failed with %s\n", \
			#expr, __LINE__, vk_Result_to_str(__result)); \
		} \
	} while (0)

/**/

static VkLayerInstanceCreateInfo *get_instance_chain_info(
	const VkInstanceCreateInfo *pCreateInfo,
	VkLayerFunction            func)
{
	vk_foreach_struct(item, pCreateInfo->pNext) {
		if (item->sType == VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO &&
			 ((VkLayerInstanceCreateInfo *) item)->function == func) {
			return (VkLayerInstanceCreateInfo *) item;
		}
	}
	unreachable("instance chain info not found");
	return NULL;
}

static VkLayerDeviceCreateInfo *get_device_chain_info(
	const VkDeviceCreateInfo *pCreateInfo,
	VkLayerFunction          func)
{
	vk_foreach_struct(item, pCreateInfo->pNext) {
		if (item->sType == VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO &&
			 ((VkLayerDeviceCreateInfo *) item)->function == func) {
			return (VkLayerDeviceCreateInfo *)item;
		}
	}
	unreachable("device chain info not found");
	return NULL;
}

static struct instance_data *new_instance_data(VkInstance instance)
{
	auto data = new instance_data();
	data->instance = instance;
	map_object(HKEY(data->instance), data);
	return data;
}

static void destroy_instance_data(struct instance_data *data)
{
	unmap_object(HKEY(data->instance));
	delete data;
}

static void instance_data_map_physical_devices(
	struct instance_data *instance_data,
	bool                 map)
{
	uint32_t physicalDeviceCount = 0;
	instance_data->vtable.EnumeratePhysicalDevices(
		instance_data->instance,
		&physicalDeviceCount,
		NULL
	);

	auto physicalDevices = new VkPhysicalDevice[physicalDeviceCount];
	instance_data->vtable.EnumeratePhysicalDevices(
		instance_data->instance,
		&physicalDeviceCount,
		physicalDevices
	);

	for (uint32_t i = 0; i < physicalDeviceCount; i++) {
		if (map)
			map_object(HKEY(physicalDevices[i]), instance_data);
		else
			unmap_object(HKEY(physicalDevices[i]));
	}

	delete[] physicalDevices;
}

/**/
static struct device_data *new_device_data(
	VkDevice             device,
	struct instance_data *instance)
{
	auto data = new device_data();
	data->instance = instance;
	data->device = device;
	map_object(HKEY(data->device), data);
	return data;
}

static struct queue_data *new_queue_data(
	VkQueue                       queue,
	const VkQueueFamilyProperties *family_props,
	uint32_t family_index,
	struct device_data *device_data)
{
	auto data = new queue_data();
	data->device = device_data;
	data->queue = queue;
	data->flags = family_props->queueFlags;
	data->timestamp_mask = (1ull << family_props->timestampValidBits) - 1;
	data->family_index = family_index;
	map_object(HKEY(data->queue), data);

	/* Fence synchronizing access to queries on that queue. */
	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	VK_CHECK(device_data->vtable.CreateFence(
		device_data->device,
		&fence_info,
		NULL,
		&data->queries_fence
	));

	return data;
}

static void destroy_queue(struct queue_data *data)
{
	struct device_data *device_data = data->device;
	device_data->vtable.DestroyFence(device_data->device, data->queries_fence, NULL);
	unmap_object(HKEY(data->queue));
	delete data;
}

static void destroy_device_data(struct device_data *data)
{
	unmap_object(HKEY(data->device));
	delete data;
}


static void device_map_queues(
	struct device_data       *data,
	const VkDeviceCreateInfo *pCreateInfo)
{
	for (uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; i++) {
		data->n_queues += pCreateInfo->pQueueCreateInfos[i].queueCount;
	}
	data->queues = new queue_data*[data->n_queues];

	struct instance_data *instance_data = data->instance;
	uint32_t n_family_props;
	instance_data->vtable.GetPhysicalDeviceQueueFamilyProperties(
		data->physical_device,
		&n_family_props,
		NULL
	);
	auto family_props = new VkQueueFamilyProperties[n_family_props];
	instance_data->vtable.GetPhysicalDeviceQueueFamilyProperties(
		data->physical_device,
		&n_family_props,
		family_props
	);

	uint32_t queue_index = 0;
	for (uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; i++) {
		for (uint32_t j = 0; j < pCreateInfo->pQueueCreateInfos[i].queueCount; j++) {
			VkQueue queue;
			data->vtable.GetDeviceQueue(
				data->device,
				pCreateInfo->pQueueCreateInfos[i].queueFamilyIndex,
				j,
				&queue
			);

			VK_CHECK(data->set_device_loader_data(data->device, queue));

			data->queues[queue_index++] =
			new_queue_data(
				queue,
				&family_props[pCreateInfo->pQueueCreateInfos[i].queueFamilyIndex],
				pCreateInfo->pQueueCreateInfos[i].queueFamilyIndex,
				data
			);
		}
	}

	delete[] family_props;
}

static void device_unmap_queues(struct device_data *data)
{
	for (uint32_t i = 0; i < data->n_queues; i++)
		destroy_queue(data->queues[i]);
	delete[] data->queues;
}

static VkResult overlay_QueuePresentKHR(
	 VkQueue                 queue,
	 const VkPresentInfoKHR* pPresentInfo)
{
	struct queue_data *queue_data = FIND(struct queue_data, queue);

	std::scoped_lock lock(mutex_lock);
	VkResult result = queue_data->device->vtable.QueuePresentKHR(queue, pPresentInfo);
	limiter( &config );
	return result;
}


static VkResult overlay_CreateDevice(
	 VkPhysicalDevice             physicalDevice,
	 const VkDeviceCreateInfo*    pCreateInfo,
	 const VkAllocationCallbacks* pAllocator,
	 VkDevice*                    pDevice)
{
	struct instance_data *instance_data =
		FIND(struct instance_data, physicalDevice);
	VkLayerDeviceCreateInfo *chain_info =
		get_device_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

	assert(chain_info->u.pLayerInfo);
	PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr =
		chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
	PFN_vkGetDeviceProcAddr fpGetDeviceProcAddr =
		chain_info->u.pLayerInfo->pfnNextGetDeviceProcAddr;
	PFN_vkCreateDevice fpCreateDevice =
		(PFN_vkCreateDevice)fpGetInstanceProcAddr(NULL, "vkCreateDevice");
	if (fpCreateDevice == NULL) {
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	// Advance the link info for the next element on the chain
	chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

	VkPhysicalDeviceFeatures device_features = {};
	VkDeviceCreateInfo device_info = *pCreateInfo;

	if (pCreateInfo->pEnabledFeatures) {
		device_features = *(pCreateInfo->pEnabledFeatures);
	}
	device_info.pEnabledFeatures = &device_features;


	VkResult result = fpCreateDevice(physicalDevice, &device_info, pAllocator, pDevice);
	if (result != VK_SUCCESS) {
		return result;
	}

	struct device_data *device_data = new_device_data(*pDevice, instance_data);
	device_data->physical_device = physicalDevice;
	vk_load_device_commands(*pDevice, fpGetDeviceProcAddr, &device_data->vtable);

	instance_data->vtable.GetPhysicalDeviceProperties(
		device_data->physical_device,
		&device_data->properties
	);

	VkLayerDeviceCreateInfo *load_data_info =
		get_device_chain_info(pCreateInfo, VK_LOADER_DATA_CALLBACK);
	device_data->set_device_loader_data = load_data_info->u.pfnSetDeviceLoaderData;

	device_map_queues(device_data, pCreateInfo);

	return result;
}

static void overlay_DestroyDevice(
	 VkDevice                     device,
	 const VkAllocationCallbacks* pAllocator)
{
	struct device_data *device_data = FIND(struct device_data, device);
	device_unmap_queues(device_data);
	device_data->vtable.DestroyDevice(device, pAllocator);
	destroy_device_data(device_data);
}

static VkResult overlay_CreateInstance(
	 const VkInstanceCreateInfo*  pCreateInfo,
	 const VkAllocationCallbacks* pAllocator,
	 VkInstance*                  pInstance)
{
	VkLayerInstanceCreateInfo *chain_info =
		get_instance_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

	assert(chain_info->u.pLayerInfo);
	PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr =
		chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
	PFN_vkCreateInstance fpCreateInstance =
		(PFN_vkCreateInstance)fpGetInstanceProcAddr(NULL, "vkCreateInstance");
	if (fpCreateInstance == NULL) {
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	// Advance the link info for the next element on the chain
	chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

	VkResult result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);
	if (result != VK_SUCCESS) return result;

	struct instance_data *instance_data = new_instance_data(*pInstance);
	vk_load_instance_commands(
		instance_data->instance,
		fpGetInstanceProcAddr,
		&instance_data->vtable
	);
	instance_data_map_physical_devices(instance_data, true);

	config = strangle_createConfig();

	return result;
}

static void overlay_DestroyInstance(
	 VkInstance                   instance,
	 const VkAllocationCallbacks* pAllocator)
{
	struct instance_data *instance_data = FIND(struct instance_data, instance);
	instance_data_map_physical_devices(instance_data, false);
	instance_data->vtable.DestroyInstance(instance, pAllocator);
	destroy_instance_data(instance_data);
}

static VkResult overlay_CreateSwapchainKHR(
	VkDevice                        device,
	const VkSwapchainCreateInfoKHR* pCreateInfo,
	const VkAllocationCallbacks*    pAllocator,
	VkSwapchainKHR*                 pSwapchain)
{
	VkSwapchainCreateInfoKHR newPCreateInfo = *pCreateInfo;
	if ( config.vsync != NULL && *config.vsync >= 0 && *config.vsync <= 3) {
		newPCreateInfo.presentMode = (VkPresentModeKHR)*config.vsync;
	}

	struct device_data *device_data = FIND(struct device_data, device);
	VkResult result = device_data->vtable.CreateSwapchainKHR(
		device,
		&newPCreateInfo,
		pAllocator,
		pSwapchain
	);

	if (result != VK_SUCCESS) {
		return result;
	}

	return result;
}

static VkResult overlay_CreateSampler(
	VkDevice                     device,
	const VkSamplerCreateInfo*   pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkSampler*                   pSampler)
{
	VkSamplerCreateInfo newSamplerCreateInfo = *pCreateInfo;

	if ( config.mipLodBias != NULL ) {
		newSamplerCreateInfo.mipLodBias = *config.mipLodBias;
	}

	if ( config.trilinear != NULL && *config.trilinear == 1 ) {
		newSamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		newSamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		newSamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	} else if ( config.retro && *config.retro == 1 ) {
		newSamplerCreateInfo.magFilter = VK_FILTER_NEAREST;
		newSamplerCreateInfo.minFilter = VK_FILTER_NEAREST;
		newSamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	} else if ( config.cubic_filter && *config.cubic_filter == 1 ) {
		newSamplerCreateInfo.magFilter = VK_FILTER_CUBIC_IMG;
		newSamplerCreateInfo.minFilter = VK_FILTER_CUBIC_IMG;
		newSamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}

	if ( config.anisotropy != NULL && *config.anisotropy >= 1 && *config.anisotropy <= 16 ) {
		newSamplerCreateInfo.anisotropyEnable = VK_TRUE;
		newSamplerCreateInfo.maxAnisotropy = *config.anisotropy;
	}

	struct device_data *device_data = FIND(struct device_data, device);
	VkResult result = device_data->vtable.CreateSampler(device, &newSamplerCreateInfo, pAllocator, pSampler);

	if (result != VK_SUCCESS) {
		return result;
	}

	return result;
}

static const struct {
	const char *name;
	void *ptr;
} name_to_funcptr_map[] = {
	{ "vkGetDeviceProcAddr", (void *) vkGetDeviceProcAddr },
#define ADD_HOOK(fn) { "vk" # fn, (void *) overlay_ ## fn }
#define ADD_ALIAS_HOOK(alias, fn) { "vk" # alias, (void *) overlay_ ## fn }

	ADD_HOOK(QueuePresentKHR),
	ADD_HOOK(CreateSwapchainKHR),
	ADD_HOOK(CreateSampler),

	ADD_HOOK(CreateDevice),
	ADD_HOOK(DestroyDevice),

	ADD_HOOK(CreateInstance),
	ADD_HOOK(DestroyInstance),
#undef ADD_HOOK
};

static void *find_ptr(const char *name)
{
	for (uint32_t i = 0; i < ARRAY_SIZE(name_to_funcptr_map); i++) {
		if (strcmp(name, name_to_funcptr_map[i].name) == 0) {
			return name_to_funcptr_map[i].ptr;
		}
	}

	return NULL;
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(
	VkDevice dev,
	const char *funcName)
{
	void *ptr = find_ptr(funcName);
	if (ptr) {
		return reinterpret_cast<PFN_vkVoidFunction>(ptr);
	}

	if (dev == NULL) {
		return NULL;
	}

	struct device_data *device_data = FIND(struct device_data, dev);
	if (device_data->vtable.GetDeviceProcAddr == NULL) {
		return NULL;
	}
	return device_data->vtable.GetDeviceProcAddr(dev, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(
	VkInstance instance,
	const char *funcName)
{
	void *ptr = find_ptr(funcName);
	if (ptr) {
		return reinterpret_cast<PFN_vkVoidFunction>(ptr);
	}

	if (instance == NULL) {
		return NULL;
	}

	struct instance_data *instance_data = FIND(struct instance_data, instance);
	if (instance_data->vtable.GetInstanceProcAddr == NULL) {
		return NULL;
	}
	return instance_data->vtable.GetInstanceProcAddr(instance, funcName);
}
