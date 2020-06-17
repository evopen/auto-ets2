#include "LaneDetector.h"

int main()
{
    LaneDetectorTRT detector("../erf_net.trt", 976, 208, {103.939, 116.779, 123.68}, {1, 1, 1});
    cv::Mat img     = cv::imread("D:/Dev/asdf/Dev/auto-ets2-bak/sample_image/crop.png");
    cv::Mat out_img = cv::Mat::zeros(208, 976, CV_8UC1);
    detector.Detect(img, out_img);
    int count = 0;
    while (1)
    {
        std::cout << count++ << std::endl;
        detector.Detect(img.clone(), out_img);
        cv::imshow("original", img);
        cv::imshow("seg", detector.lanes[0]);
        cv::imshow("seg1", detector.lanes[1]);
        cv::imshow("seg2", detector.lanes[2]);
        cv::imshow("seg3", detector.lanes[3]);
        if (cv::waitKey(1) == 27)
            break;
    }
    return 0;
}