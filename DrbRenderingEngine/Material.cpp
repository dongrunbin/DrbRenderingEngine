#include "Material.h"
#include "VulkanAPI.h"

Material::Material()
{
	vertexMatrix4UBO = new UniformBuffer(XUniformBufferTypeMatrix);
	vertexVector4UBO = new UniformBuffer(XUniformBufferTypeVector);
	fragmentMatrix4UBO = new UniformBuffer(XUniformBufferTypeMatrix);
	fragmentVector4UBO = new UniformBuffer(XUniformBufferTypeVector);

	ubos.push_back(vertexMatrix4UBO);
	ubos.push_back(vertexVector4UBO);
	ubos.push_back(fragmentMatrix4UBO);
	ubos.push_back(fragmentVector4UBO);
}
Material::~Material()
{
	for (auto iter = ubos.begin(); iter != ubos.end(); ++iter)
	{
		delete (*iter);
	}
}
void Material::Init(const char* vertex, const char* fragment)
{
	VkShaderModule vs, fs;
	int file_len = 0;
	if (vertex != nullptr)
	{
		unsigned char* file_content = LoadFileContent(vertex, file_len);
		xCreateShader(vs, file_content, file_len);
		delete[]file_content;
		program.shaderStageCount = 1;
		xAttachVertexShader(&program, vs);
	}
	if (fragment != nullptr)
	{
		unsigned char* file_content = LoadFileContent(fragment, file_len);
		xCreateShader(fs, file_content, file_len);
		delete[]file_content;
		program.shaderStageCount = 2;
		xAttachFragmentShader(&program, fs);
	}

	vertexMatrix4UBO->SetSize(8);
	vertexVector4UBO->SetSize(8);
	fragmentMatrix4UBO->SetSize(8);
	fragmentVector4UBO->SetSize(8);
	xConfigUniformBuffer(&program, 0, vertexVector4UBO, VK_SHADER_STAGE_VERTEX_BIT);
	xConfigUniformBuffer(&program, 1, vertexMatrix4UBO, VK_SHADER_STAGE_VERTEX_BIT);
	xConfigUniformBuffer(&program, 2, fragmentVector4UBO, VK_SHADER_STAGE_FRAGMENT_BIT);
	xConfigUniformBuffer(&program, 3, fragmentMatrix4UBO, VK_SHADER_STAGE_FRAGMENT_BIT);
	sMaterials.insert(this);
}
void Material::SetUniformBuffer(int binding, void* data, int size, VkShaderStageFlagBits shaderStageFlags)
{
	UniformBuffer* ubo = new UniformBuffer(XUniformBufferTypeCustom);
	ubo->SetSize(size);
	ubo->SetData(data);
	xConfigUniformBuffer(&program, binding, ubo, shaderStageFlags);
	ubos.push_back(ubo);
}
void Material::SetTexture(int binding, XTexture* texture)
{
	xConfigSampler2D(&program, binding, texture->imageView, texture->sampler);
}
void Material::SetFixedPipeline(XFixedPipeline* p)
{
	fixedPipeline = p;
}
void Material::RebindTexture(int binding, XTexture* texture)
{
	xRebindSampler(&program, binding, texture->imageView, texture->sampler);
}
void Material::SetMVP(const glm::mat4& m, const glm::mat4& v, const glm::mat4& p)
{
	glm::mat4 it_m = glm::inverseTranspose(m);
	vertexMatrix4UBO->SetMatrix(0, m);
	vertexMatrix4UBO->SetMatrix(1, v);
	vertexMatrix4UBO->SetMatrix(2, p);
	vertexMatrix4UBO->SetMatrix(3, it_m);
}
void Material::Finish()
{
	xInitDescriptorSetLayout(&program);
	xInitDescriptorPool(&program);
	xInitDescriptorSet(&program);
	fixedPipeline->descriptorSetLayout = &program.descriptorSetLayout;
	fixedPipeline->descriptorSetLayoutCount = 1;
	fixedPipeline->shaderStages = program.shaderStages;
	fixedPipeline->shaderStageCount = program.shaderStageCount;
	xInitPipelineLayout(fixedPipeline);
	xCreateFixedPipeline(fixedPipeline);
}
void Material::SubmitUniformBuffers()
{
	for (auto iter = ubos.begin(); iter != ubos.end(); ++iter)
	{
		(*iter)->SubmitData();
	}
}
std::set<Material*> Material::sMaterials;
void Material::CleanUp()
{
	for (auto iter = sMaterials.begin(); iter != sMaterials.end(); ++iter)
	{
		delete (*iter);
	}
}