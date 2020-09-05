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

            if (std::abs(nav_->angle_50_pix) > 0.6)
            {
                speed_cap = 30.f;
            }
            else
            {

                speed_cap = ((1.5 - std::abs(nav_->angle_50_pix)) + 1) * 32;
            }
            std::cout << "angle 50: " << nav_->angle_50_pix << std::endl;
            speed_cap = std::min(speed_cap, collector_->speed_limit_);
            std::cout << "speed cap: " << speed_cap << std::endl;
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
            std::cout << "left angle: " << nav_->angle_10_pix_left << std::endl;
            if (nav_->angle_10_pix_left > 0)
                input_->SetWheelAngle(std::min(nav_->angle_10_pix_left, 1.f));
            else
                input_->SetWheelAngle(std::max(nav_->angle_10_pix_left, -1.f));
        }
        else
        {
            std::cout << "right angle: " << nav_->angle_10_pix_right << std::endl;
            if (nav_->angle_10_pix_right > 0)
                input_->SetWheelAngle(std::min(nav_->angle_10_pix_right, 1.f));
            else
                input_->SetWheelAngle(std::max(nav_->angle_10_pix_right, -1.f));
        }
    }

    void SpeedUpToLimit()
    {
        if (collector_->speed_ < 30)
        {
            input_->SetThrottleAndBrake(0.8);
            std::cout << "full throttle" << std::endl;
            return;
        }
        if (collector_->speed_ < speed_cap)
        {
            input_->SetThrottleAndBrake(0.6);
            std::cout << "0.6 throttle" << std::endl;
            return;
        }
        else if (collector_->speed_ == speed_cap)
        {
            input_->SetThrottleAndBrake(0.3);
            std::cout << "0.3 throttle" << std::endl;
            return;
        }
        else if (collector_->speed_ - speed_cap > 10)
        {
            input_->SetThrottleAndBrake(-0.99);
            std::cout << "brake: 1" << std::endl;
        }
        else
        {
            float brake_force = -(float) (collector_->speed_ - speed_cap) * 4 / speed_cap;
            input_->SetThrottleAndBrake(brake_force);
            printf("brake: %.2f", brake_force);
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
            if (shifted_ms > collector_->speed_ * 500)
            {
                shift_stage      = 2;
                shift_start_time = std::chrono::steady_clock::now();
                std::cout << "stage 2 started" << std::endl;
            }
            break;
        case 2:
            input_->SetWheelAngle(0.05);
            if (shifted_ms > collector_->speed_ * 200)
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
            if (shifted_ms > collector_->speed_ * 500)
            {
                shift_stage      = 2;
                shift_start_time = std::chrono::steady_clock::now();
                std::cout << "stage 2 started" << std::endl;
            }
            break;
        case 2:
            input_->SetWheelAngle(-0.05);
            if (shifted_ms > collector_->speed_ * 200)
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
            if (pix_ptr[200 * 400 + i] != 0)
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
                if (pix_ptr[200 * 400 + i] != 0)
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
        float controller_x = (lane_pix_loc - 200.f) * 0.02 * (1.0 / (collector_->speed_ + 10) * 2);
        std::cout << "set: " << controller_x << std::endl;
        last_steering = (controller_x + last_steering) / 2;
        input_->SetWheelAngle(std::min(last_steering, 1.f));
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