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

Texture2D* texture;
Material* testMaterial;
XFixedPipeline* testPipeline;
Material* fsqMaterial;
XFixedPipeline* fsqPipeline;
FullScreenQuad* fsq;
Ground* ground;
Camera* camera;
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

	ground = new Ground();
	ground->Init();
	ground->SetMaterial(testMaterial);
}
void Draw(float deltaTime)
{
	VkCommandBuffer commandbuffer = xBeginRendering();

	//fsq->Draw(commandbuffer);
	ground->Draw(commandbuffer);
	
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
	if (fsq != nullptr)
	{
		delete fsq;
	}
	if (camera != nullptr)
	{
		delete camera;
	}
	Material::CleanUp();
	xVulkanCleanUp();
}