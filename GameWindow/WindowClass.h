#pragma once

#include <Windows.h>
#include <iostream>
#include "DisplayDescriptor.h"

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
private:
	HWND hwnd;
	DisplayDescriptor display;
	const LONG windowStyleEx;
	const LONG windowStyle;
	CHAR title[MAX_TITLE_LENGTH];
};