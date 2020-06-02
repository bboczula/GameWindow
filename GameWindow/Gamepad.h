#pragma once

#include "IWindowComponent.h"
#include <math.h>
#include <Xinput.h>

#define MAX_NUM_OF_CONTROLLERS 4

class Gamepad : public IWindowComponent
{
public:
	void handle(UINT msg, WPARAM wParam, LPARAM lParam) override;
	void initialize() override;
	void postFrame() override;
	float getPadXDelta();
	float getPadYDelta();
	float getRightPadXDelta();
	float getRightPadYDelta();
private:
	float padDeltaX, padDeltaY;
	float padDeltaRX, padDeltaRY;
	XINPUT_STATE xboxControllerState[MAX_NUM_OF_CONTROLLERS];
};