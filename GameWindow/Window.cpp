#include "Window.h"

#define WINDOW_CLASS_NAME "AstralMyWindowWin32Class"
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

Astral::Window::Window(UINT width, UINT height) : windowContext(nullptr, GetModuleHandle(NULL), width, height), title("GameWindow")
{
	std::cout << "WindowClass::WindowClass()" << std::endl;
	RegisterWindowClass();
	CreateWindowInstance();
}

Astral::Window::~Window()
{
	std::cout << "WindowClass::~WindowClass()" << std::endl;
}

void Astral::Window::RegisterWindowClass()
{
	std::cout << " WindowClass::registerWindowClass()" << std::endl;

	WNDCLASSEX windowClass;
	ZeroMemory(&windowClass, sizeof(WNDCLASSEX));
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.lpszClassName = WINDOW_CLASS_NAME;
	windowClass.lpfnWndProc = (WNDPROC)WindowProcedure;
	windowClass.hInstance = windowContext.instance;


	if (!RegisterClassEx(&windowClass))
	{
		std::cout << "ERROR: Window Class registration failed." << std::endl;
		exit(1);
	}
}

void Astral::Window::CreateWindowInstance()
{
	std::cout << " WindowClass::createWindowInstance()" << std::endl;

	RECT windowRectangle = { 0, 0, windowContext.width, windowContext.height };
	AdjustWindowRect(&windowRectangle, WS_OVERLAPPEDWINDOW, FALSE);

	windowContext.hwnd = CreateWindowEx(DEFAULT_WINDOW_STYLE_EX, WINDOW_CLASS_NAME, title, DEFAULT_WINDOW_STYLE,
		CW_USEDEFAULT, CW_USEDEFAULT, windowRectangle.right, windowRectangle.bottom, NULL, NULL, windowContext.instance, NULL);

	if (!windowContext.hwnd)
	{
		std::cout << "ERROR: Window creation failed (" << GetLastError() << ")" << std::endl;
		exit(1);
	}
}

Astral::WindowContext Astral::Window::GetWindowContext()
{
	return windowContext;
}
