#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(XUniformBufferType t)
{
	type = XBufferObjectTypeUniformBuffer;
	uniformBufferType = t;
}
UniformBuffer::~UniformBuffer()
{
}
int UniformBuffer::GetSize()
{
	if (uniformBufferType == XUniformBufferTypeMatrix)
	{
		return sizeof(XMatrix4x4f) * matrices.size();
	}
	else if (uniformBufferType == XUniformBufferTypeVector)
	{
		return sizeof(XVector4f) * vectors.size();
	}
	return size;
}
void UniformBuffer::SetSize(int count)
{
	if (uniformBufferType == XUniformBufferTypeMatrix)
	{
		matrices.resize(count);
	}
	else if (uniformBufferType == XUniformBufferTypeVector)
	{
		vectors.resize(count);
	}
	else
	{
		this->size = count;
	}
	XBufferObject::OnSetSize();
}
void UniformBuffer::SetData(void* data)
{
	this->data = data;
}
void UniformBuffer::SubmitData()
{
	if (uniformBufferType == XUniformBufferTypeMatrix)
	{
		XBufferObject::SubmitData(matrices.data(), GetSize());
	}
	else if (uniformBufferType == XUniformBufferTypeVector)
	{
		XBufferObject::SubmitData(vectors.data(), GetSize());
	}
	else
	{
		XBufferObject::SubmitData(this->data, GetSize());
	}
}
void UniformBuffer::SetMatrix(int location, const glm::mat4& m)
{
	SetMatrix(location, glm::value_ptr(m));
}
void UniformBuffer::SetMatrix(int location, const float* v)
{
	memcpy(matrices[location].data, v, sizeof(XMatrix4x4f));
}
void UniformBuffer::SetVector4(int location, const float* v)
{
	memcpy(vectors[location].data, v, sizeof(XVector4f));
}
void UniformBuffer::SetVector4(int location, float x, float y, float z, float w)
{
	vectors[location].data[0] = x;
	vectors[location].data[1] = y;
	vectors[location].data[2] = z;
	vectors[location].data[3] = w;
}