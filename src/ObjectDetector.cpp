#include "ObjectDetector.h"
#include <opencv2/opencv.hpp>

int main()
{
    ObjectDetector obj_detector("D:/Downloads/yolov3-10.onnx");
    cv::Mat img = cv::imread("../images/1591496874291.png");
    obj_detector.Detect(img);
    return 0;
}