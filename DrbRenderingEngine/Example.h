#pragma once
#include "VulkanAPI.h"
#include "Camera.h"
#include "Material.h"

class Example
{
protected:
	float moveSpeed = 0.4f;
	Camera* camera;
public:
	virtual void Init() = 0;
	virtual void Draw(float deltaTime) = 0;
	virtual void OnViewportChanged(int width, int height)
	{
		xViewport(width, height);
	}
	virtual void OnKeyboard(unsigned char key)
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
	virtual void OnMouseMove(int deltaX, int deltaY)
	{
		float angleX = (float)deltaX / 100.0f;
		float angleY = (float)deltaY / 100.0f;
		camera->Yaw(-angleX);
		camera->Pitch(-angleY);
	}
	virtual void OnQuit()
	{
		Material::CleanUp();
		xVulkanCleanUp();
	}
};
