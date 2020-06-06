#pragma once

#include "ScreenCapturer.h"

#include <opencv2/opencv.hpp>

// assuming 16:9 pixel ratio
class InfoCollector
{
public:
    void InitCollector();
    void SetScreenshot(cv::Mat screenshot) { screenshot_ = screenshot; }
    

private:
    int speed_;
    int speed_limit_;
    int cruise_speed_;

    cv::Mat screenshot_;
    cv::Mat game_window_;
    cv::Mat drive_window_;
    cv::Mat wing_mirror_left_;
    cv::Mat wing_mirror_right_;
    cv::Mat speed_limit_img_;
    cv::Mat speed_img_;
    cv::Mat cruise_speed_img_;
    cv::Mat map_img_;
};