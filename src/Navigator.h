#pragma once

#include <opencv2/opencv.hpp>

class Navigator
{
public:
    Navigator() {}

    void ExtractPath()
    {
        cv::inRange(original_map, cv::Scalar(0, 230, 0), cv::Scalar(200, 255, 200), arrow_img);
        cv::inRange(original_map, cv::Scalar(0, 0, 190), cv::Scalar(80, 80, 255), red_path_img);
        cv::bitwise_or(arrow_img, red_path_img, path_img);
    }

    void SetMap(cv::Mat map) { original_map = map; }

    void CalculatePixAngle20()
    {
        cv::Point probe_point;
        uint8_t* pix_ptr = path_img.data;
        // left
        {
            for (float angle = 0; angle < CV_PI / 2; angle += 0.001)
            {
                probe_point = GetCircleCoord(angle, short_probe_len, 0);
                // std::cout << probe_point << std::endl;
                if (pix_ptr[probe_point.y * original_map.cols + probe_point.x] != 0)
                {
                    angle_10_pix_left = angle;
                    break;
                }
            }
        }
        // right
        {
            for (float angle = 0; angle > -CV_PI / 2; angle -= 0.001)
            {
                probe_point = GetCircleCoord(angle, short_probe_len, 1);
                // std::cout << probe_point << std::endl;
                if (pix_ptr[probe_point.y * original_map.cols + probe_point.x] != 0)
                {
                    angle_10_pix_right = angle;
                    break;
                }
            }
        }
    }
    void CalculatePixAngle50()
    {
        cv::Point probe_point;
        uint8_t* pix_ptr = path_img.data;

        for (float angle = 0; angle < CV_PI / 2; angle += 0.001)
        {
            probe_point = GetCircleCoord(angle, short_probe_len);
            // std::cout << probe_point << std::endl;
            if (pix_ptr[probe_point.y * original_map.cols + probe_point.x] != 0)
            {
                angle_50_pix = angle;
                break;
            }
        }
    }

public:
    cv::Mat original_map;
    cv::Mat red_path_img;
    cv::Mat path_img;
    cv::Mat arrow_img;

    int short_probe_len = 12;
    int long_probe_len  = 30;

    float angle_10_pix_left;
    float angle_10_pix_right;
    float angle_50_pix;
    float left_right_anchor_dist = 4;

    cv::Point GetCircleCoord(float angle, float r, int right)
    {
        int x, y;
        if (right)
        {
            x = original_map.cols / 2 + left_right_anchor_dist / 2 + std::sin(angle) * r;
            y = original_map.cols / 2 - std::cos(angle) * r;
        }
        else
        {
            x = original_map.cols / 2 - left_right_anchor_dist / 2 + std::sin(angle) * r;
            y = original_map.cols / 2 - std::cos(angle) * r;
        }
        return cv::Point(x, y);
    }

    cv::Point GetCircleCoord(float angle, float r)
    {
        int x, y;

        x = original_map.cols / 2 + std::sin(angle) * r;
        y = original_map.cols / 2 - std::cos(angle) * r;

        return cv::Point(x, y);
    }
};