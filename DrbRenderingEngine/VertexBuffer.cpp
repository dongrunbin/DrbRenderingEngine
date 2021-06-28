#include "VertexBuffer.h"

VertexBuffer::VertexBuffer()
{
	type = XBufferObjectTypeVertexBuffer;
	vertices = nullptr;
	vertexCount = 0;
}

VertexBuffer::~VertexBuffer()
{
	if (vertexCount != 0) 
	{
		delete[] vertices;
	}
}

int VertexBuffer::GetSize()
{
	return vertexCount * sizeof(XVertexData);
}

void VertexBuffer::SetSize(int count)
{
	vertexCount = count;
	vertices = new XVertexData[count];
	XBufferObject::OnSetSize();
}

void VertexBuffer::SubmitData()
{
	XBufferObject::SubmitData(vertices, GetSize());
}

void VertexBuffer::SetPosition(int index, float x, float y, float z, float w)
{
	vertices[index].SetPosition(x, y, z, w);
}

void VertexBuffer::SetTexcoord(int index, float x, float y, float z, float w)
{
	vertices[index].SetTexcoord(x, y, z, w);
}

void VertexBuffer::SetNormal(int index, float x, float y, float z, float w)
{
	vertices[index].SetNormal(x, y, z, w);
}

void VertexBuffer::SetTangent(int index, float x, float y, float z, float w)
{
	vertices[index].SetTangent(x, y, z, w);
}