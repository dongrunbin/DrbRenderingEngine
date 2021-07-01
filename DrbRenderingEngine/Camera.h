#pragma once
#include <glm/glm.hpp>

class Camera
{
public:
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, -6.0f);
	glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::mat4 projection;
public:
	Camera(int width, int height);
	glm::mat4 GetViewMat();
	glm::vec3 GetViewCenter();
	void RotateView(float angle, float x, float y, float z);
	void Pitch(float angle);
	void Yaw(float angle);
};