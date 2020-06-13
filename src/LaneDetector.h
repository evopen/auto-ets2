#pragma once
#include <NvInfer.h>
#include <NvInferRuntime.h>
#include <buffers.h>
#include <logger.h>
#include <opencv2/opencv.hpp>

#include <filesystem>

class LaneDetectorTRT
{
public:
    LaneDetectorTRT(std::filesystem::path model_path)
    {
        runtime                 = nvinfer1::createInferRuntime(gLogger.getTRTLogger());
        auto engine_bytes       = ReadFile(model_path);
        engine                  = runtime->deserializeCudaEngine(engine_bytes.data(), engine_bytes.size(), nullptr);
        context                 = engine->createExecutionContext();
        input_index             = engine->getBindingIndex("input.1");
        output_seg_pred_index   = engine->getBindingIndex("2347");
        output_exist_pred_index = engine->getBindingIndex("2343");

        cudaMalloc(&io_buffers[input_index], width * height * 3 * sizeof(float));
        cudaMalloc(&io_buffers[output_seg_pred_index], (height / 8) * (width / 8) * sizeof(int8_t));
        cudaMalloc(&io_buffers[output_exist_pred_index], 4 * 1 * sizeof(float));

        output_seg_pred_blob   = cv::Mat::zeros(height / 8, width / 8, CV_8SC1);
        output_exist_pred_blob = cv::Mat(1, 4, CV_32FC1);
    }

    void Detect(cv::Mat in_img, cv::Mat out_img)
    {
        cv::resize(in_img, in_img, cv::Size(width, height));
        in_img.convertTo(in_img, CV_32FC3, 1.0 / 255);
        in_img -= cv::Scalar(0.3598, 0.3653, 0.3662);
        in_img /= cv::Scalar(0.2573, 0.2663, 0.2756);
        cv::Mat input_blob = cv::dnn::blobFromImage(in_img);

        cudaMemcpy(
            io_buffers[input_index], input_blob.data, width * height * 3 * sizeof(float), cudaMemcpyHostToDevice);

        context->executeV2(io_buffers);

        cudaMemcpy(out_img.data, io_buffers[output_seg_pred_index], (height / 8) * (width / 8) * sizeof(int8_t),
            cudaMemcpyDeviceToHost);
        ArgMaxPostprocess(out_img);
    }

    std::array<cv::Mat, 4> lanes;


private:
    nvinfer1::IRuntime* runtime;
    nvinfer1::ICudaEngine* engine;
    nvinfer1::IExecutionContext* context;

    int input_index;
    int output_seg_pred_index;
    int output_exist_pred_index;

    cv::Mat output_seg_pred_blob;
    cv::Mat output_exist_pred_blob;

    const int width  = 800;
    const int height = 288;


    void* io_buffers[3];

    std::vector<char> ReadFile(const std::filesystem::path& file_path)
    {
        std::ifstream file(file_path.c_str(), std::ios::ate | std::ios::binary);
        size_t length = file.tellg();
        file.seekg(0);

        std::vector<char> buffer;
        buffer.resize(length);
        file.read(buffer.data(), length);
        return buffer;
    }

    void ArgMaxPostprocess(cv::Mat img)
    {
        cv::Mat tmp[4];
        cv::inRange(img, cv::Scalar(32), cv::Scalar(32), lanes[0]);
        cv::inRange(img, cv::Scalar(64), cv::Scalar(64), lanes[1]);
        cv::inRange(img, cv::Scalar(95), cv::Scalar(95), lanes[2]);
        cv::inRange(img, cv::Scalar(127), cv::Scalar(127), lanes[3]);
    }
};