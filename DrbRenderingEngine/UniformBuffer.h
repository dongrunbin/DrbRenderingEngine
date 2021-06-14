#pragma once
#include "VulkanAPI.h"

class UniformBuffer : public XBufferObject
{
public:
	std::vector<XMatrix4x4f> matrices;
	std::vector<XVector4f> vectors;

	void* data;
	int size;
public:
	XUniformBufferType uniformBufferType;
	UniformBuffer(XUniformBufferType t);
	~UniformBuffer();
	void SetSize(int count);
	int GetSize();
	void SetData(void* data);
	void SubmitData();
	void SetMatrix(int location, const float* v);
	void SetMatrix(int location, const glm::mat4& m);
	void SetVector4(int location, const float* v);
	void SetVector4(int location, float x, float y, float z, float w);
};