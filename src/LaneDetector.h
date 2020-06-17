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
    LaneDetectorTRT(std::filesystem::path model_path, int input_width, int input_height, std::array<float, 3> mean,
        std::array<float, 3> std)
        : input_width_(input_width), input_height_(input_height), mean_(mean), std_(std)
    {
        runtime                 = nvinfer1::createInferRuntime(gLogger.getTRTLogger());
        auto engine_bytes       = ReadFile(model_path);
        engine                  = runtime->deserializeCudaEngine(engine_bytes.data(), engine_bytes.size(), nullptr);
        context                 = engine->createExecutionContext();
        input_index             = engine->getBindingIndex("input");
        output_seg_pred_index   = engine->getBindingIndex("seg");
        output_exist_pred_index = engine->getBindingIndex("prob");

        auto a   = engine->getBindingDataType(0);
        auto b   = engine->getBindingDataType(1);
        auto c   = engine->getBindingDataType(2);
        auto aaa = engine->getBindingDimensions(0);
        auto d   = engine->getBindingDimensions(1);

        cudaMalloc(&io_buffers[input_index], input_width_ * input_height_ * 3 * sizeof(float));
        cudaMalloc(&io_buffers[output_seg_pred_index], input_height_ * input_width_ * 5 * sizeof(float));
        cudaMalloc(&io_buffers[output_exist_pred_index], 4 * 1 * sizeof(float));

        seg_img           = cv::Mat(input_height_, input_width_, CV_8UC1);
        output_exist_blob = cv::Mat(1, 4, CV_32FC1);
        seg               = new float[input_height_ * input_width_ * 5];
    }

    void Detect(cv::Mat in_img, cv::Mat out_img)
    {
        cv::Mat input_blob = cv::dnn::blobFromImage(
            in_img, 1.0, cv::Size(input_width_, input_height_), cv::Scalar(std_[0], std_[1], std_[2]), false, false);
        cudaMemcpy(io_buffers[input_index], input_blob.data, input_height_ * input_width_ * 3 * sizeof(float),
            cudaMemcpyHostToDevice);

        context->executeV2(io_buffers);


        cudaMemcpy(seg, io_buffers[output_seg_pred_index], input_width_ * input_height_ * 5 * sizeof(float),
            cudaMemcpyDeviceToHost);
        cudaMemcpy(
            output_exist_blob.data, io_buffers[output_exist_pred_index], 4 * sizeof(float), cudaMemcpyDeviceToHost);

        uint8_t* seg_img_ptr = seg_img.data;
        for (int i = 0; i < input_height_; i++)
        {
            for (int j = 0; j < input_width_; j++)
            {
                int arg       = INT_MAX;
                float max_tmp = -FLT_MAX;
                for (int k = 0; k <= 4; k++)
                {
                    if (seg[k * input_height_ * input_width_ + i * input_width_ + j] > max_tmp)
                    {
                        max_tmp = seg[k * input_height_ * input_width_ + i * input_width_ + j];
                        arg     = k;
                    }
                }
                seg_img_ptr[i * input_width_ + j] = arg;
            }
        }

        cv::resize(seg_img, out_img, cv::Size(400, 400));
        ArgMaxPostprocess(out_img);
    }

    ~LaneDetectorTRT()
    {
        cudaFree(&io_buffers[input_index]);
        cudaFree(&io_buffers[output_seg_pred_index]);
        cudaFree(&io_buffers[output_exist_pred_index]);
    }

    std::array<cv::Mat, 4> lanes;


private:
    nvinfer1::IRuntime* runtime;
    nvinfer1::ICudaEngine* engine;
    nvinfer1::IExecutionContext* context;

    int input_index;
    int output_seg_pred_index;
    int output_exist_pred_index;

    cv::Mat seg_img;
    cv::Mat output_exist_blob;

    const int input_width_;
    const int input_height_;
    std::array<float, 3> mean_;
    std::array<float, 3> std_;
    float* seg;

    cv::Mat buffer;


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
        cv::inRange(img, cv::Scalar(1), cv::Scalar(1), lanes[0]);
        cv::inRange(img, cv::Scalar(2), cv::Scalar(2), lanes[1]);
        cv::inRange(img, cv::Scalar(3), cv::Scalar(3), lanes[2]);
        cv::inRange(img, cv::Scalar(4), cv::Scalar(4), lanes[3]);
    }
};