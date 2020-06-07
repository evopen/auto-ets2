#include "LaneDetector.h"

#include <gtest/gtest.h>

//TEST(LaneDetectorTest, Detect)
//{
//    LaneDetector detector("D:/Dev/auto-ets2/models/6.6_2060_fp32_288.800_36.100_explicitBatch.trt");
//    cv::Mat in  = cv::imread("D:/Dev/asdf/Dev/auto-ets2-bak/sample_image/crop.png");
//    cv::Mat out = cv::Mat::zeros(36, 100, CV_8UC1);
//    cv::Mat out_large = cv::Mat::zeros(360, 1000, CV_8UC1);
//    while (true)
//    {
//        detector.Detect(in, out);
//        cv::resize(out, out_large, cv::Size(1000, 360));
//        cv::imshow("asdf", out_large);
//        if (cv::waitKey(1) == 27)
//            break;
//    }
//}