#include "FullScreenQuad.h"

FullScreenQuad::~FullScreenQuad() {
	delete vertexBuffer;
}
void FullScreenQuad::Init() {
	vertexBuffer = new VertexBuffer;
	vertexBuffer->SetSize(4);
	vertexBuffer->SetPosition(0, -1.0f, 1.0f, 0.0f);
	vertexBuffer->SetTexcoord(0, 0.0f, 0.0f);
	vertexBuffer->SetPosition(1, 1.0f, 1.0f, 0.0f);
	vertexBuffer->SetTexcoord(1, 1.0f, 0.0f);
	vertexBuffer->SetPosition(2, -1.0f, -1.0f, 0.0f);
	vertexBuffer->SetTexcoord(2, 0.0f, 1.0f);
	vertexBuffer->SetPosition(3, 1.0f, -1.0f, 0.0f);
	vertexBuffer->SetTexcoord(3, 1.0f, 1.0f);
	vertexBuffer->SubmitData();
}
void FullScreenQuad::Draw(VkCommandBuffer commandbuffer) {
	xSetDynamicState(material->fixedPipeline, commandbuffer);
	VkBuffer vertexbuffers[] = { vertexBuffer->buffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		material->fixedPipeline->mPipeline);
	vkCmdBindVertexBuffers(commandbuffer, 0, 1, vertexbuffers, offsets);
	vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		material->fixedPipeline->mPipelineLayout, 0, 1, &material->program.descriptorSet,
		0, nullptr);
	vkCmdDraw(commandbuffer, 4, 1, 0, 0);
}