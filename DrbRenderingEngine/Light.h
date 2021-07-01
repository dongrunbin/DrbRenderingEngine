#pragma once
#include <glm/glm.hpp>


struct BasicLight
{
public:
	glm::vec4 pos = glm::vec4(0.0f);
	glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::mat4 projection = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
};
struct Light
{
public:
	glm::mat4 projection = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
	glm::vec4 pos = glm::vec4(0.0f);
	glm::vec4 ambientColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 diffuseColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 specularColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 direction = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	float cutoff = 15.0f;
	float spotLightPower = 0.0f;
};