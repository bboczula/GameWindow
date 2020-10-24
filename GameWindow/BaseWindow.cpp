#include "BaseWindow.h"

#include <iostream>

#ifndef LOG
#define LOG(s) std::cout << s << std::endl
#endif // !LOG

BaseWindow::BaseWindow(const char* title, int width, int height) : title(), width(width), height(height), instance(GetModuleHandle(NULL))
{
	// Here you need to initialize your char arrach
	LOG("BaseWindow::BaseWindow()");
	strcpy_s(this->title, title);
	window.registerWindowClass();
	window.createWindowInstance();

	// Create DXGI Manager
	dxgiManager = new DxgiManager();
}

BaseWindow::~BaseWindow()
{
	LOG("BaseWindow::~BaseWindow()");
}

void BaseWindow::start()
{
	LOG("BaseWindow::start()");

	for (int i = 0; i < numOfComponents; i++)
	{
		components[i]->initialize();
	}

	MSG message{ 0 };
	initialize();

	// For each thread that creates the window, the OS creates a queue for window messages.
	// This queue holds messages  for all windows that are created on that thread.
	while (message.message != WM_QUIT)
	{
		timer.start();
		// If HWND parameter is NULL, then function retrieves messages for any window that belongs to current thread,
		// so both window messages and thread messages.
		bool isMessageAvailable = PeekMessage(&message, NULL, 0, 0, PM_REMOVE);
		if (isMessageAvailable)
		{
			// This function translates keystrokes into characters.
			TranslateMessage(&message);

			// This function tells OS to call Window Procedure
			DispatchMessage(&message);
		}
		else
		{
			tick();
		}

		for (int i = 0; i < numOfComponents; i++)
		{
			components[i]->postFrame();
		}
		timer.stop();
	}

	LOG("BaseWindow::start() - finished");
}

void BaseWindow::add(IWindowComponent* component)
{
	components[numOfComponents++] = component;
}

void BaseWindow::initialize()
{
}

void BaseWindow::tick()
{
}
