#include "Timer.h"

void Timer::initialize()
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);
	QueryPerformanceCounter((LARGE_INTEGER*)&previousTimeStamp);
}

void Timer::handle(UINT msg, WPARAM wParam, LPARAM lParam)
{
}

void Timer::postFrame()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTimeStamp);
	deltaTimeStamp = currentTimeStamp - previousTimeStamp;
	previousTimeStamp = currentTimeStamp;
}

float Timer::getDeltaTime()
{
	return (float)((deltaTimeStamp) / (float)Frequency) * 1000.0f;
}
