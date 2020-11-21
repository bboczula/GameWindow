#pragma once

#include <Windows.h>
#include <iostream>

namespace Astral
{
	struct WindowContext
	{
		HWND hwnd;
		HINSTANCE instance;
		LONG width;
		LONG height;
	};
}