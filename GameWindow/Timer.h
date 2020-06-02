#pragma once

#include <Windows.h>
#include <iostream>

#include "IWindowComponent.h"

class Timer : public IWindowComponent
{
public:
	void initialize() override;
	void handle(UINT msg, WPARAM wParam, LPARAM lParam) override;
	void postFrame() override;
	float getDeltaTime();
private:
	__int64 StartingTime, EndingTime, ElapsedMicroseconds;
	__int64 previousTimeStamp, currentTimeStamp, deltaTimeStamp;
	__int64 Frequency;
};