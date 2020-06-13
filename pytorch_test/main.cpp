#include <LaneDetector_pt.h>
#include <torch/script.h>
#include <torch/torch.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

int main()
{
    try
    {
        LoadLibraryA("torch_cuda.dll");
        LaneDetector detector("D:/DHH/Downloads/erf_net.pt", 976, 208, {103.939, 116.779, 123.68}, {1, 1, 1});
        // cv::Mat img = cv::imread("../images/1591496728991.png");
        cv::Mat img = cv::imread("D:/Dev/asdf/Dev/auto-ets2-bak/sample_image/crop.png");

        cv::Mat seg;

        int count = 0;
        while (1)
        {
            std::cout << count++ << std::endl;
            detector.Detect(img.clone(), &seg);
            cv::imshow("original", img);
            cv::imshow("seg", seg * 50);
            if (cv::waitKey(1) == 27)
                break;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}