#pragma once

#include <Windows.h>
#include <iostream>
#include "WindowContext.h"

#define MAX_TITLE_LENGTH 64

class WindowClass
{
public:
	WindowClass();
	~WindowClass();
	void registerWindowClass();
	void createWindowInstance();
	HWND getHwnd();
	LONG getWidth();
	LONG getHeight();
	Astral::WindowContext GetWindowContext();
private:
	Astral::WindowContext windowContext;
	CHAR title[MAX_TITLE_LENGTH];
};