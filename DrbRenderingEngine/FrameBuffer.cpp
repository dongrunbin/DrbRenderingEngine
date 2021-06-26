#include "FrameBuffer.h"

FrameBuffer::FrameBuffer()
{
	fbo = 0;
	renderPass = 0;
	depthBuffer = nullptr;
}

FrameBuffer::~FrameBuffer()
{
	for (auto iter = attachments.begin(); iter != attachments.end(); ++iter)
	{
		delete* iter;
	}
	if (renderPass != 0) {
		vkDestroyRenderPass(xGetVulkanDevice(), renderPass, nullptr);
	}
	if (fbo != 0) {
		vkDestroyFramebuffer(xGetVulkanDevice(), fbo, nullptr);
	}
}

void FrameBuffer::SetSize(uint32_t width, uint32_t height)
{
	this->width = width;
	this->height = height;
}

void FrameBuffer::AttachColorBuffer(VkFormat format)
{
	Texture2D* color_buffer = new Texture2D;
	color_buffer->format = format;
	xGenImage(color_buffer, width, height, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
		VK_IMAGE_USAGE_SAMPLED_BIT);
	xGenImageView2D(color_buffer);
	xGenSampler(color_buffer);
	colorBufferCount++;
	attachments.push_back(color_buffer);
	VkClearValue cv;
	cv.color = { 0.0f,0.0f,0.0f,0.0f };
	clearValues.push_back(cv);
}

void FrameBuffer::AttachDepthBuffer()
{
	Texture2D* depth_buffer = new Texture2D(VK_IMAGE_ASPECT_DEPTH_BIT);
	depth_buffer->format = VK_FORMAT_D24_UNORM_S8_UINT;
	xGenImage(depth_buffer, width, height, depth_buffer->format,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	xGenImageView2D(depth_buffer);
	xGenSampler(depth_buffer);
	depthBufferIndex = attachments.size();
	attachments.push_back(depth_buffer);
	VkClearValue cv;
	cv.depthStencil = { 1.0f,0 };
	clearValues.push_back(cv);
	depthBuffer = depth_buffer;
}

void FrameBuffer::Finish()
{
	std::vector<VkAttachmentDescription> attachments;
	std::vector<VkAttachmentReference> colorattachment_refences;
	VkAttachmentReference depthattachment_ref = {};
	std::vector<VkImageView> render_targets;
	render_targets.resize(this->attachments.size());
	attachments.resize(this->attachments.size());
	int color_buffer_count = 0;
	for (size_t i = 0; i < this->attachments.size(); ++i)
	{
		Texture2D* texture = this->attachments[i];
		if (texture->imageAspectFlags == VK_IMAGE_ASPECT_COLOR_BIT)
		{
			color_buffer_count++;
		}
		else if (texture->imageAspectFlags == VK_IMAGE_ASPECT_DEPTH_BIT)
		{

		}
		render_targets[i] = texture->imageView;
	}
	colorattachment_refences.resize(color_buffer_count);
	int color_buffer_index = 0;
	int attachment_point = 0;
	for (size_t i = 0; i < this->attachments.size(); ++i)
	{
		Texture2D* texture = this->attachments[i];
		if (texture->imageAspectFlags == VK_IMAGE_ASPECT_COLOR_BIT)
		{
			attachments[i] =
			{
				0,
				texture->format,
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_CLEAR,
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};
			colorattachment_refences[color_buffer_index++] =
			{
				uint32_t(attachment_point++),VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			};
		}
		else if (texture->imageAspectFlags == VK_IMAGE_ASPECT_DEPTH_BIT)
		{
			attachments[i] =
			{
				0,
				texture->format,
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_CLEAR,
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_CLEAR,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};
			depthattachment_ref.attachment = attachment_point++;
			depthattachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
	}

	VkSubpassDescription subpasses = {};
	subpasses.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses.colorAttachmentCount = colorattachment_refences.size();
	subpasses.pColorAttachments = colorattachment_refences.data();
	subpasses.pDepthStencilAttachment = &depthattachment_ref;

	VkRenderPassCreateInfo rpci = {};
	rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rpci.attachmentCount = attachments.size();
	rpci.pAttachments = attachments.data();
	rpci.subpassCount = 1;
	rpci.pSubpasses = &subpasses;
	vkCreateRenderPass(xGetVulkanDevice(), &rpci, nullptr, &renderPass);
	VkFramebufferCreateInfo fbci = {};
	fbci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbci.pAttachments = render_targets.data();
	fbci.attachmentCount = render_targets.size();
	fbci.width = width;
	fbci.height = height;
	fbci.layers = 1;
	fbci.renderPass = renderPass;
	vkCreateFramebuffer(xGetVulkanDevice(), &fbci, nullptr, &fbo);
}

void FrameBuffer::SetClearColor(int index, float r, float g, float b, float a)
{
	clearValues[index].color = { r,g,b,a };
}

void FrameBuffer::SetClearDepthStencil(float depth, uint32_t stencil)
{
	clearValues[depthBufferIndex].depthStencil = { depth,stencil };
}

VkCommandBuffer FrameBuffer::BeginRendering(VkCommandBuffer commandbuffer)
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
	VkFramebuffer render_target = fbo;
	VkRenderPass render_pass = renderPass;

	VkRenderPassBeginInfo rpbi = {};
	rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpbi.framebuffer = render_target;
	rpbi.renderPass = render_pass;
	rpbi.renderArea.offset = { 0,0 };
	rpbi.renderArea.extent = { uint32_t(xGetViewportWidth()),uint32_t(xGetViewportHeight()) };
	rpbi.clearValueCount = clearValues.size();
	rpbi.pClearValues = clearValues.data();
	vkCmdBeginRenderPass(cmd, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
	return cmd;
}