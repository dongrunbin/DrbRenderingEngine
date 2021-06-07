#include "Texture2D.h"

Texture2D::Texture2D(VkImageAspectFlags imageaspect) :XTexture(imageaspect)
{
}
Texture2D::~Texture2D()
{
}
void Texture2D::SetImage(const char* path)
{
	format = VK_FORMAT_R8G8B8A8_UNORM;
	int image_width, image_height, channel;
	unsigned char* pixel = LoadImageFromFile(path, image_width, image_height, channel, 4);
	xGenImage(this, image_width, image_height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
		VK_IMAGE_USAGE_SAMPLED_BIT);
	xSubmitImage2D(this, image_width, image_height, pixel);
	xGenImageView2D(this);
	xGenSampler(this);
	delete[]pixel;
}
void Texture2D::SetFilter(VkFilter min_filter, VkFilter mag_filter)
{
	minFilter = min_filter;
	magFilter = mag_filter;
}
void Texture2D::SetWrapMode(VkSamplerAddressMode wrapu, VkSamplerAddressMode wrapv,
	VkSamplerAddressMode wrapw)
{
	wrapU = wrapu;
	wrapV = wrapv;
	wrapW = wrapw;
}
TextureCube::TextureCube()
{
}
TextureCube::~TextureCube()
{
}
void TextureCube::Init(const char* path) {
	format = VK_FORMAT_R8G8B8A8_UNORM;
	unsigned char* image_data = nullptr;
	int image_width, image_height, channel;
	unsigned char* pixel = LoadImageFromFile("Res/right.bmp", image_width, image_height, channel, 4, true);
	image_data = new unsigned char[image_width * image_height * 4 * 6];
	int image_unit_size = image_width * image_height * 4;
	int offset = 0;
	memcpy(image_data, pixel, image_unit_size);
	offset += image_unit_size;
	delete[]pixel;
	pixel = LoadImageFromFile("Res/left.bmp", image_width, image_height, channel, 4, true);
	memcpy(image_data + offset, pixel, image_unit_size);
	offset += image_unit_size;
	delete[]pixel;

	pixel = LoadImageFromFile("Res/top.bmp", image_width, image_height, channel, 4, true);
	memcpy(image_data + offset, pixel, image_unit_size);
	offset += image_unit_size;
	delete[]pixel;

	pixel = LoadImageFromFile("Res/bottom.bmp", image_width, image_height, channel, 4, true);
	memcpy(image_data + offset, pixel, image_unit_size);
	offset += image_unit_size;
	delete[]pixel;

	pixel = LoadImageFromFile("Res/back.bmp", image_width, image_height, channel, 4, true);
	memcpy(image_data + offset, pixel, image_unit_size);
	offset += image_unit_size;
	delete[]pixel;

	pixel = LoadImageFromFile("Res/front.bmp", image_width, image_height, channel, 4, true);
	memcpy(image_data + offset, pixel, image_unit_size);
	offset += image_unit_size;
	delete[]pixel;

	//xGenImageCube(this, image_width, image_height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
	//	VK_IMAGE_USAGE_SAMPLED_BIT);
	//xSubmitImageCube(this, image_width, image_height, image_data);
	//xGenImageViewCube(this);
	//xGenSampler(this);
	//delete[] image_data;
}
void TextureCube::SetFilter(VkFilter min_filter, VkFilter mag_filter) {
	minFilter = min_filter;
	magFilter = mag_filter;
}
void TextureCube::SetWrapMode(VkSamplerAddressMode wrapu, VkSamplerAddressMode wrapv,
	VkSamplerAddressMode wrapw) {
	wrapU = wrapu;
	wrapV = wrapv;
	wrapW = wrapw;
}