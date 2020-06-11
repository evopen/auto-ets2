#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#define CUDNN_HALF

#define OPENCV

#include "opencv2/core/version.hpp"

#include <darknet/yolo_v2_class.hpp>  // imported functions from DLL
#include <opencv2/opencv.hpp>  // C++


void draw_boxes(
    cv::Mat mat_img, std::vector<bbox_t> result_vec, std::vector<std::string> obj_names, unsigned int wait_msec = 0)
{
    for (auto& i : result_vec)
    {
        cv::Scalar color(60, 160, 260);
        cv::rectangle(mat_img, cv::Rect(i.x, i.y, i.w, i.h), color, 3);
        if (obj_names.size() > i.obj_id)
            putText(mat_img, obj_names[i.obj_id], cv::Point2f(i.x, i.y - 10), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, color);
        if (i.track_id > 0)
            putText(mat_img, std::to_string(i.track_id), cv::Point2f(i.x + 5, i.y + 15), cv::FONT_HERSHEY_COMPLEX_SMALL,
                1, color);
    }
    cv::imshow("window name", mat_img);
    cv::waitKey(wait_msec);
}


void show_result(std::vector<bbox_t> const result_vec, std::vector<std::string> const obj_names)
{
    for (auto& i : result_vec)
    {
        if (obj_names.size() > i.obj_id)
            std::cout << obj_names[i.obj_id] << " - ";
        std::cout << "obj_id = " << i.obj_id << ",  x = " << i.x << ", y = " << i.y << ", w = " << i.w
                  << ", h = " << i.h << std::setprecision(3) << ", prob = " << i.prob << std::endl;
    }
}

std::vector<std::string> objects_names_from_file(std::string const filename)
{
    std::ifstream file(filename);
    std::vector<std::string> file_lines;
    if (!file.is_open())
        return file_lines;
    for (std::string line; file >> line;)
        file_lines.push_back(line);
    std::cout << "object names loaded /n";
    return file_lines;
}

cv::Rect GetRectFromRegion(const float region[4], int width, int height)
{
    int x             = float(width) * region[0];
    int y             = float(height) * region[1];
    int region_width  = float(width) * region[2];
    int region_height = float(height) * region[3];
    return cv::Rect(x, y, region_width, region_height);
}


int main()
{
    Detector detector("D:/Dev/auto-ets2/weights/yolov3.cfg", "D:/Dev/auto-ets2/weights/yolov3.weights");

    auto obj_names = objects_names_from_file("D:/Dev/auto-ets2/weights/coco.names");

    try
    {
        std::filesystem::path image_path("D:/Dev/auto-ets2/images");

        auto dir_iter = std::filesystem::directory_iterator(image_path);

        std::cout << "ready" << std::endl;
        int count = 0;
        std::vector<cv::Mat> imgs;

        const float drive_window_region[4] = {600.0 / 1920, 280.0 / 1080, 1000.0 / 1920, 360.0 / 1080};
        auto rect                          = GetRectFromRegion(drive_window_region, 1920, 1080);
        for (auto p : dir_iter)
        {
            count++;
            if (count % 2 == 0)
                continue;
            cv::Mat img = cv::imread(p.path().string());
            img         = img(rect);
            imgs.emplace_back(img);

            if (count == 101)
            {
                break;
            }
        }
        auto start = std::chrono::high_resolution_clock::now();

        for (const auto& img : imgs)
        {
            std::vector<bbox_t> result_vec = detector.detect(img, 0.5);
            /*draw_boxes(img, result_vec, obj_names);
            show_result(result_vec, obj_names);*/
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
        std::cout << count << std::endl;
    }
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << "/n";
        getchar();
    }
    catch (...)
    {
        std::cerr << "unknown exception /n";
        getchar();
    }


    return 0;
}