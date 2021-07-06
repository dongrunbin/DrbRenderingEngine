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
#include "Terrain.h"

class DeferredShadowMapping : public Example
{
private:
	TextureCube* skybox;
	Material* sphereMaterial;
	XFixedPipeline* spherePipeline;
	Model* sphere;
	glm::mat4 sphereModelMat;

	Material* groundMaterial;
	XFixedPipeline* groundPipeline;
	Ground* ground;
	glm::mat4 groundModelMat;

	Material** depthrenderMaterials;
	XFixedPipeline** depthrenderPipelines;
	Material* fsqMaterial;
	XFixedPipeline* fsqPipeline;
	FullScreenQuad* fsq;

	FrameBuffer* fbos;
	FrameBuffer* gbuffer;

	Light* lights;
	const int LIGHT_COUNT = 2;

	Material* skyboxMaterial;
	XFixedPipeline* skyboxPipeline;
	Model* cube;

	Terrain* terrain;
	Material* terrainMaterial;
	XFixedPipeline* terrainPipeline;
	Texture2D* grassTexture;
	Texture2D* stoneTexture;
	Texture2D* terrainTexture;
	glm::mat4 terrainModelMat;

	Material* fsq2Material;
	XFixedPipeline* fsq2Pipeline;
	FullScreenQuad* fsq2;
public:
	void Init()
	{
		camera = new Camera(xGetViewportWidth(), xGetViewportHeight());
		camera->cameraPos = glm::vec3(0.0f, 10.0f, -15.0f);
		camera->Pitch(25.0f);
		sphereModelMat = glm::mat4(1.0f) * glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 3.0f, 0.0f));
		groundModelMat = glm::mat4(1.0f) * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		terrainModelMat = glm::mat4(1.0f) * glm::translate(glm::mat4(1.0f), glm::vec3(20.0f, -5.0f, 0.0f));

		lights = new Light[LIGHT_COUNT];
		//lights[0].pos = glm::vec4(0.0f, 5.0f, 0.0f, 1.0f);
		//lights[0].color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		//lights[0].view = glm::lookAt(glm::vec3(lights[0].pos), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		//lights[0].projection = glm::perspective(glm::radians(90.0f), (float)xGetViewportWidth() / (float)xGetViewportHeight(), 0.1f, 15.0f);
		//lights[1].pos = glm::vec4(5.0f, 5.0f, 5.0f, 1.0f);
		//lights[1].color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		//lights[1].view = glm::lookAt(glm::vec3(lights[1].pos), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		//lights[1].projection = glm::perspective(glm::radians(90.0f), (float)xGetViewportWidth() / (float)xGetViewportHeight(), 0.1f, 15.0f);

		//point light
		lights[0].pos = glm::vec4(-4.0f, 4.0f, 0.0f, 1.0f);
		lights[0].diffuseColor = glm::vec4(3.0f, 3.0f, 0.0f, 1.0f);
		lights[0].specularColor = glm::vec4(3.0f, 3.0f, 0.0f, 1.0f);
		//shadow
		lights[0].view = glm::lookAt(glm::vec3(lights[0].pos), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		lights[0].projection = glm::perspective(glm::radians(90.0f), (float)xGetViewportWidth() / (float)xGetViewportHeight(), 0.1f, 30.0f);
		//direction light
		lights[1].pos = glm::vec4(5.0f, 5.0f, 0.0f, 0.0f);
		lights[1].diffuseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		lights[1].specularColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		//shadow
		lights[1].view = glm::lookAt(glm::vec3(lights[1].pos), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		lights[1].projection = glm::perspective(glm::radians(90.0f), (float)xGetViewportWidth() / (float)xGetViewportHeight(), 0.1f, 100.0f);
		//lights[1].projection = glm::ortho(0.0f, 
		//	100.0f * xGetViewportWidth() / xGetViewportHeight(), 100.0f, 0.0f, 0.1f, 100.0f);
		//spot light
		//lights[2].pos = glm::vec4(0.0f, 3.0f, 0.0f, 1.0f);
		//lights[2].diffuseColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		//lights[2].specularColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		//lights[2].cutoff = 15.0f;
		//lights[2].direction = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
		//lights[2].spotLightPower = 128.0f;
		////shadow
		//lights[2].view = glm::lookAt(glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		//lights[2].projection = glm::perspective(glm::radians(90.0f), (float)xGetViewportWidth() / (float)xGetViewportHeight(), 0.1f, 15.0f);

		fbos = new FrameBuffer[LIGHT_COUNT];
		for (int i = 0; i < LIGHT_COUNT; ++i)
		{
			FrameBuffer* fbo = &fbos[i];
			fbo->SetSize(xGetViewportWidth(), xGetViewportHeight());
			fbo->AttachColorBuffer();
			fbo->AttachDepthBuffer();
			fbo->Finish();
		}


		gbuffer = new FrameBuffer;
		gbuffer->SetSize(xGetViewportWidth(), xGetViewportHeight());
		//要在gbuffer里存储世界坐标系里的position/normal/texcolor
		gbuffer->AttachColorBuffer(VK_FORMAT_R32G32B32A32_SFLOAT);
		gbuffer->AttachColorBuffer(VK_FORMAT_R32G32B32A32_SFLOAT);
		gbuffer->AttachColorBuffer();
		gbuffer->AttachDepthBuffer();
		gbuffer->Finish();

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
		sphereMaterial->Init("Res/sphere.vsb", "Res/sphere.fsb");
		sphereMaterial->SetMVP(sphereModelMat, camera->GetViewMat(), camera->projection);
		sphereMaterial->fragmentVector4UBO->SetVector4(0, camera->cameraPos.x, camera->cameraPos.y, camera->cameraPos.z, 1.0f);
		sphereMaterial->SetTexture(5, skybox);
		sphereMaterial->SubmitUniformBuffers();
		spherePipeline = new XFixedPipeline;
		xSetColorAttachmentCount(spherePipeline, 3);
		spherePipeline->renderPass = gbuffer->renderPass;
		sphereMaterial->SetFixedPipeline(spherePipeline);
		spherePipeline->viewport = { 0.0f, 0.0f, float(xGetViewportWidth()), float(xGetViewportHeight()), 0.0f, 1.0f };
		spherePipeline->scissor = { {0, 0}, { uint32_t(xGetViewportWidth()), uint32_t(xGetViewportHeight()) } };
		sphereMaterial->Finish();
		sphere = new Model;
		sphere->Init("Res/model/Sphere.raw");
		sphere->SetMaterial(sphereMaterial);

		//ground
		groundMaterial = new Material;
		groundMaterial->Init("Res/ground.vsb", "Res/ground.fsb");
		groundMaterial->SetMVP(groundModelMat, camera->GetViewMat(), camera->projection);
		groundMaterial->SubmitUniformBuffers();
		groundPipeline = new XFixedPipeline;
		xSetColorAttachmentCount(groundPipeline, 3);
		groundPipeline->renderPass = gbuffer->renderPass;
		groundMaterial->SetFixedPipeline(groundPipeline);
		groundPipeline->viewport = { 0.0f, 0.0f, float(xGetViewportWidth()), float(xGetViewportHeight()), 0.0f, 1.0f };
		groundPipeline->scissor = { {0, 0}, { uint32_t(xGetViewportWidth()), uint32_t(xGetViewportHeight()) } };
		groundMaterial->Finish();
		ground = new Ground();
		ground->Init();
		ground->SetMaterial(groundMaterial);

		// fsq
		fsqMaterial = new Material;
		fsqMaterial->Init("Res/deferred.vsb", "Res/deferred.fsb");
		fsqMaterial->SetUniformBuffer(4, lights, sizeof(Light), VK_SHADER_STAGE_FRAGMENT_BIT);
		fsqMaterial->SetTexture(5, gbuffer->attachments[0]);
		fsqMaterial->SetTexture(6, gbuffer->attachments[1]);
		fsqMaterial->SetTexture(7, gbuffer->attachments[2]);
		fsqMaterial->SetTexture(8, (&fbos[0])->depthBuffer);
		fsqMaterial->SetTexture(9, (&fbos[1])->depthBuffer);
		fsqMaterial->SetTexture(10, gbuffer->depthBuffer);
		fsqMaterial->SetUniformBuffer(11, &lights[1], sizeof(Light), VK_SHADER_STAGE_FRAGMENT_BIT);
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
		fsq->material = fsqMaterial;

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
			depthrenderMaterial->SetMVP(glm::mat4(1.0f), lights[i].view, lights[i].projection);
			depthrenderPipeline = new XFixedPipeline;
			xSetColorAttachmentCount(depthrenderPipeline, 1);
			depthrenderPipeline->renderPass = (&fbos[i])->renderPass;
			depthrenderMaterial->SetFixedPipeline(depthrenderPipeline);
			depthrenderPipeline->viewport = { 0.0f, 0.0f, float(xGetViewportWidth()), float(xGetViewportHeight()), 0.0f, 1.0f };
			depthrenderPipeline->scissor = { {0, 0}, { uint32_t(xGetViewportWidth()), uint32_t(xGetViewportHeight()) } };
			depthrenderPipeline->rasterizer.depthBiasConstantFactor = 1.25f;
			depthrenderPipeline->rasterizer.depthBiasSlopeFactor = 1.75f;
			depthrenderMaterial->Finish();
			depthrenderMaterial->SubmitUniformBuffers();
		}

		//skybox
		skyboxMaterial = new Material;
		skyboxMaterial->Init("Res/skybox.vsb", "Res/skybox.fsb");
		skyboxMaterial->SetMVP(glm::mat4(1.0f), camera->GetViewMat(), camera->projection);
		skyboxMaterial->SetTexture(4, skybox);
		skyboxMaterial->SetTexture(5, gbuffer->depthBuffer);
		skyboxMaterial->SubmitUniformBuffers();
		skyboxPipeline = new XFixedPipeline;
		xSetColorAttachmentCount(skyboxPipeline, 1);
		skyboxPipeline->renderPass = xGetGlobalRenderPass();
		skyboxMaterial->SetFixedPipeline(skyboxPipeline);
		skyboxPipeline->viewport = { 0.0f, 0.0f, float(xGetViewportWidth()), float(xGetViewportHeight()), 0.0f, 1.0f };
		skyboxPipeline->scissor = { {0, 0}, { uint32_t(xGetViewportWidth()), uint32_t(xGetViewportHeight()) } };
		skyboxPipeline->rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
		skyboxPipeline->depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		skyboxMaterial->Finish();
		cube = new Model;
		cube->LoadObjModel("Res/model/Cube.obj");
		cube->SetMaterial(skyboxMaterial);


		//terrain
		grassTexture = new Texture2D;
		grassTexture->SetImage("Res/images/grass.bmp");
		stoneTexture = new Texture2D;
		stoneTexture->SetImage("Res/images/stone.bmp");
		terrainTexture = new Texture2D;
		terrainTexture->SetImage("Res/images/terrain.bmp");
		terrainMaterial = new Material;
		terrainMaterial->Init("Res/terrain.vsb", "Res/terrain.fsb");
		terrainMaterial->SetMVP(terrainModelMat, camera->GetViewMat(), camera->projection);
		terrainMaterial->fragmentVector4UBO->SetVector4(0, camera->cameraPos.x, camera->cameraPos.y, camera->cameraPos.z, 1.0f);
		terrainMaterial->SetTexture(4, grassTexture);
		terrainMaterial->SetTexture(5, terrainTexture);
		terrainMaterial->SetTexture(6, stoneTexture);
		terrainMaterial->SubmitUniformBuffers();
		terrainPipeline = new XFixedPipeline;
		xSetColorAttachmentCount(terrainPipeline, 3);
		terrainPipeline->renderPass = gbuffer->renderPass;
		terrainMaterial->SetFixedPipeline(terrainPipeline);
		terrainPipeline->viewport = { 0.0f, 0.0f, float(xGetViewportWidth()), float(xGetViewportHeight()), 0.0f, 1.0f };
		terrainPipeline->scissor = { {0, 0}, { uint32_t(xGetViewportWidth()), uint32_t(xGetViewportHeight()) } };
		terrainMaterial->Finish();
		terrain = new Terrain();
		terrain->Init("Res/images/height.bmp");
		terrain->SetMaterial(terrainMaterial);



		//draw depth buffer
		fsq2Material = new Material;
		fsq2Material->Init("Res/renderdepth.vsb", "Res/renderdepth.fsb");
		fsq2Material->SetTexture(4, fbos[1].depthBuffer);
		fsq2Material->SubmitUniformBuffers();
		fsq2Pipeline = new XFixedPipeline;
		xSetColorAttachmentCount(fsq2Pipeline, 1);
		fsq2Pipeline->renderPass = xGetGlobalRenderPass();
		fsq2Material->SetFixedPipeline(fsq2Pipeline);
		fsq2Pipeline->viewport = { 0.0f, 0.0f, float(xGetViewportWidth()), float(xGetViewportHeight()), 0.0f, 1.0f };
		fsq2Pipeline->scissor = { {0, 0}, { uint32_t(xGetViewportWidth()), uint32_t(xGetViewportHeight()) } };
		fsq2Pipeline->inputAssetmlyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		fsq2Material->Finish();
		fsq2 = new FullScreenQuad;
		fsq2->Init();
		fsq2->material = fsq2Material;

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
		terrainMaterial->SetMVP(terrainModelMat, camera->GetViewMat(), camera->projection);
		terrainMaterial->SubmitUniformBuffers();

		////depth pass
		VkCommandBuffer commandbuffer = 0;
		for (int i = 0; i < LIGHT_COUNT; ++i)
		{
			commandbuffer = (&fbos[i])->BeginRendering(commandbuffer);
			ground->SetMaterial(depthrenderMaterials[i]);
			sphere->SetMaterial(depthrenderMaterials[i]);
			terrain->SetMaterial(depthrenderMaterials[i]);

			//vkCmdSetPrimitiveTopologyEXT(commandbuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
			depthrenderMaterials[i]->SetMVP(terrainModelMat, lights[i].view, lights[i].projection);
			depthrenderMaterials[i]->SubmitUniformBuffers();
			terrain->Draw(commandbuffer);
			depthrenderMaterials[i]->SetMVP(sphereModelMat, lights[i].view, lights[i].projection);
			depthrenderMaterials[i]->SubmitUniformBuffers();
			sphere->Draw(commandbuffer);
			//depthrenderMaterials[i]->SetMVP(groundModelMat, lights[i].view, lights[i].projection);
			//depthrenderMaterials[i]->SubmitUniformBuffers();
			//ground->Draw(commandbuffer);

			//vkCmdSetPrimitiveTopologyEXT(commandbuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
			
			vkCmdEndRenderPass(commandbuffer);
		}

		//geometry pass
		gbuffer->BeginRendering(commandbuffer);
		ground->SetMaterial(groundMaterial);
		sphere->SetMaterial(sphereMaterial);
		terrain->SetMaterial(terrainMaterial);
		//ground->Draw(commandbuffer);
		sphere->Draw(commandbuffer);
		terrain->Draw(commandbuffer);
		vkCmdEndRenderPass(commandbuffer);
		//lighting pass
		xBeginRendering(commandbuffer);
		fsq->Draw(commandbuffer);
		cube->Draw(commandbuffer);
		xEndRendering();
		xSwapBuffers(commandbuffer);
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
		if (gbuffer != nullptr)
		{
			delete gbuffer;
		}
		if (lights != nullptr)
		{
			delete[] lights;
		}
		Example::OnQuit();
	}
};