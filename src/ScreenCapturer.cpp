#include "ScreenCapturer.h"

#include <array>

#pragma comment(lib, "D3D11.lib")

void ScreenCapturer::Init()
{
    HRESULT hr(E_FAIL);

    std::array<D3D_FEATURE_LEVEL, 4> feature_levels = {
        D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_9_1};

    D3D_FEATURE_LEVEL acquired_feature_level;

    hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, feature_levels.data(), feature_levels.size(),
        D3D11_SDK_VERSION, &device_, &acquired_feature_level, &device_context_);

    if (!device_)
    {
        throw std::runtime_error("failed to create device");
    }


    hr = device_->QueryInterface(IID_PPV_ARGS(&dxgi_device));

    IDXGIAdapter* dxgi_adapter;
    hr = dxgi_device->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&dxgi_adapter));

    // dxgi_device->Release();

    UINT output = 0;
    IDXGIOutput* dxgi_output;
    hr = dxgi_adapter->EnumOutputs(output, &dxgi_output);

    dxgi_adapter->Release();

    DXGI_OUTPUT_DESC output_desc;
    hr = dxgi_output->GetDesc(&output_desc);

    IDXGIOutput1* dxgi_output1;
    hr = dxgi_output->QueryInterface(IID_PPV_ARGS(&dxgi_output1));

    dxgi_output->Release();

    hr = dxgi_output1->DuplicateOutput(device_, &output_dupl);

    dxgi_output1->Release();

        output_dupl->GetDesc(&output_dupl_desc);


    D3D11_TEXTURE2D_DESC desc;
    desc.Width              = output_dupl_desc.ModeDesc.Width;
    desc.Height             = output_dupl_desc.ModeDesc.Height;
    desc.Format             = output_dupl_desc.ModeDesc.Format;
    desc.ArraySize          = 1;
    desc.BindFlags          = D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET;
    desc.MiscFlags          = D3D11_RESOURCE_MISC_GDI_COMPATIBLE;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.MipLevels          = 1;
    desc.CPUAccessFlags     = 0;
    desc.Usage              = D3D11_USAGE_DEFAULT;
    hr                      = device_->CreateTexture2D(&desc, NULL, &staging_image);

    desc.Width              = output_dupl_desc.ModeDesc.Width;
    desc.Height             = output_dupl_desc.ModeDesc.Height;
    desc.Format             = output_dupl_desc.ModeDesc.Format;
    desc.ArraySize          = 1;
    desc.BindFlags          = 0;
    desc.MiscFlags          = 0;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.MipLevels          = 1;
    desc.CPUAccessFlags     = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    desc.Usage              = D3D11_USAGE_STAGING;
    hr                      = device_->CreateTexture2D(&desc, NULL, &dest_image);
}

cv::Mat ScreenCapturer::Capture()
{
    HRESULT hr(E_FAIL);

    DXGI_OUTDUPL_FRAME_INFO frame_info;
    hr = output_dupl->AcquireNextFrame(100, &frame_info, &desktop_resource);
    // std::cout << frame_info.AccumulatedFrames << std::endl;

    hr = desktop_resource->QueryInterface(IID_PPV_ARGS(&acquired_desktop_image));
    desktop_resource->Release();
    device_context_->CopyResource(dest_image, acquired_desktop_image);

    output_dupl->ReleaseFrame();

    D3D11_MAPPED_SUBRESOURCE resource;
    UINT subresource = D3D11CalcSubresource(0, 0, 0);
    hr               = device_context_->Map(dest_image, subresource, D3D11_MAP_READ_WRITE, 0, &resource);

    return cv::Mat(
        output_dupl_desc.ModeDesc.Height, output_dupl_desc.ModeDesc.Width, CV_8UC4, resource.pData, resource.RowPitch);
}
