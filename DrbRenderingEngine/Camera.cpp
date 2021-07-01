#include "Camera.h"
#include <glm/ext.hpp>

Camera::Camera(int width, int height)
{
	projection = glm::perspective(45.0f, float(width) / float(height), 0.1f, 1000.0f);
	projection[1][1] *= -1.0f;
}

glm::mat4 Camera::GetViewMat()
{
	return glm::lookAt(cameraPos, GetViewCenter(), up);
}

glm::vec3 Camera::GetViewCenter()
{
	return cameraPos + forward;
}

void Camera::RotateView(float angle, float x, float y, float z)
{
	glm::vec3 newForward;
	float C = cosf(angle);
	float S = sinf(angle);

	glm::vec3 tempX(C + x * x * (1 - C), x * y * (1 - C) - z * S, x * z * (1 - C) + y * S);
	newForward.x = glm::dot(tempX, forward);

	glm::vec3 tempY(x * y * (1 - C) + z * S, C + y * y * (1 - C), y * z * (1 - C) - x * S);
	newForward.y = glm::dot(tempY, forward);

	glm::vec3 tempZ(x * z * (1 - C) - y * S, y * z * (1 - C) + x * S, C + z * z * (1 - C));
	newForward.z = glm::dot(tempZ, forward);

	forward = newForward;
}

void Camera::Pitch(float angle)
{
	forward = glm::normalize(forward);
	glm::vec3 right = glm::cross(forward, up);
	right = glm::normalize(right);
	RotateView(angle, right.x, right.y, right.z);
}

void Camera::Yaw(float angle)
{
	RotateView(angle, 0.0f, 1.0f, 0.0f);
}