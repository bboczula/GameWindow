#pragma once

#include <Windows.h>
#include <iostream>
#include "WindowContext.h"

#define MAX_TITLE_LENGTH 64

namespace Astral
{
	class Window
	{
	public:
		Window(UINT width, UINT height);
		~Window();
		void RegisterWindowClass();
		void CreateWindowInstance();
		Astral::WindowContext GetWindowContext();
	private:
		Astral::WindowContext windowContext;
		CHAR title[MAX_TITLE_LENGTH];
	};
}