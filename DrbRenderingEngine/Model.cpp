#include "Model.h"
#include <sstream>
#include <string>

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

void Model::LoadObjModel(const char* filePath)
{
	struct VertexInfo
	{
		float v[3];
	};
	struct VertexDefine
	{
		int positionIndex;
		int texcoordIndex;
		int normalIndex;
	};
	char* fileContent = LoadFileContent(filePath);
	if (fileContent != nullptr)
	{
		//obj model decode

		std::vector<VertexInfo> positions;
		std::vector<VertexInfo> texcoords;
		std::vector<VertexInfo> normals;
		std::vector<unsigned int> objIndexes;
		std::vector<VertexDefine> vertices;

		std::stringstream ssObjFile(fileContent);
		char szOneLine[256];

		std::string temp;
		std::stringstream ssOneLine;
		while (!ssObjFile.eof())
		{
			memset(szOneLine, 0, 256);
			ssObjFile.getline(szOneLine, 256);
			if (strlen(szOneLine) > 0)
			{
				ssOneLine.clear();
				ssOneLine.str(szOneLine);
				if (szOneLine[0] == 'v')
				{
					if (szOneLine[1] == 't')
					{
						//vertex coord
						ssOneLine >> temp;
						VertexInfo vi;
						ssOneLine >> vi.v[0];
						ssOneLine >> vi.v[1];
						texcoords.push_back(vi);
					}
					else if (szOneLine[1] == 'n')
					{
						//normal
						ssOneLine >> temp;
						VertexInfo vi;
						ssOneLine >> vi.v[0];
						ssOneLine >> vi.v[1];
						ssOneLine >> vi.v[2];
						normals.push_back(vi);
					}
					else
					{
						//position
						ssOneLine >> temp;
						VertexInfo vi;

						ssOneLine >> vi.v[0];
						ssOneLine >> vi.v[1];
						ssOneLine >> vi.v[2];
						positions.push_back(vi);
					}
				}
				else if (szOneLine[0] == 'f')
				{
					//face
					ssOneLine >> temp;
					std::string vertexStr;
					for (int i = 0; i < 3; ++i)
					{
						ssOneLine >> vertexStr;
						size_t pos = vertexStr.find_first_of('/');
						std::string positionIndexStr = vertexStr.substr(0, pos);
						size_t pos2 = vertexStr.find_first_of('/', pos + 1);
						std::string texcoordIndexStr = vertexStr.substr(pos + 1, pos2 - pos - 1);
						std::string normalIndexStr = vertexStr.substr(pos2 + 1, vertexStr.length() - pos2 - 1);
						VertexDefine vd;
						vd.positionIndex = atoi(positionIndexStr.c_str()) - 1;
						vd.texcoordIndex = atoi(texcoordIndexStr.c_str()) - 1;
						vd.normalIndex = atoi(normalIndexStr.c_str()) - 1;

						//check if exist
						int nCurrentIndex = -1;
						size_t nCurrentVerticeCount = vertices.size();
						for (size_t j = 0; j < nCurrentVerticeCount; ++j)
						{
							if (vertices[j].positionIndex == vd.positionIndex &&
								vertices[j].texcoordIndex == vd.texcoordIndex &&
								vertices[j].normalIndex == vd.normalIndex)
							{
								nCurrentIndex = j;
								break;
							}
						}
						if (nCurrentIndex == -1)
						{
							// create new vertice
							nCurrentIndex = vertices.size();
							vertices.push_back(vd);
						}
						objIndexes.push_back(nCurrentIndex);
					}
				}
			}
		}

		int indexCount = (int)objIndexes.size();
		unsigned int *indexes = new unsigned int[indexCount];
		for (int i = 0; i < indexCount; ++i)
		{
			indexes[i] = objIndexes[i];
		}
		int vertexCount = (int)vertices.size();
		XVertexData* vertexes = new XVertexData[vertexCount];
		for (int i = 0; i < vertexCount; ++i)
		{
			memcpy(vertexes[i].position, positions[vertices[i].positionIndex].v, sizeof(float) * 3);
			vertexes[i].position[3] = 1.0f;
			if (texcoords.size() > 0)
			{
				memcpy(vertexes[i].texcoord, texcoords[vertices[i].texcoordIndex].v, sizeof(float) * 2);
			}
			vertexes[i].texcoord[1] = 1.0f - vertexes[i].texcoord[1];
			if (normals.size() > 0)
			{
				memcpy(vertexes[i].normal, normals[vertices[i].normalIndex].v, sizeof(float) * 3);
			}
			vertexes[i].normal[3] = 0.0f;
			vertexes[i].tangent[0] = 0.0f;
			vertexes[i].tangent[1] = 0.0f;
			vertexes[i].tangent[2] = 0.0f;
			vertexes[i].tangent[3] = 0.0f;
		}

		vertexBuffer = new VertexBuffer;
		indexBuffer = new IndexBuffer;
		vertexBuffer->SetSize(vertexCount);
		memcpy(vertexBuffer->vertices, vertexes, vertexCount * sizeof(XVertexData));
		indexBuffer->SetSize(indexCount);
		memcpy(indexBuffer->indices, indexes, indexCount * sizeof(unsigned int));
		vertexBuffer->SubmitData();
		indexBuffer->SubmitData();
		delete[] vertexes;
		delete[] indexes;
	}
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