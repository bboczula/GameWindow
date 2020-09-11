#pragma once

#include <Windows.h>
#include <iostream>

#define MAX_TITLE_LENGTH 64

class WindowClass
{
public:
	WindowClass();
	~WindowClass();
	void registerWindow();
	void createWindow();
private:
	HWND hwnd;
	int width;
	int height;
	const LONG windowStyleEx;
	const LONG windowStyle;
	CHAR title[MAX_TITLE_LENGTH];
};