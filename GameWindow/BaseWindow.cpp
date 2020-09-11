#include "BaseWindow.h"

#include <iostream>

#ifndef LOG
#define LOG(s) std::cout << s << std::endl
#endif // !LOG

BaseWindow::BaseWindow(const char* title, int width, int height) : title(), width(width), height(height), hWindow(NULL), instance(GetModuleHandle(NULL))
{
	// Here you need to initialize your char arrach
	LOG("BaseWindow::BaseWindow()");
	strcpy_s(this->title, title);
	window.registerWindow();
	window.createWindow();
	presentWindow();
}

void BaseWindow::presentWindow()
{
	ShowWindow(hWindow, SW_SHOW);
	UpdateWindow(hWindow);
}

BaseWindow::~BaseWindow()
{
	LOG("BaseWindow::~BaseWindow()");
}

void BaseWindow::start()
{
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
