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
	XBufferObject* vbo = (XBufferObject*)buffer;
	xGenVertexBuffer(size, vbo->buffer, vbo->memory);
	xBufferSubVertexData(vbo->buffer, data, size);
}

VkResult xGenBuffer(VkBuffer& buffer, VkDeviceMemory& memory, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage;
	VkResult result = vkCreateBuffer(GetVulkanDevice(), &bufferCreateInfo, nullptr, &buffer);
	if (result != VK_SUCCESS)
	{
		printf("failed to create buffer.");
		return result;
	}
	VkMemoryRequirements requirements;
	vkGetBufferMemoryRequirements(GetVulkanDevice(), buffer, &requirements);
	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = requirements.size;
	memoryAllocateInfo.memoryTypeIndex = FindMemoryType(requirements.memoryTypeBits, properties);
	result = vkAllocateMemory(GetVulkanDevice(), &memoryAllocateInfo, nullptr, &memory);
	if (result != VK_SUCCESS)
	{
		printf("failed to allocate memory.");
		return result;
	}
	vkBindBufferMemory(GetVulkanDevice(), buffer, memory, 0);
	return result;
}

void xBufferSubData(VkBuffer buffer, VkBufferUsageFlags usage, const void* data, VkDeviceSize size)
{
	VkBuffer tempBuffer;
	VkDeviceMemory tempMemory;
	xGenBuffer(tempBuffer, tempMemory, size, usage, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	void* hostMemory;
	vkMapMemory(GetVulkanDevice(), tempMemory, 0, size, 0, &hostMemory);
	memcpy(hostMemory, data, (size_t)size);
	vkUnmapMemory(GetVulkanDevice(), tempMemory);

	VkCommandBuffer commandBuffer;
	xBeginOneTimeCommandBuffer(&commandBuffer);
	VkBufferCopy copy = {0, 0, size};
	vkCmdCopyBuffer(commandBuffer, tempBuffer, buffer, 1, &copy);
	xEndOneTimeCommandBuffer(commandBuffer);

	vkDestroyBuffer(GetVulkanDevice(), tempBuffer, nullptr);
	vkFreeMemory(GetVulkanDevice(), tempMemory, nullptr);
}

uint32_t xGetMemoryType(uint32_t typeFilters, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(GetVulkanPhysicalDevice(), &memoryProperties);
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		uint32_t flag = 1 << i;
		if ((flag & typeFilters) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}
	return 0;
}

void xBeginOneTimeCommandBuffer(VkCommandBuffer* commandBuffer)
{
	xGenCommandBuffer(commandBuffer, 1);
	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(*commandBuffer, &info);
}

void xEndOneTimeCommandBuffer(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);
	xWaitForCommandFinish(commandBuffer);
	vkFreeCommandBuffers(GetVulkanDevice(), GetCommandPool(), 1, &commandBuffer);
}

void xGenCommandBuffer(VkCommandBuffer* commandBuffer, int count, VkCommandBufferLevel level)
{
	VkCommandBufferAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.level = level;
	info.commandPool = GetCommandPool();
	info.commandBufferCount = count;
	vkAllocateCommandBuffers(GetVulkanDevice(), &info, commandBuffer);
}

void xWaitForCommandFinish(VkCommandBuffer commandBuffer)
{
	VkSubmitInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.commandBufferCount = 1;
	info.pCommandBuffers = &commandBuffer;
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	VkFence fence;
	vkCreateFence(GetVulkanDevice(), &fenceInfo, nullptr, &fence);
	vkQueueSubmit(GetGraphicQueue(), 1, &info, fence);
	vkWaitForFences(GetVulkanDevice(), 1, &fence, VK_TRUE, 100000000);
	vkDestroyFence(GetVulkanDevice(), fence ,nullptr);
}