#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <malloc.h>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
typedef void* XVulkanHandle;
struct XBufferObject 
{
	VkBuffer buffer;
	VkDeviceMemory memory;
	XBufferObject();
	~XBufferObject();
};

void xglBufferData(XVulkanHandle vbo, int size, void *data)
{
	 
}