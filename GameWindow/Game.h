#pragma once

#include <string>
#include <Windows.h>
#include <windowsx.h>
#include <stdint.h>
#include <string>
#include <chrono>

#include "Window.h"
#include "IWindowComponent.h"
#include "RenderingContext.h"

namespace Astral
{
	class Game
	{
	public:
		Game(int width, int height);
		virtual ~Game();
		void start();
	protected:
		Window window;
		RenderingContext* renderContext;
		float frameTime;
		virtual void Initialization();
		virtual void Logic();
		virtual void Rendering();
	};
}