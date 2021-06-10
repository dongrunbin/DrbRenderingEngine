#pragma once
#include "VertexBuffer.h"
#include "Texture2D.h"
#include "Material.h"

class FrameBuffer
{
public:
	VkFramebuffer fbo;
	VkRenderPass renderPass;
	uint32_t width, height;
	std::vector<Texture2D*> attachments;
	std::vector<VkClearValue> clearValues;
	int colorBufferCount;
	int depthBufferIndex;
	Texture2D* depthBuffer;
public:
	FrameBuffer();
	~FrameBuffer();
	void SetSize(uint32_t width, uint32_t height);
	void AttachColorBuffer(VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
	void AttachDepthBuffer();
	void Finish();
	void SetClearColor(int index, float r, float g, float b, float a);
	void SetClearDepthStencil(float depth, uint32_t stencil);
	VkCommandBuffer BeginRendering(VkCommandBuffer commandbuffer = nullptr);
};