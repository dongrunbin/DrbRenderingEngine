#pragma once
#include "VulkanAPI.h"
#include "Texture2D.h"
#include "UniformBuffer.h"
#include <vector>

class Material
{
public:
	XProgram program;
	XFixedPipeline* fixedPipeline;
	UniformBuffer* vertexVector4UBO, * vertexMatrix4UBO, * fragmentVector4UBO, * fragmentMatrix4UBO;
	std::vector<UniformBuffer*> ubos;
	XTexture texture[8];
public:
	Material();
	~Material();
	void Init(const char* vs, const char* fs = nullptr);
	void SetUniformBuffer(int binding, void* data, int size, VkShaderStageFlagBits shaderStageFlags);
	void SetTexture(int binding, XTexture* texture);
	void SetFixedPipeline(XFixedPipeline* p);
	void RebindTexture(int binding, XTexture* texture);
	void SetMVP(const glm::mat4& m, const glm::mat4& v, const glm::mat4& p);
	void Finish();
	void SubmitUniformBuffers();
	static std::set<Material*> sMaterials;
	static void CleanUp();
};