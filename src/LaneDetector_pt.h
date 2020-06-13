#pragma once

#include <opencv2/opencv.hpp>
#include <torch/script.h>

#include <filesystem>

class LaneDetector
{
public:
    LaneDetector(std::filesystem::path model_path, int input_width, int input_height, std::array<float, 3> mean,
        std::array<float, 3> std)
        : input_width_(input_width), input_height_(input_height), mean_(mean), std_(std)
    {
        std::string filename = model_path.string();
        module_              = torch::jit::load(filename);
        std::cout << "module loaded" << std::endl;
        module_.to(at::kCUDA, at::kHalf);
    }

    void Detect(cv::Mat in_img, cv::Mat* out_img)
    {
        cv::resize(in_img, in_img, cv::Size(input_width_, input_height_));

        auto input_tensor = torch::from_blob(in_img.data, {1, input_height_, input_width_, 3}, at::kByte);

        input_tensor = input_tensor.to(at::kFloat);

        input_tensor       = input_tensor.permute({0, 3, 1, 2});
        input_tensor[0][0] = input_tensor[0][0].sub_(mean_[0]).div_(std_[0]);
        input_tensor[0][1] = input_tensor[0][1].sub_(mean_[1]).div_(std_[1]);
        input_tensor[0][2] = input_tensor[0][2].sub_(mean_[2]).div_(std_[2]);


        input_tensor = input_tensor.to(at::kCUDA, at::kHalf);

        auto seg_img =
            module_.forward({input_tensor}).toTuple()->elements()[0].toTensor()[0].argmax(0).to(at::kByte).to(at::kCPU);

        *out_img = cv::Mat(input_height_, input_width_, CV_8UC1, seg_img.data_ptr());
    }

private:
    cv::Size input_size_;
    torch::jit::script::Module module_;
    int input_height_;
    int input_width_;
    std::array<float, 3> mean_;
    std::array<float, 3> std_;
};