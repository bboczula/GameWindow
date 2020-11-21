#pragma once

#include <Windows.h>
#include <iostream>
#include "WindowContext.h"

namespace Astral
{
	class Window
	{
	public:
		Window(UINT width, UINT height);
		~Window();
		void RegisterWindowClass();
		void CreateWindowInstance();
		WindowContext GetWindowContext();
	private:
		WindowContext windowContext;
	};
}