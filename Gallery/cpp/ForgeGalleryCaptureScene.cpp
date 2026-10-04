#include "ForgeGalleryCaptureScene.hpp"
#include "ForgeCapture.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>

static void forge_gallery_scene_require(bool condition, const std::string& error) {
    if (!condition) throw std::runtime_error(error);
}

slint::ComponentHandle<ForgeCaptureSceneWindow> forge_gallery_create_capture_scene() {
    auto ui = ForgeCaptureSceneWindow::create();
    forge_bind_capture_service<ForgeCaptureService, ForgeCaptureResult>(ui);
    slint::SharedPixelBuffer<slint::Rgba8Pixel> pixels(100, 50);
    for (std::size_t index = 0; index < 5000; ++index)
        pixels.begin()[index] = index % 100 < 50 ? slint::Rgba8Pixel{224, 80, 64, 255} : slint::Rgba8Pixel{64, 128, 208, 255};
    ui->set_test_image(slint::Image(pixels));
    ui->show();
    return ui;
}

void forge_gallery_check_capture_scene(const slint::ComponentHandle<ForgeCaptureSceneWindow>& ui, int stage, const std::filesystem::path& directory) {
    const auto capture = forge_capture_window(ui->window());
    forge_gallery_scene_require(capture.ok(), capture.error);
    const auto& pixels = *capture.pixels;
    const auto scale = ui->window().scale_factor();
    auto error = forge_save_capture_png(pixels, directory / ("scene-" + std::to_string(stage) + ".png"));
    forge_gallery_scene_require(error.empty(), error);
    const auto size = ui->window().size();
    forge_gallery_scene_require(pixels.width() == size.width && pixels.height() == size.height, "测试窗口截图尺寸错误");
    const auto page = ui->invoke_capture_page();
    forge_gallery_scene_require(page.ok, std::string(page.error));
    const auto rect = ui->get_page_rect();
    const auto expected = forge_crop_capture(pixels, {rect.x, rect.y, rect.width, rect.height}, scale);
    const auto page_pixels = page.image.to_rgba8();
    forge_gallery_scene_require(expected.ok() && page_pixels && std::equal(page_pixels->cbegin(), page_pixels->cend(), expected.pixels->cbegin(), expected.pixels->cend()), "滚动页面截图与窗口裁剪不一致");
    const auto color_at = [&](float x, float y) {
        const auto pixel = pixels.cbegin()[static_cast<std::uint32_t>(std::floor(y * scale)) * pixels.width() + static_cast<std::uint32_t>(std::floor(x * scale))];
        return std::array<std::uint8_t, 3>{pixel.r, pixel.g, pixel.b};
    };
    forge_gallery_scene_require(color_at(320, 108) == std::array<std::uint8_t, 3>{240, 192, 32}, "页面外部覆盖层遗漏");
    forge_gallery_scene_require(color_at(90, 110) == (stage == 0 ? std::array<std::uint8_t, 3>{224, 80, 64} : std::array<std::uint8_t, 3>{64, 176, 128}), "滚动位置错误");
    forge_gallery_scene_require(color_at(90, 260) == std::array<std::uint8_t, 3>{16, 16, 16}, "页面裁剪越界");
    forge_gallery_scene_require(color_at(500, 40) == std::array<std::uint8_t, 3>{16, 16, 16}, "图片圆角错误");
    forge_gallery_scene_require(color_at(560, 42) == std::array<std::uint8_t, 3>{0, 0, 0}, "图片边框错误");
    if (stage == 0) {
        forge_gallery_scene_require(color_at(520, 54) == std::array<std::uint8_t, 3>{96, 96, 96}, "contain 留白错误");
        forge_gallery_scene_require(color_at(520, 90) == std::array<std::uint8_t, 3>{224, 80, 64} && color_at(600, 90) == std::array<std::uint8_t, 3>{64, 128, 208}, "图片缩放颜色错误");
    } else if (stage == 1) {
        forge_gallery_scene_require(color_at(520, 54) == std::array<std::uint8_t, 3>{224, 80, 64}, "cover 未铺满图片区域");
    } else if (stage == 2) {
        forge_gallery_scene_require(color_at(520, 90) == std::array<std::uint8_t, 3>{96, 96, 96}, "清空图片后未显示背景");
    } else {
        forge_gallery_scene_require(color_at(520, 80) == std::array<std::uint8_t, 3>{224, 80, 64} && color_at(650, 80) == std::array<std::uint8_t, 3>{64, 128, 208}, "fill 调整尺寸后图片错误");
        forge_gallery_scene_require(color_at(676, 80) == std::array<std::uint8_t, 3>{0, 0, 0}, "调整尺寸后边框错误");
    }
    if (stage < 2) {
        error = forge_save_capture_png(*page_pixels, directory / ("scrolled-page-" + std::to_string(stage) + ".png"));
        forge_gallery_scene_require(error.empty(), error);
    }
}
