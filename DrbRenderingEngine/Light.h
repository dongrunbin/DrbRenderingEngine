#pragma once
#include <glm/glm.hpp>

struct Light
{
public:
	glm::vec4 pos = glm::vec4(0.0f);
	glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::mat4 projection = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
};