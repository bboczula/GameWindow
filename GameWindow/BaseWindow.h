#pragma once

#include <string>
#include <Windows.h>
#include <windowsx.h>
#include <stdint.h>
#include <string>

#include "WindowClass.h"
#include "DxgiManager.h"
#include "IWindowComponent.h"
#include "Timer.h"

#define MAX_WINDOW_COMPONENTS 2

class BaseWindow
{
public:
	BaseWindow(const char* title, int width, int heigh);
	virtual ~BaseWindow();
	void start();
	void add(IWindowComponent* component);
protected:
	DxgiManager* dxgiManager;
	WindowClass window;
	Timer timer;
	CHAR title[MAX_TITLE_LENGTH];
	const UINT16 width;
	const UINT16 height;
	HINSTANCE instance;
	virtual void initialize();
	virtual void tick();
private:
	UINT16 numOfComponents;
	IWindowComponent* components[MAX_WINDOW_COMPONENTS];
};