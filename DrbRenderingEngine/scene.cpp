#include "scene.h"
#include "Example.h"
#include "Example_ForwardShadowMapping.cpp"
#include "Example_DeferredShadowMapping.cpp"

Example* example;

void Init()
{
	//example = new ForwardShadowMapping();
	example = new DeferredShadowMapping();
	example->Init();
}

void Draw(float deltaTime)
{
	example->Draw(deltaTime);
}

void OnViewportChanged(int width, int height)
{
	example->OnViewportChanged(width, height);
}

void OnKeyboard(unsigned char key)
{
	example->OnKeyboard(key);
}

void OnMouseMove(int deltaX, int deltaY)
{
	example->OnMouseMove(deltaX, deltaY);
}

void OnQuit()
{
	example->OnQuit();
}