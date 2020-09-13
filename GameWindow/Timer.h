#pragma once

#include <Windows.h>
#include <iostream>

#include "IWindowComponent.h"

class Timer
{
public:
	void start();
	void stop();
	float getElapsedTime();
private:
	LARGE_INTEGER startTimestamp;
	LARGE_INTEGER stopTimestamp;
	LARGE_INTEGER frequency;
};