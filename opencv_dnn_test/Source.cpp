#include <opencv2/opencv.hpp>

int main()
{
    try
    {
        auto net    = cv::dnn::readNetFromONNX("D:/Dev/auto-ets2/scripts/erf_net.onnx");
        cv::Mat img = cv::imread("../crop.png");
        
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);

        int count = 100;
        while (count--)
        {

            cv::Mat blob =
                cv::dnn::blobFromImage(img, 1.0, cv::Size(976, 208), cv::Scalar(103.939, 116.779, 123.68), true, false);

            net.setInput(blob);

            std::vector<std::string> output_names = net.getUnconnectedOutLayersNames();
            std::vector<cv::Mat> outs;
            net.forward(outs, output_names);
            cv::Mat seg  = outs[0];
            cv::Mat prob = outs[1];

            cv::Mat seg_img(208, 976, CV_8UC1);

            uint8_t* ptr   = seg_img.data;
            float* seg_ptr = (float*) seg.data;
            for (int i = 0; i < 208; i++)
            {
                for (int j = 0; j < 976; j++)
                {
                    int arg       = INT_MAX;
                    float max_tmp = -FLT_MAX;
                    for (int k = 0; k <= 4; k++)
                    {
                        if (seg_ptr[k * 208 * 976 + i * 976 + j] > max_tmp)
                        {
                            max_tmp = seg_ptr[k * 208 * 976 + i * 976 + j];
                            arg     = k;
                        }
                    }
                    ptr[i * 976 + j] = arg;
                }
            }
        }

        //while (1)
        //{
        //    cv::imshow("show", seg_img * 50);
        //    if (cv::waitKey(20) == 27)
        //        break;
        //}
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}