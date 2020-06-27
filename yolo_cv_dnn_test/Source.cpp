#include <opencv2/opencv.hpp>

#include <chrono>
#include <exception>
#include <iostream>

int main()
{
    try
    {

        std::cout << cv::cuda::getCudaEnabledDeviceCount() << std::endl;
        auto net          = cv::dnn::readNetFromDarknet("../weights/yolov3.cfg", "../weights/yolov3.weights");
        auto output_names = net.getUnconnectedOutLayersNames();

        net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA_FP16);

        cv::Mat img  = cv::imread("../images/1591496970279.png");
        cv::Mat blob = cv::dnn::blobFromImage(img, 1.0 / 255, {320, 320}, {1}, true, false, CV_32F);

        net.setInput(blob);

        std::vector<cv::Mat> outs;

        int count = 1000;
        while (count--)
        {
            auto start = std::chrono::high_resolution_clock::now();
            net.forward(outs, output_names);
            for (const auto& output : outs)
            {
                float* data = (float*) output.data;

                for (int detection_idx = 0; detection_idx < output.rows; ++detection_idx)
                {
                    float current_max = -DBL_MAX;
                    int current_max_idx;
                    for (int i = 5; i < 85; ++i)
                    {
                        if (data[i] > current_max)
                        {
                            current_max     = data[i];
                            current_max_idx = i;
                        }
                    }

                    int class_id     = current_max_idx;
                    float confidence = *(data + class_id);
                    if (confidence > 0.5)
                    {
                        
                    }
                    data += 85;
                }
            }

            auto end = std::chrono::high_resolution_clock::now();
            std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}