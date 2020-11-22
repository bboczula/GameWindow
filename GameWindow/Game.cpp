#include "Game.h"

#include <iostream>

#ifndef LOG
#define LOG(s) std::cout << s << std::endl
#endif // !LOG

Astral::Game::Game(int width, int height) : window(width, height)
{
	LOG("BaseWindow::BaseWindow()");
	renderContext = new RenderingContext(window.GetWindowContext());
}

Astral::Game::~Game()
{
	LOG("BaseWindow::~BaseWindow()");
}

void Astral::Game::start()
{
	LOG("BaseWindow::start()");

	Initialization();

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
			Logic();
			Rendering();
		}

		timer.stop();
	}

	LOG("BaseWindow::start() - finished");
}

void Astral::Game::Initialization()
{
}

void Astral::Game::Logic()
{
}

void Astral::Game::Rendering()
{
	renderContext->OnRender();
}
