#include "BVulkan.h"
#include "VulkanAPI.h"
#include "stb_image_aug.h"

/// <summary>
/// 缺省纹理贴图
/// </summary>
static XTexture* sDefaultTexture = nullptr;
static XProgram* sCurrentProgram = nullptr;
static XBufferObject* sCurrentVBO = nullptr;
static XBufferObject* sCurrentIBO = nullptr;
static VkCommandBuffer sMainCommandBuffer;

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
		vkMapMemory(GetVulkanDevice(), memory, 0, size, 0, &dst);
		memcpy(dst, data, size);
		vkUnmapMemory(GetVulkanDevice(), memory);
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
		vkDestroyBuffer(GetVulkanDevice(), buffer, nullptr);
	}
	if (memory != 0)
	{
		vkFreeMemory(GetVulkanDevice(), memory, nullptr);
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
		vkDestroyShaderModule(GetVulkanDevice(), vertexShader, nullptr);
	}
	if (fragmentShader != 0)
	{
		vkDestroyShaderModule(GetVulkanDevice(), fragmentShader, nullptr);
	}
	if (descriptorPool != 0)
	{
		vkDestroyDescriptorPool(GetVulkanDevice(), descriptorPool, nullptr);
	}
	if (descriptorSetLayout != 0)
	{
		vkDestroyDescriptorSetLayout(GetVulkanDevice(), descriptorSetLayout, nullptr);
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
		vkFreeMemory(GetVulkanDevice(), memory, nullptr);
	}
	if (image != 0)
	{
		vkDestroyImageView(GetVulkanDevice(), imageView, nullptr);
	}
	if (image != 0)
	{
		vkDestroyImage(GetVulkanDevice(), image, nullptr);
	}
	if (sampler != 0)
	{
		vkDestroySampler(GetVulkanDevice(), sampler, nullptr);
	}
}

XFixedPipeline::XFixedPipeline()
{
	mPipelineLayout = 0;
	mPipeline = 0;
	mInputAssetmlyState = {};
	mInputAssetmlyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	mInputAssetmlyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	mInputAssetmlyState.primitiveRestartEnable = VK_FALSE;
	mViewport = {};
	mScissor = {};
	mViewportState = {};
	mViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	mViewportState.viewportCount = 1;
	mViewportState.pViewports = &mViewport;
	mViewportState.scissorCount = 1;
	mViewportState.pScissors = &mScissor;
	mRasterizer = {};
	mRasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	mRasterizer.depthClampEnable = VK_FALSE;
	mRasterizer.rasterizerDiscardEnable = VK_FALSE;
	mRasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	mRasterizer.lineWidth = 1.0f;
	mRasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	mRasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	mRasterizer.depthBiasEnable = VK_TRUE;
	mRasterizer.depthBiasConstantFactor = 0.0f;
	mRasterizer.depthBiasClamp = 0.0f;
	mRasterizer.depthBiasSlopeFactor = 0.0f;
	mDepthStencilState = {};
	mDepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	mDepthStencilState.depthTestEnable = VK_TRUE;
	mDepthStencilState.depthWriteEnable = VK_TRUE;
	mDepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	mDepthStencilState.depthBoundsTestEnable = VK_FALSE;
	mDepthStencilState.minDepthBounds = 0.0f;
	mDepthStencilState.maxDepthBounds = 1.0f;
	mDepthStencilState.stencilTestEnable = VK_FALSE;
	mDepthStencilState.front = {};
	mDepthStencilState.back = {};
	mMultisampleState = {};
	mMultisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	mMultisampleState.sampleShadingEnable = VK_TRUE;
	mMultisampleState.rasterizationSamples = GetGlobalFrameBufferSampleCount();
	mMultisampleState.minSampleShading = 1.0f;
	mMultisampleState.pSampleMask = nullptr;
	mMultisampleState.alphaToCoverageEnable = VK_FALSE;
	mMultisampleState.alphaToOneEnable = VK_FALSE;
	mColorBlendState = {};
	mColorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	mColorBlendState.logicOpEnable = VK_FALSE;
	mColorBlendState.logicOp = VK_LOGIC_OP_COPY;
	mDescriptorSetLayout = nullptr;
	mShaderStages = nullptr;
	mShaderStageCount = 0;
	mDescriptorSetLayoutCount = 0;
	mRenderPass = 0;
	mSampleCount = GetGlobalFrameBufferSampleCount();
	mPushConstantShaderStage = VK_SHADER_STAGE_FRAGMENT_BIT;
	mPushConstantCount = 8;
	mDepthConstantFactor = 0.0f;
	mDepthClamp = 0.0f;
	mDepthSlopeFactor = 0.0f;
}

XFixedPipeline::~XFixedPipeline()
{
	CleanUp();
}

void XFixedPipeline::CleanUp()
{
	if (mPipeline != 0)
	{
		vkDestroyPipeline(GetVulkanDevice(), mPipeline, nullptr);
	}
	if (mPipelineLayout != 0)
	{
		vkDestroyPipelineLayout(GetVulkanDevice(), mPipelineLayout, nullptr);
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
	VkBufferCopy copy = { 0, 0, size };
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
	vkDestroyFence(GetVulkanDevice(), fence, nullptr);
}

void xCreateShader(VkShaderModule& shader, unsigned char* code, int codeLength)
{
	VkShaderModuleCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = codeLength;
	info.pCode = (uint32_t*)code;
	vkCreateShaderModule(GetVulkanDevice(), &info, nullptr, &shader);
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
	vkCreateDescriptorSetLayout(GetVulkanDevice(), &info, nullptr, &program->descriptorSetLayout);
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
	vkCreateDescriptorPool(GetVulkanDevice(), &info, nullptr, &program->descriptorPool);
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
	vkAllocateDescriptorSets(GetVulkanDevice(), &info, &program->descriptorSet);
	for (int i = 0; i < program->writeDescriptorSet.size(); ++i)
	{
		program->writeDescriptorSet[i].dstSet = program->descriptorSet;
	}
	vkUpdateDescriptorSets(GetVulkanDevice(), uint32_t(program->writeDescriptorSet.size()), program->writeDescriptorSet.data(), 0, nullptr);
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
	if (vkCreateImage(GetVulkanDevice(), &info, nullptr, &texture->image) != VK_SUCCESS)
	{
		printf("failed to create image.");
	}
	VkMemoryRequirements requirements;
	vkGetImageMemoryRequirements(GetVulkanDevice(), texture->image, &requirements);
	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = requirements.size;
	allocateInfo.memoryTypeIndex = xGetMemoryType(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkAllocateMemory(GetVulkanDevice(), &allocateInfo, nullptr, &texture->memory);
	vkBindImageMemory(GetVulkanDevice(), texture->image, texture->memory, 0);
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
	vkMapMemory(GetVulkanDevice(), tempMemory, 0, imageSize, 0, &hostMemory);
	memcpy(hostMemory, pixel, (size_t)imageSize);
	vkUnmapMemory(GetVulkanDevice(), tempMemory);

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

	vkDestroyBuffer(GetVulkanDevice(), tempBuffer, nullptr);
	vkFreeMemory(GetVulkanDevice(), tempMemory, nullptr);
}

void xInitSrcAccessMask(VkImageLayout oldLayout, VkImageMemoryBarrier& barrier)
{
	switch (oldLayout) {
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
	switch (newLayout) {
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
		if (barrier.srcAccessMask == 0) {
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

void xGenImageView2D(XTexture* texture, int mipmap) {
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
	vkCreateImageView(GetVulkanDevice(), &ivci, nullptr, &texture->imageView);
}

void xGenSampler(XTexture* texture) {
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
	vkCreateSampler(GetVulkanDevice(), &samplercreateinfo, nullptr, &texture->sampler);
}

void xInitDefaultTexture() {
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
	VkImageLayout layout) {
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

void xUniform4fv(XProgram* program, int location, float* v) {
	//memcpy(program->vertexShaderVectorUniformBuffer.vectors[location].data, v, sizeof(XVector4f));
	//xSubmitUniformBuffer(&program->vertexShaderVectorUniformBuffer);
}

unsigned char* LoadImageFromFile(const char* path, int& width, int& height, int& channel,
	int force_channel, bool flipY) {
	unsigned char* result = stbi_load(path, &width, &height, &channel, force_channel);
	if (result == nullptr) {
		return nullptr;
	}
	if (false == flipY) {
		for (int j = 0; j * 2 < height; ++j) {
			int index1 = j * width * channel;
			int index2 = (height - 1 - j) * width * channel;
			for (int i = width * channel; i > 0; --i) {
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

XTexture* xGetDefaultTexture() {
	return sDefaultTexture;
}

void xRebindUniformBuffer(XProgram* program, int binding, XUniformBuffer* ubo) {
	VkDescriptorBufferInfo* bufferinfo = new VkDescriptorBufferInfo;
	bufferinfo->buffer = ubo->buffer;
	bufferinfo->offset = 0;
	if (ubo->type == XUniformBufferTypeMatrix) {
		bufferinfo->range = sizeof(XMatrix4x4f) * ubo->matrices.size();
	}
	else {
		bufferinfo->range = sizeof(XVector4f) * ubo->vectors.size();
	}
	delete program->writeDescriptorSet[binding].pBufferInfo;
	program->writeDescriptorSet[binding].pBufferInfo = bufferinfo;
	vkUpdateDescriptorSets(GetVulkanDevice(), uint32_t(program->writeDescriptorSet.size()),
		program->writeDescriptorSet.data(), 0, nullptr);
}

void xRebindSampler(XProgram* program, int binding, VkImageView iv, VkSampler s,
	VkImageLayout layout) {
	VkDescriptorImageInfo* bufferinfo = new VkDescriptorImageInfo;
	bufferinfo->imageView = iv;
	bufferinfo->imageLayout = layout;
	bufferinfo->sampler = s;
	delete program->writeDescriptorSet[binding].pImageInfo;
	program->writeDescriptorSet[binding].pImageInfo = bufferinfo;
	vkUpdateDescriptorSets(GetVulkanDevice(), uint32_t(program->writeDescriptorSet.size()),
		program->writeDescriptorSet.data(), 0, nullptr);
}

void xUseProgram(XProgram* program) {
	sCurrentProgram = program;
}
void xBindVertexBuffer(XBufferObject* vbo) {
	sCurrentVBO = vbo;
}
void xBindElementBuffer(XBufferObject* ibo) {
	sCurrentIBO = ibo;
}
void xDrawArrays(VkCommandBuffer commandbuffer, int offset, int count) {
	aSetDynamicState(&sCurrentProgram->fixedPipeline, commandbuffer);
	VkBuffer vertexbuffers[] = { sCurrentVBO->buffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		sCurrentProgram->fixedPipeline.mPipeline);
	vkCmdBindVertexBuffers(commandbuffer, 0, 1, vertexbuffers, offsets);
	if (sCurrentProgram->descriptorSet != 0) {
		vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			sCurrentProgram->fixedPipeline.mPipelineLayout, 0, 1, &sCurrentProgram->descriptorSet,
			0, nullptr);
	}
	vkCmdDraw(commandbuffer, count, 1, offset, 0);
}

void xDrawElements(VkCommandBuffer commandbuffer, int offset, int count) {
	aSetDynamicState(&sCurrentProgram->fixedPipeline, commandbuffer);
	VkBuffer vertexbuffers[] = { sCurrentVBO->buffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		sCurrentProgram->fixedPipeline.mPipeline);
	vkCmdBindVertexBuffers(commandbuffer, 0, 1, vertexbuffers, offsets);
	vkCmdBindIndexBuffer(commandbuffer, sCurrentIBO->buffer, 0, VK_INDEX_TYPE_UINT32);
	if (sCurrentProgram->descriptorSet != 0) {
		vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			sCurrentProgram->fixedPipeline.mPipelineLayout, 0, 1, &sCurrentProgram->descriptorSet,
			0, nullptr);
	}
	vkCmdDrawIndexed(commandbuffer, count, 1, offset, 0, 0);
}

VkCommandBuffer xBeginRendering(VkCommandBuffer commandbuffer) {
	VkCommandBuffer cmd;
	if (commandbuffer != nullptr) {
		cmd = commandbuffer;
	}
	else {
		xBeginOneTimeCommandBuffer(&cmd);
	}
	VkFramebuffer render_target = AquireRenderTarget();
	VkRenderPass render_pass = GetGlobalRenderPass();
	VkClearValue clearvalues[2] = {};
	clearvalues[0].color = { 0.1f,0.4f,0.6f,1.0f };
	clearvalues[1].depthStencil = { 1.0f,0 };

	VkRenderPassBeginInfo rpbi = {};
	rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpbi.framebuffer = render_target;
	rpbi.renderPass = render_pass;
	rpbi.renderArea.offset = { 0,0 };
	rpbi.renderArea.extent = { uint32_t(GetViewportWidth()),uint32_t(GetViewportHeight()) };
	rpbi.clearValueCount = 2;
	rpbi.pClearValues = clearvalues;
	vkCmdBeginRenderPass(cmd, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
	sMainCommandBuffer = cmd;
	return cmd;
}

void xEndRendering() {
	vkCmdEndRenderPass(sMainCommandBuffer);
	vkEndCommandBuffer(sMainCommandBuffer);
}

static void xSubmitDrawCommand(VkCommandBuffer commandbuffer)
{
	VkSemaphore ready_to_render[] = { GetReadyToRenderSemaphore() };
	VkSemaphore ready_to_present[] = { GetReadyToPresentSemaphore() };
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
	vkQueueSubmit(GetGraphicQueue(), 1, &submitinfo, VK_NULL_HANDLE);
}

static void PresentFrameBuffer()
{
	VkSemaphore ready_to_present[] = { GetReadyToPresentSemaphore() };
	VkSwapchainKHR swapchain = GetSwapchain();
	VkPresentInfoKHR presentinfo = {};
	uint32_t current_render_target_index = GetCurrentRenderTargetIndex();
	presentinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentinfo.pWaitSemaphores = ready_to_present;
	presentinfo.waitSemaphoreCount = 1;
	presentinfo.pSwapchains = &swapchain;
	presentinfo.swapchainCount = 1;
	presentinfo.pImageIndices = &current_render_target_index;
	vkQueuePresentKHR(GetPresentQueue(), &presentinfo);
	vkQueueWaitIdle(GetPresentQueue());
}

void xSwapBuffers(VkCommandBuffer commandbuffer) {
	VkCommandBuffer cmd;
	if (commandbuffer == nullptr) {
		cmd = sMainCommandBuffer;
	}
	else {
		cmd = commandbuffer;
	}
	xSubmitDrawCommand(cmd);
	PresentFrameBuffer();
	vkFreeCommandBuffers(GetVulkanDevice(), GetCommandPool(), 1, &cmd);
	sMainCommandBuffer = nullptr;
}

void xSetColorAttachmentCount(XFixedPipeline* pipeline, int count) {
	pipeline->mColorBlendAttachmentStates.resize(count);
	for (int i = 0; i < count; ++i) {
		pipeline->mColorBlendAttachmentStates[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		pipeline->mColorBlendAttachmentStates[i].blendEnable = VK_FALSE;
		pipeline->mColorBlendAttachmentStates[i].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		pipeline->mColorBlendAttachmentStates[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		pipeline->mColorBlendAttachmentStates[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		pipeline->mColorBlendAttachmentStates[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		pipeline->mColorBlendAttachmentStates[i].colorBlendOp = VK_BLEND_OP_ADD;
		pipeline->mColorBlendAttachmentStates[i].alphaBlendOp = VK_BLEND_OP_ADD;
	}
}
void xEnableBlend(XFixedPipeline* pipeline, int attachment, VkBool32 enable) {
	pipeline->mColorBlendAttachmentStates[attachment].blendEnable = enable;
}
void xBlend(XFixedPipeline* p, int attachment, VkBlendFactor s_c, VkBlendFactor s_a,
	VkBlendFactor d_c, VkBlendFactor d_a) {
	p->mColorBlendAttachmentStates[attachment].srcColorBlendFactor = s_c;
	p->mColorBlendAttachmentStates[attachment].srcAlphaBlendFactor = s_a;
	p->mColorBlendAttachmentStates[attachment].dstColorBlendFactor = d_c;
	p->mColorBlendAttachmentStates[attachment].dstAlphaBlendFactor = d_a;
}
void xBlendOp(XFixedPipeline* p, int attachment, VkBlendOp color, VkBlendOp alpha) {
	p->mColorBlendAttachmentStates[attachment].colorBlendOp = color;
	p->mColorBlendAttachmentStates[attachment].alphaBlendOp = alpha;
}
void xPolygonMode(XFixedPipeline* p, VkPolygonMode mode) {
	p->mRasterizer.polygonMode = mode;
}
void xDisableRasterizer(XFixedPipeline* p, VkBool32 disable) {
	p->mRasterizer.rasterizerDiscardEnable = disable;
}
void xEnableDepthTest(XFixedPipeline* p, VkBool32 enable) {
	p->mDepthStencilState.depthTestEnable = enable;
}
void xInitPipelineLayout(XFixedPipeline* p) {
	VkPushConstantRange pushconstancrange = {};
	pushconstancrange.stageFlags = p->mPushConstantShaderStage;
	pushconstancrange.offset = 0;
	pushconstancrange.size = sizeof(XVector4f) * p->mPushConstantCount;
	VkPipelineLayoutCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	ci.pSetLayouts = p->mDescriptorSetLayout;
	ci.setLayoutCount = p->mDescriptorSetLayoutCount;
	ci.pPushConstantRanges = &pushconstancrange;
	ci.pushConstantRangeCount = 1;
	vkCreatePipelineLayout(GetVulkanDevice(), &ci, nullptr, &p->mPipelineLayout);
}
void xCreateFixedPipeline(XFixedPipeline* p) {
	const auto& bindingdescriptions = XVertexData::BindingDescription();
	const auto& attributeDescriptions = XVertexData::AttributeDescriptions();
	VkPipelineVertexInputStateCreateInfo vertexinputinfo = {};
	vertexinputinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexinputinfo.vertexBindingDescriptionCount = 1;
	vertexinputinfo.pVertexBindingDescriptions = &bindingdescriptions;
	vertexinputinfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
	vertexinputinfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	p->mColorBlendState.attachmentCount = p->mColorBlendAttachmentStates.size();
	p->mColorBlendState.pAttachments = p->mColorBlendAttachmentStates.data();
	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_DEPTH_BIAS
	};
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 4;
	dynamicState.pDynamicStates = dynamicStates;
	VkGraphicsPipelineCreateInfo pipelineinfo = {};
	pipelineinfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineinfo.stageCount = p->mShaderStageCount;
	pipelineinfo.pStages = p->mShaderStages;
	pipelineinfo.pVertexInputState = &vertexinputinfo;
	pipelineinfo.pInputAssemblyState = &p->mInputAssetmlyState;
	pipelineinfo.pViewportState = &p->mViewportState;
	pipelineinfo.pRasterizationState = &p->mRasterizer;
	pipelineinfo.pMultisampleState = &p->mMultisampleState;
	pipelineinfo.pDepthStencilState = &p->mDepthStencilState;
	pipelineinfo.pColorBlendState = &p->mColorBlendState;
	pipelineinfo.pDynamicState = &dynamicState;
	pipelineinfo.layout = p->mPipelineLayout;
	pipelineinfo.renderPass = p->mRenderPass;
	pipelineinfo.subpass = 0;
	pipelineinfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineinfo.basePipelineIndex = -1;
	vkCreateGraphicsPipelines(GetVulkanDevice(), VK_NULL_HANDLE, 1, &pipelineinfo, nullptr,
		&p->mPipeline);
}
void xSetDynamicState(XFixedPipeline* p, VkCommandBuffer commandbuffer)
{
	vkCmdSetViewport(commandbuffer, 0, 1, &p->mViewport);
	vkCmdSetScissor(commandbuffer, 0, 1, &p->mScissor);
	vkCmdSetDepthBias(commandbuffer, p->mDepthConstantFactor, p->mDepthClamp, p->mDepthSlopeFactor);
	vkCmdPushConstants(commandbuffer, p->mPipelineLayout, p->mPushConstantShaderStage, 0,
		sizeof(XVector4f) * p->mPushConstantCount, p->mPushConstants);
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
	if (vkCreateImage(GetVulkanDevice(), &ici, nullptr, &texture->image) != VK_SUCCESS) {
		printf("failed to create image\n");
	}
	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(GetVulkanDevice(), texture->image, &memory_requirements);
	VkMemoryAllocateInfo mai = {};
	mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mai.allocationSize = memory_requirements.size;
	mai.memoryTypeIndex = xGetMemoryType(memory_requirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkAllocateMemory(GetVulkanDevice(), &mai, nullptr, &texture->memory);
	vkBindImageMemory(GetVulkanDevice(), texture->image, texture->memory, 0);
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
	vkMapMemory(GetVulkanDevice(), tempmemory, 0, imagesize, 0, &data);
	memcpy(data, pixel, imagesize);
	vkUnmapMemory(GetVulkanDevice(), tempmemory);

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

	vkDestroyBuffer(GetVulkanDevice(), tempbuffer, nullptr);
	vkFreeMemory(GetVulkanDevice(), tempmemory, nullptr);
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
	vkCreateImageView(GetVulkanDevice(), &ivci, nullptr, &texture->imageView);
}

void xVulkanCleanUp() {
	if (sDefaultTexture != nullptr) {
		delete sDefaultTexture;
	}
}