#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <malloc.h>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
typedef uint64_t GLuint;
typedef void* AVulkanHandle;
struct Vector4f {
	float v[4];
	Vector4f() {
		memset(v, 0, sizeof(float) * 4);
	}
	Vector4f(float x, float y, float z, float w) {
		v[0] = x;
		v[1] = y;
		v[2] = z;
		v[3] = w;
	}
};
#define A_TRIANGLES VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
#define A_TRIANGLE_STRIP VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP
enum AUniformBufferType {
	kUniformBufferTypeMatrix,
	kUniformBufferTypeVector,
	kUniformBufferTypeCount
};
struct Vertex {
	float mPosition[4];
	float mTexcoord[4];
	float mNormal[4];
	float mTangent[4];
	void SetPosition(float x, float y, float z, float w = 1.0f) {
		mPosition[0] = x;
		mPosition[1] = y;
		mPosition[2] = z;
		mPosition[3] = w;
	}
	void SetTexcoord(float x, float y, float z = 0.0f, float w = 0.0f) {
		mTexcoord[0] = x;
		mTexcoord[1] = y;
		mTexcoord[2] = z;
		mTexcoord[3] = w;
	}
	void SetNormal(float x, float y, float z, float w = 0.0f) {
		mNormal[0] = x;
		mNormal[1] = y;
		mNormal[2] = z;
		mNormal[3] = w;
	}
	void SetTangent(float x, float y, float z, float w = 0.0f) {
		mTangent[0] = x;
		mTangent[1] = y;
		mTangent[2] = z;
		mTangent[3] = w;
	}
	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		attributeDescriptions.resize(4);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[0].offset = 0;

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[1].offset = sizeof(float) * 4;

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[2].offset = sizeof(float) * 8;

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[3].offset = sizeof(float) * 12;
		return attributeDescriptions;
	}
};
struct VulkanTexture {
	VkImage mImage;
	VkDeviceMemory mMemory;
	VkImageView mImageView;
	VkSampler mSampler;
	VkImageLayout mInitLayout;
	VkImageLayout mTargetLayout;
	VkPipelineStageFlags mSrcStage;
	VkPipelineStageFlags mTargetStage;
	VkImageAspectFlags mImageAspectFlag;
	VkFormat mFormat;
	VkFilter mMinFilter, mMagFilter;
	VkSamplerAddressMode mWrapU, mWrapV, mWrapW;
	VkBool32 mbEnableAnisotropy;
	float mMaxAnisotropy;
	VulkanTexture(VkImageAspectFlags imageAspectFlag = VK_IMAGE_ASPECT_COLOR_BIT);
	virtual ~VulkanTexture();
};
struct VulkanRenderPass{
	VkSampleCountFlagBits mSampleCount;
	VkRenderPass mRenderPass;
	int mColorBufferCount;
	int mDepthBufferCount;
};
struct VulkanFrameBuffer{
	VkFramebuffer mFrameBuffer;
	VkImageView mColorBuffer;
	VkImageView mDepthBuffer;
	VkImageView mResolveBuffer;
	VkSampleCountFlagBits mSampleCount;
	VulkanFrameBuffer();
	~VulkanFrameBuffer();
};
class GraphicPipeline {
public:
	VkPipeline mPipeline;
	VkPipelineLayout mPipelineLayout;
	VkDescriptorSetLayout *mDescriptorSetLayout;
	VkPipelineShaderStageCreateInfo *mShaderStage;
	int mShaderStageCount, mDescriptorSetLayoutCount;
	VkRenderPass mRenderPass;
	VkSampleCountFlagBits mSampleCount;
	VkPipelineInputAssemblyStateCreateInfo mInputAssemblyState;
	VkPipelineViewportStateCreateInfo mViewportState;
	VkViewport mViewport;
	VkRect2D mScissor;
	VkPipelineRasterizationStateCreateInfo mRasterizer;
	VkPipelineDepthStencilStateCreateInfo mDepthStencilState;
	VkPipelineMultisampleStateCreateInfo mMultisampleState;
	std::vector<VkPipelineColorBlendAttachmentState> mColorBlendAttachmentStates;
	VkPipelineColorBlendStateCreateInfo mColorBlendState;
	Vector4f mPushConstants[16];
	int mPushConstantCount;
	VkShaderStageFlags mPushConstantShaderStage;
	float mDepthConstantFactor, mDepthClamp, mDepthSlopeFactor;
public:
	GraphicPipeline();
	~GraphicPipeline();
	void CleanUp();
};
struct SwapChainInfo {
	VkSurfaceCapabilitiesKHR mCapabilities;
	std::vector<VkSurfaceFormatKHR> mFormats;
	std::vector<VkPresentModeKHR> mPresentModes;
};
void aSetDescriptorSetLayout(AVulkanHandle program, VkDescriptorSetLayout*layout, int count = 1);
void aSetShaderStage(AVulkanHandle program, VkPipelineShaderStageCreateInfo*ss, int count);
void aSetRenderPass(AVulkanHandle program, VkRenderPass render_pass);
void aSetBufferSampleCount(AVulkanHandle program, VkSampleCountFlagBits sample_count);
void aInitPipelineLayout(AVulkanHandle program);
void aSetColorAttachmentCount(AVulkanHandle program, int count);
void aCreateGraphicPipeline(AVulkanHandle program);
void aSetCullMode(AVulkanHandle program, VkCullModeFlags mode);
void aSetFrontFace(AVulkanHandle program, VkFrontFace front);
void aEnableBlend(AVulkanHandle program, int attachment_index, VkBool32 enable);
void aBlend(AVulkanHandle program, int attachment_index, VkBlendFactor src_color, VkBlendFactor src_alpha, VkBlendFactor dst_color, VkBlendFactor dst_alpha);
void aBlendOp(AVulkanHandle program, int attachment_index, VkBlendOp color, VkBlendOp alpha);
void aEnableZTest(AVulkanHandle program, VkBool32 enable);
void aEnableZWrite(AVulkanHandle program, VkBool32 enable);
void aEnableStencilTest(AVulkanHandle program, VkBool32 enable, VkCompareOp compare, VkStencilOp failed, VkStencilOp depthfailed, VkStencilOp passed);
void aSetDepthFunc(AVulkanHandle program, VkCompareOp func);
void aSetPrimitiveType(AVulkanHandle program, VkPrimitiveTopology t, VkBool32 restart = VK_FALSE);
void aSetConstant(AVulkanHandle program, int index, Vector4f&v);
void aSetDynamicState(AVulkanHandle program, VkCommandBuffer commandbuffer);
void aSetDepthBias(AVulkanHandle program, float constant, float clamp, float slope);
unsigned char * LoadFileContent(const char *path, int &filesize);
bool InitVulkan(void*param, int width, int height);
void VulkanCleanUp();
VkRenderPass GetGlobalRenderPass();
VkFramebuffer AquireRenderTarget();
void VulkanSubmitDrawCommand(VkCommandBuffer*commandbuffer, int count);
void VulkanSwapBuffers();
VkInstance GetVulkanInstance();
VkSurfaceKHR GetVulkanSurface();
VkDevice GetVulkanDevice();
VkPhysicalDevice GetVulkanPhysicalDevice();
VkCommandPool GetCommandPool();
VkFormat GetSwapChainImageFormat();
VkFramebuffer GetFrameBuffer(int index);
VkSampleCountFlagBits GetGlobalFrameBufferSampleCount();
int GetFrameBufferCount();
VkImageView GetSwapChainImageView(int index);
void InitSwapChainInfo();
SwapChainInfo GetSwapChainInfo();
void aViewport(int width, int height);
int GetViewportWidth();
int GetViewportHeight();
VkQueue GetGraphicQueue();
VkQueue GetPresentQueue();
int GetGraphicQueueFamily();
int GetPresentQueueFamily();
VkSemaphore GetReadyToRenderSemaphore();
VkSemaphore GetReadyToPresentSemaphore();
VkSwapchainKHR GetSwapchain();
uint32_t GetCurrentRenderTargetIndex();
void aClearColorBuffer(VkCommandBuffer commandbuffer,float r,float g,float b,float a,int color_buffer_index=0);
void aClearDepthBuffer(VkCommandBuffer commandbuffer, float depth);
void aClearStencilBuffer(VkCommandBuffer commandbuffer, uint32_t stencil);
VkResult aGenCommandBuffer(VkCommandBuffer *command_buffer, int count, VkCommandBufferLevel level=VK_COMMAND_BUFFER_LEVEL_PRIMARY);
void aDeleteCommandBuffer(VkCommandBuffer *command_buffer,int count);
VkResult aBeginOneTimeCommandBuffer(VkCommandBuffer * command_buffer);
VkResult aEndOneTimeCommandBuffer(VkCommandBuffer command_buffer);
VkResult aGenBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
#define aGenVertexBuffer(size,buffer,bufferMemory) \
	aGenBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory)
#define aGenIndexBuffer(size,buffer,bufferMemory) \
	aGenBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory)
#define aGenUniformBuffer(size,buffer,bufferMemory) \
	aGenBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, bufferMemory)
#define aGenImageBuffer(size,buffer,bufferMemory) \
	aGenBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, bufferMemory)
void aBufferSubData(VkBuffer buffer, VkBufferUsageFlags usage,const void* data, VkDeviceSize size);
#define aBufferSubVertexData(buffer,data,size) \
	aBufferSubData(buffer,VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,data,size)
#define aBufferSubIndexData(buffer,data,size) \
	aBufferSubData(buffer,VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,data,size)
void aDeleteBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory);
void aCreateShader(VkShaderModule& shaderModule, unsigned char *code, int code_len);
void aDestroyShader(VkShaderModule&shaderModule);
AVulkanHandle glGenBuffer();
void glBufferData(AVulkanHandle buffer, int size, void *data);
void glDeleteBufferObject(AVulkanHandle buffer);
AVulkanHandle aCreateProgram();
void aAttachVertexShader(AVulkanHandle program, VkShaderModule shader);
void aAttachFragmentShader(AVulkanHandle program, VkShaderModule shader);
void aLinkProgram(AVulkanHandle program);
void InitDescriptorSetLayout(AVulkanHandle program);
void InitDescriptorPool(AVulkanHandle program);
void InitDescriptorSet(AVulkanHandle program);
void glUseProgram(AVulkanHandle program);
void aDeleteProgram(AVulkanHandle program);
void glBindVertexBuffer(AVulkanHandle buffer);
void glDrawArrays(int mode, int offset, int count);
void aMapMemory(VkDeviceMemory memory,VkDeviceSize offset,VkDeviceSize size,VkMemoryMapFlags flags,void**ppData);
void aUnmapMemory(VkDeviceMemory memory);
void aFrameBufferSampleCount(VulkanFrameBuffer*framebuffer, VkSampleCountFlagBits sample_count);
void aFrameBufferColorBuffer(VulkanFrameBuffer*framebuffer, VkImageView colorbuffer);
void aFrameBufferDepthBuffer(VulkanFrameBuffer*framebuffer, VkImageView depthbuffer);
void aFrameBufferResolveBuffer(VulkanFrameBuffer*framebuffer, VkImageView resolvebuffer);
void aFrameBufferFinish(VulkanFrameBuffer*framebuffer);
bool IsFormatSupported(VkFormat format,VkImageTiling tiling,VkFormatFeatureFlags feature);
void CreateImage(VulkanTexture*texture, uint32_t w, uint32_t h, VkFormat f, 
	VkImageUsageFlags usage, VkSampleCountFlagBits sample_count= VK_SAMPLE_COUNT_1_BIT, int mipmaplevelcount=1);
void CreateImageCube(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
	VkImage& image, VkDeviceMemory& imageMemory, VkSampleCountFlagBits numSample = VK_SAMPLE_COUNT_1_BIT, int mipmap_level = 1);
VkImageView Create2DImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT, int mipmap_level = 1);
VkImageView CreateCubeMapImageView(VkImage image, VkFormat format, int mipmap_level = 1);
void SetImageLayout(VkCommandBuffer commandBuffer, VkImage image,VkImageLayout oldLayout,VkImageLayout newLayout,
	VkImageSubresourceRange subresourceRange,VkPipelineStageFlags srcStageMask=VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
void aGenTexture(void**texture);
void aTexImage2D(void*texture, VkFormat format, VkImageAspectFlags aspectFlags, int width, int height, const void * pixel);
void aTexCube(void*texture, VkFormat format, int width, int height, const void * pixel);
void aWaitForCommmandFinish(VkCommandBuffer commandbuffer);
uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
void aClearColor(float r, float g, float b, float a);
void aClearDepthStencil(float depth, uint32_t stencil);
VkCommandBuffer aBeginRendering(VkCommandBuffer commandbuffer=nullptr);
void aEndRenderingCommand();
void aSwapBuffers(VkCommandBuffer commandbuffer=nullptr);