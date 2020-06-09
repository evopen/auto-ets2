#pragma once

#include "cuda_provider_factory.h"

#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>

#include <filesystem>

struct DetectedObj
{
};

class ObjectDetector
{
public:
    ObjectDetector(std::filesystem::path model_path)
    {
        env = new Ort::Env();
        Ort::SessionOptions options;
        OrtSessionOptionsAppendExecutionProvider_CUDA(options, 0);
        options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
        session = new Ort::Session(*env, model_path.c_str(), options);

        size_t num_input_nodes = session->GetInputCount();

        for (int i = 0; i < num_input_nodes; ++i)
        {
            input_node_names.push_back(session->GetInputName(i, allocator_));
        }
    }

    void Detect(cv::Mat img)
    {
        img = img.clone();
        cv::resize(img, img, cv::Size(416, 416));
        img.convertTo(img, CV_32F, 1 / 255.0);
        cv::Mat input_blob = cv::dnn::blobFromImage(img);

        Ort::Value input_tensor{nullptr};
        Ort::Value output_tensor{nullptr};
        auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeDefault);

        input_tensor = Ort::Value::CreateTensor<float>(
            memory_info, (float*) input_blob.data, input_blob.total(), input_shape_.data(), input_shape_.size());
        output_tensor = Ort::Value::CreateTensor<uint8_t>(
            memory_info, output.data, height * width, output_shape_.data(), output_shape_.size());
    }


private:
    Ort::Env* env;
    Ort::Session* session;
    std::vector<std::string> input_node_names;
    Ort::AllocatorWithDefaultOptions allocator_;
};