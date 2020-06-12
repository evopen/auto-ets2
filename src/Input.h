#pragma once

// clang-format off

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <ViGEm/Client.h>
// clang-format on

#pragma comment(lib, "Setupapi.lib")


#include <limits>

class Input
{
public:
    Input()
    {
        client_ = vigem_alloc();
        ret_    = vigem_connect(client_);
        x360_   = vigem_target_x360_alloc();
        ret_    = vigem_target_add(client_, x360_);

        XUSB_REPORT_INIT(&report_);

        int count = 3;
        while (count--)
        {
            ret_ = vigem_target_x360_update(client_, x360_, report_);
            report_.sThumbLX += 100;

            Sleep(10);
        }
        report_.sThumbLX = 0;
        ret_             = vigem_target_x360_update(client_, x360_, report_);
    }


    void SetWheelAngle(float angle)
    {
        report_.sThumbLX = angle * std::numeric_limits<short>::max();
        ret_             = vigem_target_x360_update(client_, x360_, report_);
    }

private:
    PVIGEM_CLIENT client_;
    PVIGEM_TARGET x360_;
    XUSB_REPORT report_;
    VIGEM_ERROR ret_;
};