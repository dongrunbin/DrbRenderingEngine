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

Texture2D* texture;
TextureCube* skybox;
Material* sphereMaterial;
XFixedPipeline* spherePipeline;
Material* depthrenderMaterial;
XFixedPipeline* depthrenderPipeline;
Material* fsqMaterial;
XFixedPipeline* fsqPipeline;
FullScreenQuad* fsq;
XFixedPipeline* groundPipeline;
Material* groundMaterial;
Ground* ground;
Camera* camera;
Model* sphere;
FrameBuffer* fbo;
void Init()
{
	camera = new Camera;
	camera->cameraPos = glm::vec3(0.0f, 5.0f, -15.0f);
	camera->Pitch(25.0f);
	glm::mat4 model(1.0);
	glm::mat4 projection = glm::perspective(45.0f, float(GetViewportWidth()) / float(GetViewportHeight()), 0.1f, 100.0f);
	projection[1][1] *= -1.0f;

	glm::vec3 lightPos(0.0f, 5.0f, 0.0f);
	glm::vec4 lightColor(10.0f, 10.0f, 10.0f, 1.0f);
	glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f));

	fbo = new FrameBuffer;
	fbo->SetSize(GetViewportWidth(), GetViewportHeight());
	fbo->AttachColorBuffer();
	fbo->AttachDepthBuffer();
	fbo->Finish();

	xInitDefaultTexture();

	// sphere
	sphereMaterial = new Material;
	sphereMaterial->Init("Res/sphere.vsb", "Res/sphere.fsb");
	sphereMaterial->SetMVP(model, camera->GetViewMat(), projection);
	sphereMaterial->fragmentVector4UBO->SetVector4(0, lightPos.x, lightPos.y, lightPos.z, 1.0f);
	sphereMaterial->fragmentVector4UBO->SetVector4(1, lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	sphereMaterial->fragmentVector4UBO->SetVector4(2, camera->cameraPos.x, camera->cameraPos.y, camera->cameraPos.z, 1.0f);
	sphereMaterial->SubmitUniformBuffers();
	spherePipeline = new XFixedPipeline;
	xSetColorAttachmentCount(spherePipeline, 1);
	spherePipeline->mRenderPass = GetGlobalRenderPass();
	sphereMaterial->SetFixedPipeline(spherePipeline);
	spherePipeline->mViewport = { 0.0f, 0.0f, float(GetViewportWidth()), float(GetViewportHeight()), 0.0f, 1.0f };
	spherePipeline->mScissor = { {0, 0}, { uint32_t(GetViewportWidth()), uint32_t(GetViewportHeight()) } };
	sphereMaterial->Finish();
	texture = new Texture2D;
	texture->SetImage("Res/test.bmp");
	skybox = new TextureCube;
	skybox->Init("");
	sphereMaterial->SetTexture(0, skybox);
	sphere = new Model;
	sphere->Init("Res/Sphere.raw");
	sphere->SetMaterial(sphereMaterial);

	// fsq
	fsqMaterial = new Material;
	fsqMaterial->Init("Res/renderdepth.vsb", "Res/renderdepth.fsb");
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
	fsqMaterial->SetTexture(0, fbo->attachments[1]);
	fsq->material = fsqMaterial;

	//ground
	groundMaterial = new Material;
	groundMaterial->Init("Res/ground.vsb", "Res/ground.fsb");
	groundMaterial->SetMVP(model, camera->GetViewMat(), projection);
	groundMaterial->fragmentVector4UBO->SetVector4(0, lightPos.x, lightPos.y, lightPos.z, 1.0f);
	groundMaterial->fragmentVector4UBO->SetVector4(1, lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	groundMaterial->fragmentVector4UBO->SetVector4(2, camera->cameraPos.x, camera->cameraPos.y, camera->cameraPos.z, 1.0f);
	groundMaterial->vertexMatrix4UBO->SetMatrix(4, projection);
	groundMaterial->vertexMatrix4UBO->SetMatrix(5, lightView);
	groundMaterial->SubmitUniformBuffers();
	groundPipeline = new XFixedPipeline;
	xSetColorAttachmentCount(groundPipeline, 1);
	groundPipeline->mRenderPass = GetGlobalRenderPass();
	groundMaterial->SetFixedPipeline(groundPipeline);
	groundPipeline->mViewport = { 0.0f, 0.0f, float(GetViewportWidth()), float(GetViewportHeight()), 0.0f, 1.0f };
	groundPipeline->mScissor = { {0, 0}, { uint32_t(GetViewportWidth()), uint32_t(GetViewportHeight()) } };
	groundMaterial->SetTexture(0, fbo->depthBuffer);
	groundMaterial->Finish();
	ground = new Ground();
	ground->Init();
	ground->SetMaterial(groundMaterial);

	//depth render
	depthrenderMaterial = new Material;
	depthrenderMaterial->Init("Res/depthrender.vsb", "Res/depthrender.fsb");
	depthrenderMaterial->SetMVP(model, lightView, projection);
	depthrenderMaterial->SubmitUniformBuffers();
	depthrenderPipeline = new XFixedPipeline;
	xSetColorAttachmentCount(depthrenderPipeline, 1);
	depthrenderPipeline->mRenderPass = fbo->renderPass;
	depthrenderMaterial->SetFixedPipeline(depthrenderPipeline);
	depthrenderPipeline->mViewport = { 0.0f, 0.0f, float(GetViewportWidth()), float(GetViewportHeight()), 0.0f, 1.0f };
	depthrenderPipeline->mScissor = { {0, 0}, { uint32_t(GetViewportWidth()), uint32_t(GetViewportHeight()) } };
	depthrenderMaterial->Finish();
}
void Draw(float deltaTime)
{
	VkCommandBuffer commandbuffer = fbo->BeginRendering();
	ground->SetMaterial(depthrenderMaterial);
	sphere->SetMaterial(depthrenderMaterial);
	ground->Draw(commandbuffer);
	sphere->Draw(commandbuffer);
	vkCmdEndRenderPass(commandbuffer);



	xBeginRendering(commandbuffer);
	//fsq->Draw(commandbuffer);
	ground->SetMaterial(groundMaterial);
	sphere->SetMaterial(sphereMaterial);
	ground->Draw(commandbuffer);
	sphere->Draw(commandbuffer);
	xEndRendering();
	xSwapBuffers(commandbuffer);
}
void OnViewportChanged(int width, int height)
{
	aViewport(width, height);
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
	Material::CleanUp();
	xVulkanCleanUp();
}