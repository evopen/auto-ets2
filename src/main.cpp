#include "InfoCollector.h"
#include "ScreenCapturer.h"

#include <Windows.h>
#include <opencv2/opencv.hpp>

int main()
{
    LoadLibraryA("torch_cuda.dll");
    try
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
        collector.InitLaneDetector("D:/DHH/Downloads/erf_net.pt");
        collector.InitObjectDetector("D:/Dev/auto-ets2/weights/yolov3.cfg", "D:/Dev/auto-ets2/weights/yolov3.weights",
            "D:/Dev/auto-ets2/weights/coco.names");

        ScreenCapturer capturer;
        cv::Mat screenshot;
        capturer.Init();

        cv::namedWindow("game");
        cv::namedWindow("speed");
        cv::namedWindow("limit");
        cv::namedWindow("cruise");
        cv::namedWindow("lane");

        std::string speed_window_title        = "speed";
        std::string speed_limit_window_title  = "limit";
        std::string cruise_speed_window_title = "cruise";

        std::cout << "here1" << std::endl;
        while (true)
        {
            screenshot = capturer.Capture();
            collector.SetScreenshot(screenshot);
            collector.CropToGameWindow(rect);
            collector.CropRegion();
            collector.ReadInfo();
            // collector.LanePostprocess();
            cv::imshow("game", collector.game_window_);
            // cv::imshow("drive", collector.drive_window_);
            cv::imshow("speed", collector.speed_img_);
            cv::imshow("limit", collector.speed_limit_img_);
            cv::imshow("cruise", collector.cruise_speed_img_);
            cv::imshow("lane", collector.lane_img * 50);
            cv::imshow("objs", collector.obj_img);
            cv::resizeWindow("speed", 300, 50);
            cv::resizeWindow("limit", 300, 50);
            cv::resizeWindow("cruise", 300, 50);

            cv::setWindowTitle(speed_window_title, "speed" + std::to_string(collector.speed_));
            cv::setWindowTitle(speed_limit_window_title, "limit" + std::to_string(collector.speed_limit_));
            cv::setWindowTitle(cruise_speed_window_title, "cruise" + std::to_string(collector.cruise_speed_));

            if (cv::waitKey(1) == 27)
                break;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}