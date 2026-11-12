// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#include "ForgeCapture.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <vector>
#ifndef _WIN32
#include "ForgePngPortable.hpp"
#endif
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <wincodec.h>
#include <wrl/client.h>
#endif

ForgeCapturedImage forge_capture_window(const slint::Window& window) {
    if (!window.is_visible()) return {{}, "窗口未显示"};
    auto pixels = window.take_snapshot();
    if (!pixels || pixels->width() == 0 || pixels->height() == 0)
        return {{}, "渲染器截图失败或窗口截图为空"};
    return {std::move(pixels), {}};
}

ForgeCapturedImage forge_capture_region(const slint::Window& window, ForgeLogicalCaptureRect rect) {
    const auto capture = forge_capture_window(window);
    if (!capture.ok()) return capture;
    return forge_crop_capture(*capture.pixels, rect, window.scale_factor());
}

ForgeCapturedImage forge_crop_capture(const slint::SharedPixelBuffer<slint::Rgba8Pixel>& pixels, ForgeLogicalCaptureRect rect, double scale) {
    if (!std::isfinite(rect.x) || !std::isfinite(rect.y) || !std::isfinite(rect.width) ||
        !std::isfinite(rect.height) || !std::isfinite(scale) || rect.width <= 0 || rect.height <= 0 || scale <= 0 ||
        !std::isfinite((static_cast<double>(rect.x) + rect.width) * scale) || !std::isfinite((static_cast<double>(rect.y) + rect.height) * scale))
        return {{}, "截图区域或缩放系数无效"};
    const auto left = static_cast<uint32_t>(std::clamp(std::floor(rect.x * scale), 0.0, static_cast<double>(pixels.width())));
    const auto top = static_cast<uint32_t>(std::clamp(std::floor(rect.y * scale), 0.0, static_cast<double>(pixels.height())));
    const auto right = static_cast<uint32_t>(std::clamp(std::ceil((static_cast<double>(rect.x) + rect.width) * scale), 0.0, static_cast<double>(pixels.width())));
    const auto bottom = static_cast<uint32_t>(std::clamp(std::ceil((static_cast<double>(rect.y) + rect.height) * scale), 0.0, static_cast<double>(pixels.height())));
    if (right <= left || bottom <= top) return {{}, "截图区域与窗口没有交集"};
    if (static_cast<uint64_t>(right - left) * (bottom - top) > std::numeric_limits<uint32_t>::max())
        return {{}, "截图区域过大"};
    slint::SharedPixelBuffer<slint::Rgba8Pixel> cropped(right - left, bottom - top);
    for (uint32_t row = 0; row < bottom - top; ++row) {
        const auto start = pixels.cbegin() + (static_cast<size_t>(top) + row) * pixels.width() + left;
        std::copy_n(start, right - left, cropped.begin() + static_cast<size_t>(row) * (right - left));
    }
    return {std::move(cropped), {}};
}

#ifdef _WIN32
class ForgeCaptureComScope {
public:
    HRESULT status = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    ~ForgeCaptureComScope() { if (SUCCEEDED(status)) CoUninitialize(); }
};

static std::string forge_capture_wic_error(HRESULT status) {
    std::ostringstream text;
    text << "PNG 保存失败，WIC HRESULT=0x" << std::hex << static_cast<unsigned long>(status);
    return text.str();
}
#endif

std::string forge_save_capture_png(const slint::SharedPixelBuffer<slint::Rgba8Pixel>& pixels, const std::filesystem::path& path) {
    if (pixels.width() == 0 || pixels.height() == 0) return "不能保存空截图";
#ifdef _WIN32
    const auto byte_count = static_cast<uint64_t>(pixels.width()) * pixels.height() * 4;
    if (byte_count > std::numeric_limits<UINT>::max()) return "PNG 图像过大";
    ForgeCaptureComScope scope;
    if (FAILED(scope.status) && scope.status != RPC_E_CHANGED_MODE) return forge_capture_wic_error(scope.status);
    Microsoft::WRL::ComPtr<IWICImagingFactory> factory;
    Microsoft::WRL::ComPtr<IWICStream> stream;
    Microsoft::WRL::ComPtr<IWICBitmapEncoder> encoder;
    Microsoft::WRL::ComPtr<IWICBitmapFrameEncode> frame;
    auto status = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(factory.GetAddressOf()));
    if (SUCCEEDED(status)) status = factory->CreateStream(stream.GetAddressOf());
    if (SUCCEEDED(status)) status = stream->InitializeFromFilename(path.c_str(), GENERIC_WRITE);
    if (SUCCEEDED(status)) status = factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, encoder.GetAddressOf());
    if (SUCCEEDED(status)) status = encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache);
    if (SUCCEEDED(status)) status = encoder->CreateNewFrame(frame.GetAddressOf(), nullptr);
    if (SUCCEEDED(status)) status = frame->Initialize(nullptr);
    if (SUCCEEDED(status)) status = frame->SetSize(pixels.width(), pixels.height());
    WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;
    if (SUCCEEDED(status)) status = frame->SetPixelFormat(&format);
    if (SUCCEEDED(status) && format != GUID_WICPixelFormat32bppBGRA) status = WINCODEC_ERR_UNSUPPORTEDPIXELFORMAT;
    if (FAILED(status)) return forge_capture_wic_error(status);
    // WIC 使用 BGRA；输入的 alpha 保持不预乘。
    std::vector<BYTE> bytes(static_cast<size_t>(byte_count));
    for (size_t index = 0; index < byte_count / 4; ++index) {
        const auto pixel = pixels.cbegin()[index];
        bytes[index * 4] = pixel.b; bytes[index * 4 + 1] = pixel.g;
        bytes[index * 4 + 2] = pixel.r; bytes[index * 4 + 3] = pixel.a;
    }
    status = frame->WritePixels(pixels.height(), pixels.width() * 4, static_cast<UINT>(byte_count), bytes.data());
    if (SUCCEEDED(status)) status = frame->Commit();
    if (SUCCEEDED(status)) status = encoder->Commit();
    if (SUCCEEDED(status)) status = stream->Commit(STGC_DEFAULT);
    return FAILED(status) ? forge_capture_wic_error(status) : std::string{};
#else
    return forge_png_write_rgba(reinterpret_cast<const uint8_t*>(pixels.cbegin()),pixels.width(),pixels.height(),path);
#endif
}
