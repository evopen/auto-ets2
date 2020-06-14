#include <ScreenCapturer.h>
#include <opencv2/opencv.hpp>

#include <chrono>
#include <iostream>


const int kSampleTime = 1000;  // millisecond

int main()
{
    cv::Mat screenshot;
    ScreenCapturer capturer;
    capturer.Init();

    //while (true)
    //{
        std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(kSampleTime));
        screenshot = capturer.Capture();
        std::string filename(std::to_string(
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
                .count()));
        std::cout << filename << std::endl;
        cv::imwrite("warp/" + filename + ".png", screenshot);
    //    if (cv::waitKey(1) == 27)
    //    {
    //        break;
    //    }
    //}

    return 0;
}