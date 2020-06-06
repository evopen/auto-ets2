#include "ScreenCapturer.h"

#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>

#include <chrono>

TEST(ScreenCapturerTest, CreateDevice)
{
    ScreenCapturer capturer;

    cv::Mat img;
    capturer.Init();
    cv::namedWindow("asdf");

    while (true)
    {

        img = capturer.Capture();
        cv::resize(img, img, cv::Size(640, 360));
        cv::imshow("asdf", img);
        if (cv::waitKey(1) == 27)
            break;
    }
}