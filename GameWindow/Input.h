#pragma once

#include "IWindowComponent.h"
#include <Windows.h>
#include <windowsx.h>
#include <iostream>
#include <hidusage.h>

#define MAX_NUM_OF_KEYS 254

class Input : public IWindowComponent
{
public:
	void handle(UINT msg, WPARAM wParam, LPARAM lParam) override;
	void initialize() override;
	void postFrame() override;
	long getMouseXDelta();
	long getMouseYDelta();
	int getMousePositionX();
	int getMousePositionY();
	bool isKeyDown(WPARAM virtualKeyCode);
private:
	long deltaX, deltaY;
	short mousePosX, mousePosY;
	BOOL virtualKeyState[MAX_NUM_OF_KEYS];
};