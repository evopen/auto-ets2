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
        buffer_ = cv::Mat::zeros(input_height_, input_width_, CV_8UC1);
    }

    void Detect(cv::Mat in_img, cv::Mat out_img)
    {
        cv::resize(in_img, in_img, cv::Size(input_width_, input_height_));

        auto input_tensor = torch::from_blob(in_img.data, {1, input_height_, input_width_, 3}, at::kByte);

        input_tensor = input_tensor.to(at::kFloat);

        input_tensor       = input_tensor.permute({0, 3, 1, 2});
        input_tensor[0][0] = input_tensor[0][0].sub_(mean_[0]).div_(std_[0]);
        input_tensor[0][1] = input_tensor[0][1].sub_(mean_[1]).div_(std_[1]);
        input_tensor[0][2] = input_tensor[0][2].sub_(mean_[2]).div_(std_[2]);


        input_tensor = input_tensor.to(at::kCUDA, at::kHalf);

        auto tuple = module_.forward({input_tensor}).toTuple();

        auto seg_img = tuple->elements()[0].toTensor()[0].argmax(0).to(at::kByte).to(at::kCPU);
        auto prob    = tuple->elements()[1].toTensor().to(at::kCPU);


        std::memcpy(buffer_.data, seg_img.data_ptr(), input_height_ * input_width_ * sizeof(uint8_t));

        cv::resize(buffer_, out_img, cv::Size(400, 400));

        ArgMaxPostprocess(out_img);
    }

    std::array<cv::Mat, 4> lanes;

private:
    cv::Size input_size_;
    torch::jit::script::Module module_;
    int input_height_;
    int input_width_;
    std::array<float, 3> mean_;
    std::array<float, 3> std_;
    cv::Mat buffer_;


    void ArgMaxPostprocess(cv::Mat img)
    {
        cv::Mat tmp[4];
        cv::inRange(img, cv::Scalar(1), cv::Scalar(1), lanes[0]);
        cv::inRange(img, cv::Scalar(2), cv::Scalar(2), lanes[1]);
        cv::inRange(img, cv::Scalar(3), cv::Scalar(3), lanes[2]);
        cv::inRange(img, cv::Scalar(4), cv::Scalar(4), lanes[3]);
    }
};