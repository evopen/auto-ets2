#include "InfoCollector.h"
#include "Navigator.h"
#include "ScreenCapturer.h"

#include <opencv2/opencv.hpp>

int main()
{
    HWND hwnd         = FindWindowA(0, "Euro Truck Simulator 2");
    auto window_style = GetWindowLongPtrA(hwnd, GWL_STYLE);


    bool has_title_bar;
    if ((window_style & WS_CAPTION) > 1)
    {
        has_title_bar = true;
        std::cout << "window has title bar" << std::endl;
    }
    else
    {
        has_title_bar = false;
    }

    RECT rcClient, rcWind;
    GetClientRect(hwnd, &rcClient);
    GetWindowRect(hwnd, &rcWind);
    POINT start_point;
    auto title_bar_height =
        GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CYEDGE) * 2 + 3;

    start_point.x = rcWind.left + ((rcWind.right - rcWind.left) - rcClient.right) / 2;
    start_point.y = rcWind.top + title_bar_height;
    cv::Rect rect(cv::Point(start_point.x, start_point.y), cv::Size(rcClient.right, rcClient.bottom));

    InfoCollector collector(rcClient.right, rcClient.bottom);
    collector.InitCollector();
    ScreenCapturer capturer;
    capturer.Init();

    Navigator nav;
    cv::Mat screenshot;
    while (1)
    {
        screenshot = capturer.Capture();
        collector.SetScreenshot(screenshot);
        collector.CropToGameWindow(rect);
        collector.CropRegion();

        nav.SetMap(collector.map_img_);
        cv::imshow("map", nav.original_map);
        nav.ExtractPath();
        // cv::imshow("arrow", nav.arrow_img);
        // cv::imshow("red path", nav.red_path_img);


        nav.CalculatePixAngle20();

        cv::circle(
            nav.path_img, cv::Point(40 - nav.left_right_anchor_dist / 2, nav.short_probe_len), 2, cv::Scalar(150));
        cv::circle(
            nav.path_img, cv::Point(40 + nav.left_right_anchor_dist / 2, nav.short_probe_len), 2, cv::Scalar(150));
        cv::imshow("path", nav.path_img);

        printf("left: %.3f\n", nav.angle_10_pix_left);
        printf("right: %.3f\n", nav.angle_10_pix_right);

        if (cv::waitKey(20) == 27)
            break;
    }
    return 0;
}