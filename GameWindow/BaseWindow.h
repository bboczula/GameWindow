#pragma once

#include <string>
#include <Windows.h>
#include <windowsx.h>
#include <stdint.h>
#include <string>

#include "WindowClass.h"
#include "IWindowComponent.h"
#include "Timer.h"
#include "RenderingContext.h"

class BaseWindow
{
public:
	BaseWindow(const char* title, int width, int heigh);
	virtual ~BaseWindow();
	void start();
protected:
	WindowClass window;
	RenderingContext* renderContext;
	Timer timer;
	virtual void initialize();
	virtual void tick();
	virtual void render();
};