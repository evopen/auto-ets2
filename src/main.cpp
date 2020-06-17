#include "Control.h"
#include "InfoCollector.h"
#include "Input.h"
#include "Navigator.h"
#include "ScreenCapturer.h"

#include <Windows.h>
#include <opencv2/opencv.hpp>

int main()
{
    LoadLibraryA("torch_cuda.dll");
    SetProcessDPIAware();
    try
    {
        Input input;

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
        ScreenCapturer capturer;
        Control control(&input, &collector);

        // std::thread init_obj_detector_thread(&InfoCollector::InitObjectDetector, &collector,
        //    "D:/Dev/auto-ets2/weights/yolov4.cfg", "D:/Dev/auto-ets2/weights/yolov4.weights",
        //    "D:/Dev/auto-ets2/weights/coco.names");
        std::thread init_collector_thread(&InfoCollector::InitCollector, &collector);
        std::thread init_lane_detector_thread(
            &InfoCollector::InitLaneDetector, &collector, "D:/Dev/auto-ets2/erf_net.trt");
        std::thread init_capturer_thread(&ScreenCapturer::Init, &capturer);
        Navigator nav;
        control.SetNavigator(&nav);

        init_collector_thread.join();
        init_lane_detector_thread.join();
        // init_obj_detector_thread.join();
        init_capturer_thread.join();

        cv::Mat screenshot;

        std::string speed_window_title       = "speed";
        std::string speed_limit_window_title = "limit";
        // std::string cruise_speed_window_title = "cruise";

        std::cout << "here1" << std::endl;
        while (true)
        {
            screenshot = capturer.Capture();
            collector.SetScreenshot(screenshot);
            collector.CropToGameWindow(rect);
            collector.CropRegion();
            collector.ReadInfo();
            collector.MergeLaneResult();
            collector.WarpLane();

            nav.SetMap(collector.map_img_);
            // cv::imshow("map", nav.original_map);
            nav.ExtractPath();
            nav.CalculatePixAngle20();
            nav.CalculatePixAngle50();
            // cv::imshow("map_gray", nav.map_gray_img);
            // cv::imshow("arrow", nav.arrow_img);
            // cv::imshow("red path", nav.red_path_img);
            cv::circle(nav.path_img, cv::Point(40 + nav.left_right_anchor_dist / 2, 40 - nav.short_probe_len), 2,
                cv::Scalar(150));
            cv::circle(nav.path_img, cv::Point(40 - nav.left_right_anchor_dist / 2, 40 - nav.short_probe_len), 2,
                cv::Scalar(150));
            cv::circle(nav.path_img, cv::Point(40, 40 - nav.long_probe_len), 2, cv::Scalar(150));
            cv::imshow("path", nav.path_img);
            // std::cout << "left angle: " << nav.angle_10_pix_left << std::endl;
            // std::cout << "right angle: " << nav.angle_10_pix_right << std::endl;


            // collector.HoughLane();

            control.Process();

            // cv::imshow("game", collector.game_window_);
            // cv::imshow("drive", collector.drive_window_);
            // cv::imshow("speed", collector.speed_img_);
            // cv::imshow("limit", collector.speed_limit_img_);
            // cv::imshow("cruise", collector.cruise_speed_img_);
            // cv::imshow("lane_erf", collector.erf_lane_img * 50);
            // cv::imshow("lane_scnn", collector.scnn_lane_img);
            // cv::imshow("lane1", collector.lanes[0]);
            // cv::imshow("lane2", collector.lanes[1]);
            // cv::imshow("lane1", collector.warp_lanes[1]);
            // cv::imshow("lane2", collector.warp_lanes[2]);
            // cv::imshow("lane3", collector.lanes[2]);
            // cv::imshow("lane4", collector.lanes[3]);
            // cv::imshow("all", collector.all_lanes);
            // cv::imshow("warp", collector.lane_warp_img_);
            // cv::imshow("hough", collector.lane_hough_img_);
            // cv::imshow("objs", collector.obj_img);

            // cv::resizeWindow("speed", 300, 50);
            // cv::resizeWindow("limit", 300, 50);
            // cv::resizeWindow("cruise", 300, 50);

            // cv::setWindowTitle(speed_window_title, "speed" + std::to_string(collector.speed_));
            // cv::setWindowTitle(speed_limit_window_title, "limit" + std::to_string(collector.speed_limit_));
            // cv::setWindowTitle(cruise_speed_window_title, "cruise" + std::to_string(collector.cruise_speed_));

            if (cv::waitKey(1) == 27)
                break;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}