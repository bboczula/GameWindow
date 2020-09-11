#pragma once

#include <string>
#include <Windows.h>
#include <windowsx.h>
#include <stdint.h>
#include "WindowClass.h"
typedef uint64_t QWORD;

#include "IWindowComponent.h"

#define MAX_WINDOW_COMPONENTS 2

class BaseWindow
{
public:
	BaseWindow(const char* title, int width, int heigh);
	virtual ~BaseWindow();
	void start();
	void add(IWindowComponent* component);
protected:
	CHAR title[MAX_TITLE_LENGTH];
	const UINT16 width;
	const UINT16 height;
	HWND hWindow;
	HINSTANCE instance;
	virtual void initialize();
	virtual void update();
	virtual void render();
private:
	WindowClass window;
	void presentWindow();
	UINT16 numOfComponents;
	IWindowComponent* components[MAX_WINDOW_COMPONENTS];
};