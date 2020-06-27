#pragma once
#include <NvInfer.h>
#include <NvInferRuntime.h>
#include <buffers.h>
#include <logger.h>
#include <opencv2/opencv.hpp>

#include <filesystem>

struct BoundingBox
{
    float x;
    float y;
    float w;
    float h;
};

struct Object
{
    BoundingBox box;
    float confidence;
    int class_id;

    Object(BoundingBox b, float c, int id) : box(b), confidence(c), class_id(id) {}
};

class ObjectDetector
{
public:
    ObjectDetector(std::filesystem::path cfg_path, std::filesystem::path model_path)
    {
        net          = cv::dnn::readNetFromDarknet("../weights/yolov3.cfg", "../weights/yolov3.weights");
        output_names = net.getUnconnectedOutLayersNames();

        net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA_FP16);
    }

    std::vector<Object> Detect(cv::Mat in_img)
    {
        std::vector<Object> detected_objs;

        cv::Mat blob = cv::dnn::blobFromImage(in_img, 1.0 / 255, {320, 320}, {1}, true, false, CV_32F);
        net.setInput(blob);
        std::vector<cv::Mat> outs;
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


                float confidence = *(data + current_max_idx);
                int class_id     = current_max_idx - 5;
                if (confidence > 0.5)
                {
                    detected_objs.emplace_back(Object({data[0], data[1], data[2], data[3]}, confidence, class_id));
                }

                data += 85;
            }
        }

        return detected_objs;
    }

private:
    cv::dnn::dnn4_v20200310::Net net;
    std::vector<std::string> output_names;
};