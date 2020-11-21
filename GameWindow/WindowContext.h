#pragma once

#include <Windows.h>
#include <iostream>

namespace Astral
{
	struct WindowContext
	{
		WindowContext() : hwnd(nullptr), instance(nullptr), width(1280), height(720)
		{
		}
		WindowContext(HWND hwnd, HINSTANCE instance, UINT width, UINT height) : hwnd(hwnd), instance(instance), width(width), height(height)
		{
		}
		HWND hwnd;
		HINSTANCE instance;
		LONG width;
		LONG height;
	};
}