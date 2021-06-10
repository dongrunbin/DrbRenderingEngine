#pragma once
#include "VulkanAPI.h"

class Texture2D : public XTexture
{
public:
	Texture2D(VkImageAspectFlags imageaspect = VK_IMAGE_ASPECT_COLOR_BIT);
	~Texture2D();
	void SetImage(const char* path);
	void SetFilter(VkFilter min_filter, VkFilter mag_filter);
	void SetWrapMode(VkSamplerAddressMode wrapu, VkSamplerAddressMode wrapv, VkSamplerAddressMode wrapw);
};

class TextureCube : public XTexture
{
public:
	TextureCube();
	~TextureCube();
	void Init(const char* path);
	void SetFilter(VkFilter min_filter, VkFilter mag_filter);
	void SetWrapMode(VkSamplerAddressMode wrapu, VkSamplerAddressMode wrapv, VkSamplerAddressMode wrapw);
};