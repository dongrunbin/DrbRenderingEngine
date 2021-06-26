#include <windows.h>
#include "VulkanAPI.h"
#include "scene.h"
#pragma comment(lib,"winmm.lib")

int mousePosX, mousePosY;
POINT originalMousePos;
bool rotateView = false;

LRESULT CALLBACK EngineWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SIZE:
		RECT rect;
		GetClientRect(hwnd, &rect);
		OnViewportChanged(rect.right - rect.left, rect.bottom - rect.top);
		return 0;
	case WM_RBUTTONDOWN:
		GetCursorPos(&originalMousePos);
		ShowCursor(false);
		rotateView = true;
		return 0;
	case WM_RBUTTONUP:
		SetCursorPos(originalMousePos.x, originalMousePos.y);
		ShowCursor(true);
		rotateView = false;
		return 0;
	case WM_MOUSEMOVE:
		if (rotateView)
		{
			POINT currentPos;
			GetCursorPos(&currentPos);
			int deltaX = currentPos.x - originalMousePos.x;
			int deltaY = currentPos.y - originalMousePos.y;
			OnMouseMove(deltaX, deltaY);
			originalMousePos.x = currentPos.x;
			originalMousePos.y = currentPos.y;
			//SetCursorPos(currentPos.x, currentPos.y);
		}
		return 0;
	case WM_KEYDOWN:
		OnKeyboard(wParam);
		return 0;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
INT WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	WNDCLASSEX wndclassex;
	wndclassex.cbSize = sizeof(WNDCLASSEX);
	wndclassex.cbClsExtra = 0;
	wndclassex.cbWndExtra = 0;
	wndclassex.hbrBackground = nullptr;
	wndclassex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndclassex.hIcon = nullptr;
	wndclassex.hIconSm = nullptr;
	wndclassex.hInstance = hInstance;
	wndclassex.lpfnWndProc = EngineWindowProc;
	wndclassex.lpszMenuName = nullptr;
	wndclassex.lpszClassName = L"DrbRenderingEngine";
	wndclassex.style = CS_VREDRAW | CS_HREDRAW;
	ATOM atom = RegisterClassEx(&wndclassex);
	if (atom==0)
	{
		MessageBox(NULL, L"fail to register window.", L"error", MB_OK);
		return -1;
	}
	RECT rect = {0,0,1280,720};
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
	HWND hwnd = CreateWindowEx(NULL, L"DrbRenderingEngine", L"DrbRenderingEngine", WS_OVERLAPPEDWINDOW, 
		0, 0, rect.right-rect.left, rect.bottom-rect.top,
		nullptr, nullptr, hInstance, nullptr);
	xInitVulkan(hwnd, 1280, 720);
	Init();
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);
	MSG msg;
	float last_time = timeGetTime() / 1000.0f;
	while (true)
	{
		if (PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
		{
			if (msg.message==WM_QUIT)
			{
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		float current_time = timeGetTime() / 1000.0f;
		float deltaTime = current_time - last_time;
		last_time = current_time;
		Draw(deltaTime);
		//printf("fps:%d\n", (int)(1 / deltaTime));
	}
	OnQuit();
	return 0;
}