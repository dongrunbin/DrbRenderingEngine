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

Texture2D* texture;
Material* testMaterial;
XFixedPipeline* testPipeline;
Material* fsqMaterial;
XFixedPipeline* fsqPipeline;
FullScreenQuad* fsq;
XFixedPipeline* groundPipeline;
Material* groundMaterial;
Ground* ground;
Camera* camera;
Model* sphere;
void Init()
{
	camera = new Camera;
	camera->cameraPos = glm::vec3(0.0f, 5.0f, -20.0f);
	camera->Pitch(25.0f);
	
	xInitDefaultTexture();
	testMaterial = new Material;
	testMaterial->Init("Res/test.vsb", "Res/test.fsb");
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f)) * glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 projection = glm::perspective(45.0f, float(GetViewportWidth()) / float(GetViewportHeight()), 0.1f, 60.0f);
	projection[1][1] *= -1.0f;
	testMaterial->SetMVP(model, camera->GetViewMat(), projection);
	testMaterial->SubmitUniformBuffers();
	testPipeline = new XFixedPipeline;
	xSetColorAttachmentCount(testPipeline, 1);
	testPipeline->mRenderPass = GetGlobalRenderPass();
	testMaterial->SetFixedPipeline(testPipeline);
	testPipeline->mViewport = { 0.0f, 0.0f, float(GetViewportWidth()), float(GetViewportHeight()), 0.0f, 1.0f };
	testPipeline->mScissor = { {0, 0}, { uint32_t(GetViewportWidth()), uint32_t(GetViewportHeight()) } };
	testMaterial->Finish();
	texture = new Texture2D;
	texture->SetImage("Res/test.bmp");
	testMaterial->SetTexture(0, texture);

	// fsq
	fsqMaterial = new Material;
	fsqMaterial->Init("Res/fsq.vsb", "Res/fsq.fsb");
	fsqMaterial->SetMVP(model, camera->GetViewMat(), projection);
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
	fsqMaterial->SetTexture(0, texture);
	fsq->material = fsqMaterial;

	//ground
	groundMaterial = new Material;
	groundMaterial->Init("Res/ground.vsb", "Res/ground.fsb");
	groundMaterial->SetMVP(model, camera->GetViewMat(), projection);
	groundMaterial->fragmentVector4UBO->SetVector4(0, 0.0f, 5.0f, 0.0f, 1.0f);
	groundMaterial->fragmentVector4UBO->SetVector4(1, 1.0f, 1.0f, 1.0f, 1.0f);
	groundMaterial->fragmentVector4UBO->SetVector4(2, camera->cameraPos.x, camera->cameraPos.y, camera->cameraPos.z, 1.0f);
	groundMaterial->SubmitUniformBuffers();
	groundPipeline = new XFixedPipeline;
	xSetColorAttachmentCount(groundPipeline, 1);
	groundPipeline->mRenderPass = GetGlobalRenderPass();
	groundMaterial->SetFixedPipeline(groundPipeline);
	groundPipeline->mViewport = { 0.0f, 0.0f, float(GetViewportWidth()), float(GetViewportHeight()), 0.0f, 1.0f };
	groundPipeline->mScissor = { {0, 0}, { uint32_t(GetViewportWidth()), uint32_t(GetViewportHeight()) } };
	groundMaterial->Finish();
	ground = new Ground();
	ground->Init();
	ground->SetMaterial(groundMaterial);

	sphere = new Model;
	sphere->Init("Res/Sphere.raw");
	sphere->SetMaterial(testMaterial);
}
void Draw(float deltaTime)
{
	VkCommandBuffer commandbuffer = xBeginRendering();

	//fsq->Draw(commandbuffer);
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
	if (testPipeline != nullptr)
	{
		delete testPipeline;
	}
	if (fsqPipeline != nullptr)
	{
		delete fsqPipeline;
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
	Material::CleanUp();
	xVulkanCleanUp();
}