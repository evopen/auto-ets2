#pragma once

// clang-format off
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

const int lane_view_height = 400;
const int lane_view_width  = 400;


// assuming 16:9 pixel ratio
class InfoCollector
{
public:
    InfoCollector(int width, int height) : width_(width), height_(height)
    {
        speed_       = 0;
        speed_limit_ = 0;
        // cruise_speed_ = 0;

        speed_ocr_       = new tesseract::TessBaseAPI();
        speed_limit_ocr_ = new tesseract::TessBaseAPI();
        // cruise_speed_ocr_ = new tesseract::TessBaseAPI();

        speed_ocr_->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);
        speed_limit_ocr_->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);
        // cruise_speed_ocr_->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);

        speed_ocr_->SetVariable("tessedit_char_whitelist", "0123456789");
        speed_ocr_->SetPageSegMode(tesseract::PSM_SINGLE_WORD);
        speed_limit_ocr_->SetVariable("tessedit_char_whitelist", "0123456789");
        speed_limit_ocr_->SetPageSegMode(tesseract::PSM_SINGLE_WORD);
        // cruise_speed_ocr_->SetVariable("tessedit_char_whitelist", "0123456789");
        // cruise_speed_ocr_->SetPageSegMode(tesseract::PSM_SINGLE_WORD);
    }
    void InitCollector()
    {
        drive_window_rect_ = GetRectFromRegion(config::drive_window_region);
        speed_rect_        = GetRectFromRegion(config::speed_region);
        speed_limit_rect_  = GetRectFromRegion(config::speed_limit_region);
        cruise_speed_rect_ = GetRectFromRegion(config::cruise_control_speed_region);
        map_rect_          = GetRectFromRegion(config::map_region);
        right_mirror_rect_ = GetRectFromRegion(config::right_mirror_region);
        left_mirror_rect_  = GetRectFromRegion(config::left_mirror_region);

        {
            std::array<cv::Point2f, 4> original_points;
            std::array<cv::Point2f, 4> new_points;
            original_points[0] = {300 / 2, 700 / 2};
            original_points[1] = {745 / 2, 700 / 2};
            original_points[2] = {450 / 2, 400 / 2};
            original_points[3] = {600 / 2, 400 / 2};
            new_points[0]      = {200 / 2, 800 / 2};
            new_points[1]      = {600 / 2, 800 / 2};
            new_points[2]      = {300 / 2, 100 / 2};
            new_points[3]      = {900 / 2, 100 / 2};

            lane_warp_mat_ = cv::getPerspectiveTransform(original_points.data(), new_points.data());

            lane_warp_img_  = cv::Mat::zeros(lane_view_height, lane_view_width, CV_8UC1);
            lane_hough_img_ = cv::Mat::zeros(lane_view_height, lane_view_width, CV_8UC1);
        }

        {
            std::array<cv::Point2f, 4> original_points;
            std::array<cv::Point2f, 4> new_points;
            original_points[0] = {55, 400};
            original_points[1] = {130, 400};
            original_points[2] = {100, 250};
            original_points[3] = {150, 250};
            new_points[0]      = {100, 400};
            new_points[1]      = {300, 400};
            new_points[2]      = {100, 100};
            new_points[3]      = {300, 100};

            lane_warp_mat[1] = cv::getPerspectiveTransform(original_points.data(), new_points.data());
        }

        {
            std::array<cv::Point2f, 4> original_points;
            std::array<cv::Point2f, 4> new_points;
            original_points[0] = {205, 400};
            original_points[1] = {270, 400};
            original_points[2] = {150, 250};
            original_points[3] = {200, 250};
            new_points[0]      = {100, 400};
            new_points[1]      = {300, 400};
            new_points[2]      = {100, 100};
            new_points[3]      = {300, 100};

            lane_warp_mat[2] = cv::getPerspectiveTransform(original_points.data(), new_points.data());
        }
    }

    void InitLaneDetector(std::filesystem::path model_path)
    {
        if (erf_lane_detector)
        {
            delete (erf_lane_detector);
        }
        else
        {
            erf_lane_detector = new LaneDetectorTRT(model_path, 976, 208, {103.939, 116.779, 123.68}, {1, 1, 1});
            erf_lane_img      = cv::Mat::zeros(lane_view_height, lane_view_width, CV_8UC1);
        }


        for (int i = 0; i < lanes.size(); i++)
        {
            lanes[i] = cv::Mat::zeros(lane_view_height, lane_view_width, CV_8UC1);
        }
    }

    void InitObjectDetector(
        std::filesystem::path cfg_path, std::filesystem::path weight_path, std::filesystem::path obj_name_file)
    {
        obj_detector = new Detector(cfg_path.string(), weight_path.string());

        obj_detector->nms = 0;
        obj_names_        = ObjectNamesFromFile(obj_name_file.string());
        obj_img           = cv::Mat::zeros(100, 100, CV_8UC1);

        enable_obj_detect = true;
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

        // cruise_speed_img_ = game_window_(cruise_speed_rect_);
        // cv::cvtColor(cruise_speed_img_, cruise_speed_img_, cv::COLOR_BGR2GRAY);
        // cruise_speed_img_ = cv::Scalar(255) - cruise_speed_img_;

        map_img_           = game_window_(map_rect_);
        wing_mirror_left_  = game_window_(left_mirror_rect_);
        wing_mirror_right_ = game_window_(right_mirror_rect_);
    }
    void ReadInfo()
    {
        std::thread erf_thread(&LaneDetectorTRT::Detect, erf_lane_detector, drive_window_.clone(), erf_lane_img);

        std::future<std::vector<bbox_t>> obj_detect;
        if (enable_obj_detect && !obj_detecting)
        {
            std::thread(&InfoCollector::detect, this, drive_window_, obj_detector, 0.5f, false).detach();
        }

        std::thread t1(&InfoCollector::ReadNumber, this, speed_ocr_, speed_img_, &speed_, true);
        std::thread t2(&InfoCollector::ReadNumber, this, speed_limit_ocr_, speed_limit_img_, &speed_limit_, false);
        if (speed_limit_ == 0)
            speed_limit_ = 60;
        // std::thread t3(&InfoCollector::ReadNumber, this, cruise_speed_ocr_, cruise_speed_img_, &cruise_speed_);

        t1.join();
        t2.join();
        // t3.join();

        erf_thread.join();
    }

    void MergeLaneResult()
    {
        std::array<cv::Mat, 4> erf_tmp;
        cv::resize(erf_lane_detector->lanes[0], erf_tmp[0], cv::Size(lane_view_width, lane_view_height), 0, 0,
            cv::INTER_NEAREST);
        cv::resize(erf_lane_detector->lanes[1], erf_tmp[1], cv::Size(lane_view_width, lane_view_height), 0, 0,
            cv::INTER_NEAREST);
        cv::resize(erf_lane_detector->lanes[2], erf_tmp[2], cv::Size(lane_view_width, lane_view_height), 0, 0,
            cv::INTER_NEAREST);
        cv::resize(erf_lane_detector->lanes[3], erf_tmp[3], cv::Size(lane_view_width, lane_view_height), 0, 0,
            cv::INTER_NEAREST);

        for (int i = 0; i < 4; ++i)
        {
            lanes[i] = erf_lane_detector->lanes[i];
        }

        cv::bitwise_or(erf_tmp[0], erf_tmp[1], all_lanes);
        cv::bitwise_or(erf_tmp[2], all_lanes, all_lanes);
        cv::bitwise_or(erf_tmp[3], all_lanes, all_lanes);
    }

    void WarpLane()
    {
        cv::warpPerspective(all_lanes, lane_warp_img_, lane_warp_mat_, cv::Size(lane_view_width, lane_view_height));
        cv::dilate(lane_warp_img_, lane_warp_img_, cv::Mat::ones(45, 45, CV_8SC1));
        cv::erode(lane_warp_img_, lane_warp_img_, cv::Mat::ones(45, 45, CV_8SC1));

        cv::warpPerspective(lanes[1], warp_lanes[1], lane_warp_mat[1], cv::Size(lane_view_width, lane_view_height));
        cv::dilate(warp_lanes[1], warp_lanes[1], cv::Mat::ones(10, 50, CV_8SC1));
        cv::erode(warp_lanes[1], warp_lanes[1], cv::Mat::ones(60, 45, CV_8SC1));

        cv::warpPerspective(lanes[2], warp_lanes[2], lane_warp_mat[2], cv::Size(lane_view_width, lane_view_height));
        cv::dilate(warp_lanes[2], warp_lanes[2], cv::Mat::ones(10, 50, CV_8SC1));
        cv::erode(warp_lanes[2], warp_lanes[2], cv::Mat::ones(60, 45, CV_8SC1));
    }

    /*void DetectLeftMirror()
    {
        std::thread obj_detect;
        if (enable_obj_detect)
        {
            obj_detect = std::thread(&InfoCollector::detect, this, wing_mirror_left_, obj_detector, 0.5f, false);
        }
        obj_left_img = wing_mirror_left_.clone();
        obj_detect.join();
        size_t obj_count = objs.size();
        for (int i = 0; i < obj_count; ++i)
        {
            if (objs[i].obj_id != 2 && objs[i].obj_id != 5 && objs[i].obj_id != 6 && objs[i].obj_id != 7
                && objs[i].obj_id != 9)
            {
                objs.erase(objs.begin() + i);
                obj_count--;
                i--;
            }
        }
        draw_boxes(obj_left_img, objs, obj_names_);
    }

    void DetectRightMirror()
    {
        std::future<std::vector<bbox_t>> obj_detect;
        if (enable_obj_detect)
        {
            obj_detect = std::async(&InfoCollector::detect, this, wing_mirror_right_, obj_detector, 0.5f, false);
        }
        obj_right_img            = wing_mirror_right_.clone();
        std::vector<bbox_t> objs = obj_detect.get();
        size_t obj_count         = objs.size();
        for (int i = 0; i < obj_count; ++i)
        {
            if (objs[i].obj_id != 2 && objs[i].obj_id != 5 && objs[i].obj_id != 6 && objs[i].obj_id != 7
                && objs[i].obj_id != 9)
            {
                objs.erase(objs.begin() + i);
                obj_count--;
                i--;
            }
        }
        draw_boxes(obj_right_img, objs, obj_names_);
    }*/

    void HoughLane()
    {
        std::vector<cv::Vec4i> lines;
        cv::HoughLinesP(lane_warp_img_, lines, 1, CV_PI / 180, 200, 100, 50);
        lane_hough_img_ = 0;
        for (const auto& line : lines)
        {
            cv::line(lane_hough_img_, cv::Point(line[0], line[1]), {line[2], line[3]}, cv::Scalar(255));
        }
    }

public:
    int speed_;
    int speed_limit_;
    // int cruise_speed_;

    int width_;
    int height_;

    cv::Mat screenshot_;
    cv::Mat game_window_;
    cv::Mat drive_window_;

    cv::Mat wing_mirror_left_;
    cv::Mat wing_mirror_right_;
    cv::Mat speed_limit_img_;
    cv::Mat speed_img_;
    // cv::Mat cruise_speed_img_;
    cv::Mat map_img_;
    cv::Mat erf_lane_img;
    cv::Mat lane_img_large;
    cv::Mat obj_img;
    cv::Mat obj_left_img;
    cv::Mat obj_right_img;


    std::array<cv::Mat, 4> lanes;
    std::array<cv::Mat, 4> warp_lanes;
    std::array<cv::Mat, 4> lane_warp_mat;

    cv::Mat all_lanes;

    cv::Mat lane_warp_img_;
    cv::Mat lane_warp_mat_;
    cv::Mat lane_hough_img_;

    bool enable_obj_detect = false;

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
    cv::Rect right_mirror_rect_;
    cv::Rect left_mirror_rect_;
    std::atomic_bool obj_detecting = false;

    tesseract::TessBaseAPI* speed_ocr_;
    tesseract::TessBaseAPI* speed_limit_ocr_;
    // tesseract::TessBaseAPI* cruise_speed_ocr_;

    std::vector<std::string> obj_names_;

    Detector* obj_detector;

    LaneDetectorTRT* erf_lane_detector = nullptr;

    bool ReadNumber(tesseract::TessBaseAPI* ocr, cv::Mat img, int* number, bool steady = false)
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
            int new_reading = atoi(outText.c_str());
            if (steady)
            {
                if (std::abs(new_reading - *number) > 25)
                    return false;
                else
                    *number = new_reading;
            }
            else
            {
                *number = new_reading;
            }
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

    void detect(cv::Mat mat, Detector* detector, float thresh = 0.2, bool use_mean = false)
    {
        obj_detecting = true;
        obj_img       = drive_window_.clone();
        if (mat.data == NULL)
            throw std::runtime_error("Image is empty");
        auto image_ptr           = detector->mat_to_image_resize(mat);
        std::vector<bbox_t> objs = detector->detect_resized(*image_ptr, mat.cols, mat.rows, thresh, use_mean);

        size_t obj_count = objs.size();
        for (int i = 0; i < obj_count; ++i)
        {
            if (objs[i].obj_id != 2 && objs[i].obj_id != 5 && objs[i].obj_id != 6 && objs[i].obj_id != 7
                && objs[i].obj_id != 9)
            {
                objs.erase(objs.begin() + i);
                obj_count--;
                i--;
            }
        }
        draw_boxes(obj_img, objs, obj_names_);
        obj_detecting = false;
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