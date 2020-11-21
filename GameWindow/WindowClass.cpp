#include "WindowClass.h"

#define WINDOW_CLASS_NAME "MyWindowWin32Class"
#define DEFAULT_WINDOW_STYLE WS_VISIBLE | WS_CLIPCHILDREN | WS_OVERLAPPED  | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX
#define DEFAULT_WINDOW_STYLE_EX WS_EX_APPWINDOW | WS_EX_CLIENTEDGE

static LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
	{
		// This function sends WM_DESTROY and WM_NCDESTROY messages to the window and removes keyboard focus from it
		// Also it fushes the thread message queue, destroys timers, remove clipboard ownership and breaks clipboard viewer chain
		DestroyWindow(hwnd);
		break;
	}
	case WM_DESTROY:
	{
		// This function puts WM_QUIT message on top of the queue - a speciall message that causes GetMessage to return 0
		PostQuitMessage(0);
		break;
	}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

WindowClass::WindowClass() : title("GameWindow")
{
	std::cout << "WindowClass::WindowClass()" << std::endl;
	registerWindowClass();
	createWindowInstance();
}

WindowClass::~WindowClass()
{
	std::cout << "WindowClass::~WindowClass()" << std::endl;
}

void WindowClass::registerWindowClass()
{
	std::cout << " WindowClass::registerWindowClass()" << std::endl;

	WNDCLASSEX windowClass;
	ZeroMemory(&windowClass, sizeof(WNDCLASSEX));
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.lpszClassName = WINDOW_CLASS_NAME;
	windowClass.lpfnWndProc = (WNDPROC)WindowProcedure;
	windowClass.hInstance = GetModuleHandle(NULL);


	if (!RegisterClassEx(&windowClass))
	{
		std::cout << "ERROR: Window Class registration failed." << std::endl;
	}
}

void WindowClass::createWindowInstance()
{
	std::cout << " WindowClass::createWindowInstance()" << std::endl;

	windowContext.width = 1280;
	windowContext.height = 720;

	RECT windowRectangle = { 0, 0, windowContext.width, windowContext.height };
	AdjustWindowRect(&windowRectangle, WS_OVERLAPPEDWINDOW, FALSE);

	INT startPositionX = CW_USEDEFAULT;
	INT startPositionY = CW_USEDEFAULT;

	windowContext.instance = GetModuleHandle(NULL);

	HWND hwnd = CreateWindowEx(DEFAULT_WINDOW_STYLE_EX, WINDOW_CLASS_NAME, title, DEFAULT_WINDOW_STYLE, startPositionX, startPositionY,
		windowRectangle.right, windowRectangle.bottom, NULL, NULL, windowContext.instance, NULL);

	windowContext.hwnd = hwnd;

	if (windowContext.hwnd == NULL)
	{
		std::cout << "ERROR: Window creation failed (" << GetLastError() << ")." << std::endl;
	}
}

HWND WindowClass::getHwnd()
{
	return windowContext.hwnd;
}

LONG WindowClass::getWidth()
{
	return windowContext.width;
}

LONG WindowClass::getHeight()
{
	return windowContext.height;
}

Astral::WindowContext WindowClass::GetWindowContext()
{
	return windowContext;
}
