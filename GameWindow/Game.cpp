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
	MSG msg{ 0 };
	while (1)
	{
		// Start the timer
		auto t1 = std::chrono::high_resolution_clock::now();

		//// If HWND parameter is NULL, then function retrieves messages for any window that belongs to current thread,
		//// so both window messages and thread messages.
		bool isMessageAvailable = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
		if (isMessageAvailable)
		{
			if (msg.message == WM_CLOSE)
			{
				std::cout << "(WM_CLOSE)" << std::endl;
			}
			// WM_QUIT meanst stop the message loop and exit the application
			if (msg.message == WM_QUIT)
			{
				std::cout << "(WM_QUIT)" << std::endl;
				break;
			}

			// This function translates keystrokes into characters.
			TranslateMessage(&msg);
		
			// This function tells OS to call Window Procedure
			DispatchMessage(&msg);
		}
		else
		{
			Logic();
			Rendering();
		}

		// Stop the timer
		auto t2 = std::chrono::high_resolution_clock::now();

		auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
		frameTime = elapsed_time;
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
	renderContext->OnRender(frameTime);
}
