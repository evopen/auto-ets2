#pragma once

// clang-format off
#include "LaneDetector_pt.h"
#include "LaneDetector.h"
// clang-format on

#include "config.h"

#define OPENCV

#include <darknet/yolo_v2_class.hpp>
#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <future>
#include <vector>

const int lane_view_height = 300;
const int lane_view_width  = 500;


// assuming 16:9 pixel ratio
class InfoCollector
{
public:
    InfoCollector(int width, int height) : width_(width), height_(height)
    {
        speed_        = 0;
        speed_limit_  = 0;
        cruise_speed_ = 0;

        speed_ocr_        = new tesseract::TessBaseAPI();
        speed_limit_ocr_  = new tesseract::TessBaseAPI();
        cruise_speed_ocr_ = new tesseract::TessBaseAPI();

        speed_ocr_->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);
        speed_limit_ocr_->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);
        cruise_speed_ocr_->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);

        speed_ocr_->SetVariable("tessedit_char_whitelist", "0123456789");
        speed_ocr_->SetPageSegMode(tesseract::PSM_SINGLE_WORD);
        speed_limit_ocr_->SetVariable("tessedit_char_whitelist", "0123456789");
        speed_limit_ocr_->SetPageSegMode(tesseract::PSM_SINGLE_WORD);
        cruise_speed_ocr_->SetVariable("tessedit_char_whitelist", "0123456789");
        cruise_speed_ocr_->SetPageSegMode(tesseract::PSM_SINGLE_WORD);
    }
    void InitCollector()
    {
        drive_window_rect_ = GetRectFromRegion(config::drive_window_region);
        speed_rect_        = GetRectFromRegion(config::speed_region);
        speed_limit_rect_  = GetRectFromRegion(config::speed_limit_region);
        cruise_speed_rect_ = GetRectFromRegion(config::cruise_control_speed_region);
        map_rect_          = GetRectFromRegion(config::map_region);
    }

    void InitLaneDetector(std::filesystem::path model_path)
    {
        if (erf_lane_detector)
        {
            delete (erf_lane_detector);
        }
        else
        {
            erf_lane_detector = new LaneDetector(model_path, 976, 208, {103.939, 116.779, 123.68}, {1, 1, 1});
            erf_lane_img      = cv::Mat::zeros(208, 976, CV_8UC1);
        }

        if (scnn_lane_detector)
        {
            delete (scnn_lane_detector);
        }
        else
        {
            scnn_lane_detector =
                new LaneDetectorTRT("D:/Dev/auto-ets2/frozen_models/6.6_2060_fp32_288.800_36.100_explicitBatch.trt");
            scnn_lane_img = cv::Mat::zeros(36, 100, CV_8SC1);
        }

        for (int i = 0; i < lanes.size(); i++)
        {
            lanes[i] = cv::Mat::zeros(lane_view_height, lane_view_width, CV_8UC1);
        }
    }

    void InitObjectDetector(
        std::filesystem::path cfg_path, std::filesystem::path weight_path, std::filesystem::path obj_name_file)
    {
        obj_detector      = new Detector(cfg_path.string(), weight_path.string());
        obj_detector->nms = 0.02;
        obj_names_        = ObjectNamesFromFile(obj_name_file.string());
        obj_img           = cv::Mat::zeros(100, 100, CV_8UC1);
    }

    void SetScreenshot(cv::Mat screenshot) { screenshot_ = screenshot; }
    void CropToGameWindow(cv::Rect rect) { game_window_ = screenshot_(rect); }
    void CropRegion()
    {
        drive_window_ = game_window_(drive_window_rect_);
        speed_img_    = game_window_(speed_rect_);
        cv::cvtColor(speed_img_, speed_img_, cv::COLOR_BGR2GRAY);
        speed_img_       = cv::Scalar(255) - speed_img_;
        speed_limit_img_ = game_window_(speed_limit_rect_);
        cv::cvtColor(speed_limit_img_, speed_limit_img_, cv::COLOR_BGR2GRAY);

        cruise_speed_img_ = game_window_(cruise_speed_rect_);
        cv::cvtColor(cruise_speed_img_, cruise_speed_img_, cv::COLOR_BGR2GRAY);
        cruise_speed_img_ = cv::Scalar(255) - cruise_speed_img_;

        map_img_ = game_window_(map_rect_);
    }
    void ReadInfo()
    {
        std::thread erf_thread(&LaneDetector::Detect, erf_lane_detector, drive_window_.clone(), erf_lane_img);
        std::thread scnn_thread(&LaneDetectorTRT::Detect, scnn_lane_detector, drive_window_.clone(), scnn_lane_img);

        auto obj_detect = std::async(&InfoCollector::detect, this, drive_window_, 0.5f, false);

        std::thread t1(&InfoCollector::ReadNumber, this, speed_ocr_, speed_img_, &speed_);
        std::thread t2(&InfoCollector::ReadNumber, this, speed_limit_ocr_, speed_limit_img_, &speed_limit_);
        std::thread t3(&InfoCollector::ReadNumber, this, cruise_speed_ocr_, cruise_speed_img_, &cruise_speed_);

        t1.join();
        t2.join();
        t3.join();
        std::vector<bbox_t> objs = obj_detect.get();
        objs                     = obj_detector->tracking_id(objs);
        erf_thread.join();
        scnn_thread.join();

        obj_img = drive_window_.clone();
        draw_boxes(obj_img, objs, obj_names_);
    }

    void MergeLaneResult()
    {
        std::array<cv::Mat, 4> scnn_tmp;
        cv::resize(scnn_lane_detector->lanes[0], scnn_tmp[0], cv::Size(lane_view_width, lane_view_height), 0, 0,
            cv::INTER_NEAREST);
        cv::resize(scnn_lane_detector->lanes[1], scnn_tmp[1], cv::Size(lane_view_width, lane_view_height), 0, 0,
            cv::INTER_NEAREST);
        cv::resize(scnn_lane_detector->lanes[2], scnn_tmp[2], cv::Size(lane_view_width, lane_view_height), 0, 0,
            cv::INTER_NEAREST);
        cv::resize(scnn_lane_detector->lanes[3], scnn_tmp[3], cv::Size(lane_view_width, lane_view_height), 0, 0,
            cv::INTER_NEAREST);

        std::array<cv::Mat, 4> erf_tmp;
        cv::resize(erf_lane_detector->lanes[0], erf_tmp[0], cv::Size(lane_view_width, lane_view_height), 0, 0,
            cv::INTER_NEAREST);
        cv::resize(erf_lane_detector->lanes[1], erf_tmp[1], cv::Size(lane_view_width, lane_view_height), 0, 0,
            cv::INTER_NEAREST);
        cv::resize(erf_lane_detector->lanes[2], erf_tmp[2], cv::Size(lane_view_width, lane_view_height), 0, 0,
            cv::INTER_NEAREST);
        cv::resize(erf_lane_detector->lanes[3], erf_tmp[3], cv::Size(lane_view_width, lane_view_height), 0, 0,
            cv::INTER_NEAREST);

        for (int i = 0; i < 4; i++)
        {
            cv::bitwise_or(scnn_tmp[i], erf_tmp[i], lanes[i]);
        }

        cv::bitwise_or(lanes[0], lanes[1], all_lanes);
        cv::bitwise_or(lanes[2], all_lanes, all_lanes);
        cv::bitwise_or(lanes[3], all_lanes, all_lanes);
    }


public:
    int speed_;
    int speed_limit_;
    int cruise_speed_;

    int width_;
    int height_;

    cv::Mat screenshot_;
    cv::Mat game_window_;
    cv::Mat drive_window_;

    cv::Mat wing_mirror_left_;
    cv::Mat wing_mirror_right_;
    cv::Mat speed_limit_img_;
    cv::Mat speed_img_;
    cv::Mat cruise_speed_img_;
    cv::Mat map_img_;
    cv::Mat erf_lane_img;
    cv::Mat scnn_lane_img;
    cv::Mat lane_img_large;
    cv::Mat obj_img;


    std::array<cv::Mat, 4> lanes;
    cv::Mat all_lanes;

    cv::Rect GetRectFromRegion(const float region[4])
    {
        int x             = float(width_) * region[0];
        int y             = float(height_) * region[1];
        int region_width  = float(width_) * region[2];
        int region_height = float(height_) * region[3];
        return cv::Rect(x, y, region_width, region_height);
    }

private:
    cv::Rect drive_window_rect_;
    cv::Rect speed_rect_;
    cv::Rect speed_limit_rect_;
    cv::Rect cruise_speed_rect_;
    cv::Rect map_rect_;

    tesseract::TessBaseAPI* speed_ocr_;
    tesseract::TessBaseAPI* speed_limit_ocr_;
    tesseract::TessBaseAPI* cruise_speed_ocr_;

    std::vector<std::string> obj_names_;

    Detector* obj_detector;

    LaneDetectorTRT* scnn_lane_detector = nullptr;

    LaneDetector* erf_lane_detector = nullptr;

    bool ReadNumber(tesseract::TessBaseAPI* ocr, cv::Mat img, int* number)
    {
        ocr->SetImage(img.data, img.size().width, img.size().height, img.channels(), img.step1());
        ocr->SetSourceResolution(300);
        ocr->Recognize(0);

        std::string outText = std::string(ocr->GetUTF8Text());
        if (outText.empty())
        {
            return false;
        }
        else
        {
            *number = atoi(outText.c_str());
            return true;
        }
    }

    std::vector<std::string> ObjectNamesFromFile(std::string const filename)
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

    std::vector<bbox_t> detect(cv::Mat mat, float thresh = 0.2, bool use_mean = false)
    {
        if (mat.data == NULL)
            throw std::runtime_error("Image is empty");
        auto image_ptr = obj_detector->mat_to_image_resize(mat);
        return obj_detector->detect_resized(*image_ptr, mat.cols, mat.rows, thresh, use_mean);
    }

    void draw_boxes(cv::Mat mat_img, std::vector<bbox_t> result_vec, std::vector<std::string> obj_names)
    {
        for (auto& i : result_vec)
        {
            cv::Scalar color(60, 160, 260);
            cv::rectangle(mat_img, cv::Rect(i.x, i.y, i.w, i.h), color, 3);
            if (obj_names.size() > i.obj_id)
                putText(
                    mat_img, obj_names[i.obj_id], cv::Point2f(i.x, i.y - 10), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, color);
            if (i.track_id > 0)
                putText(mat_img, std::to_string(i.track_id), cv::Point2f(i.x + 5, i.y + 15),
                    cv::FONT_HERSHEY_COMPLEX_SMALL, 1, color);
        }
    }
};