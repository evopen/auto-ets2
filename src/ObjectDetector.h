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

        // initialize session options
        Ort::SessionOptions session_options;
        session_options.SetIntraOpNumThreads(1);
        OrtSessionOptionsAppendExecutionProvider_CUDA(session_options, 0);
        session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

        session = new Ort::Session(*env, model_path.c_str(), session_options);


        // print number of model input nodes
        size_t num_input_nodes = session->GetInputCount();
        std::vector<const char*> input_node_names(num_input_nodes);
        std::vector<int64_t> input_node_dims;  // simplify... this model has only 1 input node {1, 3, 224, 224}.
                                               // Otherwise need vector<vector<>>

        printf("Number of inputs = %zu\n", num_input_nodes);

        // iterate over all input nodes
        for (int i = 0; i < num_input_nodes; i++)
        {
            // print input node names
            char* input_name = session->GetInputName(i, allocator_);
            printf("Input %d : name=%s\n", i, input_name);
            input_node_names[i] = input_name;

            // print input node types
            Ort::TypeInfo type_info = session->GetInputTypeInfo(i);
            auto tensor_info        = type_info.GetTensorTypeAndShapeInfo();

            ONNXTensorElementDataType type = tensor_info.GetElementType();
            printf("Input %d : type=%d\n", i, type);

            // print input shapes/dims
            input_node_dims = tensor_info.GetShape();
            printf("Input %d : num_dims=%zu\n", i, input_node_dims.size());
            for (int j = 0; j < input_node_dims.size(); j++)
                printf("Input %d : dim %d=%jd\n", i, j, input_node_dims[j]);
        }

        // Results should be...
        // Number of inputs = 1
        // Input 0 : name = data_0
        // Input 0 : type = 1
        // Input 0 : num_dims = 4
        // Input 0 : dim 0 = 1
        // Input 0 : dim 1 = 3
        // Input 0 : dim 2 = 224
        // Input 0 : dim 3 = 224
    }

    void Detect(cv::Mat img)
    {
        //img = img.clone();
        //cv::resize(img, img, cv::Size(416, 416));
        //img.convertTo(img, CV_32F, 1 / 255.0);
        //cv::Mat input_blob = cv::dnn::blobFromImage(img);

        //Ort::Value input_tensor{nullptr};
        //Ort::Value output_tensor{nullptr};
        //Ort::MemoryInfo::CreateCpu(allocator_, OrtMemTypeDefault);
        //auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeDefault);

        //input_tensor = Ort::Value::CreateTensor<float>(
        //    memory_info, (float*) input_blob.data, input_blob.total(), input_shape_.data(), input_shape_.size());
        //output_tensor = Ort::Value::CreateTensor<uint8_t>(
        //    memory_info, output.data, height * width, output_shape_.data(), output_shape_.size());
    }


private:
    Ort::Env* env;
    Ort::Session* session;
    std::vector<std::string> input_node_names;
    Ort::AllocatorWithDefaultOptions allocator_;
};