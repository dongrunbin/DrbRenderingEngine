#include "Terrain.h"

float Terrain::GetHeight(int x, int z)
{
	x = x < 0 ? 0 : x;
	x = x > 255 ? 255 : x;
	z = z < 0 ? 0 : z;
	z = z > 255 ? 255 : z;
	return float(heightMapDatas[(x + z * heightMapWidth) * 3]) / 255.0f;
}

void Terrain::GetNormal(int x, int z, float* normal)
{
	float heightL = GetHeight(x - 1, z);
	float heightR = GetHeight(x + 1, z);
	float heightU = GetHeight(x, z + 1);
	float heightD = GetHeight(x, z - 1);
	normal[0] = heightL - heightR;
	normal[1] = 2.0f;
	normal[2] = heightD - heightU;
	float len = sqrtf(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
	normal[0] /= len;
	normal[1] /= len;
	normal[2] /= len;
}

void Terrain::Init(const char* heightMapPath)
{
	vertexBuffer = new VertexBuffer;
	vertexBuffer->SetSize(256 * 256 * 4);
	indexBuffer = new IndexBuffer;
	indexBuffer->SetSize(256 * 256 * 6);
	unsigned int* temp = indexBuffer->indices;
	int image_width, image_height, channel;
	heightMapDatas = LoadImageFromFile(heightMapPath, image_width, image_height, channel, 3);

	float normal[3];
	for (int z = 0; z < 256; ++z) {
		for (int x = 0; x < 256; ++x) {
			float start_z = 128.0f - z;
			float start_x = -128.0f + x;
			float height = 10.0f;
			int quad_index = x + z * 256;
			int vertex_start_index = quad_index * 4;
			vertexBuffer->vertices[vertex_start_index].position[0] = start_x;
			vertexBuffer->vertices[vertex_start_index].position[1] = height * GetHeight(x, z);
			vertexBuffer->vertices[vertex_start_index].position[2] = start_z;
			vertexBuffer->vertices[vertex_start_index].position[3] = 1.0f;
			GetNormal(x, z, normal);
			
			memcpy(vertexBuffer->vertices[vertex_start_index].normal, normal, sizeof(float) * 3);
			vertexBuffer->vertices[vertex_start_index].normal[3] = 1.0f;
			vertexBuffer->vertices[vertex_start_index].texcoord[0] = float(x % 32) / 32.0f;
			vertexBuffer->vertices[vertex_start_index].texcoord[1] = float(z % 32) / 32.0f;
			vertexBuffer->vertices[vertex_start_index].texcoord[2] = float(x) / 256.0f;
			vertexBuffer->vertices[vertex_start_index].texcoord[3] = float(z) / 256.0f;

			vertexBuffer->vertices[vertex_start_index + 1].position[0] = start_x + 1.0f;
			vertexBuffer->vertices[vertex_start_index + 1].position[1] = height * GetHeight(x + 1, z);
			vertexBuffer->vertices[vertex_start_index + 1].position[2] = start_z;
			vertexBuffer->vertices[vertex_start_index + 1].position[3] = 1.0f;
			GetNormal(x + 1, z, normal);
			memcpy(vertexBuffer->vertices[vertex_start_index + 1].normal, normal, sizeof(float) * 3);
			vertexBuffer->vertices[vertex_start_index + 1].normal[3] = 1.0f;
			vertexBuffer->vertices[vertex_start_index + 1].texcoord[0] = float((x + 1) % 32) / 32.0f;
			vertexBuffer->vertices[vertex_start_index + 1].texcoord[1] = float(z % 32) / 32.0f;
			vertexBuffer->vertices[vertex_start_index + 1].texcoord[2] = float(x + 1) / 256.0f;
			vertexBuffer->vertices[vertex_start_index + 1].texcoord[3] = float(z) / 256.0f;

			vertexBuffer->vertices[vertex_start_index + 2].position[0] = start_x;
			vertexBuffer->vertices[vertex_start_index + 2].position[1] = height * GetHeight(x, z + 1);
			vertexBuffer->vertices[vertex_start_index + 2].position[2] = start_z - 1.0f;
			vertexBuffer->vertices[vertex_start_index + 2].position[3] = 1.0f;
			GetNormal(x, z + 1, normal);
			memcpy(vertexBuffer->vertices[vertex_start_index + 2].normal, normal, sizeof(float) * 3);
			vertexBuffer->vertices[vertex_start_index + 2].normal[3] = 1.0f;
			vertexBuffer->vertices[vertex_start_index + 2].texcoord[0] = float((x) % 32) / 32.0f;
			vertexBuffer->vertices[vertex_start_index + 2].texcoord[1] = float((z + 1) % 32) / 32.0f;
			vertexBuffer->vertices[vertex_start_index + 2].texcoord[2] = float(x) / 256.0f;
			vertexBuffer->vertices[vertex_start_index + 2].texcoord[3] = float(z + 1) / 256.0f;

			vertexBuffer->vertices[vertex_start_index + 3].position[0] = start_x + 1.0f;
			vertexBuffer->vertices[vertex_start_index + 3].position[1] = height * GetHeight(x + 1, z + 1);
			vertexBuffer->vertices[vertex_start_index + 3].position[2] = start_z - 1.0f;
			vertexBuffer->vertices[vertex_start_index + 3].position[3] = 1.0f;
			GetNormal(x + 1, z + 1, normal);
			memcpy(vertexBuffer->vertices[vertex_start_index + 3].normal, normal, sizeof(float) * 3);
			vertexBuffer->vertices[vertex_start_index + 3].normal[3] = 1.0f;
			vertexBuffer->vertices[vertex_start_index + 3].texcoord[0] = float((x + 1) % 32) / 32.0f;
			vertexBuffer->vertices[vertex_start_index + 3].texcoord[1] = float((z + 1) % 32) / 32.0f;
			vertexBuffer->vertices[vertex_start_index + 3].texcoord[2] = float(x + 1) / 256.0f;
			vertexBuffer->vertices[vertex_start_index + 3].texcoord[3] = float(z + 1) / 256.0f;

			*(temp++) = vertex_start_index;
			*(temp++) = vertex_start_index + 1;
			*(temp++) = vertex_start_index + 2;
			*(temp++) = vertex_start_index + 3;
			*(temp++) = vertex_start_index + 2;
			*(temp++) = vertex_start_index + 1;
		}
	}
	vertexBuffer->SubmitData();
	indexBuffer->SubmitData();
}

void Terrain::Draw(VkCommandBuffer commandbuffer)
{
	Model::Draw(commandbuffer);
	//xSetDynamicState(material->fixedPipeline, commandbuffer);
	//VkBuffer vertexbuffers[] = { vertexBuffer->buffer };
	//VkDeviceSize offsets[] = { 0 };
	//vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
	//	material->fixedPipeline->pipeline);
	//vkCmdBindVertexBuffers(commandbuffer, 0, 1, vertexbuffers, offsets);
	//vkCmdBindIndexBuffer(commandbuffer, indexBuffer->buffer, 0, VK_INDEX_TYPE_UINT32);
	//vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
	//	material->fixedPipeline->pipelineLayout, 0, 1, &material->program.descriptorSet,
	//	0, nullptr);
	//for (int z = 0; z < 256; ++z)
	//{
	//	for (int x = 0; x < 256; ++x)
	//	{
	//		int quad_index = x + z * 256;
	//		int vertex_start_index = quad_index * 4;
	//		vkCmdDrawIndexed(commandbuffer, 6, 1, vertex_start_index, 0, 0);
	//	}
	//}

}