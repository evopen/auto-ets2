#pragma once

#include <opencv2/opencv.hpp>

class Navigator
{
public:
    Navigator() {}

    void ExtractPath()
    {
        cv::cvtColor(original_map, map_gray_img, cv::COLOR_RGB2GRAY);
        cv::inRange(map_gray_img, cv::Scalar(150), cv::Scalar(160), arrow_img);
        cv::inRange(map_gray_img, cv::Scalar(70), cv::Scalar(90), red_path_img);
        cv::bitwise_or(arrow_img, red_path_img, path_img);
    }

    void CalculatePixAngle20()
    {
        int probe_length = 20;
        cv::Point probe_point;
        int lane_pix_loc = 0;
        uint8_t* pix_ptr = path_img.data;


        for (float angle = 0; angle < CV_PI / 2; angle += 0.1)
        {
            probe_point = GetCircleCoord(angle, probe_length);
            if (pix_ptr[probe_point.y * 120 + probe_point.x] != 0)
            {
                angle_20_pix = angle;
                break;
            }

            probe_point.x = 120 - probe_point.x;
            if (pix_ptr[probe_point.y * 120 + probe_point.x] != 0)
            {
                angle_20_pix = -angle;
                break;
            }
        }
    }
    void CalculatePixAngle50() {}

private:
    cv::Mat original_map;
    cv::Mat map_gray_img;
    cv::Mat red_path_img;
    cv::Mat path_img;
    cv::Mat arrow_img;

    float angle_20_pix;
    float angle_50_pix;

    cv::Point GetCircleCoord(float angle, float r)
    {
        int x = 60 + std::sin(angle) * r;
        int y = 60 - std::cos(angle) * r;
        return cv::Point(x, y);
    }
};