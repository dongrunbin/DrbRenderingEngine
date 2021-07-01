#pragma once
#include "Model.h"

class Terrain : public Model
{
private:
	unsigned char* heightMapDatas = nullptr;
	int heightMapWidth;
	int heightMapHeight;
private:
	float GetHeight(int x, int z);
	void GetNormal(int x, int z, float* normal);
public:
	void Init(const char* heightMapPath);
	void Draw(VkCommandBuffer commandbuffer);
};