// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#include "ForgeGalleryCapture.hpp"
#include "ForgeCapture.hpp"
#include "ForgeGalleryCaptureScene.hpp"
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <stdexcept>

static std::string forge_gallery_capture_path_utf8(const std::filesystem::path& path) {
    const auto bytes = path.u8string();
    return {bytes.begin(), bytes.end()};
}

void forge_gallery_connect_capture(const slint::ComponentHandle<GalleryWindow>& ui) {
    forge_bind_capture_service<ForgeCaptureService, ForgeCaptureResult>(ui);
    ui->on_export_capture([](slint::Image image) -> slint::SharedString {
        try {
            const auto pixels = image.to_rgba8();
            if (!pixels) return "无法读取截图像素";
            const auto directory = std::filesystem::absolute("build/captures");
            std::filesystem::create_directories(directory);
            const auto stamp = std::chrono::system_clock::now().time_since_epoch().count();
            const auto path = directory / ("capture-" + std::to_string(stamp) + ".png");
            const auto error = forge_save_capture_png(*pixels, path);
            return error.empty() ? forge_gallery_capture_path_utf8(path).c_str() : error.c_str();
        } catch (const std::exception& error) { return error.what(); }
    });
}

static void forge_gallery_capture_require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

static slint::SharedPixelBuffer<slint::Rgba8Pixel> forge_gallery_check_window_capture(const slint::ComponentHandle<GalleryWindow>& ui, const std::filesystem::path& path) {
    const auto result = ui->invoke_qa_capture_window();
    forge_gallery_capture_require(result.ok, std::string(result.error));
    const auto size = ui->window().size();
    forge_gallery_capture_require(result.pixel_width == size.width && result.pixel_height == size.height, "窗口截图尺寸错误");
    const auto pixels = result.image.to_rgba8();
    forge_gallery_capture_require(pixels.has_value(), "截图像素无法读取");
    const auto error = forge_save_capture_png(*pixels, path);
    forge_gallery_capture_require(error.empty(), error);
    const auto decoded = slint::Image::load_from_path(forge_gallery_capture_path_utf8(path).c_str());
    forge_gallery_capture_require(decoded.size().width == size.width && decoded.size().height == size.height, "PNG 解码尺寸错误");
    return *pixels;
}

void forge_gallery_start_capture_verification(const slint::ComponentHandle<GalleryWindow>& ui, slint::Timer& timer, bool preview) {
    const auto stamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    const auto directory = std::filesystem::absolute("build/captures") / ("cpp-" + std::to_string(stamp));
    std::filesystem::create_directories(directory);
    const slint::ComponentWeakHandle<GalleryWindow> weak(ui);
    timer.start(slint::TimerMode::Repeated, std::chrono::milliseconds(350), [weak, directory, preview, step = 0, scene = std::optional<slint::ComponentHandle<ForgeCaptureSceneWindow>>{}]() mutable {
        const auto locked = weak.lock();
        forge_gallery_capture_require(locked.has_value(), "截图窗口已销毁");
        const auto& ui = *locked;
        if (preview) {
            forge_gallery_check_window_capture(ui, directory / "preview.png");
            std::cout << "CAPTURE_PREVIEW_OK host=cpp path=" << forge_gallery_capture_path_utf8(directory) << '\n';
            slint::quit_event_loop();
            return;
        }
        if (step == 0) {
            const auto window_pixels = forge_gallery_check_window_capture(ui, directory / "window.png");
            const auto result = ui->invoke_qa_capture_page();
            forge_gallery_capture_require(result.ok, std::string(result.error));
            const auto rect = ui->get_qa_page_rect();
            const auto scale = ui->window().scale_factor();
            forge_gallery_capture_require(result.pixel_width == std::ceil((static_cast<double>(rect.x) + rect.width) * scale) - std::floor(static_cast<double>(rect.x) * scale), "页面截图宽度错误");
            forge_gallery_capture_require(result.pixel_height == std::ceil((static_cast<double>(rect.y) + rect.height) * scale) - std::floor(static_cast<double>(rect.y) * scale), "页面截图高度错误");
            const auto pixels = result.image.to_rgba8();
            forge_gallery_capture_require(pixels.has_value(), "页面像素无法读取");
            const auto expected = forge_crop_capture(window_pixels, {rect.x, rect.y, rect.width, rect.height}, scale);
            forge_gallery_capture_require(expected.ok() && std::equal(pixels->cbegin(), pixels->cend(), expected.pixels->cbegin(), expected.pixels->cend()), "页面截图与窗口裁剪像素不一致");
            const auto color_row = pixels->height() - static_cast<std::uint32_t>(34 * scale);
            const slint::Rgba8Pixel colors[] = {{0xe0, 0x50, 0x40, 255}, {0x40, 0xb0, 0x80, 255}, {0x40, 0x80, 0xd0, 255}};
            for (std::uint32_t i = 0; i < 3; ++i) {
                const auto pixel = pixels->cbegin()[color_row * pixels->width() + (2 * i + 1) * pixels->width() / 6];
                forge_gallery_capture_require(pixel.r == colors[i].r && pixel.g == colors[i].g && pixel.b == colors[i].b, "页面色块定位错误");
            }
            const auto corner = pixels->cend()[-3];
            forge_gallery_capture_require(corner.r == 0x20 && corner.g == 0x40 && corner.b == 0x60, "页面截图定位错误");
            const auto error = forge_save_capture_png(*pixels, directory / "page.png");
            forge_gallery_capture_require(error.empty(), error);
            ui->invoke_qa_set_page_active(false);
            forge_gallery_capture_require(!ui->invoke_qa_capture_page().ok, "未激活页面不得截图");
            ui->invoke_qa_set_page_active(true);
            ui->global<ForgeCaptureService>().set_connected(false);
            forge_gallery_capture_require(!ui->invoke_qa_capture_window().ok, "未接入服务不得截图");
            ui->global<ForgeCaptureService>().set_connected(true);
            forge_gallery_capture_require(ui->invoke_qa_capture_page().ok, "页面恢复后截图失败");
            const auto exported = ui->invoke_qa_export_page();
            const auto decoded = slint::Image::load_from_path(exported);
            forge_gallery_capture_require(decoded.size().width == result.pixel_width && decoded.size().height == result.pixel_height, "预览导出 PNG 尺寸错误");
            const auto hidden = GalleryWindow::create();
            forge_gallery_capture_require(!hidden->global<ForgeCaptureService>().get_connected(), "截图服务不能跨窗口共享");
            forge_gallery_capture_require(!forge_capture_window(hidden->window()).ok(), "未显示窗口不得截图");
            ui->window().set_size(slint::LogicalSize{{1100, 740}});
            scene = forge_gallery_create_capture_scene();
        } else if (step == 1) {
            forge_gallery_check_window_capture(ui, directory / "resized.png");
            forge_gallery_check_capture_scene(*scene, 0, directory);
            (*scene)->set_scroll_offset(64);
            (*scene)->set_fit_index(0);
            ui->invoke_language_requested("en");
            ui->invoke_font_scale_requested(1.5f);
            ui->invoke_qa_set_maximized(true);
        } else if (step == 2) {
            forge_gallery_check_window_capture(ui, directory / "maximized.png");
            forge_gallery_check_capture_scene(*scene, 1, directory);
            (*scene)->set_image_empty(true);
        } else if (step == 3) {
            forge_gallery_check_capture_scene(*scene, 2, directory);
            (*scene)->set_image_empty(false);
            (*scene)->set_fit_index(2);
            (*scene)->set_image_width(180);
            (*scene)->set_image_height(80);
        } else if (step == 4) {
            forge_gallery_check_capture_scene(*scene, 3, directory);
            (*scene)->hide();
            std::cout << "CAPTURE_OK host=cpp scale=" << ui->window().scale_factor() << " window=matched page=matched resized=matched maximized=matched scroll=matched overlay=matched image-fit=checked export=checked errors=checked path=" << forge_gallery_capture_path_utf8(directory) << '\n';
            ui->invoke_qa_set_maximized(false);
            slint::quit_event_loop();
        }
        ++step;
    });
}
