#include "ForgeCapture.hpp"
#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <wincodec.h>
#include <wrl/client.h>

static void forge_capture_test_require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

class ForgeCaptureTestComScope {
public:
    HRESULT status = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    ~ForgeCaptureTestComScope() { if (SUCCEEDED(status)) CoUninitialize(); }
};

static slint::SharedPixelBuffer<slint::Rgba8Pixel> forge_capture_test_decode_png(const std::filesystem::path& path) {
    ForgeCaptureTestComScope scope;
    forge_capture_test_require(SUCCEEDED(scope.status) || scope.status == RPC_E_CHANGED_MODE, "WIC 解码初始化失败");
    Microsoft::WRL::ComPtr<IWICImagingFactory> factory;
    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
    Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
    auto status = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(factory.GetAddressOf()));
    if (SUCCEEDED(status)) status = factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, decoder.GetAddressOf());
    if (SUCCEEDED(status)) status = decoder->GetFrame(0, frame.GetAddressOf());
    if (SUCCEEDED(status)) status = factory->CreateFormatConverter(converter.GetAddressOf());
    if (SUCCEEDED(status)) status = converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeCustom);
    UINT width = 0, height = 0;
    if (SUCCEEDED(status)) status = converter->GetSize(&width, &height);
    forge_capture_test_require(SUCCEEDED(status), "PNG 解码失败");
    slint::SharedPixelBuffer<slint::Rgba8Pixel> pixels(width, height);
    status = converter->CopyPixels(nullptr, width * 4, width * height * 4, reinterpret_cast<BYTE*>(pixels.begin()));
    forge_capture_test_require(SUCCEEDED(status), "PNG 像素读取失败");
    return pixels;
}

int main() {
    try {
        slint::SharedPixelBuffer<slint::Rgba8Pixel> source(40, 30);
        for (size_t index = 0; index < 1200; ++index)
            source.begin()[index] = {static_cast<uint8_t>(index % 40), static_cast<uint8_t>(index / 40), 77, 128};
        const ForgeLogicalCaptureRect rect{1.2f, 2.4f, 7.3f, 4.2f};
        for (const double scale : {1.0, 1.25, 1.5, 2.0}) {
            const auto result = forge_crop_capture(source, rect, scale);
            forge_capture_test_require(result.ok(), result.error);
            const auto left = std::floor(static_cast<double>(rect.x) * scale);
            const auto top = std::floor(static_cast<double>(rect.y) * scale);
            const auto pixel = result.pixels->cbegin()[0];
            forge_capture_test_require(pixel.r == left && pixel.g == top && pixel.b == 77 && pixel.a == 128, "裁剪像素错误");
            forge_capture_test_require(result.pixels->width() == std::ceil((static_cast<double>(rect.x) + rect.width) * scale) - left, "缩放宽度错误");
            forge_capture_test_require(result.pixels->height() == std::ceil((static_cast<double>(rect.y) + rect.height) * scale) - top, "缩放高度错误");
        }
        const auto edge = forge_crop_capture(source, {-5, -4, 9, 10}, 1);
        forge_capture_test_require(edge.ok() && edge.pixels->width() == 4 && edge.pixels->height() == 6, "越界裁剪错误");
        for (const auto invalid : std::array<ForgeLogicalCaptureRect, 3>{{{90, 0, 2, 2}, {0, 0, 0, 2}, {std::numeric_limits<float>::quiet_NaN(), 0, 2, 2}}})
            forge_capture_test_require(!forge_crop_capture(source, invalid, 1).ok(), "异常区域没有报错");
        forge_capture_test_require(!forge_crop_capture(source, rect, 0).ok(), "异常缩放没有报错");

        const auto stamp = std::chrono::system_clock::now().time_since_epoch().count();
        const auto directory = std::filesystem::temp_directory_path() / ("slintforge-capture-" + std::to_string(stamp));
        std::filesystem::create_directories(directory);
        const auto path = directory / std::filesystem::u8path("透明像素.png");
        slint::SharedPixelBuffer<slint::Rgba8Pixel> transparent(2, 1);
        transparent.begin()[0] = {255, 10, 0, 128};
        transparent.begin()[1] = {0, 20, 255, 0};
        const auto error = forge_save_capture_png(transparent, path);
        forge_capture_test_require(error.empty(), error);
        // 直接解码文件，避免渲染器对透明像素的预乘/反预乘量化影响编码验证。
        const auto decoded = forge_capture_test_decode_png(path);
        forge_capture_test_require(decoded.width() == 2 && decoded.height() == 1, "PNG 解码尺寸错误");
        for (size_t index = 0; index < 2; ++index) {
            const auto expected = transparent.cbegin()[index], actual = decoded.cbegin()[index];
            forge_capture_test_require(expected.r == actual.r && expected.g == actual.g && expected.b == actual.b && expected.a == actual.a, "PNG RGBA 不一致");
        }
        forge_capture_test_require(!forge_save_capture_png(transparent, directory).empty(), "保存失败没有报错");
        std::filesystem::remove(path);
        std::filesystem::remove(directory);
        std::cout << "CAPTURE_UNIT_OK scales=4 rgba=matched unicode_path=matched errors=checked\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
