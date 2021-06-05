#include "BVulkan.h"
#include "scene.h"
AVulkanHandle program;
AVulkanHandle vbo;
void Init() {
	Vertex vertexes[3];
	vertexes[0].SetPosition(-0.5f, 0.5f, 0.0f);
	vertexes[0].SetTexcoord(1.0f, 0.0f, 1.0f, 1.0f);
	vertexes[1].SetPosition(0.5f, 0.5f, 0.0f);
	vertexes[1].SetTexcoord(1.0f, 1.0f, 0.0f, 1.0f);
	vertexes[2].SetPosition(0.0f, -0.5f, 0.0f);
	vertexes[2].SetTexcoord(0.0f, 1.0f, 1.0f, 1.0f);
	vbo = glGenBuffer();
	glBufferData(vbo, sizeof(Vertex) * 3, vertexes);
	program = aCreateProgram();
	GLuint vs, fs;
	int file_len = 0;
	unsigned char *file_content = LoadFileContent("Res/test.vsb", file_len);
	aCreateShader(vs, file_content, file_len);
	delete[]file_content;
	file_content = LoadFileContent("Res/test.fsb", file_len);
	aCreateShader(fs, file_content, file_len);
	delete[]file_content;
	aAttachVertexShader(program, vs);
	aAttachFragmentShader(program, fs);
	aLinkProgram(program);
}
void Draw(float deltaTime) {
	aClearColor(0.1f, 0.4f, 0.6f, 1.0f);
	VkCommandBuffer commandbuffer = aBeginRendering();
	glUseProgram(program);
	glBindVertexBuffer(vbo);
	glDrawArrays(A_TRIANGLES, 0, 3);
	aEndRenderingCommand();
	aSwapBuffers();
}
void OnViewportChanged(int width, int height) {
	aViewport(width, height);
}
void OnQuit() {
	if (program!=nullptr){
		aDeleteProgram(program);
	}
	if (vbo!=nullptr){
		glDeleteBufferObject(vbo);
	}
}