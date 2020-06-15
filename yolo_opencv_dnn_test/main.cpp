#include <opencv2/opencv.hpp>


int main()
{
    auto net =
        cv::dnn::readNetFromDarknet("D:/Dev/auto-ets2/weights/yolov4.cfg", "D:/Dev/auto-ets2/weights/yolov4.weights");
    return 0;
}