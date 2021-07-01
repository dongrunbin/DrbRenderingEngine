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

class ForwardShadowMapping : public Example
{
private:
	TextureCube* skybox;
	Material* sphereMaterial;
	XFixedPipeline* spherePipeline;
	Material* groundMaterial;
	XFixedPipeline* groundPipeline;
	Material** depthrenderMaterials;
	XFixedPipeline** depthrenderPipelines;
	Material* fsqMaterial;
	XFixedPipeline* fsqPipeline;
	FullScreenQuad* fsq;
	Ground* ground;
	Model* sphere;
	FrameBuffer* fbos;
	glm::mat4 sphereModelMat;
	glm::mat4 groundModelMat;
	BasicLight* lights;
	const int LIGHT_COUNT = 2;

	Material* skyboxMaterial;
	XFixedPipeline* skyboxPipeline;
	Model* cube;

public:
	void Init()
	{
		camera = new Camera(xGetViewportWidth(), xGetViewportHeight());
		camera->cameraPos = glm::vec3(0.0f, 5.0f, -15.0f);
		camera->Pitch(25.0f);
		sphereModelMat = glm::mat4(1.0f);
		groundModelMat = glm::mat4(1.0f);

		lights = new BasicLight[LIGHT_COUNT];
		lights[0].pos = glm::vec4(0.0f, 5.0f, 0.0f, 1.0f);
		lights[0].color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		lights[0].view = glm::lookAt(glm::vec3(lights[0].pos), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		lights[0].projection = camera->projection;
		lights[1].pos = glm::vec4(0.0f, 2.0f, 5.0f, 1.0f);
		lights[1].color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		lights[1].view = glm::lookAt(glm::vec3(lights[1].pos), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		lights[1].projection = camera->projection;
		fbos = new FrameBuffer[LIGHT_COUNT];
		for (int i = 0; i < LIGHT_COUNT; ++i)
		{
			FrameBuffer* fbo = &fbos[i];
			fbo->SetSize(xGetViewportWidth(), xGetViewportHeight());
			fbo->AttachColorBuffer();
			fbo->AttachDepthBuffer();
			fbo->Finish();
		}

		xInitDefaultTexture();

		skybox = new TextureCube;
		skybox->Init(
			"Res/skybox/right.bmp",
			"Res/skybox/left.bmp",
			"Res/skybox/top.bmp",
			"Res/skybox/bottom.bmp",
			"Res/skybox/back.bmp",
			"Res/skybox/front.bmp");

		//sphere
		sphereMaterial = new Material;
		sphereMaterial->Init("Res/sphere_forward.vsb", "Res/sphere_forward.fsb");
		sphereMaterial->SetMVP(sphereModelMat, camera->GetViewMat(), camera->projection);
		sphereMaterial->fragmentVector4UBO->SetVector4(0, camera->cameraPos.x, camera->cameraPos.y, camera->cameraPos.z, 1.0f);
		sphereMaterial->SetUniformBuffer(4, lights, LIGHT_COUNT * sizeof(BasicLight), VK_SHADER_STAGE_FRAGMENT_BIT);
		sphereMaterial->SetTexture(5, skybox);
		sphereMaterial->SubmitUniformBuffers();
		spherePipeline = new XFixedPipeline;
		xSetColorAttachmentCount(spherePipeline, 1);
		spherePipeline->renderPass = xGetGlobalRenderPass();
		sphereMaterial->SetFixedPipeline(spherePipeline);
		spherePipeline->viewport = { 0.0f, 0.0f, float(xGetViewportWidth()), float(xGetViewportHeight()), 0.0f, 1.0f };
		spherePipeline->scissor = { {0, 0}, { uint32_t(xGetViewportWidth()), uint32_t(xGetViewportHeight()) } };
		sphereMaterial->Finish();
		sphere = new Model;
		sphere->Init("Res/model/Sphere.raw");
		sphere->SetMaterial(sphereMaterial);

		//ground
		groundMaterial = new Material;
		groundMaterial->Init("Res/ground_forward.vsb", "Res/ground_forward.fsb");
		groundMaterial->SetMVP(groundModelMat, camera->GetViewMat(), camera->projection);
		groundMaterial->fragmentVector4UBO->SetVector4(0, camera->cameraPos.x, camera->cameraPos.y, camera->cameraPos.z, 1.0f);
		groundMaterial->SetUniformBuffer(4, lights, LIGHT_COUNT * sizeof(BasicLight), VK_SHADER_STAGE_FRAGMENT_BIT);
		groundMaterial->SetTexture(5, (&fbos[0])->depthBuffer);
		//groundMaterial->SetTexture(6, (&fbos[1])->depthBuffer);
		groundMaterial->SubmitUniformBuffers();
		groundPipeline = new XFixedPipeline;
		xSetColorAttachmentCount(groundPipeline, 1);
		groundPipeline->renderPass = xGetGlobalRenderPass();
		groundMaterial->SetFixedPipeline(groundPipeline);
		groundPipeline->viewport = { 0.0f, 0.0f, float(xGetViewportWidth()), float(xGetViewportHeight()), 0.0f, 1.0f };
		groundPipeline->scissor = { {0, 0}, { uint32_t(xGetViewportWidth()), uint32_t(xGetViewportHeight()) } };
		groundMaterial->Finish();
		ground = new Ground();
		ground->Init();
		ground->SetMaterial(groundMaterial);

		//depth render
		depthrenderMaterials = new Material * [LIGHT_COUNT];
		depthrenderPipelines = new XFixedPipeline * [LIGHT_COUNT];
		for (int i = 0; i < LIGHT_COUNT; ++i)
		{
			Material* depthrenderMaterial = new Material;
			depthrenderMaterials[i] = depthrenderMaterial;
			XFixedPipeline* depthrenderPipeline = new XFixedPipeline;
			depthrenderPipelines[i] = depthrenderPipeline;
			depthrenderMaterial->Init("Res/depthrender.vsb", "Res/depthrender.fsb");
			depthrenderMaterial->SetMVP(glm::mat4(1.0f), lights[i].view, camera->projection);
			depthrenderPipeline = new XFixedPipeline;
			xSetColorAttachmentCount(depthrenderPipeline, 1);
			depthrenderPipeline->renderPass = (&fbos[i])->renderPass;
			depthrenderMaterial->SetFixedPipeline(depthrenderPipeline);
			depthrenderPipeline->viewport = { 0.0f, 0.0f, float(xGetViewportWidth()), float(xGetViewportHeight()), 0.0f, 1.0f };
			depthrenderPipeline->scissor = { {0, 0}, { uint32_t(xGetViewportWidth()), uint32_t(xGetViewportHeight()) } };
			depthrenderMaterial->Finish();
			depthrenderMaterial->SubmitUniformBuffers();
		}

		skyboxMaterial = new Material;
		skyboxMaterial->Init("Res/skybox.vsb", "Res/skybox.fsb");
		skyboxMaterial->SetMVP(glm::mat4(1.0f), camera->GetViewMat(), camera->projection);
		skyboxMaterial->SetTexture(4, skybox);
		skyboxMaterial->SubmitUniformBuffers();
		skyboxPipeline = new XFixedPipeline;
		xSetColorAttachmentCount(skyboxPipeline, 1);
		skyboxPipeline->renderPass = xGetGlobalRenderPass();
		skyboxMaterial->SetFixedPipeline(skyboxPipeline);
		skyboxPipeline->viewport = { 0.0f, 0.0f, float(xGetViewportWidth()), float(xGetViewportHeight()), 0.0f, 1.0f };
		skyboxPipeline->scissor = { {0, 0}, { uint32_t(xGetViewportWidth()), uint32_t(xGetViewportHeight()) } };
		skyboxPipeline->rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
		skyboxMaterial->Finish();
		cube = new Model;
		cube->LoadObjModel("Res/model/Cube.obj");
		cube->SetMaterial(skyboxMaterial);

		//draw depth buffer
		//fsqMaterial = new Material;
		//fsqMaterial->Init("Res/renderdepth.vsb", "Res/renderdepth.fsb");
		//fsqMaterial->SetTexture(4, (&fbos[0])->depthBuffer);
		//fsqMaterial->SubmitUniformBuffers();
		//fsqPipeline = new XFixedPipeline;
		//xSetColorAttachmentCount(fsqPipeline, 1);
		//fsqPipeline->renderPass = xGetGlobalRenderPass();
		//fsqMaterial->SetFixedPipeline(fsqPipeline);
		//fsqPipeline->viewport = { 0.0f, 0.0f, float(xGetViewportWidth()), float(xGetViewportHeight()), 0.0f, 1.0f };
		//fsqPipeline->scissor = { {0, 0}, { uint32_t(xGetViewportWidth()), uint32_t(xGetViewportHeight()) } };
		//fsqPipeline->inputAssetmlyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		//fsqMaterial->Finish();
		//fsq = new FullScreenQuad;
		//fsq->Init();
		//fsq->material = fsqMaterial;
	}

	void Draw(float deltaTime)
	{
		sphereMaterial->SetMVP(sphereModelMat, camera->GetViewMat(), camera->projection);
		sphereMaterial->fragmentVector4UBO->SetVector4(0, camera->cameraPos.x, camera->cameraPos.y, camera->cameraPos.z, 1.0f);
		sphereMaterial->SubmitUniformBuffers();
		groundMaterial->SetMVP(groundModelMat, camera->GetViewMat(), camera->projection);
		groundMaterial->SubmitUniformBuffers();
		skyboxMaterial->SetMVP(glm::mat4(1.0f), camera->GetViewMat(), camera->projection);
		skyboxMaterial->vertexVector4UBO->SetVector4(0, camera->cameraPos.x, camera->cameraPos.y, camera->cameraPos.z, 1.0f);
		skyboxMaterial->SubmitUniformBuffers();

		VkCommandBuffer commandbuffer = 0;
		////depth pass
		for (int i = 0; i < LIGHT_COUNT; ++i)
		{
			commandbuffer = (&fbos[i])->BeginRendering(commandbuffer);
			ground->SetMaterial(depthrenderMaterials[i]);
			sphere->SetMaterial(depthrenderMaterials[i]);
			depthrenderMaterials[i]->SetMVP(groundModelMat, lights[i].view, lights[i].projection);
			depthrenderMaterials[i]->SubmitUniformBuffers();
			ground->Draw(commandbuffer);
			depthrenderMaterials[i]->SetMVP(sphereModelMat, lights[i].view, lights[i].projection);
			depthrenderMaterials[i]->SubmitUniformBuffers();
			sphere->Draw(commandbuffer);
			vkCmdEndRenderPass(commandbuffer);
		}

		//lighting pass
		commandbuffer = xBeginRendering(commandbuffer);
		ground->SetMaterial(groundMaterial);
		sphere->SetMaterial(sphereMaterial);
		ground->Draw(commandbuffer);
		sphere->Draw(commandbuffer);
		cube->Draw(commandbuffer);
		//fsq->Draw(commandbuffer);
		xEndRendering();
		xSwapBuffers(commandbuffer);
	}

	void OnViewportChanged(int width, int height)
	{
		Example::OnViewportChanged(width, height);
		skyboxPipeline->renderPass = xGetGlobalRenderPass();
		skyboxPipeline->viewport = { 0.0f,0.0f,float(xGetViewportWidth()),float(xGetViewportHeight()),0.0f,1.0f };
		skyboxPipeline->scissor = { {0,0},{uint32_t(xGetViewportWidth()),uint32_t(xGetViewportHeight())} };

		groundPipeline->renderPass = xGetGlobalRenderPass();
		groundPipeline->viewport = { 0.0f,0.0f,float(xGetViewportWidth()),float(xGetViewportHeight()),0.0f,1.0f };
		groundPipeline->scissor = { {0,0},{uint32_t(xGetViewportWidth()),uint32_t(xGetViewportHeight())} };

		spherePipeline->renderPass = xGetGlobalRenderPass();
		spherePipeline->viewport = { 0.0f,0.0f,float(xGetViewportWidth()),float(xGetViewportHeight()),0.0f,1.0f };
		spherePipeline->scissor = { {0,0},{uint32_t(xGetViewportWidth()),uint32_t(xGetViewportHeight())} };
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
		if (depthrenderPipelines != nullptr)
		{
			delete[] depthrenderPipelines;
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
		if (fbos != nullptr)
		{
			delete[] fbos;
		}
		if (lights != nullptr)
		{
			delete[] lights;
		}
		Example::OnQuit();
	}
};