#pragma once
#include "VulkanAPI.h"

class VertexBuffer : public XBufferObject {
public:
	int vertexCount;
	XVertexData* vertices;
public:
	VertexBuffer();
	~VertexBuffer();
	void SetSize(int count);
	int GetSize();
	void SetPosition(int index, float x, float y, float z, float w = 1.0f);
	void SetTexcoord(int index, float x, float y, float z = 0.0f, float w = 0.0f);
	void SetNormal(int index, float x, float y, float z, float w = 0.0f);
	void SetTangent(int index, float x, float y, float z, float w = 0.0f);
	void SubmitData();
};