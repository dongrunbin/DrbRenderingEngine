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

class ExampleLighting : public Example
{
private:
	TextureCube* skybox;
	Material* sphereMaterial;
	XFixedPipeline* spherePipeline;
	Material* groundMaterial;
	XFixedPipeline* groundPipeline;
	Ground* ground;
	Model* sphere;
	glm::mat4 model;
	glm::mat4 projection;
	Light* lights;
	const int LIGHT_COUNT = 1;
public:
	void Init()
	{
		camera = new Camera;
		camera->cameraPos = glm::vec3(0.0f, 5.0f, -15.0f);
		camera->Pitch(25.0f);
		model = glm::mat4(1.0f);
		projection = glm::perspective(45.0f, float(xGetViewportWidth()) / float(xGetViewportHeight()), 0.1f, 100.0f);
		projection[1][1] *= -1.0f;

		lights = new Light[LIGHT_COUNT];
		lights[0].pos = glm::vec4(0.0f, 5.0f, 0.0f, 1.0f);
		lights[0].color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		lights[0].view = glm::lookAt(glm::vec3(lights[0].pos), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		lights[0].projection = projection;

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
		sphereMaterial->SetMVP(model, camera->GetViewMat(), projection);
		sphereMaterial->fragmentVector4UBO->SetVector4(0, camera->cameraPos.x, camera->cameraPos.y, camera->cameraPos.z, 1.0f);
		sphereMaterial->SetUniformBuffer(4, lights, LIGHT_COUNT * sizeof(Light), VK_SHADER_STAGE_FRAGMENT_BIT);
		sphereMaterial->SetTexture(5, skybox);
		sphereMaterial->SubmitUniformBuffers();
		spherePipeline = new XFixedPipeline;
		xSetColorAttachmentCount(spherePipeline, 3);
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
		groundMaterial->Init("Res/ground_lighting.vsb", "Res/ground_lighting.fsb");
		groundMaterial->SetMVP(model, camera->GetViewMat(), projection);
		groundMaterial->fragmentVector4UBO->SetVector4(0, camera->cameraPos.x, camera->cameraPos.y, camera->cameraPos.z, 1.0f);
		groundMaterial->SetUniformBuffer(4, lights, LIGHT_COUNT * sizeof(Light), VK_SHADER_STAGE_FRAGMENT_BIT);
		groundMaterial->SubmitUniformBuffers();
		groundPipeline = new XFixedPipeline;
		xSetColorAttachmentCount(groundPipeline, 3);
		groundPipeline->renderPass = xGetGlobalRenderPass();
		groundMaterial->SetFixedPipeline(groundPipeline);
		groundPipeline->viewport = { 0.0f, 0.0f, float(xGetViewportWidth()), float(xGetViewportHeight()), 0.0f, 1.0f };
		groundPipeline->scissor = { {0, 0}, { uint32_t(xGetViewportWidth()), uint32_t(xGetViewportHeight()) } };
		groundMaterial->Finish();
		ground = new Ground();
		ground->Init();
		ground->SetMaterial(groundMaterial);
	}
	void Draw(float deltaTime)
	{
		for (auto it = Material::sMaterials.begin(); it != Material::sMaterials.end(); it++)
		{
			(*it)->SetMVP(model, camera->GetViewMat(), projection);
			(*it)->fragmentVector4UBO->SetVector4(0, camera->cameraPos.x, camera->cameraPos.y, camera->cameraPos.z, 1.0f);
			(*it)->vertexVector4UBO->SetVector4(0, camera->cameraPos.x, camera->cameraPos.y, camera->cameraPos.z, 1.0f);
			(*it)->SubmitUniformBuffers();
		}
		//lighting pass
		VkCommandBuffer commandbuffer = xBeginRendering();
		sphere->Draw(commandbuffer);
		ground->Draw(commandbuffer);
		xEndRendering();
		xSwapBuffers(commandbuffer);
	}

	void OnQuit()
	{
		if (spherePipeline != nullptr)
		{
			delete spherePipeline;
		}
		if (ground != nullptr)
		{
			delete ground;
		}
		if (groundPipeline != nullptr)
		{
			delete groundPipeline;
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
		if (lights != nullptr)
		{
			delete[] lights;
		}
		Example::OnQuit();
	}
};