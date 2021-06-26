#include "Model.h"
Model::~Model()
{
	delete vertexBuffer;
	delete indexBuffer;
}

void Model::Init(const char* path)
{
	vertexBuffer = new VertexBuffer;
	indexBuffer = new IndexBuffer;
	FILE* file = fopen(path, "rb");
	fread(&vertexBuffer->vertexCount, 1, sizeof(int), file);
	vertexBuffer->SetSize(vertexBuffer->vertexCount);
	fread(vertexBuffer->vertices, sizeof(XVertexData), vertexBuffer->vertexCount, file);
	fread(&indexBuffer->indexCount, 1, sizeof(int), file);
	indexBuffer->SetSize(indexBuffer->indexCount);
	fread(indexBuffer->indices, sizeof(unsigned int), indexBuffer->indexCount, file);
	fclose(file);
	vertexBuffer->SubmitData();
	indexBuffer->SubmitData();
}

void Model::Draw(VkCommandBuffer commandbuffer)
{
	xSetDynamicState(material->fixedPipeline, commandbuffer);
	VkBuffer vertexbuffers[] = { vertexBuffer->buffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		material->fixedPipeline->pipeline);
	vkCmdBindVertexBuffers(commandbuffer, 0, 1, vertexbuffers, offsets);
	vkCmdBindIndexBuffer(commandbuffer, indexBuffer->buffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		material->fixedPipeline->pipelineLayout, 0, 1, &material->program.descriptorSet,
		0, nullptr);
	vkCmdDrawIndexed(commandbuffer, indexBuffer->indexCount, 1, 0, 0, 0);
}

void Model::SetMaterial(Material* material)
{
	this->material = material;
}