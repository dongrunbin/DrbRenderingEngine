#include "IndexBuffer.h"

IndexBuffer::IndexBuffer()
{
	type = XBufferObjectTypeIndexBuffer;
	indices = nullptr;
	indexCount = 0;
	currentIndex = 0;
}
IndexBuffer::~IndexBuffer()
{
	if (indexCount != 0)
	{
		delete[] indices;
	}
}
int IndexBuffer::GetSize()
{
	return indexCount * sizeof(unsigned int);
}
void IndexBuffer::SetSize(int count)
{
	indexCount = count;
	indices = new unsigned int[count];
	XBufferObject::OnSetSize();
}
void IndexBuffer::SubmitData()
{
	XBufferObject::SubmitData(indices, GetSize());
}
void IndexBuffer::AppendIndex(int index)
{
	indices[currentIndex++] = index;
}