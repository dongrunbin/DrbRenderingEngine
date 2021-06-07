#pragma once
#include "VertexBuffer.h"
#include "Texture2D.h"
#include "Material.h"

class FullScreenQuad
{
public:
	VertexBuffer* vertexBuffer;
	Material* material;
public:
	~FullScreenQuad();
	void Init();
	void Draw(VkCommandBuffer commandbuffer);
};