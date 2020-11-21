#pragma once

#include <string>
#include <Windows.h>
#include <windowsx.h>
#include <stdint.h>
#include <string>

#include "Window.h"
#include "IWindowComponent.h"
#include "Timer.h"
#include "RenderingContext.h"

namespace Astral
{
	class Engine
	{
	public:
		Engine(int width, int heigh);
		virtual ~Engine();
		void start();
	protected:
		Astral::Window window;
		Timer timer;
		RenderingContext* renderContext;
		virtual void initialize();
		virtual void tick();
		virtual void render();
	};
}