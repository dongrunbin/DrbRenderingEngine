#include "BVulkan.h"
#include "scene.h"
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

Texture2D* texture;
TextureCube* skybox;
Material* sphereMaterial;
XFixedPipeline* spherePipeline;
Material* groundMaterial;
XFixedPipeline* groundPipeline;
Material* depthrenderMaterial;
XFixedPipeline* depthrenderPipeline;
Material* fsqMaterial;
XFixedPipeline* fsqPipeline;
FullScreenQuad* fsq;

Ground* ground;
Camera* camera;
Model* sphere;
FrameBuffer* fbo;
FrameBuffer* gbuffer;

float moveSpeed = 0.4f;

glm::mat4 model(1.0);
glm::mat4 projection;

Light* lights;
//mutiple threading
//VkCommandBuffer* drawcommands;
//static float time_since_time = 0.0f;

void Init()
{
	camera = new Camera;
	camera->cameraPos = glm::vec3(0.0f, 5.0f, -15.0f);
	camera->Pitch(25.0f);
	projection = glm::perspective(45.0f, float(GetViewportWidth()) / float(GetViewportHeight()), 0.1f, 100.0f);
	projection[1][1] *= -1.0f;

	lights = new Light[3];
	lights[0].pos = glm::vec4(0.0f, 5.0f, 0.0f, 1.0f);
	lights[0].color = glm::vec4(10.0f, 10.0f, 10.0f, 1.0f);
	lights[0].view = glm::lookAt(glm::vec3(lights[0].pos), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	lights[0].projection = projection;
	lights[1].pos = glm::vec4(0.0f, 5.0f, 5.0f, 1.0f);
	lights[1].color = glm::vec4(10.0f, 10.0f, 10.0f, 1.0f);
	lights[1].view = glm::lookAt(glm::vec3(lights[1].pos), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	lights[1].projection = projection;
	int a = sizeof(glm::mat4);
	fbo = new FrameBuffer;
	fbo->SetSize(GetViewportWidth(), GetViewportHeight());
	fbo->AttachColorBuffer();
	fbo->AttachDepthBuffer();
	fbo->Finish();

	gbuffer = new FrameBuffer;
	gbuffer->SetSize(GetViewportWidth(), GetViewportHeight());
	//要在gbuffer里存储世界坐标系里的position/normal/texcolor
	gbuffer->AttachColorBuffer(VK_FORMAT_R16G16B16A16_SFLOAT);
	gbuffer->AttachColorBuffer(VK_FORMAT_R16G16B16A16_SFLOAT);
	gbuffer->AttachColorBuffer();
	gbuffer->AttachDepthBuffer();
	gbuffer->Finish();

	xInitDefaultTexture();

	skybox = new TextureCube;
	skybox->Init("");
	texture = new Texture2D;
	texture->SetImage("Res/test.bmp");

	//sphere
	sphereMaterial = new Material;
	sphereMaterial->Init("Res/sphere.vsb", "Res/sphere.fsb");
	sphereMaterial->SetMVP(model, camera->GetViewMat(), projection);
	sphereMaterial->fragmentVector4UBO->SetVector4(0, camera->cameraPos.x, camera->cameraPos.y, camera->cameraPos.z, 1.0f);
	sphereMaterial->SetUniformBuffer(4, lights, 3 * sizeof(Light), VK_SHADER_STAGE_FRAGMENT_BIT);
	sphereMaterial->SetTexture(5, skybox);
	sphereMaterial->SubmitUniformBuffers();
	spherePipeline = new XFixedPipeline;
	xSetColorAttachmentCount(spherePipeline, 3);
	spherePipeline->mRenderPass = gbuffer->renderPass;
	sphereMaterial->SetFixedPipeline(spherePipeline);
	spherePipeline->mViewport = { 0.0f, 0.0f, float(GetViewportWidth()), float(GetViewportHeight()), 0.0f, 1.0f };
	spherePipeline->mScissor = { {0, 0}, { uint32_t(GetViewportWidth()), uint32_t(GetViewportHeight()) } };
	sphereMaterial->Finish();
	sphere = new Model;
	sphere->Init("Res/Sphere.raw");
	sphere->SetMaterial(sphereMaterial);

	//ground
	groundMaterial = new Material;
	groundMaterial->Init("Res/ground.vsb", "Res/ground.fsb");
	groundMaterial->SetMVP(model, camera->GetViewMat(), projection);
	groundMaterial->fragmentVector4UBO->SetVector4(0, camera->cameraPos.x, camera->cameraPos.y, camera->cameraPos.z, 1.0f);
	groundMaterial->SetUniformBuffer(4, lights, 3 * sizeof(Light), VK_SHADER_STAGE_FRAGMENT_BIT);
	groundMaterial->SetTexture(5, fbo->depthBuffer);
	groundMaterial->SubmitUniformBuffers();
	groundPipeline = new XFixedPipeline;
	xSetColorAttachmentCount(groundPipeline, 3);
	groundPipeline->mRenderPass = gbuffer->renderPass;
	groundMaterial->SetFixedPipeline(groundPipeline);
	groundPipeline->mViewport = { 0.0f, 0.0f, float(GetViewportWidth()), float(GetViewportHeight()), 0.0f, 1.0f };
	groundPipeline->mScissor = { {0, 0}, { uint32_t(GetViewportWidth()), uint32_t(GetViewportHeight()) } };
	groundMaterial->Finish();
	ground = new Ground();
	ground->Init();
	ground->SetMaterial(groundMaterial);

	// fsq
	fsqMaterial = new Material;
	//fsqMaterial->Init("Res/renderdepth.vsb", "Res/renderdepth.fsb");
	//fsqMaterial->Init("Res/fsq.vsb", "Res/fsq.fsb");
	fsqMaterial->Init("Res/deferred.vsb", "Res/deferred.fsb");
	fsqMaterial->SetUniformBuffer(4, lights, 3 * sizeof(Light), VK_SHADER_STAGE_FRAGMENT_BIT);
	fsqMaterial->SetTexture(5, gbuffer->attachments[0]);
	fsqMaterial->SetTexture(6, gbuffer->attachments[1]);
	fsqMaterial->SetTexture(7, gbuffer->attachments[2]);
	fsqMaterial->SetTexture(8, fbo->depthBuffer);
	fsqMaterial->SubmitUniformBuffers();
	fsqPipeline = new XFixedPipeline;
	xSetColorAttachmentCount(fsqPipeline, 1);
	fsqPipeline->mRenderPass = GetGlobalRenderPass();
	fsqMaterial->SetFixedPipeline(fsqPipeline);
	fsqPipeline->mViewport = { 0.0f, 0.0f, float(GetViewportWidth()), float(GetViewportHeight()), 0.0f, 1.0f };
	fsqPipeline->mScissor = { {0, 0}, { uint32_t(GetViewportWidth()), uint32_t(GetViewportHeight()) } };
	fsqPipeline->mInputAssetmlyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	fsqMaterial->Finish();
	fsq = new FullScreenQuad;
	fsq->Init();
	fsq->material = fsqMaterial;

	//depth render
	depthrenderMaterial = new Material;
	depthrenderMaterial->Init("Res/depthrender.vsb", "Res/depthrender.fsb");
	depthrenderMaterial->SetMVP(model, lights[0].view, projection);
	depthrenderPipeline = new XFixedPipeline;
	xSetColorAttachmentCount(depthrenderPipeline, 1);
	depthrenderPipeline->mRenderPass = fbo->renderPass;
	depthrenderMaterial->SetFixedPipeline(depthrenderPipeline);
	depthrenderPipeline->mViewport = { 0.0f, 0.0f, float(GetViewportWidth()), float(GetViewportHeight()), 0.0f, 1.0f };
	depthrenderPipeline->mScissor = { {0, 0}, { uint32_t(GetViewportWidth()), uint32_t(GetViewportHeight()) } };
	depthrenderMaterial->Finish();
		depthrenderMaterial->SubmitUniformBuffers();


	//mutiple threading
	//fsqMaterial = new Material;
	//fsqMaterial->Init("Res/fsq.vsb", "Res/fsq.fsb");
	//fsqMaterial->SubmitUniformBuffers();
	//fsqPipeline = new XFixedPipeline;
	//xSetColorAttachmentCount(fsqPipeline, 1);
	//fsqPipeline->mRenderPass = GetGlobalRenderPass();
	//fsqMaterial->SetFixedPipeline(fsqPipeline);
	//fsqPipeline->mViewport = { 0.0f, 0.0f, float(GetViewportWidth()), float(GetViewportHeight()), 0.0f, 1.0f };
	//fsqPipeline->mScissor = { {0, 0}, { uint32_t(GetViewportWidth()), uint32_t(GetViewportHeight()) } };
	//fsqPipeline->mInputAssetmlyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	//fsqMaterial->Finish();
	//fsq = new FullScreenQuad;
	//fsq->Init();
	//fsqMaterial->SetTexture(0, texture);
	//fsq->material = fsqMaterial;

	//drawcommands = new VkCommandBuffer[GetFrameBufferCount()];
	//xGenCommandBuffer(drawcommands, GetFrameBufferCount(), VK_COMMAND_BUFFER_LEVEL_SECONDARY);

	//std::thread t([]() {
	//	for (int i = 0; i < GetFrameBufferCount(); ++i)
	//	{
	//		VkCommandBufferInheritanceInfo inheritanceInfo = {};
	//		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	//		inheritanceInfo.framebuffer = GetFrameBuffer(i);
	//		inheritanceInfo.renderPass = GetGlobalRenderPass();

	//		VkCommandBufferBeginInfo beginInfo = {};
	//		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	//		beginInfo.pInheritanceInfo = &inheritanceInfo;
	//		vkBeginCommandBuffer(drawcommands[i], &beginInfo);

	//		fsq->Draw(drawcommands[i]);
	//		vkEndCommandBuffer(drawcommands[i]);
	//	}
	//	VkCommandBuffer drawCommandbuffer;
	//	xGenCommandBuffer(&drawCommandbuffer, 1, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
	//	});
	//t.detach();
}
void Draw(float deltaTime)
{
	for (auto it = Material::sMaterials.begin(); it != Material::sMaterials.end(); it++)
	{
		if ((*it) == depthrenderMaterial)
		{
			continue;
		}
		(*it)->SetMVP(model, camera->GetViewMat(), projection);
		(*it)->fragmentVector4UBO->SetVector4(0, camera->cameraPos.x, camera->cameraPos.y, camera->cameraPos.z, 1.0f);
		(*it)->SubmitUniformBuffers();
	}

	////depth pass
	VkCommandBuffer commandbuffer = fbo->BeginRendering();
	ground->SetMaterial(depthrenderMaterial);
	sphere->SetMaterial(depthrenderMaterial);
	ground->Draw(commandbuffer);
	sphere->Draw(commandbuffer);
	vkCmdEndRenderPass(commandbuffer);
	//gbuffer pass
	gbuffer->BeginRendering(commandbuffer);
	ground->SetMaterial(groundMaterial);
	sphere->SetMaterial(sphereMaterial);
	ground->Draw(commandbuffer);
	sphere->Draw(commandbuffer);
	vkCmdEndRenderPass(commandbuffer);
	//lighting pass
	xBeginRendering(commandbuffer);
	fsq->Draw(commandbuffer);
	xEndRendering();
	xSwapBuffers(commandbuffer);



	//mutiple threading
	//time_since_time += deltaTime;
	//VkCommandBuffer cmd;
	//xBeginOneTimeCommandBuffer(&cmd);
	//VkFramebuffer render_target = AquireRenderTarget();
	//VkRenderPass render_pass = GetGlobalRenderPass();
	//VkClearValue clearvalues[2] = {};
	//clearvalues[0].color = { 0.1f,0.4f,0.6f,1.0f };
	//clearvalues[1].depthStencil = { 1.0f,0 };

	//VkRenderPassBeginInfo rpbi = {};
	//rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	//rpbi.framebuffer = render_target;
	//rpbi.renderPass = render_pass;
	//rpbi.renderArea.offset = { 0,0 };
	//rpbi.renderArea.extent = { uint32_t(GetViewportWidth()),uint32_t(GetViewportHeight()) };
	//rpbi.clearValueCount = 2;
	//rpbi.pClearValues = clearvalues;
	//vkCmdBeginRenderPass(cmd, &rpbi, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
	//if (time_since_time > 3.0f)
	//{
	//	vkCmdExecuteCommands(cmd, 1, &drawcommands[GetCurrentRenderTargetIndex()]);
	//}
	//vkCmdEndRenderPass(cmd);
	//vkEndCommandBuffer(cmd);
	//xSwapBuffers(cmd);
}
void OnViewportChanged(int width, int height)
{
	aViewport(width, height);
}

void OnKeyboard(unsigned char key)
{
	camera->forward = glm::normalize(camera->forward);
	glm::vec3 right = glm::cross(camera->forward, camera->up);
	right = glm::normalize(right);
	switch (key)
	{
	case 'W':
		camera->cameraPos += camera->forward * moveSpeed;
		break;
	case 'S':
		camera->cameraPos -= camera->forward * moveSpeed;
		break;
	case 'A':
		camera->cameraPos -= right * moveSpeed;
		break;
	case 'D':
		camera->cameraPos += right * moveSpeed;
		break;
	default:
		break;
	}
}

void OnMouseMove(int deltaX, int deltaY)
{
	float angleX = (float)deltaX / 100.0f;
	float angleY = (float)deltaY / 100.0f;
	camera->Yaw(-angleX);
	camera->Pitch(-angleY);
}

void OnQuit()
{
	if (spherePipeline != nullptr)
	{
		delete spherePipeline;
	}
	if (fsqPipeline != nullptr)
	{
		delete fsqPipeline;
	}
	if (depthrenderPipeline != nullptr)
	{
		delete depthrenderPipeline;
	}
	if (texture != nullptr)
	{
		delete texture;
	}
	if (ground != nullptr)
	{
		delete ground;
	}
	if (groundPipeline != nullptr)
	{
		delete groundPipeline;
	}
	if (fsq != nullptr)
	{
		delete fsq;
	}
	if (camera != nullptr)
	{
		delete camera;
	}
	if (sphere != nullptr)
	{
		delete sphere;
	}
	if (skybox != nullptr)
	{
		delete skybox;
	}
	if (fbo != nullptr)
	{
		delete fbo;
	}
	if (gbuffer != nullptr)
	{
		delete gbuffer;
	}
	if (lights != nullptr)
	{
		delete[] lights;
	}
	Material::CleanUp();
	xVulkanCleanUp();
}