#include "Ground.h"

void Ground::Init()
{
	vertexBuffer = new VertexBuffer;
	vertexBuffer->SetSize(1600);
	indexBuffer = new IndexBuffer;
	indexBuffer->SetSize(2400);
	for (int z = 0; z < 20; ++z)
	{
		float zStart = 10.0f - z * 1.0f;
		for (int x = 0; x < 20; ++x)
		{
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
			if ((x % 2) ^ (z % 2))
			{
				vertexBuffer->SetTexcoord(offset, 0.1f, 0.1f, 0.1f, 0.1f);
				vertexBuffer->SetTexcoord(offset + 1, 0.1f, 0.1f, 0.1f, 0.1f);
				vertexBuffer->SetTexcoord(offset + 2, 0.1f, 0.1f, 0.1f, 0.1f);
				vertexBuffer->SetTexcoord(offset + 3, 0.1f, 0.1f, 0.1f, 0.1f);
			}
			else
			{
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