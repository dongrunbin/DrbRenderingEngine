#include "VulkanAPI.h"
#include "BVulkan.h"

XBufferObject::XBufferObject() 
{
	buffer = 0;
	memory = 0;
}

XBufferObject::~XBufferObject()
{
	if (buffer != 0)
	{
		vkDestroyBuffer(GetVulkanDevice(), buffer, nullptr);
	}
	if (memory != 0)
	{
		vkFreeMemory(GetVulkanDevice(), memory, nullptr);
	}
}

void xglBufferData(XVulkanHandle buffer, int size, void *data)
{

}