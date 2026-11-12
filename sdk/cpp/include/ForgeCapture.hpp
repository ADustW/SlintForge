// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <slint.h>
#include <filesystem>
#include <optional>
#include <string>

// 与消费工程生成的 ForgeCaptureRect 区分，避免全局类型重名。
struct ForgeLogicalCaptureRect {
    float x = 0, y = 0, width = 0, height = 0;
};

struct ForgeCapturedImage {
    std::optional<slint::SharedPixelBuffer<slint::Rgba8Pixel>> pixels;
    std::string error;
    [[nodiscard]] bool ok() const { return pixels.has_value(); }
};

// 窗口截图必须在所属 UI 线程、窗口显示并完成布局后调用。
[[nodiscard]] ForgeCapturedImage forge_capture_window(const slint::Window& window);
[[nodiscard]] ForgeCapturedImage forge_capture_region(const slint::Window& window, ForgeLogicalCaptureRect rect);
[[nodiscard]] ForgeCapturedImage forge_crop_capture(const slint::SharedPixelBuffer<slint::Rgba8Pixel>& pixels, ForgeLogicalCaptureRect rect, double scale);
// 空字符串表示成功；父目录由调用方创建。
[[nodiscard]] std::string forge_save_capture_png(const slint::SharedPixelBuffer<slint::Rgba8Pixel>& pixels, const std::filesystem::path& path);

template<typename UiResult>
UiResult forge_capture_ui_result(const ForgeCapturedImage& capture) {
    UiResult result{};
    result.ok = capture.ok();
    result.error = capture.error.c_str();
    if (capture.pixels) {
        result.pixel_width = static_cast<int>(capture.pixels->width());
        result.pixel_height = static_cast<int>(capture.pixels->height());
        result.image = slint::Image(*capture.pixels);
    }
    return result;
}

// 宿主传入自身生成的服务/结果类型，回调仅持有窗口弱引用。
template<typename Service, typename UiResult, typename Ui>
void forge_bind_capture_service(const slint::ComponentHandle<Ui>& ui) {
    const slint::ComponentWeakHandle<Ui> weak(ui);
    const auto& service = ui->template global<Service>();
    service.on_capture_window([weak]() {
        if (const auto locked = weak.lock())
            return forge_capture_ui_result<UiResult>(forge_capture_window((*locked)->window()));
        return forge_capture_ui_result<UiResult>({{}, "窗口已销毁"});
    });
    service.on_capture_region([weak](auto rect) {
        if (const auto locked = weak.lock())
            return forge_capture_ui_result<UiResult>(forge_capture_region((*locked)->window(), {rect.x, rect.y, rect.width, rect.height}));
        return forge_capture_ui_result<UiResult>({{}, "窗口已销毁"});
    });
    service.set_connected(true);
}
