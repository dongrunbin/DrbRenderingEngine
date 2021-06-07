#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <malloc.h>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include "glm/glm.hpp"
#include "glm/ext.hpp"

typedef void* XVulkanHandle;

enum XBufferObjectType
{
	XBufferObjectTypeVertexBuffer,
	XBufferObjectTypeIndexBuffer,
	XBufferObjectTypeUniformBuffer,
	XBufferObjectTypeCount
};

struct XBufferObject 
{
	VkBuffer buffer;
	VkDeviceMemory memory;
	XBufferObjectType type;
	XBufferObject();
	virtual ~XBufferObject();
	void OnSetSize();
	void SubmitData(const void* data, int size);
	virtual int GetSize();
};

struct XMatrix4x4f
{
	float data[16];

	XMatrix4x4f()
	{
		memset(data, 0, sizeof(float) * 16);
		data[0] = 1.0f;
		data[5] = 1.0f;
		data[10] = 1.0f;
		data[15] = 1.0f;
	}
};

struct XVector4f
{
	float data[4];
};

struct XVertexData
{
	float position[4];
	float texcoord[4];
	float normal[4];
	float tangent[4];
	void SetPosition(float x, float y, float z, float w = 1.0f);
	void SetTexcoord(float x, float y, float z = 0.0f, float w = 0.0f);
	void SetNormal(float x, float y, float z, float w = 0.0f);
	void SetTangent(float x, float y, float z, float w = 0.0f);
	static const VkVertexInputBindingDescription& BindingDescription();
	static const std::vector<VkVertexInputAttributeDescription>& AttributeDescriptions();
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
	XUniformBufferType type;
	std::vector<XMatrix4x4f> matrices;
	std::vector<XVector4f> vectors;
	XUniformBuffer();
	~XUniformBuffer();
};

struct XFixedPipeline
{
	//渲染管线状态
	VkPipeline mPipeline;
	//负责shader的输入输出
	VkPipelineLayout mPipelineLayout;
	VkDescriptorSetLayout* mDescriptorSetLayout;
	VkPipelineShaderStageCreateInfo* mShaderStages;
	int mShaderStageCount, mDescriptorSetLayoutCount;
	VkRenderPass mRenderPass;
	//是否MSAA
	VkSampleCountFlagBits mSampleCount;
	//输入的图元
	VkPipelineInputAssemblyStateCreateInfo mInputAssetmlyState;
	VkPipelineViewportStateCreateInfo mViewportState;
	VkViewport mViewport;
	VkRect2D mScissor;
	//线框模式/填充模式
	VkPipelineRasterizationStateCreateInfo mRasterizer;
	VkPipelineDepthStencilStateCreateInfo mDepthStencilState;
	VkPipelineMultisampleStateCreateInfo mMultisampleState;
	std::vector<VkPipelineColorBlendAttachmentState> mColorBlendAttachmentStates;
	VkPipelineColorBlendStateCreateInfo mColorBlendState;
	XVector4f mPushConstants[16];
	int mPushConstantCount;
	VkShaderStageFlags mPushConstantShaderStage;
	float mDepthConstantFactor, mDepthClamp, mDepthSlopeFactor;
	XFixedPipeline();
	~XFixedPipeline();
	void CleanUp();
};

struct XProgram
{
	VkPipelineShaderStageCreateInfo shaderStages[2];
	int shaderStageCount;
	VkShaderModule vertexShader, fragmentShader;
	VkDescriptorSetLayout descriptorSetLayout;
	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
	std::vector<VkDescriptorPoolSize> descriptorPoolSize;
	std::vector<VkWriteDescriptorSet> writeDescriptorSet;
	VkDescriptorSet descriptorSet;
	VkDescriptorPool descriptorPool;

	XUniformBuffer vertexShaderMatrixUniformBuffer;
	XUniformBuffer fragmentShaderMatrixUniformBuffer;
	XUniformBuffer vertexShaderVectorUniformBuffer;
	XUniformBuffer fragmentShaderVectorUniformBuffer;
	XFixedPipeline fixedPipeline;
	XProgram();
	~XProgram();
};

struct XTexture
{
	VkImage image;
	VkDeviceMemory memory;
	VkImageView imageView;
	VkSampler sampler;
	//纹理状态
	VkImageLayout srcLayout;
	VkImageLayout dstLayout;
	VkPipelineStageFlags srcStage;
	VkPipelineStageFlags dstStage;
	//color depth
	VkImageAspectFlags imageAspectFlags;
	VkFormat format;
	VkFilter minFilter, magFilter;
	VkSamplerAddressMode wrapU, wrapV, wrapW;
	VkBool32 enableAnisotropy;
	float maxAnisotropy;
	XTexture(VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT);
	~XTexture();
};

/// <summary>
/// 将数据从CPU传到GPU
/// </summary>
/// <param name="vbo"></param>
/// <param name="size"></param>
/// <param name="data"></param>
void xglBufferData(XVulkanHandle buffer, int size, void* data);

/// <summary>
/// 生成显存缓冲区
/// </summary>
/// <param name="buffer"></param>
/// <param name="memory"></param>
/// <param name="size"></param>
/// <param name="usage"></param>
/// <param name="properties"></param>
/// <returns></returns>
VkResult xGenBuffer(VkBuffer& buffer, VkDeviceMemory& memory, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

/// <summary>
/// 缓冲区数据传输
/// </summary>
/// <param name="buffer"></param>
/// <param name="usage"></param>
/// <param name="data"></param>
/// <param name="size"></param>
void xBufferSubData(VkBuffer buffer, VkBufferUsageFlags usage, const void *data, VkDeviceSize size);

/// <summary>
/// 获取内存类型
/// </summary>
/// <param name="typeFilters"></param>
/// <param name="properties"></param>
/// <returns></returns>
uint32_t xGetMemoryType(uint32_t typeFilters, VkMemoryPropertyFlags properties);

/// <summary>
/// 开始一次绘制命令
/// </summary>
/// <param name="commandBuffer"></param>
void xBeginOneTimeCommandBuffer(VkCommandBuffer *commandBuffer);
/// <summary>
/// 结束一次绘制命令
/// </summary>
/// <param name="commandBuffer"></param>
void xEndOneTimeCommandBuffer(VkCommandBuffer commandBuffer);
/// <summary>
/// 生成命令缓冲区
/// </summary>
/// <param name="commandBuffer"></param>
/// <param name="count"></param>
/// <param name="level"></param>
void xGenCommandBuffer(VkCommandBuffer *commandBuffer, int count, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
/// <summary>
/// 等待命令结束
/// </summary>
/// <param name="commandBuffer"></param>
void xWaitForCommandFinish(VkCommandBuffer commandBuffer);

void xCreateShader(VkShaderModule &shader, unsigned char *code, int codeLength);

void xAttachVertexShader(XProgram *program, VkShaderModule shader);
void xAttachFragmentShader(XProgram* program, VkShaderModule shader);
void xLinkProgram(XProgram* program);

void xInitDescriptorSetLayout(XProgram* program);
/// <summary>
/// 生成Uniform插槽管理器
/// </summary>
/// <param name="program"></param>
void xInitDescriptorPool(XProgram* program);
/// <summary>
/// 生成Uniform插槽
/// </summary>
/// <param name="program"></param>
void xInitDescriptorSet(XProgram* program);

void xSubmitUniformBuffer(XUniformBuffer* uniformBuffer);

void xConfigUniformBuffer(XVulkanHandle param, int binding, XBufferObject* ubo, VkShaderStageFlags shaderStageFlags);

void xGenImage(XTexture* texture, uint32_t w, uint32_t h, VkFormat format, VkImageUsageFlags usage, 
	VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, int mipmap = 1);

//像素数据从CPU拷贝到GPU
void xSubmitImage2D(XTexture* texture, int width, int height, const void* pixel);

void xSetImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
	VkImageSubresourceRange range, VkPipelineStageFlags src = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags dst = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

void xGenImageView2D(XTexture* texture, int mipmap = 1);
void xGenSampler(XTexture* texture);
void xInitDefaultTexture();
void xVulkanCleanUp();
void xConfigSampler2D(XProgram* program, int binding, VkImageView imageview, VkSampler sampler,
	VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
/// <summary>
/// 更新Uniform
/// </summary>
/// <param name="program"></param>
/// <param name="location"></param>
/// <param name="v"></param>
void xUniform4fv(XProgram* program, int location, float* v);

unsigned char* LoadImageFromFile(const char* path, int& width, int& height, int& channel, int force_channel
	, bool flipY = false);
XTexture* xGetDefaultTexture();

void xRebindUniformBuffer(XProgram* program, int binding, XUniformBuffer* ubo);
void xRebindSampler(XProgram* program, int binding, VkImageView iv, VkSampler s,
	VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

void xUseProgram(XProgram* program);
void xBindVertexBuffer(XBufferObject* vbo);
void xBindElementBuffer(XBufferObject* ibo);
void xDrawArrays(VkCommandBuffer commandbuffer, int offset, int count);
void xDrawElements(VkCommandBuffer commandbuffer, int offset, int count);
VkCommandBuffer xBeginRendering(VkCommandBuffer commandbuffer = nullptr);
void xEndRendering();
void xSwapBuffers(VkCommandBuffer commandbuffer = nullptr);

void xSetColorAttachmentCount(XFixedPipeline* pipeline, int count);
void xEnableBlend(XFixedPipeline* pipeline, int attachment, VkBool32 enable);
void xBlend(XFixedPipeline* p, int attachment, VkBlendFactor s_c, VkBlendFactor s_a,
	VkBlendFactor d_c, VkBlendFactor d_a);
void xBlendOp(XFixedPipeline* p, int attachment, VkBlendOp color, VkBlendOp alpha);
void xPolygonMode(XFixedPipeline* p, VkPolygonMode mode);
void xDisableRasterizer(XFixedPipeline* p, VkBool32 disable);
void xEnableDepthTest(XFixedPipeline* p, VkBool32 enable);
void xInitPipelineLayout(XFixedPipeline* p);
void xCreateFixedPipeline(XFixedPipeline* p);
void xSetDynamicState(XFixedPipeline* p, VkCommandBuffer commandbuffer);

// 生成VBO显存缓冲区
#define xGenVertexBuffer(size, buffer, memory) \
	xGenBuffer(buffer, memory, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
#define xGenIndexBuffer(size, buffer, memory) \
	xGenBuffer(buffer, memory, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

#define xBufferSubVertexData(buffer, data, size) \
	xBufferSubData(buffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, data, size);
#define xBufferSubIndexData(buffer, data, size) \
	xBufferSubData(buffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, data, size);