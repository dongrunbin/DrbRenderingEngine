#pragma once
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Texture2D.h"
#include "Material.h"

class Ground {
public:
	VertexBuffer* vertexBuffer;
	IndexBuffer* indexBuffer;
	Material* material;
public:
	~Ground();
	void Init();
	void Draw(VkCommandBuffer commandbuffer);
	void SetMaterial(Material* material);
};