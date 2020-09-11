#include "WindowClass.h"

#define WINDOW_CLASS_NAME "MyWindowWin32Class"
#define DEFAULT_WINDOW_STYLE WS_VISIBLE | WS_CLIPCHILDREN | WS_OVERLAPPED  | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX
#define DEFAULT_WINDOW_STYLE_EX WS_EX_APPWINDOW | WS_EX_CLIENTEDGE

static LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
	{
		// This function puts WM_QUIT message on top of the queue - a speciall message that causes GetMessage to return 0
		PostQuitMessage(0);
		break;
	}
	case WM_CLOSE:
	{
		break;
	}
	case WM_KILLFOCUS:
	{
		break;
	}
	case WM_SETFOCUS:
	{
		break;
	}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

WindowClass::WindowClass() : title("GameWindow"), windowStyle(DEFAULT_WINDOW_STYLE), windowStyleEx(DEFAULT_WINDOW_STYLE_EX), hwnd(NULL)
{
	std::cout << "WindowClass::WindowClass()" << std::endl;
}

WindowClass::~WindowClass()
{
	std::cout << "WindowClass::~WindowClass()" << std::endl;
}

void WindowClass::registerWindowClass()
{
	std::cout << " WindowClass::registerWindowClass()" << std::endl;

	HINSTANCE instance = GetModuleHandle(NULL);

	WNDCLASSEX windowClass;
	ZeroMemory(&windowClass, sizeof(WNDCLASSEX));
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.lpszClassName = WINDOW_CLASS_NAME;
	windowClass.lpfnWndProc = (WNDPROC)WindowProcedure;
	windowClass.hInstance = instance;


	if (!RegisterClassEx(&windowClass))
	{
		std::cout << "ERROR: Window Class registration failed." << std::endl;
	}
}

void WindowClass::createWindowInstance()
{
	std::cout << " WindowClass::createWindowInstance()" << std::endl;

	HINSTANCE instance = GetModuleHandle(NULL);

	RECT windowRectangle = { 0, 0, display.getWidth(), display.getHeight() };
	AdjustWindowRect(&windowRectangle, WS_OVERLAPPEDWINDOW, FALSE);

	hwnd = CreateWindowEx(windowStyleEx, WINDOW_CLASS_NAME, title, windowStyle, CW_USEDEFAULT, CW_USEDEFAULT,
		windowRectangle.right - windowRectangle.left, windowRectangle.bottom - windowRectangle.top, NULL, NULL, instance, NULL);

	if (hwnd == NULL)
	{
		std::cout << "ERROR: Window creation failed (" << GetLastError() << ")." << std::endl;
	}
}

HWND WindowClass::getHwnd()
{
	return hwnd;
}
