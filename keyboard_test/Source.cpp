#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <iostream>

int main()
{
    while (1)
    {

        if (GetKeyState('S') & 0x8000)
        {
            std::cout << "S\n";
        }
        if (GetKeyState(VK_OEM_PLUS) & 0x8000)
        {
            std::cout << "=\n";
        }
        if (GetKeyState(VK_LEFT) & 0x8000)
        {
            std::cout << "left\n";
        }

        INPUT input;
        WORD vkey            = 'C';  // see link below
        input.type           = INPUT_KEYBOARD;
        input.ki.wScan       = MapVirtualKey(vkey, MAPVK_VK_TO_VSC);
        input.ki.time        = 0;
        input.ki.dwExtraInfo = 0;
        input.ki.wVk         = vkey;
        input.ki.dwFlags     = 0;  // there is no KEYEVENTF_KEYDOWN
        SendInput(1, &input, sizeof(INPUT));

        input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));

        Sleep(20);
    }
    return 0;
}