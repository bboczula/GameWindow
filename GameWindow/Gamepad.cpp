#include "Gamepad.h"

void Gamepad::handle(UINT msg, WPARAM wParam, LPARAM lParam)
{
}

void Gamepad::initialize()
{
}

void Gamepad::postFrame()
{
    DWORD dwResult;
    XINPUT_STATE tempInputState;
    // For performance reasons, don't call XInputGetState for an 'empty' user slot every frame. We recommend that you space out checks for new controllers every few seconds instead. To implement.
    for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
    {
        ZeroMemory(&tempInputState, sizeof(XINPUT_STATE));

        // Simply get the state of the controller from XInput.
        // Not really sure if this would be here, what if there is no message on the quque? Shouldn't this be run every frame?
        // It is also important to know that this state always have some value, so probably need to calculate delta
        dwResult = XInputGetState(i, &tempInputState);

        if (dwResult == ERROR_SUCCESS)
        {
            // Here you should check if anything changed first, otherwise this will be run for each message
            if (tempInputState.dwPacketNumber != xboxControllerState[i].dwPacketNumber)
            {
                {
                    // Here we know that something has changed for given pad
                    //std::cout << (int)xboxControllerState[i].Gamepad.sThumbLX << " ";

                    // Old solution
                    //padDeltaX = fmaxf(-1.0f, (float)tempInputState.Gamepad.sThumbLX / 32767.0f);
                    //padDeltaY = fmaxf(-1.0f, (float)tempInputState.Gamepad.sThumbLY / 32767.0f);

                    float lX = tempInputState.Gamepad.sThumbLX;
                    float lY = tempInputState.Gamepad.sThumbLY;

                    // Determine how far the controller is pushed
                    float magnitude = sqrt(lX * lX + lY * lY);

                    // Determine the direction the controller is pushed
                    float normalizedX = lX / magnitude;
                    float normalizedY = lY / magnitude;

                    float normalizedMagnitude = 0;

                    // Check if the controller is outside a circular dead zone
                    if (magnitude > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
                    {
                        // Clip the magnitude at its expected maximum value
                        if (magnitude > 32767)
                        {
                            magnitude = 32767;
                        }

                        // Adjust magnitude relative to the end of dead zone
                        magnitude -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;

                        // Optionally normalize magnitude with respect to its expected range giving a magnitude value of 0.0 to 1.0f
                        normalizedMagnitude = magnitude / (32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

                        padDeltaX = fmaxf(-1.0f, lX / 32767.0f);
                        padDeltaY = fmaxf(-1.0f, lY / 32767.0f);
                    }
                    else
                    {
                        magnitude = 0.0f;
                        normalizedMagnitude = 0.0f;
                        padDeltaX = 0.0f;
                        padDeltaY = 0.0f;
                    }
                }

                {
                    // Here we know that something has changed for given pad
                    //std::cout << (int)xboxControllerState[i].Gamepad.sThumbLX << " ";

                    // Old solution
                    //padDeltaX = fmaxf(-1.0f, (float)tempInputState.Gamepad.sThumbLX / 32767.0f);
                    //padDeltaY = fmaxf(-1.0f, (float)tempInputState.Gamepad.sThumbLY / 32767.0f);

                    float rX = tempInputState.Gamepad.sThumbRX;
                    float rY = tempInputState.Gamepad.sThumbRY;

                    // Determine how far the controller is pushed
                    float magnitude = sqrt(rX * rX + rY * rY);

                    // Determine the direction the controller is pushed
                    float normalizedX = rX / magnitude;
                    float normalizedY = rY / magnitude;

                    float normalizedMagnitude = 0;

                    // Check if the controller is outside a circular dead zone
                    if (magnitude > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
                    {
                        // Clip the magnitude at its expected maximum value
                        if (magnitude > 32767)
                        {
                            magnitude = 32767;
                        }

                        // Adjust magnitude relative to the end of dead zone
                        magnitude -= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;

                        // Optionally normalize magnitude with respect to its expected range giving a magnitude value of 0.0 to 1.0f
                        normalizedMagnitude = magnitude / (32767 - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

                        padDeltaRX = fmaxf(-1.0f, rX / 32767.0f);
                        padDeltaRY = fmaxf(-1.0f, rY / 32767.0f);
                    }
                    else
                    {
                        magnitude = 0.0f;
                        normalizedMagnitude = 0.0f;
                        padDeltaRX = 0.0f;
                        padDeltaRY = 0.0f;
                    }
                }

                // Remember to copy back the state. Is it really necessary though?
                xboxControllerState[i] = tempInputState;
            }
        }
        else
        {
            // Controller is not connected 
        }
    }
}

float Gamepad::getPadXDelta()
{
    return padDeltaX;
}

float Gamepad::getPadYDelta()
{
    return padDeltaY;
}

float Gamepad::getRightPadXDelta()
{
    return padDeltaRX;
}

float Gamepad::getRightPadYDelta()
{
    return padDeltaRY;
}
