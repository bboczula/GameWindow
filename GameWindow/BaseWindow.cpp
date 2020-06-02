#include "BaseWindow.h"

#include <iostream>

#ifndef LOG
#define LOG(s) std::cout << s << std::endl
#endif // !LOG

#define WINDOW_CLASS_NAME "MyMainBaseWindowClass"
#define DEFAULT_WINDOW_STYLE WS_VISIBLE | WS_CLIPCHILDREN | WS_OVERLAPPED  | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX
#define DEFAULT_WINDOW_STYLE_EX WS_EX_APPWINDOW | WS_EX_CLIENTEDGE

BaseWindow::BaseWindow(const char* title, int width, int height) : title(), width(width), height(height), hWindow(NULL), instance(GetModuleHandle(NULL)),
windowStyle(DEFAULT_WINDOW_STYLE), windowStyleEx(DEFAULT_WINDOW_STYLE_EX)
{
	// Here you need to initialize your char arrach
	strcpy_s(this->title, title);
	registerWindow();
	createWindow();
	presentWindow();
}

void BaseWindow::registerWindow()
{
	WNDCLASSEX windowClass;
	ZeroMemory(&windowClass, sizeof(WNDCLASSEX));
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.lpszClassName = WINDOW_CLASS_NAME;
	windowClass.lpfnWndProc = BaseWindow::ThisWindowProcedure;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hInstance = instance;
	windowClass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;


	if (!RegisterClassEx(&windowClass))
	{
		LOG("ERROR: Window Class registration failed.");
	}
}

void BaseWindow::createWindow()
{
	RECT windowRectangle = { 0, 0, width, height };
	AdjustWindowRect(&windowRectangle, WS_OVERLAPPEDWINDOW, FALSE);

	hWindow = CreateWindowEx(windowStyleEx, WINDOW_CLASS_NAME, title, windowStyle, CW_USEDEFAULT, CW_USEDEFAULT,
		windowRectangle.right - windowRectangle.left, windowRectangle.bottom - windowRectangle.top, NULL, NULL, instance, this);

	if (hWindow == NULL)
	{
		LOG("ERROR: Window creation failed (" << GetLastError() << ").");
	}
}

void BaseWindow::presentWindow()
{
	ShowWindow(hWindow, SW_SHOW);
	UpdateWindow(hWindow);
}

LRESULT CALLBACK BaseWindow::ThisWindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BaseWindow* self = nullptr;
	if (msg == WM_NCCREATE)
	{
		LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		self = static_cast<BaseWindow*>(lpcs->lpCreateParams);

		SetLastError(0);
		if (SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self)) == 0)
		{
			if (GetLastError() != 0)
			{
				return false;
			}
		}
	}
	else
	{
		self = reinterpret_cast<BaseWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	}

	if (self)
	{
		return self->WindowProcedure(hwnd, msg, wParam, lParam);
	}
	else
	{
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

LRESULT BaseWindow::WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Here you should find component to handle the message
	for (int i = 0; i < numOfComponents; i++)
	{
		components[i]->handle(msg, wParam, lParam);
	}

	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

BaseWindow::~BaseWindow()
{
	LOG("Deleting BaseWindow object.");
}

void BaseWindow::start()
{
	// Initialize Components
	for (int i = 0; i < numOfComponents; i++)
	{
		components[i]->initialize();
	}

	MSG message{ 0 };
	initialize();
	while (message.message != WM_QUIT)
	{
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		update();
		render();
		for (int i = 0; i < numOfComponents; i++)
		{
			components[i]->postFrame();
		}
	}
}

void BaseWindow::add(IWindowComponent* component)
{
	components[numOfComponents++] = component;
}

void BaseWindow::initialize()
{
}

void BaseWindow::update()
{
}

void BaseWindow::render()
{
}
