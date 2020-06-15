#pragma once

#include "InfoCollector.h"
#include "Input.h"
#include "Navigator.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


enum class Directive
{
    kStraight,
    kShiftRight,
    kShiftLeft,
    kTurn
};

enum class Goal
{
    kShiftRightMost,
    kShiftLeftMost,
    kOvertake,
};


class Control
{
public:
    Control(Input* input, InfoCollector* collector) : input_(input), collector_(collector)
    {
        key_pressed_time = std::chrono::steady_clock::now();
    }

    void SetNavigator(Navigator* nav) { nav_ = nav; }

    void Process()
    {
        GetDirectiveFromKB();
        if (!auto_pilot_enabled)
        {
            return;
        }
        if (speed_control_enabled)
        {
            SpeedUpToLimit();
        }

        if (nav_ != nullptr)
        {
            float dominate_angle = std::max(std::abs(nav_->angle_10_pix_left), std::abs(nav_->angle_10_pix_right));
            if (dominate_angle > CV_PI / 60)
            {
                directive = Directive::kTurn;
                std::cout << "set directive <Turn>" << std::endl;
            }
            else if (directive == Directive::kTurn && dominate_angle < CV_PI / 60)
            {
                /*input_->SetWheelAngle(0);*/
                directive = Directive::kStraight;
            }
        }

        switch (directive)
        {
        case Directive::kStraight:
            KeepStraight();
            break;
        case Directive::kShiftLeft:
            ShiftLeft();
            break;
        case Directive::kShiftRight:
            ShiftRight();
            break;
        case Directive::kTurn:
            Turn();
            break;
        }
    }

    void Turn()
    {
        if (std::abs(nav_->angle_10_pix_left) > std::abs(nav_->angle_10_pix_right))
        {
            input_->SetWheelAngle(nav_->angle_10_pix_left * 1.5);
        }
        else
        {
            input_->SetWheelAngle(nav_->angle_10_pix_right * 1.5);
        }
    }

    void SpeedUpToLimit()
    {
        if (collector_->speed_ < 50)
        {
            input_->SetThrottleAndBrake(0.99);
            std::cout << "full throttle" << std::endl;
            return;
        }
        if (collector_->speed_ < collector_->speed_limit_ - 5)
        {
            input_->SetThrottleAndBrake(0.6);
            std::cout << "0.6 throttle" << std::endl;
            return;
        }

        int speed_up_room = collector_->speed_limit_ - collector_->speed_;
        if (speed_up_room > 0)
        {
            input_->SetThrottleAndBrake(0.3);
            std::cout << "0.3 throttle" << std::endl;
        }
        else
        {
            input_->SetThrottleAndBrake(-0.1);
            std::cout << "0.1 brake" << std::endl;
        }
    }

    void ShiftLeft()
    {
        auto shifted_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - shift_start_time)
                .count();
        switch (shift_stage)
        {
        case 0:
            std::cout << "stage 0" << std::endl;
            shift_stage      = 1;
            shift_start_time = std::chrono::steady_clock::now();
            std::cout << "stage 1 started" << std::endl;
            break;
        case 1:
            std::cout << "stage 1" << std::endl;

            input_->SetWheelAngle(-0.05);
            if (shifted_ms > 1000)
            {
                shift_stage      = 2;
                shift_start_time = std::chrono::steady_clock::now();
                std::cout << "stage 2 started" << std::endl;
            }
            break;
        case 2:
            input_->SetWheelAngle(0.05);
            if (shifted_ms > 1000)
            {
                shift_stage = 0;
                directive   = Directive::kStraight;
                input_->SetWheelAngle(0);
            }
            break;
        }
    }

    void ShiftRight()
    {
        auto shifted_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - shift_start_time)
                .count();
        switch (shift_stage)
        {
        case 0:
            std::cout << "stage 0" << std::endl;
            shift_stage      = 1;
            shift_start_time = std::chrono::steady_clock::now();
            std::cout << "stage 1 started" << std::endl;
            break;
        case 1:
            std::cout << "stage 1" << std::endl;

            input_->SetWheelAngle(0.05);
            if (shifted_ms > 1000)
            {
                shift_stage      = 2;
                shift_start_time = std::chrono::steady_clock::now();
                std::cout << "stage 2 started" << std::endl;
            }
            break;
        case 2:
            input_->SetWheelAngle(-0.05);
            if (shifted_ms > 1200)
            {
                shift_stage = 0;
                directive   = Directive::kStraight;
                input_->SetWheelAngle(0);
            }
            break;
        }
    }


    void KeepStraight()
    {
        int lane_pix_loc = 0;
        uint8_t* pix_ptr = collector_->warp_lanes[1].data;
        for (int i = 0; i < 400; i++)
        {
            if (pix_ptr[300 * 400 + i] != 0)
            {
                lane_pix_loc = i;
                break;
            }
        }

        // backup plan
        if (lane_pix_loc == 0)
        {
            pix_ptr = collector_->warp_lanes[2].data;
            for (int i = 0; i < 400; i++)
            {
                if (pix_ptr[300 * 400 + i] != 0)
                {
                    lane_pix_loc = i;
                    break;
                }
            }
        }

        // do nothing
        if (lane_pix_loc == 0)
        {
            /*if (nav_ && std::abs(nav_->angle_10_pix) < 0.3)
            {
                float angle = nav_->CalculateStraightAngle();
                input_->SetWheelAngle(angle / 8);
                std::cout << "use map calculated angle: " << angle << std::endl;
                return;
            }*/
            std::cout << "use last angle in straight" << std::endl;
            return;
        }
        std::cout << "find: " << lane_pix_loc << std::endl;
        float controller_x = (lane_pix_loc - 200.f) * 0.03 * (1.0 / (collector_->speed_ + 10));
        std::cout << "set: " << controller_x << std::endl;
        last_steering = (controller_x + last_steering) / 2;
        input_->SetWheelAngle(last_steering);
    }

    void PressKey(WORD vkey)
    {
        INPUT input;
        input.type           = INPUT_KEYBOARD;
        input.ki.wScan       = MapVirtualKey(vkey, MAPVK_VK_TO_VSC);
        input.ki.time        = 0;
        input.ki.dwExtraInfo = 0;
        input.ki.wVk         = vkey;
        input.ki.dwFlags     = 0;  // there is no KEYEVENTF_KEYDOWN
        SendInput(1, &input, sizeof(INPUT));

        input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
    }

    void GetDirectiveFromKB()
    {
        if (GetKeyState('S') & 0x8000)
        {
            auto_pilot_enabled = false;
            std::cout << "auto pilot off" << std::endl;
            input_->SetThrottleAndBrake(0);
            input_->SetWheelAngle(0);
        }
        else if (GetKeyState(VK_OEM_PLUS) & 0x8000)
        {
            auto_pilot_enabled = true;
            std::cout << "auto pilot on" << std::endl;
        }

        if (GetKeyState(VK_OEM_MINUS) & 0x8000)
        {
            speed_control_enabled = !speed_control_enabled;
            if (speed_control_enabled)
                std::cout << "speed up to limit" << std::endl;
            else
                std::cout << "speed control off" << std::endl;
        }

        if (GetKeyState(VK_RIGHT) & 0x8000)
        {
            directive = Directive::kShiftRight;
            std::cout << "Shift Right" << std::endl;
        }
        else if (GetKeyState(VK_LEFT) & 0x8000)
        {
            directive = Directive::kShiftLeft;
            std::cout << "Shift Left" << std::endl;
        }
    }

    Directive directive        = Directive::kStraight;
    bool auto_pilot_enabled    = false;
    bool speed_control_enabled = false;

    float last_steering = 0;

    std::chrono::steady_clock::time_point key_pressed_time;
    std::chrono::steady_clock::time_point shift_start_time;


    int shift_stage = 0;
    int speed_cap;

private:
    Input* input_;
    InfoCollector* collector_;
    Navigator* nav_;
};