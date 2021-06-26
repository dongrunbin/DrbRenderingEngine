#include "Ground.h"
Ground::~Ground() {
	delete vertexBuffer;
	delete indexBuffer;
}
void Ground::Init() {
	vertexBuffer = new VertexBuffer;
	vertexBuffer->SetSize(1600);
	indexBuffer = new IndexBuffer;
	indexBuffer->SetSize(2400);
	for (int z = 0; z < 20; ++z) {
		float zStart = 10.0f - z * 1.0f;
		for (int x = 0; x < 20; ++x) {
			int offset = (x + z * 20) * 4;
			float xStart = x * 1.0f - 10.0f;
			vertexBuffer->SetPosition(offset, xStart, -1.0f, zStart);
			vertexBuffer->SetPosition(offset + 1, xStart + 1.0f, -1.0f, zStart);
			vertexBuffer->SetPosition(offset + 2, xStart, -1.0f, zStart - 1.0f);
			vertexBuffer->SetPosition(offset + 3, xStart + 1.0f, -1.0f, zStart - 1.0f);
			vertexBuffer->SetNormal(offset, 0.0f, 1.0f, 0.0f);
			vertexBuffer->SetNormal(offset + 1, 0.0f, 1.0f, 0.0f);
			vertexBuffer->SetNormal(offset + 2, 0.0f, 1.0f, 0.0f);
			vertexBuffer->SetNormal(offset + 3, 0.0f, 1.0f, 0.0f);
			if ((x % 2) ^ (z % 2)) {
				vertexBuffer->SetTexcoord(offset, 0.1f, 0.1f, 0.1f, 0.1f);
				vertexBuffer->SetTexcoord(offset + 1, 0.1f, 0.1f, 0.1f, 0.1f);
				vertexBuffer->SetTexcoord(offset + 2, 0.1f, 0.1f, 0.1f, 0.1f);
				vertexBuffer->SetTexcoord(offset + 3, 0.1f, 0.1f, 0.1f, 0.1f);
			}
			else {
				vertexBuffer->SetTexcoord(offset, 0.9f, 0.9f, 0.9f, 0.1f);
				vertexBuffer->SetTexcoord(offset + 1, 0.9f, 0.9f, 0.9f, 0.1f);
				vertexBuffer->SetTexcoord(offset + 2, 0.9f, 0.9f, 0.9f, 0.1f);
				vertexBuffer->SetTexcoord(offset + 3, 0.9f, 0.9f, 0.9f, 0.1f);
			}
			indexBuffer->AppendIndex(offset);
			indexBuffer->AppendIndex(offset + 1);
			indexBuffer->AppendIndex(offset + 3);
			indexBuffer->AppendIndex(offset);
			indexBuffer->AppendIndex(offset + 3);
			indexBuffer->AppendIndex(offset + 2);
		}
	}
	vertexBuffer->SubmitData();
	indexBuffer->SubmitData();
}
void Ground::Draw(VkCommandBuffer commandbuffer) {
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
void Ground::SetMaterial(Material* material) {
	this->material = material;
}