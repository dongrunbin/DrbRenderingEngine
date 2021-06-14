#pragma once
void Init();
void Draw(float deltaTime);
void OnViewportChanged(int width, int height);
void OnKeyboard(unsigned char key);
void OnMouseMove(int deltaX, int deltaY);
void OnQuit();