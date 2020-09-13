#include "Timer.h"

void Timer::start()
{
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&startTimestamp);
}

void Timer::stop()
{
	QueryPerformanceCounter(&stopTimestamp);
}

float Timer::getElapsedTime()
{
	return (float)(((double)(stopTimestamp.QuadPart - startTimestamp.QuadPart)) / ((double)frequency.QuadPart));;
}
