#include "VulkanAPI.h"
#include "stb_image_aug.h"

#pragma comment(lib,"vulkan-1.lib")
//#define MSAA 1

static XTexture* sDefaultTexture = nullptr;
static VkCommandBuffer sMainCommandBuffer;

static XProgram* sCurrentProgram = nullptr;
static XBufferObject* sCurrentVBO = nullptr;
static XBufferObject* sCurrentIBO = nullptr;

//fbo
static float sClearColor[] = { 0.1f, 0.4f, 0.6f, 1.0f };
static float sClearDepth = 1.0f;
static uint32_t sClearStencil = 0.0f;
static VkRenderPass sRenderPass = 0;
static XTexture* sColorBuffer = nullptr;
static XTexture* sDepthBuffer = nullptr;
static int sFrameBufferCount = 0;
static int sViewportWidth = 0;
static int sViewportHeight = 0;
static XSystemFrameBuffer* sSystemFramebuffer = nullptr;

//command pool and semaphore
static VkCommandPool sCommandPool;
static VkSemaphore sReadyToRender, sReadyToPresent;

//swap chain
static VkSwapchainKHR sSwapChain;
static VkFormat sSwapChainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;
static VkExtent2D sSwapChainExtent;
static std::vector<VkImage> sSwapChainImages;
static std::vector<VkImageView> sSwapChainImageViews;

//logical device
static VkDevice sVulkanDevice;
static VkQueue sGraphicQueue, sPresentQueue;

//physical device
static VkPhysicalDevice sPhysicalDevice;
static VkSampleCountFlagBits sMaxSampleCount;
static int sGraphicQueueFamily, sPresentQueueFamily;

//vulkan engine
static void* sWindowHWND = nullptr;
static VkInstance sVulkanInstance;
static VkSurfaceKHR sVulkanSurface;
static VkDebugReportCallbackEXT sVulkanDebugger;
static PFN_vkCreateDebugReportCallbackEXT __vkCreateDebugReportCallback = nullptr;
static PFN_vkDestroyDebugReportCallbackEXT __vkDestroyDebugReportCallback = nullptr;
static PFN_vkCreateWin32SurfaceKHR __vkCreateWin32SurfaceKHR = nullptr;
static uint32_t sCurrentRenderFrameBufferIndex = 0;


XBufferObject::XBufferObject()
{
	buffer = 0;
	memory = 0;
}

XBufferObject::~XBufferObject()
{
	if (buffer != 0)
	{
		vkDestroyBuffer(sVulkanDevice, buffer, nullptr);
	}
	if (memory != 0)
	{
		vkFreeMemory(sVulkanDevice, memory, nullptr);
	}
}

void XBufferObject::OnSetSize()
{
	if (type == XBufferObjectTypeVertexBuffer)
	{
		xGenVertexBuffer(GetSize(), buffer, memory);
	}
	else if (type == XBufferObjectTypeIndexBuffer)
	{
		xGenIndexBuffer(GetSize(), buffer, memory);
	}
	else if (type == XBufferObjectTypeUniformBuffer)
	{
		xGenBuffer(buffer, memory, GetSize(),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	}
}

void XBufferObject::SubmitData(const void* data, int size)
{
	if (type == XBufferObjectTypeVertexBuffer)
	{
		xBufferSubVertexData(buffer, data, size);
	}
	else if (type == XBufferObjectTypeIndexBuffer)
	{
		xBufferSubIndexData(buffer, data, size);
	}
	else if (type == XBufferObjectTypeUniformBuffer)
	{
		void* dst;
		vkMapMemory(sVulkanDevice, memory, 0, size, 0, &dst);
		memcpy(dst, data, size);
		vkUnmapMemory(sVulkanDevice, memory);
	}
}

int XBufferObject::GetSize()
{
	return 0;
}

XUniformBuffer::XUniformBuffer()
{
	buffer = 0;
	memory = 0;
}

XUniformBuffer::~XUniformBuffer()
{
	if (buffer != 0)
	{
		vkDestroyBuffer(sVulkanDevice, buffer, nullptr);
	}
	if (memory != 0)
	{
		vkFreeMemory(sVulkanDevice, memory, nullptr);
	}
}

XProgram::XProgram()
{
	shaderStageCount = 0;
	vertexShader = 0;
	fragmentShader = 0;
	descriptorPool = 0;
	descriptorSetLayout = 0;
	descriptorSet = 0;
	memset(shaderStages, 0, sizeof(VkPipelineShaderStageCreateInfo) * 2);
}

XProgram::~XProgram()
{
	if (vertexShader != 0)
	{
		vkDestroyShaderModule(sVulkanDevice, vertexShader, nullptr);
	}
	if (fragmentShader != 0)
	{
		vkDestroyShaderModule(sVulkanDevice, fragmentShader, nullptr);
	}
	if (descriptorPool != 0)
	{
		vkDestroyDescriptorPool(sVulkanDevice, descriptorPool, nullptr);
	}
	if (descriptorSetLayout != 0)
	{
		vkDestroyDescriptorSetLayout(sVulkanDevice, descriptorSetLayout, nullptr);
	}

	for (int i = 0; i < writeDescriptorSet.size(); ++i)
	{
		VkWriteDescriptorSet* wds = &writeDescriptorSet[i];
		if (wds->pBufferInfo != nullptr)
		{
			delete wds->pBufferInfo;
		}
		if (wds->pImageInfo != nullptr)
		{
			delete wds->pImageInfo;
		}
	}
}

XTexture::XTexture(VkImageAspectFlags imageAspect)
{
	imageAspectFlags = imageAspect;
	image = 0;
	imageView = 0;
	memory = 0;
	sampler = 0;
	srcLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	dstLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	minFilter = VK_FILTER_LINEAR;
	magFilter = VK_FILTER_LINEAR;
	wrapU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	wrapV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	wrapW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	enableAnisotropy = VK_FALSE;
	maxAnisotropy = 0.0f;
}

XTexture::~XTexture()
{
	if (memory != 0)
	{
		vkFreeMemory(sVulkanDevice, memory, nullptr);
	}
	if (image != 0)
	{
		vkDestroyImageView(sVulkanDevice, imageView, nullptr);
	}
	if (image != 0)
	{
		vkDestroyImage(sVulkanDevice, image, nullptr);
	}
	if (sampler != 0)
	{
		vkDestroySampler(sVulkanDevice, sampler, nullptr);
	}
}

XSystemFrameBuffer::XSystemFrameBuffer()
{
	framebuffer = 0;
	colorbuffer = 0;
	depthbuffer = 0;
	resolvebuffer = 0;
	sampleCount = VK_SAMPLE_COUNT_1_BIT;
}

XSystemFrameBuffer::~XSystemFrameBuffer()
{
	if (framebuffer != 0)
	{
		vkDestroyFramebuffer(sVulkanDevice, framebuffer, nullptr);
	}
}

XFixedPipeline::XFixedPipeline()
{
	pipelineLayout = 0;
	pipeline = 0;
	inputAssetmlyState = {};
	inputAssetmlyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssetmlyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssetmlyState.primitiveRestartEnable = VK_FALSE;
	viewport = {};
	scissor = {};
	viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;
	rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_TRUE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;
	depthStencilState = {};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilState.depthBoundsTestEnable = VK_FALSE;
	depthStencilState.minDepthBounds = 0.0f;
	depthStencilState.maxDepthBounds = 1.0f;
	depthStencilState.stencilTestEnable = VK_FALSE;
	depthStencilState.front = {};
	depthStencilState.back = {};
	multisampleState = {};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.sampleShadingEnable = VK_TRUE;
	multisampleState.rasterizationSamples = xGetGlobalFrameBufferSampleCount();
	multisampleState.minSampleShading = 1.0f;
	multisampleState.pSampleMask = nullptr;
	multisampleState.alphaToCoverageEnable = VK_FALSE;
	multisampleState.alphaToOneEnable = VK_FALSE;
	colorBlendState = {};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.logicOp = VK_LOGIC_OP_COPY;
	descriptorSetLayout = nullptr;
	shaderStages = nullptr;
	shaderStageCount = 0;
	descriptorSetLayoutCount = 0;
	renderPass = 0;
	sampleCount = xGetGlobalFrameBufferSampleCount();
	pushConstantShaderStage = VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantCount = 8;
	depthConstantFactor = 0.0f;
	depthClamp = 0.0f;
	depthSlopeFactor = 0.0f;
}

XFixedPipeline::~XFixedPipeline()
{
	CleanUp();
}

void XFixedPipeline::CleanUp()
{
	if (pipeline != 0)
	{
		vkDestroyPipeline(sVulkanDevice, pipeline, nullptr);
	}
	if (pipelineLayout != 0)
	{
		vkDestroyPipelineLayout(sVulkanDevice, pipelineLayout, nullptr);
	}
}

void XVertexData::SetPosition(float x, float y, float z, float w)
{
	position[0] = x;
	position[1] = y;
	position[2] = z;
	position[3] = w;
}

void XVertexData::SetTexcoord(float x, float y, float z, float w)
{
	texcoord[0] = x;
	texcoord[1] = y;
	texcoord[2] = z;
	texcoord[3] = w;
}

void XVertexData::SetNormal(float x, float y, float z, float w)
{
	normal[0] = x;
	normal[1] = y;
	normal[2] = z;
	normal[3] = w;
}

void XVertexData::SetTangent(float x, float y, float z, float w)
{
	tangent[0] = x;
	tangent[1] = y;
	tangent[2] = z;
	tangent[3] = w;
}

const VkVertexInputBindingDescription& XVertexData::BindingDescription()
{
	static VkVertexInputBindingDescription binding_description = {};
	binding_description.binding = 0;
	binding_description.stride = sizeof(XVertexData);
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return binding_description;
}

const std::vector<VkVertexInputAttributeDescription>& XVertexData::AttributeDescriptions()
{
	static std::vector<VkVertexInputAttributeDescription> attributesDescriptions;
	attributesDescriptions.resize(4);
	attributesDescriptions[0].binding = 0;
	attributesDescriptions[0].location = 0;
	attributesDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributesDescriptions[0].offset = 0;

	attributesDescriptions[1].binding = 0;
	attributesDescriptions[1].location = 1;
	attributesDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributesDescriptions[1].offset = sizeof(float) * 4;

	attributesDescriptions[2].binding = 0;
	attributesDescriptions[2].location = 2;
	attributesDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributesDescriptions[2].offset = sizeof(float) * 8;

	attributesDescriptions[3].binding = 0;
	attributesDescriptions[3].location = 3;
	attributesDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributesDescriptions[3].offset = sizeof(float) * 12;
	return attributesDescriptions;
}

void xglBufferData(XVulkanHandle buffer, int size, void* data)
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
	VkResult result = vkCreateBuffer(sVulkanDevice, &bufferCreateInfo, nullptr, &buffer);
	if (result != VK_SUCCESS)
	{
		printf("failed to create buffer.");
		return result;
	}
	VkMemoryRequirements requirements;
	vkGetBufferMemoryRequirements(sVulkanDevice, buffer, &requirements);
	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = requirements.size;
	memoryAllocateInfo.memoryTypeIndex = xGetMemoryType(requirements.memoryTypeBits, properties);
	result = vkAllocateMemory(sVulkanDevice, &memoryAllocateInfo, nullptr, &memory);
	if (result != VK_SUCCESS)
	{
		printf("failed to allocate memory.");
		return result;
	}
	vkBindBufferMemory(sVulkanDevice, buffer, memory, 0);
	return result;
}

void xBufferSubData(VkBuffer buffer, VkBufferUsageFlags usage, const void* data, VkDeviceSize size)
{
	VkBuffer tempBuffer;
	VkDeviceMemory tempMemory;
	xGenBuffer(tempBuffer, tempMemory, size, usage, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	void* hostMemory;
	vkMapMemory(sVulkanDevice, tempMemory, 0, size, 0, &hostMemory);
	memcpy(hostMemory, data, (size_t)size);
	vkUnmapMemory(sVulkanDevice, tempMemory);

	VkCommandBuffer commandBuffer;
	xBeginOneTimeCommandBuffer(&commandBuffer);
	VkBufferCopy copy = { 0, 0, size };
	vkCmdCopyBuffer(commandBuffer, tempBuffer, buffer, 1, &copy);
	xEndOneTimeCommandBuffer(commandBuffer);

	vkDestroyBuffer(sVulkanDevice, tempBuffer, nullptr);
	vkFreeMemory(sVulkanDevice, tempMemory, nullptr);
}

uint32_t xGetMemoryType(uint32_t typeFilters, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(sPhysicalDevice, &memoryProperties);
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
	vkFreeCommandBuffers(sVulkanDevice, sCommandPool, 1, &commandBuffer);
}

void xGenCommandBuffer(VkCommandBuffer* commandBuffer, int count, VkCommandBufferLevel level)
{
	VkCommandBufferAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.level = level;
	info.commandPool = sCommandPool;
	info.commandBufferCount = count;
	vkAllocateCommandBuffers(sVulkanDevice, &info, commandBuffer);
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
	vkCreateFence(sVulkanDevice, &fenceInfo, nullptr, &fence);
	vkQueueSubmit(sGraphicQueue, 1, &info, fence);
	vkWaitForFences(sVulkanDevice, 1, &fence, VK_TRUE, 100000000);
	vkDestroyFence(sVulkanDevice, fence, nullptr);
}

void xCreateShader(VkShaderModule& shader, unsigned char* code, int codeLength)
{
	VkShaderModuleCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = codeLength;
	info.pCode = (uint32_t*)code;
	vkCreateShaderModule(sVulkanDevice, &info, nullptr, &shader);
}

void xAttachVertexShader(XProgram* program, VkShaderModule shader)
{
	program->shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	program->shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	program->shaderStages[0].module = shader;
	program->shaderStages[0].pName = "main";
	program->vertexShader = shader;
}

void xAttachFragmentShader(XProgram* program, VkShaderModule shader)
{
	program->shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	program->shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	program->shaderStages[1].module = shader;
	program->shaderStages[1].pName = "main";
	program->fragmentShader = shader;
}

void xLinkProgram(XProgram* program)
{
	//program->vertexShaderVectorUniformBuffer.type = XUniformBufferTypeVector;
	//program->vertexShaderVectorUniformBuffer.vectors.resize(8);
	//program->vertexShaderVectorUniformBuffer.vectors[0].data[0] = 1.0f;
	//program->vertexShaderVectorUniformBuffer.vectors[0].data[1] = 0.0f;
	//program->vertexShaderVectorUniformBuffer.vectors[0].data[2] = 0.0f;
	//program->vertexShaderVectorUniformBuffer.vectors[0].data[3] = 1.0f;
	//program->vertexShaderVectorUniformBuffer.vectors[1].data[0] = 0.0f;
	//program->vertexShaderVectorUniformBuffer.vectors[1].data[1] = 1.0f;
	//program->vertexShaderVectorUniformBuffer.vectors[1].data[2] = 0.0f;
	//program->vertexShaderVectorUniformBuffer.vectors[1].data[3] = 1.0f;
	//program->vertexShaderVectorUniformBuffer.vectors[2].data[0] = 0.0f;
	//program->vertexShaderVectorUniformBuffer.vectors[2].data[1] = 0.0f;
	//program->vertexShaderVectorUniformBuffer.vectors[2].data[2] = 1.0f;
	//program->vertexShaderVectorUniformBuffer.vectors[2].data[3] = 1.0f;

	//xGenBuffer(program->vertexShaderVectorUniformBuffer.buffer, program->vertexShaderVectorUniformBuffer.memory, sizeof(XVector4f) * 8,
	//	VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	//xSubmitUniformBuffer(&program->vertexShaderVectorUniformBuffer);
	//xConfigUniformBuffer(program, 0, &program->vertexShaderVectorUniformBuffer, VK_SHADER_STAGE_VERTEX_BIT);

	//program->vertexShaderMatrixUniformBuffer.type = XUniformBufferTypeMatrix;
	//program->vertexShaderMatrixUniformBuffer.matrices.resize(8);
	//glm::mat4 projection = glm::perspective(45.0f, float(GetViewportWidth()) / float(GetViewportHeight()), 0.1f, 60.0f);
	//projection[1][1] *= -1.0f;
	//memcpy(program->vertexShaderMatrixUniformBuffer.matrices[0].data, glm::value_ptr(projection), sizeof(XMatrix4x4f));

	//xGenBuffer(program->vertexShaderMatrixUniformBuffer.buffer, program->vertexShaderMatrixUniformBuffer.memory, sizeof(XMatrix4x4f) * 8,
	//	VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	//xSubmitUniformBuffer(&program->vertexShaderMatrixUniformBuffer);
	//xConfigUniformBuffer(program, 1, &program->vertexShaderMatrixUniformBuffer, VK_SHADER_STAGE_VERTEX_BIT);

	//xGenBuffer(program->fragmentShaderVectorUniformBuffer.buffer, program->fragmentShaderVectorUniformBuffer.memory, sizeof(XVector4f) * 8,
	//	VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	//program->fragmentShaderVectorUniformBuffer.type = XUniformBufferTypeVector;
	//program->fragmentShaderVectorUniformBuffer.vectors.resize(8);
	//xConfigUniformBuffer(program, 2, &program->fragmentShaderVectorUniformBuffer, VK_SHADER_STAGE_FRAGMENT_BIT);
	//xGenBuffer(program->fragmentShaderMatrixUniformBuffer.buffer, program->fragmentShaderMatrixUniformBuffer.memory, sizeof(XMatrix4x4f) * 8,
	//	VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	//program->fragmentShaderMatrixUniformBuffer.type = XUniformBufferTypeMatrix;
	//program->fragmentShaderMatrixUniformBuffer.matrices.resize(8);
	//xConfigUniformBuffer(program, 3, &program->fragmentShaderMatrixUniformBuffer, VK_SHADER_STAGE_FRAGMENT_BIT);

	//xConfigSampler2D(program, 4, sDefaultTexture->imageView, sDefaultTexture->sampler);

	////配置GPU程序

	////描述GPU程序有些什么输入的
	//xInitDescriptorSetLayout(program);
	////生成Uniform插槽的管理器
	//xInitDescriptorPool(program);
	////生成真正的插槽，这些插槽可以插入真正的Uniform buffer或者sampler2d
	//xInitDescriptorSet(program);

	//program->fixedPipeline.mDescriptorSetLayout = &program->descriptorSetLayout;
	//program->fixedPipeline.mDescriptorSetLayoutCount = 1;
	//program->fixedPipeline.mPushConstants[0].data[1] = 0.5f;

	////生成渲染管线

	////给渲染管线配置上生成的Uniform的信息
	//aSetDescriptorSetLayout(&program->fixedPipeline, &program->descriptorSetLayout);
	////设置渲染管线上有什么shader
	//aSetShaderStage(&program->fixedPipeline, program->shaderStages, 2);
	////设置渲染管线最终会渲染到多少张颜色缓冲区上去
	//aSetColorAttachmentCount(&program->fixedPipeline, 1);

	////xEnableBlend(&program->fixedPipeline, 0, VK_TRUE);
	////xBlend(&program->fixedPipeline, 0, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_SRC_ALPHA,
	////	VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE);
	////xPolygonMode(&program->fixedPipeline, VK_POLYGON_MODE_LINE);
	////xDisableRasterizer(&program->fixedPipeline, VK_TRUE);
	////xEnableDepthTest(&program->fixedPipeline, VK_TRUE);

	////设置render pass， 它描述了fbo上最终有多少个颜色缓冲区、是否有深度缓冲区、是否开启了MSAA
	//aSetRenderPass(&program->fixedPipeline, GetGlobalRenderPass());

	//program->fixedPipeline.mViewport = { 0.0f, 0.0f, float(GetViewportWidth()), float(GetViewportHeight()) };
	////绘图的可见区域
	//program->fixedPipeline.mScissor = { {0, 0}, {uint32_t(GetViewportWidth()), uint32_t(GetViewportHeight())} };
	//aCreateGraphicPipeline(&program->fixedPipeline);
}

void xInitDescriptorSetLayout(XProgram* program)
{
	VkDescriptorSetLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	//uniform buffer 和 uniform sampler的数量总和
	info.bindingCount = (uint32_t)program->descriptorSetLayoutBindings.size();
	info.pBindings = program->descriptorSetLayoutBindings.data();
	vkCreateDescriptorSetLayout(sVulkanDevice, &info, nullptr, &program->descriptorSetLayout);
}

void xInitDescriptorPool(XProgram* program)
{
	if (program->writeDescriptorSet.empty())
	{
		return;
	}
	VkDescriptorPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	info.pPoolSizes = program->descriptorPoolSize.data();
	info.poolSizeCount = uint32_t(program->descriptorPoolSize.size());
	info.maxSets = 1;
	vkCreateDescriptorPool(sVulkanDevice, &info, nullptr, &program->descriptorPool);
}

void xInitDescriptorSet(XProgram* program)
{
	if (program->writeDescriptorSet.empty())
	{
		return;
	}
	VkDescriptorSetAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	info.descriptorPool = program->descriptorPool;
	info.descriptorSetCount = 1;
	info.pSetLayouts = &program->descriptorSetLayout;
	vkAllocateDescriptorSets(sVulkanDevice, &info, &program->descriptorSet);
	for (int i = 0; i < program->writeDescriptorSet.size(); ++i)
	{
		program->writeDescriptorSet[i].dstSet = program->descriptorSet;
	}
	vkUpdateDescriptorSets(sVulkanDevice, uint32_t(program->writeDescriptorSet.size()), program->writeDescriptorSet.data(), 0, nullptr);
}

void xConfigUniformBuffer(XVulkanHandle param, int binding, XBufferObject* ubo, VkShaderStageFlags shaderStageFlags)
{
	XProgram* program = (XProgram*)param;
	VkDescriptorSetLayoutBinding bind = {};
	bind.binding = binding;
	bind.descriptorCount = 1;
	bind.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bind.stageFlags = shaderStageFlags;
	program->descriptorSetLayoutBindings.push_back(bind);
	VkDescriptorPoolSize poolSize = {};
	poolSize.descriptorCount = 1;
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	program->descriptorPoolSize.push_back(poolSize);

	VkDescriptorBufferInfo* bufferInfo = new VkDescriptorBufferInfo;
	bufferInfo->offset = 0;
	bufferInfo->buffer = ubo->buffer;
	bufferInfo->range = ubo->GetSize();
	VkWriteDescriptorSet set = {};
	set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	set.dstSet = program->descriptorSet;
	set.dstBinding = binding;
	set.dstArrayElement = 0;
	set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	set.descriptorCount = 1;
	set.pBufferInfo = bufferInfo;
	program->writeDescriptorSet.push_back(set);
}

void xGenImage(XTexture* texture, uint32_t w, uint32_t h, VkFormat format, VkImageUsageFlags usage,
	VkSampleCountFlagBits sampleCount, int mipmap)
{
	VkImageCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.imageType = VK_IMAGE_TYPE_2D;
	info.extent = { w, h, 1 };
	info.mipLevels = mipmap;
	info.arrayLayers = 1;
	info.format = format;
	info.initialLayout = texture->srcLayout;
	info.usage = usage;
	info.samples = sampleCount;
	if (vkCreateImage(sVulkanDevice, &info, nullptr, &texture->image) != VK_SUCCESS)
	{
		printf("failed to create image.");
	}
	VkMemoryRequirements requirements;
	vkGetImageMemoryRequirements(sVulkanDevice, texture->image, &requirements);
	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = requirements.size;
	allocateInfo.memoryTypeIndex = xGetMemoryType(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkAllocateMemory(sVulkanDevice, &allocateInfo, nullptr, &texture->memory);
	vkBindImageMemory(sVulkanDevice, texture->image, texture->memory, 0);
}

void xSubmitImage2D(XTexture* texture, int width, int height, const void* pixel)
{
	VkDeviceSize imageSize = width * height;
	if (texture->format == VK_FORMAT_R8G8B8A8_UNORM)
	{
		imageSize *= 4;
	}

	VkBuffer tempBuffer;
	VkDeviceMemory tempMemory;
	xGenBuffer(tempBuffer, tempMemory, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	void* hostMemory;
	vkMapMemory(sVulkanDevice, tempMemory, 0, imageSize, 0, &hostMemory);
	memcpy(hostMemory, pixel, (size_t)imageSize);
	vkUnmapMemory(sVulkanDevice, tempMemory);

	VkCommandBuffer commandBuffer;
	xBeginOneTimeCommandBuffer(&commandBuffer);
	VkImageSubresourceRange range = { texture->imageAspectFlags, 0, 1, 0, 1 };
	xSetImageLayout(commandBuffer, texture->image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, range);
	VkBufferImageCopy copy = {};
	copy.imageSubresource.aspectMask = texture->imageAspectFlags;
	copy.imageSubresource.mipLevel = 0;
	copy.imageSubresource.baseArrayLayer = 0;
	copy.imageSubresource.layerCount = 1;
	copy.imageOffset = { 0, 0, 0 };
	copy.imageExtent = { uint32_t(width), uint32_t(height), 1 };
	vkCmdCopyBufferToImage(commandBuffer, tempBuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
	xSetImageLayout(commandBuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, range);
	xEndOneTimeCommandBuffer(commandBuffer);

	vkDestroyBuffer(sVulkanDevice, tempBuffer, nullptr);
	vkFreeMemory(sVulkanDevice, tempMemory, nullptr);
}

void xInitSrcAccessMask(VkImageLayout oldLayout, VkImageMemoryBarrier& barrier)
{
	switch (oldLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		barrier.srcAccessMask = 0;
		break;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		printf("init src access mask : unprocessed %d\n", oldLayout);
		break;
	}
}

void xInitDstAccessMask(VkImageLayout newLayout, VkImageMemoryBarrier& barrier)
{
	switch (newLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | barrier.dstAccessMask;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		if (barrier.srcAccessMask == 0)
		{
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		printf("init dst access mask : unprocessed %d\n", newLayout);
		break;
	}
}

void xSetImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
	VkImageSubresourceRange range, VkPipelineStageFlags src, VkPipelineStageFlags dst)
{
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.image = image;
	barrier.subresourceRange = range;
	xInitSrcAccessMask(oldLayout, barrier);
	xInitDstAccessMask(newLayout, barrier);
	vkCmdPipelineBarrier(commandBuffer, src, dst, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void xGenImageView2D(XTexture* texture, int mipmap)
{
	VkImageViewCreateInfo ivci = {};
	ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivci.image = texture->image;
	ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ivci.format = texture->format;
	ivci.subresourceRange.aspectMask = texture->imageAspectFlags;
	ivci.subresourceRange.baseMipLevel = 0;
	ivci.subresourceRange.levelCount = mipmap;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount = 1;
	vkCreateImageView(sVulkanDevice, &ivci, nullptr, &texture->imageView);
}

void xGenSampler(XTexture* texture)
{
	VkSamplerCreateInfo samplercreateinfo = {};
	samplercreateinfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplercreateinfo.minFilter = texture->minFilter;
	samplercreateinfo.magFilter = texture->magFilter;
	samplercreateinfo.addressModeU = texture->wrapU;
	samplercreateinfo.addressModeV = texture->wrapV;
	samplercreateinfo.addressModeW = texture->wrapW;
	samplercreateinfo.anisotropyEnable = texture->enableAnisotropy;
	samplercreateinfo.maxAnisotropy = texture->maxAnisotropy;
	samplercreateinfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	vkCreateSampler(sVulkanDevice, &samplercreateinfo, nullptr, &texture->sampler);
}

void xInitDefaultTexture()
{
	sDefaultTexture = new XTexture;
	sDefaultTexture->format = VK_FORMAT_R8G8B8A8_UNORM;
	unsigned char* pixel = new unsigned char[256 * 256 * 4];
	memset(pixel, 255, sizeof(unsigned char) * 256 * 256 * 4);
	//初始化Image，不仅初始化Image逻辑对象，还要初始化物理内存
	xGenImage(sDefaultTexture, 256, 256, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
		VK_IMAGE_USAGE_SAMPLED_BIT);
	//把像素数据传给GPU
	xSubmitImage2D(sDefaultTexture, 256, 256, pixel);
	xGenImageView2D(sDefaultTexture);
	xGenSampler(sDefaultTexture);
	delete[] pixel;
}

void xConfigSampler2D(XProgram* program, int binding, VkImageView imageview, VkSampler sampler,
	VkImageLayout layout)
{
	VkDescriptorSetLayoutBinding layoutbinding = {};
	layoutbinding.binding = binding;
	layoutbinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutbinding.descriptorCount = 1;
	layoutbinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutbinding.pImmutableSamplers = nullptr;
	program->descriptorSetLayoutBindings.push_back(layoutbinding);
	VkDescriptorPoolSize poolsize = {};
	poolsize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolsize.descriptorCount = 1;
	program->descriptorPoolSize.push_back(poolsize);
	VkDescriptorImageInfo* imageinfo = new VkDescriptorImageInfo;
	imageinfo->imageLayout = layout;
	imageinfo->imageView = imageview;
	imageinfo->sampler = sampler;
	VkWriteDescriptorSet wds = {};
	wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	wds.dstSet = program->descriptorSet;
	wds.dstBinding = binding;
	wds.dstArrayElement = 0;
	wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	wds.descriptorCount = 1;
	wds.pImageInfo = imageinfo;
	program->writeDescriptorSet.push_back(wds);
}

void xUniform4fv(XProgram* program, int location, float* v)
{
	//memcpy(program->vertexShaderVectorUniformBuffer.vectors[location].data, v, sizeof(XVector4f));
	//xSubmitUniformBuffer(&program->vertexShaderVectorUniformBuffer);
}

XTexture* xGetDefaultTexture()
{
	return sDefaultTexture;
}

void xRebindUniformBuffer(XProgram* program, int binding, XUniformBuffer* ubo)
{
	VkDescriptorBufferInfo* bufferinfo = new VkDescriptorBufferInfo;
	bufferinfo->buffer = ubo->buffer;
	bufferinfo->offset = 0;
	if (ubo->type == XUniformBufferTypeMatrix)
	{
		bufferinfo->range = sizeof(XMatrix4x4f) * ubo->matrices.size();
	}
	else
	{
		bufferinfo->range = sizeof(XVector4f) * ubo->vectors.size();
	}
	delete program->writeDescriptorSet[binding].pBufferInfo;
	program->writeDescriptorSet[binding].pBufferInfo = bufferinfo;
	vkUpdateDescriptorSets(sVulkanDevice, uint32_t(program->writeDescriptorSet.size()),
		program->writeDescriptorSet.data(), 0, nullptr);
}

void xRebindSampler(XProgram* program, int binding, VkImageView iv, VkSampler s,
	VkImageLayout layout)
{
	VkDescriptorImageInfo* bufferinfo = new VkDescriptorImageInfo;
	bufferinfo->imageView = iv;
	bufferinfo->imageLayout = layout;
	bufferinfo->sampler = s;
	delete program->writeDescriptorSet[binding].pImageInfo;
	program->writeDescriptorSet[binding].pImageInfo = bufferinfo;
	vkUpdateDescriptorSets(sVulkanDevice, uint32_t(program->writeDescriptorSet.size()),
		program->writeDescriptorSet.data(), 0, nullptr);
}

void xUseProgram(XProgram* program)
{
	sCurrentProgram = program;
}

void xBindVertexBuffer(XBufferObject* vbo)
{
	sCurrentVBO = vbo;
}

void xBindElementBuffer(XBufferObject* ibo)
{
	sCurrentIBO = ibo;
}

void xDrawArrays(VkCommandBuffer commandbuffer, int offset, int count)
{
	xSetDynamicState(&sCurrentProgram->fixedPipeline, commandbuffer);
	VkBuffer vertexbuffers[] = { sCurrentVBO->buffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		sCurrentProgram->fixedPipeline.pipeline);
	vkCmdBindVertexBuffers(commandbuffer, 0, 1, vertexbuffers, offsets);
	if (sCurrentProgram->descriptorSet != 0)
	{
		vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			sCurrentProgram->fixedPipeline.pipelineLayout, 0, 1, &sCurrentProgram->descriptorSet,
			0, nullptr);
	}
	vkCmdDraw(commandbuffer, count, 1, offset, 0);
}

void xDrawElements(VkCommandBuffer commandbuffer, int offset, int count)
{
	xSetDynamicState(&sCurrentProgram->fixedPipeline, commandbuffer);
	VkBuffer vertexbuffers[] = { sCurrentVBO->buffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		sCurrentProgram->fixedPipeline.pipeline);
	vkCmdBindVertexBuffers(commandbuffer, 0, 1, vertexbuffers, offsets);
	vkCmdBindIndexBuffer(commandbuffer, sCurrentIBO->buffer, 0, VK_INDEX_TYPE_UINT32);
	if (sCurrentProgram->descriptorSet != 0)
	{
		vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			sCurrentProgram->fixedPipeline.pipelineLayout, 0, 1, &sCurrentProgram->descriptorSet,
			0, nullptr);
	}
	vkCmdDrawIndexed(commandbuffer, count, 1, offset, 0, 0);
}

VkCommandBuffer xBeginRendering(VkCommandBuffer commandbuffer)
{
	VkCommandBuffer cmd;
	if (commandbuffer != nullptr)
	{
		cmd = commandbuffer;
	}
	else
	{
		xBeginOneTimeCommandBuffer(&cmd);
	}
	VkFramebuffer render_target = xAquireRenderTarget();
	VkRenderPass render_pass = sRenderPass;
	VkClearValue clearvalues[2] = {};
	clearvalues[0].color = { 0.1f,0.4f,0.6f,1.0f };
	clearvalues[1].depthStencil = { 1.0f,0 };

	VkRenderPassBeginInfo rpbi = {};
	rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpbi.framebuffer = render_target;
	rpbi.renderPass = render_pass;
	rpbi.renderArea.offset = { 0,0 };
	rpbi.renderArea.extent = { uint32_t(sViewportWidth),uint32_t(sViewportHeight) };
	rpbi.clearValueCount = 2;
	rpbi.pClearValues = clearvalues;
	vkCmdBeginRenderPass(cmd, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
	sMainCommandBuffer = cmd;
	return cmd;
}

void xEndRendering()
{
	vkCmdEndRenderPass(sMainCommandBuffer);
	vkEndCommandBuffer(sMainCommandBuffer);
}

static void xSubmitDrawCommand(VkCommandBuffer commandbuffer)
{
	VkSemaphore ready_to_render[] = { sReadyToRender };
	VkSemaphore ready_to_present[] = { sReadyToPresent };
	VkSubmitInfo submitinfo = {};
	submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkPipelineStageFlags waitstages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitinfo.waitSemaphoreCount = 1;
	submitinfo.pWaitSemaphores = ready_to_render;
	submitinfo.pWaitDstStageMask = waitstages;
	submitinfo.pCommandBuffers = &commandbuffer;
	submitinfo.commandBufferCount = 1;
	submitinfo.signalSemaphoreCount = 1;
	submitinfo.pSignalSemaphores = ready_to_present;
	vkQueueSubmit(sGraphicQueue, 1, &submitinfo, VK_NULL_HANDLE);
}

static void PresentFrameBuffer()
{
	VkSemaphore ready_to_present[] = { sReadyToPresent };
	VkPresentInfoKHR presentinfo = {};
	uint32_t current_render_target_index = sCurrentRenderFrameBufferIndex;
	presentinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentinfo.pWaitSemaphores = ready_to_present;
	presentinfo.waitSemaphoreCount = 1;
	presentinfo.pSwapchains = &sSwapChain;
	presentinfo.swapchainCount = 1;
	presentinfo.pImageIndices = &current_render_target_index;
	vkQueuePresentKHR(sPresentQueue, &presentinfo);
	vkQueueWaitIdle(sPresentQueue);
}

void xSwapBuffers(VkCommandBuffer commandbuffer)
{
	VkCommandBuffer cmd;
	if (commandbuffer == nullptr)
	{
		cmd = sMainCommandBuffer;
	}
	else
	{
		cmd = commandbuffer;
	}
	xSubmitDrawCommand(cmd);
	PresentFrameBuffer();
	vkFreeCommandBuffers(sVulkanDevice, sCommandPool, 1, &cmd);
	sMainCommandBuffer = nullptr;
}

void xSetColorAttachmentCount(XFixedPipeline* pipeline, int count)
{
	pipeline->colorBlendAttachmentStates.resize(count);
	for (int i = 0; i < count; ++i)
	{
		pipeline->colorBlendAttachmentStates[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		pipeline->colorBlendAttachmentStates[i].blendEnable = VK_FALSE;
		pipeline->colorBlendAttachmentStates[i].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		pipeline->colorBlendAttachmentStates[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		pipeline->colorBlendAttachmentStates[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		pipeline->colorBlendAttachmentStates[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		pipeline->colorBlendAttachmentStates[i].colorBlendOp = VK_BLEND_OP_ADD;
		pipeline->colorBlendAttachmentStates[i].alphaBlendOp = VK_BLEND_OP_ADD;
	}
}

void xEnableBlend(XFixedPipeline* pipeline, int attachment, VkBool32 enable)
{
	pipeline->colorBlendAttachmentStates[attachment].blendEnable = enable;
}

void xBlend(XFixedPipeline* p, int attachment, VkBlendFactor s_c, VkBlendFactor s_a,
	VkBlendFactor d_c, VkBlendFactor d_a)
{
	p->colorBlendAttachmentStates[attachment].srcColorBlendFactor = s_c;
	p->colorBlendAttachmentStates[attachment].srcAlphaBlendFactor = s_a;
	p->colorBlendAttachmentStates[attachment].dstColorBlendFactor = d_c;
	p->colorBlendAttachmentStates[attachment].dstAlphaBlendFactor = d_a;
}

void xBlendOp(XFixedPipeline* p, int attachment, VkBlendOp color, VkBlendOp alpha)
{
	p->colorBlendAttachmentStates[attachment].colorBlendOp = color;
	p->colorBlendAttachmentStates[attachment].alphaBlendOp = alpha;
}

void xPolygonMode(XFixedPipeline* p, VkPolygonMode mode)
{
	p->rasterizer.polygonMode = mode;
}

void xDisableRasterizer(XFixedPipeline* p, VkBool32 disable)
{
	p->rasterizer.rasterizerDiscardEnable = disable;
}

void xEnableDepthTest(XFixedPipeline* p, VkBool32 enable)
{
	p->depthStencilState.depthTestEnable = enable;
}

void xInitPipelineLayout(XFixedPipeline* p)
{
	VkPushConstantRange pushconstancrange = {};
	pushconstancrange.stageFlags = p->pushConstantShaderStage;
	pushconstancrange.offset = 0;
	pushconstancrange.size = sizeof(XVector4f) * p->pushConstantCount;
	VkPipelineLayoutCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	ci.pSetLayouts = p->descriptorSetLayout;
	ci.setLayoutCount = p->descriptorSetLayoutCount;
	ci.pPushConstantRanges = &pushconstancrange;
	ci.pushConstantRangeCount = 1;
	vkCreatePipelineLayout(sVulkanDevice, &ci, nullptr, &p->pipelineLayout);
}

void xCreateFixedPipeline(XFixedPipeline* p)
{
	const auto& bindingdescriptions = XVertexData::BindingDescription();
	const auto& attributeDescriptions = XVertexData::AttributeDescriptions();
	VkPipelineVertexInputStateCreateInfo vertexinputinfo = {};
	vertexinputinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexinputinfo.vertexBindingDescriptionCount = 1;
	vertexinputinfo.pVertexBindingDescriptions = &bindingdescriptions;
	vertexinputinfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
	vertexinputinfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	p->colorBlendState.attachmentCount = p->colorBlendAttachmentStates.size();
	p->colorBlendState.pAttachments = p->colorBlendAttachmentStates.data();
	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_DEPTH_BIAS,
		VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT
	};
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 4;
	dynamicState.pDynamicStates = dynamicStates;
	VkGraphicsPipelineCreateInfo pipelineinfo = {};
	pipelineinfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineinfo.stageCount = p->shaderStageCount;
	pipelineinfo.pStages = p->shaderStages;
	pipelineinfo.pVertexInputState = &vertexinputinfo;
	pipelineinfo.pInputAssemblyState = &p->inputAssetmlyState;
	pipelineinfo.pViewportState = &p->viewportState;
	pipelineinfo.pRasterizationState = &p->rasterizer;
	pipelineinfo.pMultisampleState = &p->multisampleState;
	pipelineinfo.pDepthStencilState = &p->depthStencilState;
	pipelineinfo.pColorBlendState = &p->colorBlendState;
	pipelineinfo.pDynamicState = &dynamicState;
	pipelineinfo.layout = p->pipelineLayout;
	pipelineinfo.renderPass = p->renderPass;
	pipelineinfo.subpass = 0;
	pipelineinfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineinfo.basePipelineIndex = -1;
	vkCreateGraphicsPipelines(sVulkanDevice, VK_NULL_HANDLE, 1, &pipelineinfo, nullptr,
		&p->pipeline);
}

void xSetDynamicState(XFixedPipeline* p, VkCommandBuffer commandbuffer)
{
	vkCmdSetViewport(commandbuffer, 0, 1, &p->viewport);
	vkCmdSetScissor(commandbuffer, 0, 1, &p->scissor);
	vkCmdSetDepthBias(commandbuffer, p->depthConstantFactor, p->depthClamp, p->depthSlopeFactor);
	vkCmdPushConstants(commandbuffer, p->pipelineLayout, p->pushConstantShaderStage, 0,
		sizeof(XVector4f) * p->pushConstantCount, p->pushConstants);
}

void xGenImageCube(XTexture* texture, uint32_t w, uint32_t h, VkFormat f,
	VkImageUsageFlags usage, VkSampleCountFlagBits sample_count, int mipmap)
{
	VkImageCreateInfo ici = {};
	ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ici.imageType = VK_IMAGE_TYPE_2D;
	ici.extent = { w,h,1 };
	ici.mipLevels = mipmap;
	ici.arrayLayers = 6;
	ici.format = f;
	ici.initialLayout = texture->srcLayout;
	ici.usage = usage;
	ici.samples = sample_count;
	ici.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	if (vkCreateImage(sVulkanDevice, &ici, nullptr, &texture->image) != VK_SUCCESS) {
		printf("failed to create image\n");
	}
	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(sVulkanDevice, texture->image, &memory_requirements);
	VkMemoryAllocateInfo mai = {};
	mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mai.allocationSize = memory_requirements.size;
	mai.memoryTypeIndex = xGetMemoryType(memory_requirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkAllocateMemory(sVulkanDevice, &mai, nullptr, &texture->memory);
	vkBindImageMemory(sVulkanDevice, texture->image, texture->memory, 0);
}

void xSubmitImageCube(XTexture* texture, int width, int height, const void* pixel)
{
	VkDeviceSize offset_unit = width * height;
	if (texture->format == VK_FORMAT_R8G8B8A8_UNORM)
	{
		offset_unit *= 4;
	}
	int imagesize = offset_unit * 6;
	VkBuffer tempbuffer;
	VkDeviceMemory tempmemory;
	xGenBuffer(tempbuffer, tempmemory, imagesize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	void* data;
	vkMapMemory(sVulkanDevice, tempmemory, 0, imagesize, 0, &data);
	memcpy(data, pixel, imagesize);
	vkUnmapMemory(sVulkanDevice, tempmemory);

	std::vector<VkBufferImageCopy> copies;
	for (uint32_t face = 0; face < 6; ++face)
	{
		VkBufferImageCopy copy = {};
		copy.imageSubresource.aspectMask = texture->imageAspectFlags;
		copy.imageSubresource.mipLevel = 0;
		copy.imageSubresource.baseArrayLayer = face;
		copy.imageSubresource.layerCount = 1;

		copy.imageOffset = { 0,0,0 };
		copy.imageExtent = { uint32_t(width),uint32_t(height),1 };
		copy.bufferOffset = offset_unit * face;
		copies.push_back(copy);
	}
	VkCommandBuffer commandbuffer;
	xBeginOneTimeCommandBuffer(&commandbuffer);
	VkImageSubresourceRange subresourcerange = { texture->imageAspectFlags,0,1,0,6 };
	xSetImageLayout(commandbuffer, texture->image, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourcerange);

	vkCmdCopyBufferToImage(commandbuffer, tempbuffer, texture->image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6, copies.data());
	xSetImageLayout(commandbuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourcerange);
	xEndOneTimeCommandBuffer(commandbuffer);

	vkDestroyBuffer(sVulkanDevice, tempbuffer, nullptr);
	vkFreeMemory(sVulkanDevice, tempmemory, nullptr);
}

void xGenImageViewCube(XTexture* texture, int mipmap)
{
	VkImageViewCreateInfo ivci = {};
	ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivci.image = texture->image;
	ivci.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	ivci.format = texture->format;
	ivci.subresourceRange.aspectMask = texture->imageAspectFlags;
	ivci.subresourceRange.baseMipLevel = 0;
	ivci.subresourceRange.levelCount = mipmap;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount = 6;
	ivci.components = { VK_COMPONENT_SWIZZLE_R,VK_COMPONENT_SWIZZLE_G,VK_COMPONENT_SWIZZLE_B,
		VK_COMPONENT_SWIZZLE_A };
	vkCreateImageView(sVulkanDevice, &ivci, nullptr, &texture->imageView);
}

void xVulkanCleanUp()
{
	if (sDefaultTexture != nullptr)
	{
		delete sDefaultTexture;
	}
	delete sColorBuffer;
	delete sDepthBuffer;
	delete[] sSystemFramebuffer;
	for (int i = 0; i < sFrameBufferCount; ++i) {
		vkDestroyImageView(sVulkanDevice, sSwapChainImageViews[i], nullptr);
	}
	vkDestroySwapchainKHR(sVulkanDevice, sSwapChain, nullptr);
	vkDestroyRenderPass(sVulkanDevice, sRenderPass, nullptr);
	vkDestroySemaphore(sVulkanDevice, sReadyToRender, nullptr);
	vkDestroySemaphore(sVulkanDevice, sReadyToPresent, nullptr);
	vkDestroyCommandPool(sVulkanDevice, sCommandPool, nullptr);
	vkDestroyDevice(sVulkanDevice, nullptr);
}

static void xInitGlobalRenderPass()
{
	VkSampleCountFlagBits sample_count = xGetGlobalFrameBufferSampleCount();
	VkAttachmentDescription colorAttachment = {
		0,
		sSwapChainImageFormat,
		sample_count,
		VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_ATTACHMENT_STORE_OP_STORE,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VK_ATTACHMENT_STORE_OP_DONT_CARE,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};
	//MSAA的情况下，color buffer不需要显示
	if (sample_count == VK_SAMPLE_COUNT_1_BIT)
	{
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}
	VkAttachmentDescription depthAttachment = {
		0,
		VK_FORMAT_D24_UNORM_S8_UINT,
		sample_count,
		VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_ATTACHMENT_STORE_OP_STORE,
		VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_ATTACHMENT_STORE_OP_DONT_CARE,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};
	VkAttachmentDescription colorAttachmentResolve = {
		0,
		sSwapChainImageFormat,
		VK_SAMPLE_COUNT_1_BIT,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VK_ATTACHMENT_STORE_OP_STORE,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VK_ATTACHMENT_STORE_OP_DONT_CARE,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};
	VkAttachmentReference colorAttachmentRef = { 0,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	VkAttachmentReference depthAttachmentRef = { 1,VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
	VkAttachmentReference colorResolveRef = { 2,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	VkAttachmentDescription attachments[3];
	int attachmentcount = 2;
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	if (sample_count == VK_SAMPLE_COUNT_1_BIT)
	{
		memcpy(&attachments[0], &colorAttachment, sizeof(VkAttachmentDescription));
		memcpy(&attachments[1], &depthAttachment, sizeof(VkAttachmentDescription));
	}
	else
	{
		subpass.pResolveAttachments = &colorResolveRef;
		attachmentcount = 3;
		memcpy(&attachments[0], &colorAttachment, sizeof(VkAttachmentDescription));
		memcpy(&attachments[1], &depthAttachment, sizeof(VkAttachmentDescription));
		memcpy(&attachments[2], &colorAttachmentResolve, sizeof(VkAttachmentDescription));
	}
	VkRenderPassCreateInfo cpci = {};
	cpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	cpci.attachmentCount = attachmentcount;
	cpci.pAttachments = attachments;
	cpci.subpassCount = 1;
	cpci.pSubpasses = &subpass;
	vkCreateRenderPass(sVulkanDevice, &cpci, nullptr, &sRenderPass);
}

static void xInitSystemColorBuffer()
{
	sColorBuffer = new XTexture;
	sColorBuffer->format = sSwapChainImageFormat;
	xGenImage(sColorBuffer, sViewportWidth, sViewportHeight, sColorBuffer->format,
		VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		xGetGlobalFrameBufferSampleCount());
	xGenImageView2D(sColorBuffer);
}

static void xInitSystemDepthBuffer()
{
	sDepthBuffer = new XTexture(VK_IMAGE_ASPECT_DEPTH_BIT);
	sDepthBuffer->format = VK_FORMAT_D24_UNORM_S8_UINT;
	xGenImage(sDepthBuffer, sViewportWidth, sViewportHeight, sDepthBuffer->format,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		xGetGlobalFrameBufferSampleCount());
	xGenImageView2D(sDepthBuffer);
}

static void xFrameBufferFinish(XSystemFrameBuffer* fbo)
{
	VkImageView render_targets[3];
	int render_target_count = 2;
	if (fbo->sampleCount != VK_SAMPLE_COUNT_1_BIT)
	{
		render_target_count = 3;
		render_targets[0] = fbo->colorbuffer;
		render_targets[1] = fbo->depthbuffer;
		render_targets[2] = fbo->resolvebuffer;
	}
	else
	{
		render_targets[0] = fbo->resolvebuffer;
		render_targets[1] = fbo->depthbuffer;
	}
	VkFramebufferCreateInfo fbci = {};
	fbci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbci.renderPass = sRenderPass;
	fbci.pAttachments = render_targets;
	fbci.attachmentCount = render_target_count;
	fbci.width = sViewportWidth;
	fbci.height = sViewportHeight;
	fbci.layers = 1;
	vkCreateFramebuffer(sVulkanDevice, &fbci, nullptr, &fbo->framebuffer);
}

void xInitSystemFrameBuffer()
{
	if (sRenderPass == 0)
	{
		xInitGlobalRenderPass();
	}
	xInitSystemColorBuffer();
	xInitSystemDepthBuffer();
	if (sSystemFramebuffer == nullptr)
	{
		sSystemFramebuffer = new XSystemFrameBuffer[sFrameBufferCount];
	}
	VkSampleCountFlagBits sample_count = xGetGlobalFrameBufferSampleCount();

	for (int i = 0; i < sFrameBufferCount; ++i)
	{
		sSystemFramebuffer[i].sampleCount = sample_count;
		sSystemFramebuffer[i].depthbuffer = sDepthBuffer->imageView;
		sSystemFramebuffer[i].resolvebuffer = sSwapChainImageViews[i];
		if (sample_count != VK_SAMPLE_COUNT_1_BIT)
		{
			sSystemFramebuffer[i].colorbuffer = sColorBuffer->imageView;
		}
		xFrameBufferFinish(&sSystemFramebuffer[i]);
	}
}

XVulkanHandle xGetSystemFrameBuffer(int index)
{
	return &sSystemFramebuffer[index];
}

int xGetSystemFrameBufferCount()
{
	return sFrameBufferCount;
}

void xViewport(int width, int height)
{
	//先销毁之前的buffers
	//生成新的framebuffer
	if (sViewportWidth != width || sViewportHeight != height)
	{
		sViewportWidth = width;
		sViewportHeight = height;
		if (sDepthBuffer != nullptr)
		{
			delete sDepthBuffer;
		}
		if (sColorBuffer != nullptr)
		{
			delete sColorBuffer;
		}
		for (int i = 0; i < sFrameBufferCount; ++i)
		{
			vkDestroyFramebuffer(sVulkanDevice, sSystemFramebuffer[i].framebuffer, nullptr);
			vkDestroyImageView(sVulkanDevice, sSwapChainImageViews[i], nullptr);
		}
		vkDestroySwapchainKHR(sVulkanDevice, sSwapChain, nullptr);
		xInitSwapChain();
		xInitSystemFrameBuffer();
	}
}

VkRenderPass xGetGlobalRenderPass()
{
	return sRenderPass;
}

void xInitCommandPool()
{
	VkCommandPoolCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	ci.queueFamilyIndex = sGraphicQueueFamily;
	vkCreateCommandPool(sVulkanDevice, &ci, nullptr, &sCommandPool);
}

VkCommandPool xGetCommandPool()
{
	return sCommandPool;
}

void xInitSemaphores()
{
	VkSemaphoreCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkCreateSemaphore(sVulkanDevice, &ci, nullptr, &sReadyToRender);
	vkCreateSemaphore(sVulkanDevice, &ci, nullptr, &sReadyToPresent);
}

VkSemaphore xGetReadyToRenderSemaphore()
{
	return sReadyToRender;
}

VkSemaphore xGetReadyToPresentSemaphore()
{
	return sReadyToPresent;
}

void xInitSwapChain()
{
	uint32_t image_count = 2;//double buffer
	VkSwapchainCreateInfoKHR ci = {};
	ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	ci.surface = sVulkanSurface;
	ci.minImageCount = image_count;
	ci.imageFormat = sSwapChainImageFormat;
	ci.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	ci.imageExtent = { uint32_t(sViewportWidth),uint32_t(sViewportHeight) };
	ci.imageArrayLayers = 1;
	ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	uint32_t queuefamilyindices[] = { uint32_t(sGraphicQueueFamily),uint32_t(sPresentQueueFamily) };
	if (sGraphicQueueFamily != sPresentQueueFamily)
	{
		ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		ci.pQueueFamilyIndices = queuefamilyindices;
		ci.queueFamilyIndexCount = 2;
	}
	else
	{
		ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(sPhysicalDevice, sVulkanSurface, &capabilities);
	ci.preTransform = capabilities.currentTransform;
	ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	ci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	ci.clipped = VK_TRUE;
	ci.oldSwapchain = VK_NULL_HANDLE;
	vkCreateSwapchainKHR(sVulkanDevice, &ci, nullptr, &sSwapChain);
	vkGetSwapchainImagesKHR(sVulkanDevice, sSwapChain, &image_count, nullptr);
	sSwapChainImages.resize(image_count);
	sFrameBufferCount = image_count;
	vkGetSwapchainImagesKHR(sVulkanDevice, sSwapChain, &image_count, sSwapChainImages.data());
	sSwapChainExtent = { uint32_t(sViewportWidth),uint32_t(sViewportHeight) };
	sSwapChainImageViews.resize(image_count);
	for (int i = 0; i < image_count; ++i)
	{
		VkImageViewCreateInfo ivci = {};
		ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ivci.image = sSwapChainImages[i];
		ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ivci.format = sSwapChainImageFormat;
		ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ivci.subresourceRange.baseMipLevel = 0;
		ivci.subresourceRange.levelCount = 1;
		ivci.subresourceRange.baseArrayLayer = 0;
		ivci.subresourceRange.layerCount = 1;
		vkCreateImageView(sVulkanDevice, &ivci, nullptr, &sSwapChainImageViews[i]);
	}
}

VkSwapchainKHR xGetSwapChain()
{
	return sSwapChain;
}

VkFormat xGetSwapChainImageFormat()
{
	return sSwapChainImageFormat;
}

void xInitVulkanDevice()
{
	VkDeviceQueueCreateInfo queue_create_info[2];
	int queue_count = 2;
	float priority = 1.0f;
	queue_create_info[0] = {};
	queue_create_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info[0].queueCount = 1;
	queue_create_info[0].queueFamilyIndex = sGraphicQueueFamily;
	queue_create_info[0].pQueuePriorities = &priority;
	queue_create_info[1] = {};
	queue_create_info[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info[1].queueCount = 1;
	queue_create_info[1].queueFamilyIndex = sPresentQueueFamily;
	queue_create_info[1].pQueuePriorities = &priority;
	if (sGraphicQueueFamily == sPresentQueueFamily)
	{
		queue_count = 1;
	}
	const char* extensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	//物理特性，取决于GPU电脑是否支持
	VkPhysicalDeviceFeatures features = {};
	features.samplerAnisotropy = VK_TRUE;
	features.geometryShader = VK_TRUE;
	features.tessellationShader = VK_TRUE;
	features.fillModeNonSolid = VK_TRUE;
	features.sampleRateShading = VK_TRUE;

	const char* layers[] = {
		"VK_LAYER_LUNARG_standard_validation"
	};

	VkDeviceCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	ci.pQueueCreateInfos = queue_create_info;
	ci.queueCreateInfoCount = queue_count;
	ci.enabledExtensionCount = 1;
	//使用swap chain
	ci.ppEnabledExtensionNames = extensions;
	ci.enabledLayerCount = 1;
	//高版本Vulkan已忽略该参数
	ci.ppEnabledLayerNames = layers;
	ci.pEnabledFeatures = &features;
	vkCreateDevice(sPhysicalDevice, &ci, nullptr, &sVulkanDevice);
	vkGetDeviceQueue(sVulkanDevice, sGraphicQueueFamily, 0, &sGraphicQueue);
	vkGetDeviceQueue(sVulkanDevice, sPresentQueueFamily, 0, &sPresentQueue);
}

VkDevice xGetVulkanDevice()
{
	return sVulkanDevice;
}

VkQueue xGetGraphicQueue()
{
	return sGraphicQueue;
}

VkQueue xGetPresentQueue()
{
	return sPresentQueue;
}

int xGetGraphicQueueFamily()
{
	return sGraphicQueueFamily;
}

int xGetPresentQueueFamily()
{
	return sPresentQueueFamily;
}

VkPhysicalDevice xGetVulkanPhysicalDevice()
{
	return sPhysicalDevice;
}

VkSampleCountFlagBits xGetMaxMSAASampleCount()
{
	VkPhysicalDeviceProperties physicalproperties;
	vkGetPhysicalDeviceProperties(sPhysicalDevice, &physicalproperties);
	//取颜色缓冲区和深度缓冲区支持采样数的最小值
	VkSampleCountFlags count = physicalproperties.limits.framebufferColorSampleCounts >
		physicalproperties.limits.framebufferDepthSampleCounts ?
		physicalproperties.limits.framebufferDepthSampleCounts :
		physicalproperties.limits.framebufferColorSampleCounts;
	if (count & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (count & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
	if (count & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
	if (count & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	if (count & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (count & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
	return VK_SAMPLE_COUNT_1_BIT;
}

void xInitVulkanPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(sVulkanInstance, &deviceCount, nullptr);
	VkPhysicalDevice* devices = new VkPhysicalDevice[deviceCount];
	vkEnumeratePhysicalDevices(sVulkanInstance, &deviceCount, devices);
	for (uint32_t i = 0; i < deviceCount; ++i)
	{
		VkPhysicalDevice* current_device = &devices[i];
		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceProperties(*current_device, &properties);
		vkGetPhysicalDeviceFeatures(*current_device, &features);
		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(*current_device, &queue_family_count, nullptr);
		VkQueueFamilyProperties* queuefamilyproperties = new VkQueueFamilyProperties[queue_family_count];
		vkGetPhysicalDeviceQueueFamilyProperties(*current_device, &queue_family_count, queuefamilyproperties);
		sPresentQueueFamily = -1;
		sGraphicQueueFamily = -1;
		for (uint32_t j = 0; j < queue_family_count; ++j)
		{
			VkBool32 present_support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(*current_device, j, sVulkanSurface, &present_support);
			if (queuefamilyproperties[j].queueCount > 0 && present_support)
			{
				sPresentQueueFamily = j;
			}
			if (queuefamilyproperties[j].queueCount > 0 &&
				queuefamilyproperties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				sGraphicQueueFamily = j;
			}
			if (sGraphicQueueFamily != -1 && sPresentQueueFamily != -1)
			{
				sPhysicalDevice = *current_device;
				sMaxSampleCount = xGetMaxMSAASampleCount();
				return;
			}
		}
		delete[] queuefamilyproperties;
	}
	delete[]devices;
}

static void InitVulkanInstance()
{
	VkApplicationInfo appinfo = {};
	appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appinfo.pApplicationName = "DrbRenderingEngine";
	appinfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appinfo.pEngineName = "DrbRenderingEngine";
	appinfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appinfo.apiVersion = VK_API_VERSION_1_2;

	const char* extensions[] = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
		//VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
	};
	const char* layers[] = {
		"VK_LAYER_LUNARG_standard_validation"
	};

	VkInstanceCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	ci.pApplicationInfo = &appinfo;
	ci.enabledExtensionCount = 3;
	ci.ppEnabledExtensionNames = extensions;
	ci.enabledLayerCount = 1;
	ci.ppEnabledLayerNames = layers;
	vkCreateInstance(&ci, nullptr, &sVulkanInstance);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
	VkDebugReportFlagsEXT                       flags,
	VkDebugReportObjectTypeEXT                  objectType,
	uint64_t                                    object,
	size_t                                      location,
	int32_t                                     messageCode,
	const char* pLayerPrefix,
	const char* pMessage,
	void* pUserData
) 
{
	printf("validation layer : %s\n", pMessage);
	return VK_FALSE;
}

static void InitDebugger()
{
	__vkCreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
		sVulkanInstance,
		"vkCreateDebugReportCallbackEXT");
	__vkDestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
		sVulkanInstance,
		"vkDestroyDebugReportCallback");
	__vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(
		sVulkanInstance,
		"vkCreateWin32SurfaceKHR");
	VkDebugReportCallbackCreateInfoEXT ci = {};
	ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	ci.flags = 
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | 
		VK_DEBUG_REPORT_ERROR_BIT_EXT | 
		VK_DEBUG_REPORT_DEBUG_BIT_EXT;
	ci.pfnCallback = debug_callback;
	__vkCreateDebugReportCallback(sVulkanInstance, &ci, nullptr, &sVulkanDebugger);
}

static void InitSurface()
{
	VkWin32SurfaceCreateInfoKHR ci = {};
	ci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	ci.hinstance = GetModuleHandle(NULL);
	ci.hwnd = (HWND)sWindowHWND;
	__vkCreateWin32SurfaceKHR(sVulkanInstance, &ci, nullptr, &sVulkanSurface);
}

void xInitVulkan(void* param, int width, int height)
{
	sWindowHWND = param;
	sViewportWidth = width;
	sViewportHeight = height;

	InitVulkanInstance();
	InitDebugger();
	InitSurface();
	xInitVulkanPhysicalDevice();
	xInitVulkanDevice();
	xInitSwapChain();
	xInitSystemFrameBuffer();
	xInitCommandPool();
	xInitSemaphores();
	xInitDefaultTexture();
}

VkSampleCountFlagBits xGetGlobalFrameBufferSampleCount()
{
#if MSAA
	return sMaxSampleCount;
#else
	return VK_SAMPLE_COUNT_1_BIT;
#endif
}

int xGetViewportWidth()
{
	return sViewportWidth;
}

int xGetViewportHeight()
{
	return sViewportHeight;
}

VkFramebuffer xAquireRenderTarget()
{
	vkAcquireNextImageKHR(sVulkanDevice, sSwapChain, 1000000000, sReadyToRender, VK_NULL_HANDLE,
		&sCurrentRenderFrameBufferIndex);
	return sSystemFramebuffer[sCurrentRenderFrameBufferIndex].framebuffer;
}

uint32_t xGetCurrenRenderTargetIndex()
{
	return sCurrentRenderFrameBufferIndex;
}

unsigned char* LoadFileContent(const char* path, int& filesize)
{
	unsigned char* fileContent = nullptr;
	filesize = 0;
	FILE* pFile = fopen(path, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		int nLen = ftell(pFile);
		if (nLen > 0)
		{
			rewind(pFile);
			fileContent = new unsigned char[nLen + 1];
			fread(fileContent, sizeof(unsigned char), nLen, pFile);
			fileContent[nLen] = '\0';
			filesize = nLen;
		}
		fclose(pFile);
	}
	return fileContent;
}

char* LoadFileContent(const char* path)
{
	FILE* pFile;
	errno_t err = fopen_s(&pFile, path, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		int nLen = ftell(pFile);
		char* buffer = nullptr;
		if (nLen != 0)
		{
			char* buffer = new char[nLen + 1];
			rewind(pFile);
			fread(buffer, nLen, 1, pFile);
			buffer[nLen] = '\0';
			fclose(pFile);
			return buffer;
		}
		else
		{
			printf("load file %s fail. content length is 0.\n", path);
		}
	}
	else
	{
		printf("open file %s fail.\n", path);
	}
	fclose(pFile);
	return nullptr;
}

unsigned char* LoadImageFromFile(const char* path, int& width, int& height, int& channel,
	int force_channel, bool flipY)
{
	unsigned char* result = stbi_load(path, &width, &height, &channel, force_channel);
	if (result == nullptr)
	{
		return nullptr;
	}
	if (false == flipY)
	{
		for (int j = 0; j * 2 < height; ++j)
		{
			int index1 = j * width * channel;
			int index2 = (height - 1 - j) * width * channel;
			for (int i = width * channel; i > 0; --i)
			{
				unsigned char temp = result[index1];
				result[index1] = result[index2];
				result[index2] = temp;
				++index1;
				++index2;
			}
		}
	}
	return result;
}