#pragma once

#include <Windows.h>

class IWindowComponent
{
public:
	virtual void handle(UINT msg, WPARAM wParam, LPARAM lParam) = 0;
	virtual void initialize() = 0;
	virtual void postFrame() = 0;
};