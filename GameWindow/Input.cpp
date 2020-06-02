#include "Input.h"

void Input::handle(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INPUT:
    {
#if 1
        UINT dwSize = { 0 };
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));

        LPBYTE lpb = new BYTE[dwSize];
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

        RAWINPUT* raw = (RAWINPUT*)lpb;
        if (raw->header.dwType == RIM_TYPEKEYBOARD)
        {
            // std::cout << "Keyboard: makeCode: " << raw->data.keyboard.MakeCode << ", flags: " << raw->data.keyboard.Flags << ", reserved: " << raw->data.keyboard.Reserved << ", extraInformation: " << raw->data.keyboard.ExtraInformation << ", message: " << raw->data.keyboard.Message << ", vKey: " << raw->data.keyboard.VKey << std::endl;
            if (raw->data.keyboard.Flags == 0)
            {
                virtualKeyState[raw->data.keyboard.VKey] = true;
            }
            else
            {
                virtualKeyState[raw->data.keyboard.VKey] = false;
            }
        }
        else if (raw->header.dwType == RIM_TYPEMOUSE)
        {
            //std::cout << "Mouse: usFlags: " << raw->data.mouse.usFlags << ", ulButtons: " << raw->data.mouse.ulButtons << ", usButtonFlags: " << raw->data.mouse.usButtonFlags << ", usButtonData: " << raw->data.mouse.usButtonData << ", ulRawButtons: " << raw->data.mouse.ulRawButtons << ", lastX: " << raw->data.mouse.lLastX << ", lastY: " << raw->data.mouse.lLastY << ", extraInfo: " << raw->data.mouse.ulExtraInformation << std::endl;
            deltaX = raw->data.mouse.lLastX;
            deltaY = raw->data.mouse.lLastY;
        }
        delete[] lpb;
#else
#endif
        break;
    }
    case WM_MOUSEMOVE:
    {
        mousePosX = GET_X_LPARAM(lParam);
        mousePosY = GET_Y_LPARAM(lParam);
        //std::cout << "INPUT->WM_MOUSEMOVE xPos: " << xPos << ", yPos: " << yPos << std::endl;
        break;
    }
    case WM_LBUTTONDOWN:
    {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);
        //std::cout << "Click! (" << xPos << "," << yPos << ") ";
    }
    }
}

void Input::initialize()
{
    RAWINPUTDEVICE devices[2];

    devices[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    devices[0].usUsage = HID_USAGE_GENERIC_MOUSE;
    devices[0].dwFlags = 0;
    devices[0].hwndTarget = 0;

    devices[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
    devices[1].usUsage = HID_USAGE_GENERIC_KEYBOARD;
    devices[1].dwFlags = 0;
    devices[1].hwndTarget = 0;

    if (RegisterRawInputDevices(devices, 2, sizeof(RAWINPUTDEVICE)) == FALSE)
    {
        std::cout << "ERROR: RawInput devices registration failes!" << std::endl;
    }
    else
    {
        std::cout << "RawInput devices registration succeded." << std::endl;
    }
}

void Input::postFrame()
{
    deltaX = 0;
    deltaY = 0;
}

long Input::getMouseXDelta()
{
    return deltaX;
}

long Input::getMouseYDelta()
{
    return deltaY;
}

int Input::getMousePositionX()
{
    return mousePosX;
}

int Input::getMousePositionY()
{
    return mousePosY;
}

bool Input::isKeyDown(WPARAM virtualKeyCode)
{
    return virtualKeyState[virtualKeyCode];
}
