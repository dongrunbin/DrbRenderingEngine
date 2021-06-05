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

struct XMatrix4x4f
{
	float data[16];
};

struct XVector4f
{
	float data[4];
};

enum XUniformBufferType
{
	XUniformBufferTypeMatrix,
	XUniformBufferTypeVector,
	XUniformBufferTypeCount,
};

struct XUniformBuffer
{
	VkBuffer buffer;
	VkDeviceMemory memory;
};

/// <summary>
/// �����ݴ�CPU����GPU
/// </summary>
/// <param name="vbo"></param>
/// <param name="size"></param>
/// <param name="data"></param>
void xglBufferData(XVulkanHandle buffer, int size, void* data);

/// <summary>
/// �����Դ滺����
/// </summary>
/// <param name="buffer"></param>
/// <param name="memory"></param>
/// <param name="size"></param>
/// <param name="usage"></param>
/// <param name="properties"></param>
/// <returns></returns>
VkResult xGenBuffer(VkBuffer& buffer, VkDeviceMemory& memory, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

/// <summary>
/// ���������ݴ���
/// </summary>
/// <param name="buffer"></param>
/// <param name="usage"></param>
/// <param name="data"></param>
/// <param name="size"></param>
void xBufferSubData(VkBuffer buffer, VkBufferUsageFlags usage, const void *data, VkDeviceSize size);

/// <summary>
/// ��ȡ�ڴ�����
/// </summary>
/// <param name="typeFilters"></param>
/// <param name="properties"></param>
/// <returns></returns>
uint32_t xGetMemoryType(uint32_t typeFilters, VkMemoryPropertyFlags properties);

/// <summary>
/// ��ʼһ�λ�������
/// </summary>
/// <param name="commandBuffer"></param>
void xBeginOneTimeCommandBuffer(VkCommandBuffer *commandBuffer);
/// <summary>
/// ����һ�λ�������
/// </summary>
/// <param name="commandBuffer"></param>
void xEndOneTimeCommandBuffer(VkCommandBuffer commandBuffer);
/// <summary>
/// �����������
/// </summary>
/// <param name="commandBuffer"></param>
/// <param name="count"></param>
/// <param name="level"></param>
void xGenCommandBuffer(VkCommandBuffer *commandBuffer, int count, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
/// <summary>
/// �ȴ��������
/// </summary>
/// <param name="commandBuffer"></param>
void xWaitForCommandFinish(VkCommandBuffer commandBuffer);

// ����VBO�Դ滺����
#define xGenVertexBuffer(size, buffer, memory) \
	xGenBuffer(buffer, memory, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

#define xBufferSubVertexData(buffer, data, size) \
	xBufferSubData(buffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, data, size);