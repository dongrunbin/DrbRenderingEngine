#include "BVulkan.h"
#include "scene.h"
#include "VulkanAPI.h"
XProgram* program;
XBufferObject* vbo;
XUniformBuffer* ubo;
XBufferObject* ibo;
XTexture* texture;
void Init()
{
	xInitDefaultTexture();
	Vertex vertexes[3];
	vertexes[0].SetPosition(-0.5f, -0.5f, -2.0f);
	vertexes[0].SetTexcoord(0.0f, 0.0f);
	vertexes[0].SetNormal(1.0f, 0.0f, 1.0f, 0.2f);
	vertexes[1].SetPosition(0.5f, -0.5f, -2.0f);
	vertexes[1].SetTexcoord(1.0f, 0.0f);
	vertexes[1].SetNormal(1.0f, 1.0f, 0.0f, 1.0f);
	vertexes[2].SetPosition(0.0f, 0.5f, -2.0f);
	vertexes[2].SetTexcoord(0.5f, 1.0f);
	vertexes[2].SetNormal(0.0f, 1.0f, 1.0f, 1.0f);
	vbo = new XBufferObject;
	xglBufferData(vbo, sizeof(Vertex) * 3, vertexes);
	ibo = new XBufferObject;
	unsigned int indexes[] = { 0, 1, 2 };
	xGenIndexBuffer(sizeof(unsigned int) * 3, ibo->buffer, ibo->memory);
	xBufferSubIndexData(ibo->buffer, indexes, sizeof(unsigned int) * 3);
	program = new XProgram;
	GLuint vs, fs;
	int file_len = 0;
	unsigned char *file_content = LoadFileContent("Res/test.vsb", file_len);
	xCreateShader(vs, file_content, file_len);
	delete[]file_content;
	file_content = LoadFileContent("Res/test.fsb", file_len);
	xCreateShader(fs, file_content, file_len);
	delete[]file_content;
	xAttachVertexShader(program, vs);
	xAttachFragmentShader(program, fs);
	xLinkProgram(program);

	ubo = new XUniformBuffer;
	ubo->type = XUniformBufferTypeMatrix;
	ubo->matrices.resize(8);
	glm::mat4 projection = glm::perspective(45.0f, float(GetViewportWidth()) / float(GetViewportHeight()), 0.1f, 60.0f);
	projection[1][1] *= -1.0f;
	memcpy(ubo->matrices[2].data, glm::value_ptr(projection), sizeof(XMatrix4x4f));

	xGenBuffer(ubo->buffer, ubo->memory, sizeof(XMatrix4x4f) * 8,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	xSubmitUniformBuffer(ubo);

	texture = new XTexture;
	texture->format = VK_FORMAT_R8G8B8A8_UNORM;
	int imageWidth, imageHeight, channel;
	unsigned char* pixel = LoadImageFromFile("Res/test.bmp", imageWidth, imageHeight, channel, 4, false);
	xGenImage(texture, imageWidth, imageHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	xSubmitImage2D(texture, imageWidth, imageHeight, pixel);
	xGenImageView2D(texture);
	xGenSampler(texture);
	delete[] pixel;
}
void Draw(float deltaTime)
{
	static float r = 0.0f;
	static float accTime = 0.0f;
	static bool modifiedTexture = false;
	static bool modifiedUBO = false;
	r += deltaTime;
	accTime += deltaTime;

	if (r >= 1.0f)
	{
		r = 0.0f;
	}
	if (accTime > 3.0f && !modifiedTexture)
	{
		modifiedTexture = true;
		//int imageWidth, imageHeight, channel;
		//unsigned char* pixel = LoadImageFromFile("Res/test.bmp", imageWidth, imageHeight, channel, 4, false);
		//xSubmitImage2D(xGetDefaultTexture(), imageWidth, imageHeight, pixel);
		//delete[] pixel;
		xRebindSampler(program, 4, texture->imageView, texture->sampler);
	}
	if (accTime > 3.0f && !modifiedUBO)
	{
		modifiedUBO = true;
		xRebindUniformBuffer(program, 1, ubo);
	}
	float color[] = { r,r,r,1.0f };
	aClearColor(0.1f, 0.4f, 0.6f, 1.0f);
	VkCommandBuffer commandbuffer = xBeginRendering();
	xUseProgram(program);
	xBindVertexBuffer(vbo);
	xBindElementBuffer(ibo);
	xUniform4fv(program, 2, color);
	//xDrawArrays(commandbuffer, 0, 3);
	xDrawElements(commandbuffer, 0, 3);
	xEndRendering();
	xSwapBuffers(commandbuffer);
}
void OnViewportChanged(int width, int height)
{
	aViewport(width, height);
}
void OnQuit()
{
	if (program != nullptr)
	{
		delete (XProgram*)program;
	}
	if (vbo != nullptr)
	{
		glDeleteBufferObject(vbo);
	}
	if (texture != nullptr)
	{
		delete texture;
	}
	if (ubo != nullptr)
	{
		delete ubo;
	}
	if (ibo != nullptr)
	{
		glDeleteBufferObject(ibo);
	}

	xVulkanCleanUp();
}