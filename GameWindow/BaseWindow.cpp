#include "BaseWindow.h"

#include <iostream>

#ifndef LOG
#define LOG(s) std::cout << s << std::endl
#endif // !LOG

BaseWindow::BaseWindow(const char* title, int width, int height)
{
	// Here you need to initialize your char arrach
	LOG("BaseWindow::BaseWindow()");
	std::cout << "sizeof(WindowClass): " << sizeof(WindowClass) << std::endl;
	std::cout << "sizeof(Timer): " << sizeof(Timer) << std::endl;
	std::cout << "sizeof(RenderingContext): " << sizeof(RenderingContext) << std::endl;
	renderContext = new RenderingContext(window.GetWindowContext());
}

BaseWindow::~BaseWindow()
{
	LOG("BaseWindow::~BaseWindow()");
}

void BaseWindow::start()
{
	LOG("BaseWindow::start()");

	initialize();

	// For each thread that creates the window, the OS creates a queue for window messages.
	// This queue holds messages  for all windows that are created on that thread.
	MSG message{ 0 };
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
			render();
		}

		timer.stop();
	}

	LOG("BaseWindow::start() - finished");
}

void BaseWindow::initialize()
{
}

void BaseWindow::tick()
{
}

void BaseWindow::render()
{
	renderContext->OnRender();
}
