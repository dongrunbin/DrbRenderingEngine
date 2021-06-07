#include "Material.h"
#include "BVulkan.h"

Material::Material()
{
	vertexMatrix4UBO = new UniformBuffer(XUniformBufferTypeMatrix);
	vertexVector4UBO = new UniformBuffer(XUniformBufferTypeVector);
	fragmentMatrix4UBO = new UniformBuffer(XUniformBufferTypeMatrix);
	fragmentVector4UBO = new UniformBuffer(XUniformBufferTypeVector);
}
Material::~Material()
{
	delete vertexMatrix4UBO;
	delete vertexVector4UBO;
	delete fragmentMatrix4UBO;
	delete fragmentVector4UBO;
}
void Material::Init(const char* vertex, const char* fragment)
{
	VkShaderModule vs, fs;
	int file_len = 0;
	unsigned char* file_content = LoadFileContent(vertex, file_len);
	xCreateShader(vs, file_content, file_len);
	delete[]file_content;
	program.shaderStageCount = 1;
	if (fragment != nullptr) {
		file_content = LoadFileContent(fragment, file_len);
		xCreateShader(fs, file_content, file_len);
		delete[]file_content;
		program.shaderStageCount = 2;
	}
	xAttachVertexShader(&program, vs);
	xAttachFragmentShader(&program, fs);
	vertexMatrix4UBO->SetSize(8);
	vertexVector4UBO->SetSize(8);
	fragmentMatrix4UBO->SetSize(8);
	fragmentVector4UBO->SetSize(8);
	xConfigUniformBuffer(&program, 0, vertexVector4UBO, VK_SHADER_STAGE_VERTEX_BIT);
	xConfigUniformBuffer(&program, 1, vertexMatrix4UBO, VK_SHADER_STAGE_VERTEX_BIT);
	xConfigUniformBuffer(&program, 2, fragmentVector4UBO, VK_SHADER_STAGE_FRAGMENT_BIT);
	xConfigUniformBuffer(&program, 3, fragmentMatrix4UBO, VK_SHADER_STAGE_FRAGMENT_BIT);
	xConfigSampler2D(&program, 4, xGetDefaultTexture()->imageView, xGetDefaultTexture()->sampler);
	xConfigSampler2D(&program, 5, xGetDefaultTexture()->imageView, xGetDefaultTexture()->sampler);
	xConfigSampler2D(&program, 6, xGetDefaultTexture()->imageView, xGetDefaultTexture()->sampler);
	xConfigSampler2D(&program, 7, xGetDefaultTexture()->imageView, xGetDefaultTexture()->sampler);
	xConfigSampler2D(&program, 8, xGetDefaultTexture()->imageView, xGetDefaultTexture()->sampler);
	xConfigSampler2D(&program, 9, xGetDefaultTexture()->imageView, xGetDefaultTexture()->sampler);
	xConfigSampler2D(&program, 10, xGetDefaultTexture()->imageView, xGetDefaultTexture()->sampler);
	xConfigSampler2D(&program, 11, xGetDefaultTexture()->imageView, xGetDefaultTexture()->sampler);
	xInitDescriptorSetLayout(&program);
	xInitDescriptorPool(&program);
	xInitDescriptorSet(&program);
	sMaterials.insert(this);
}
void Material::SetFixedPipeline(XFixedPipeline* p)
{
	fixedPipeline = p;
}
void Material::SetTexture(int binding, XTexture* texture)
{
	xRebindSampler(&program, binding + 4, texture->imageView, texture->sampler);
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
	fixedPipeline->mDescriptorSetLayout = &program.descriptorSetLayout;
	fixedPipeline->mDescriptorSetLayoutCount = 1;
	fixedPipeline->mShaderStages = program.shaderStages;
	fixedPipeline->mShaderStageCount = program.shaderStageCount;
	xInitPipelineLayout(fixedPipeline);
	xCreateFixedPipeline(fixedPipeline);
}
void Material::SubmitUniformBuffers()
{
	vertexMatrix4UBO->SubmitData();
	vertexVector4UBO->SubmitData();
	fragmentMatrix4UBO->SubmitData();
	fragmentVector4UBO->SubmitData();
}
std::set<Material*> Material::sMaterials;
void Material::CleanUp()
{
	for (auto iter = sMaterials.begin(); iter != sMaterials.end(); ++iter) {
		delete (*iter);
	}
}