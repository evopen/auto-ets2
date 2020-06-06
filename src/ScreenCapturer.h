#pragma once

#define WIN32_LEAN_AND_MEAN

#include <d3d11.h>
#include <dxgi1_2.h>
#include <opencv2/opencv.hpp>
#include <shellapi.h>
#include <shlobj.h>
#include <windows.h>

#include <algorithm>
#include <chrono>
#include <memory>
#include <string>

class ScreenCapturer
{
public:
    void Init();
    cv::Mat Capture();


private:
    ID3D11Device* device_;
    ID3D11DeviceContext* device_context_;
    IDXGIOutputDuplication* output_dupl;
    IDXGIResource* desktop_resource;
    ID3D11Texture2D* acquired_desktop_image;
    IDXGIDevice* dxgi_device;
    ID3D11Texture2D* staging_image;
    ID3D11Texture2D* dest_image;
    DXGI_OUTDUPL_DESC output_dupl_desc;

    int count = 0;
};