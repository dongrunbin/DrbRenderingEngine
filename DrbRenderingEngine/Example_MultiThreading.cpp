#include "Example.h"
#include "VulkanAPI.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "UniformBuffer.h"
#include "Texture2D.h"
#include "Material.h"
#include "FullScreenQuad.h"
#include "Ground.h"
#include "Camera.h"
#include "Model.h"
#include "FrameBuffer.h"
#include "Light.h"
#include <thread>

class MultiThreading : public Example
{
private:
	Material* fsqMaterial;
	XFixedPipeline* fsqPipeline;
	static FullScreenQuad* fsq;
	Texture2D *texture;
	static VkCommandBuffer* drawcommands;
	static float time_since_time;

public:
	void Init()
	{
		time_since_time = 0.0f;
		fsqMaterial = new Material;
		fsqMaterial->Init("Res/fsq.vsb", "Res/fsq.fsb");
		fsqMaterial->SubmitUniformBuffers();
		fsqPipeline = new XFixedPipeline;
		xSetColorAttachmentCount(fsqPipeline, 1);
		fsqPipeline->renderPass = xGetGlobalRenderPass();
		fsqMaterial->SetFixedPipeline(fsqPipeline);
		fsqPipeline->viewport = { 0.0f, 0.0f, float(xGetViewportWidth()), float(xGetViewportHeight()), 0.0f, 1.0f };
		fsqPipeline->scissor = { {0, 0}, { uint32_t(xGetViewportWidth()), uint32_t(xGetViewportHeight()) } };
		fsqPipeline->inputAssetmlyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		fsqMaterial->Finish();
		fsq = new FullScreenQuad;
		fsq->Init();
		texture = new Texture2D;
		texture->SetImage("Res/images/test.bmp");
		fsqMaterial->SetTexture(0, texture);
		fsq->material = fsqMaterial;

		drawcommands = new VkCommandBuffer[xGetSystemFrameBufferCount()];
		xGenCommandBuffer(drawcommands, xGetSystemFrameBufferCount(), VK_COMMAND_BUFFER_LEVEL_SECONDARY);

		std::thread t([]()
			{
			for (int i = 0; i < xGetSystemFrameBufferCount(); ++i)
			{
				VkCommandBufferInheritanceInfo inheritanceInfo = {};
				inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
				inheritanceInfo.framebuffer = ((XSystemFrameBuffer*)xGetSystemFrameBuffer(i))->framebuffer;
				inheritanceInfo.renderPass = xGetGlobalRenderPass();

				VkCommandBufferBeginInfo beginInfo = {};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
				beginInfo.pInheritanceInfo = &inheritanceInfo;
				vkBeginCommandBuffer(drawcommands[i], &beginInfo);

				fsq->Draw(drawcommands[i]);
				vkEndCommandBuffer(drawcommands[i]);
			}
			VkCommandBuffer drawCommandbuffer;
			xGenCommandBuffer(&drawCommandbuffer, 1, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
			});
		t.detach();
	}
	void Draw(float deltaTime)
	{
		time_since_time += deltaTime;
		VkCommandBuffer cmd;
		xBeginOneTimeCommandBuffer(&cmd);
		VkFramebuffer render_target = xAquireRenderTarget();
		VkRenderPass render_pass = xGetGlobalRenderPass();
		VkClearValue clearvalues[2] = {};
		clearvalues[0].color = { 0.1f,0.4f,0.6f,1.0f };
		clearvalues[1].depthStencil = { 1.0f,0 };

		VkRenderPassBeginInfo rpbi = {};
		rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpbi.framebuffer = render_target;
		rpbi.renderPass = render_pass;
		rpbi.renderArea.offset = { 0,0 };
		rpbi.renderArea.extent = { uint32_t(xGetViewportWidth()),uint32_t(xGetViewportHeight()) };
		rpbi.clearValueCount = 2;
		rpbi.pClearValues = clearvalues;
		vkCmdBeginRenderPass(cmd, &rpbi, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
		if (time_since_time > 3.0f)
		{
			vkCmdExecuteCommands(cmd, 1, &drawcommands[xGetCurrenRenderTargetIndex()]);
		}
		vkCmdEndRenderPass(cmd);
		vkEndCommandBuffer(cmd);
		xSwapBuffers(cmd);
	}
	void OnViewportChanged(int width, int height)
	{
		xViewport(width, height);
	}
	void OnKeyboard(unsigned char key)
	{
	}
	void OnMouseMove(int deltaX, int deltaY)
	{
	}
	void OnQuit()
	{
		if (fsqPipeline != nullptr)
		{
			delete fsqPipeline;
		}
		if (texture != nullptr)
		{
			delete texture;
		}
		if (fsq != nullptr)
		{
			delete fsq;
		}
		Material::CleanUp();
		xVulkanCleanUp();
	}
};