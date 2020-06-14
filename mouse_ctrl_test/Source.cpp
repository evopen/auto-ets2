#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <iostream>
#include <thread>

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0500

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>


#define SCREEN_WIDTH  1920
#define SCREEN_HEIGHT 1080


void MouseSetup(INPUT* buffer)
{
    buffer->type           = INPUT_MOUSE;
    buffer->mi.dx          = (0 * (0xFFFF / SCREEN_WIDTH));
    buffer->mi.dy          = (0 * (0xFFFF / SCREEN_HEIGHT));
    buffer->mi.mouseData   = 0;
    buffer->mi.dwFlags     = MOUSEEVENTF_ABSOLUTE;
    buffer->mi.time        = 0;
    buffer->mi.dwExtraInfo = 0;
}


void MouseMoveAbsolute(INPUT* buffer, int x, int y)
{
    buffer->mi.dx      = (x * (0xFFFF / SCREEN_WIDTH));
    buffer->mi.dy      = (y * (0xFFFF / SCREEN_HEIGHT));
    buffer->mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE);

    SendInput(1, buffer, sizeof(INPUT));
}


int main(int argc, char* argv[])
{
    SetProcessDPIAware();

    INPUT buffer;

    MouseSetup(&buffer);


    POINT p;
    int x = 100;
    while (1)
    {
        GetPhysicalCursorPos(&p);
        std::cout << p.x << '\t' << p.y << std::endl;
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(0.5s);

        MouseMoveAbsolute(&buffer, x, p.y);
        x += 1;
    }

    return 0;
}
