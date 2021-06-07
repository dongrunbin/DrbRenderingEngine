#pragma once
#include "VulkanAPI.h"

class IndexBuffer : public XBufferObject {
public:
	int indexCount;
	int currentIndex;
	unsigned int* indices;
public:
	IndexBuffer();
	~IndexBuffer();
	void SetSize(int count);
	int GetSize();
	void AppendIndex(int index);
	void SubmitData();
};