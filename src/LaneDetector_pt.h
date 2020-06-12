#pragma once

#include <opencv2/opencv.hpp>
#include <torch/script.h>

#include <filesystem>

class LaneDetector
{
public:
    LaneDetector(std::filesystem::path model_path)
    {
        module_ = torch::jit::load(model_path.string());
        module_.to(at::kCUDA);
    }

private:
    cv::Size input_size_;
    torch::jit::script::Module module_;
};