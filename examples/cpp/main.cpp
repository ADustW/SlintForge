// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#include "minimal.h"
#include "ForgeCapture.hpp"
#include "ForgeWidgets.hpp"
#include "ForgePlot.hpp"
#include "ForgeGrids.hpp"
#include "ForgeRendering.hpp"
#include <cassert>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include "ForgeBusinessSmoke.hpp"

static int forge_example_widgets_smoke() {
    auto window = WidgetsWindow::create();
    forge_bind_widget_services<ForgeWidgetServices, ForgeQRCodeResult, ForgeSyntaxLine>(window);
    slint::SharedPixelBuffer<slint::Rgba8Pixel> pixels(64, 48);
    for (std::uint32_t y = 0; y < 48; ++y) for (std::uint32_t x = 0; x < 64; ++x) pixels.begin()[y * 64 + x] = {static_cast<std::uint8_t>(x * 3), static_cast<std::uint8_t>(y * 4), 160, 255};
    window->set_source(slint::Image(pixels));
    slint::Timer timer; const slint::ComponentWeakHandle<WidgetsWindow> weak(window);
    timer.start(slint::TimerMode::SingleShot, std::chrono::milliseconds(350), [weak]() {
        auto ui = *weak.lock();
        assert(ui->get_qr_result().ok); assert(ui->get_syntax_line_count() == 4);
        assert(ui->get_processed().size().width == 64 && ui->get_processed().size().height == 48);
        const auto directory = std::filesystem::path("build/component-qa"); std::filesystem::create_directories(directory);
        const auto capture = forge_capture_window(ui->window()); assert(capture.ok());
        assert(forge_save_capture_png(*capture.pixels, directory / "consumer-cpp.png").empty());
        std::cout << "CONSUMER_WIDGETS_OK host=cpp qr=encoded syntax=4 backdrop=64x48\n";
        slint::quit_event_loop();
    });
    window->run(); return 0;
}

static int forge_example_charts_smoke() {
    auto window = ChartsWindow::create();
    forge_bind_plot_services<ForgePlotServices, ForgePlotScene, ForgePlotAxis, ForgePlotRange, ForgePlotRect, ForgePlotHit, ForgePlotSelection, ForgePlotExportResult>(window);
    const auto directory = std::filesystem::path("build/charts-qa");
    std::filesystem::create_directories(directory);
    auto phase = std::make_shared<int>(0);
    slint::Timer timer; const slint::ComponentWeakHandle<ChartsWindow> weak(window);
    timer.start(slint::TimerMode::Repeated, std::chrono::milliseconds(120), [weak, phase, directory]() {
        auto ui = *weak.lock(); const auto index = *phase;
        if (index % 2 == 0) ui->set_kind(index / 2);
        else {
            const auto kind = index / 2;
            assert(ui->get_status() == 0 && ui->get_mark_count() == ((kind == 2 || kind == 3) ? 2 : 6));
            const auto capture = forge_capture_window(ui->window()); assert(capture.ok());
            assert(forge_save_capture_png(*capture.pixels, directory / ("consumer-cpp-" + std::to_string(kind) + ".png")).empty());
            if (kind == 6) { std::cout << "CONSUMER_CHARTS_OK host=cpp kinds=7\n"; slint::quit_event_loop(); }
        }
        ++*phase;
    });
    window->run(); return 0;
}

static int forge_example_grids_smoke() {
    auto window = GridsWindow::create();
    forge_bind_grid_services<ForgeGridServices, ForgeGridScene, ForgeGridViewRow, ForgeGridViewColumn,
        ForgeGridRow, ForgeGridCell, ForgeGridColumn, ForgeGridEditResult>(window);
    const auto& services = window->global<ForgeGridServices>();
    // 空数组和空嵌套模型也是公共入口的合法输入。
    assert(services.invoke_prepare({}, {}, false, -1, false, "", 0).status == 1);
    assert(services.invoke_find_row({}, "missing") == -1);
    assert(services.invoke_find_column({}, 0) == -1);
    assert(services.invoke_step({}, -1, 1) == -1);
    assert(services.invoke_commit({}, {}, 0, 0, "value").status == 1);
    auto blank = std::make_shared<slint::VectorModel<ForgeGridRow>>(std::vector<ForgeGridRow>{ForgeGridRow{}});
    assert(services.invoke_prepare(blank, {}, true, -1, false, "", 0).status == 2);
    const auto directory = std::filesystem::path("build/grids-qa"); std::filesystem::create_directories(directory);
    auto phase = std::make_shared<int>(0); slint::Timer timer;
    const slint::ComponentWeakHandle<GridsWindow> weak(window);
    timer.start(slint::TimerMode::Repeated, std::chrono::milliseconds(140), [weak, phase, directory]() {
        auto ui = *weak.lock(); const auto index = *phase, mode = index / 2;
        if (index % 2 == 0) ui->invoke_prepare(mode);
        else {
            assert(ui->get_status() == (mode == 2 ? 1 : 0));
            assert(ui->get_visible_count() == (mode == 2 ? 0 : mode == 6 ? 2 : 3));
            if (mode == 3) assert(ui->get_selected_count() == 0);
            if (mode == 4) assert(ui->get_selected_count() == 1);
            if (mode == 5) assert(ui->get_editing());
            const auto capture = forge_capture_window(ui->window()); assert(capture.ok());
            assert(forge_save_capture_png(*capture.pixels, directory / ("consumer-cpp-" + std::to_string(mode) + ".png")).empty());
            if (mode == 6) { std::cout << "CONSUMER_GRIDS_OK host=cpp states=7\n"; slint::quit_event_loop(); }
        }
        ++*phase;
    });
    window->run(); return 0;
}

int main(int argc, char** argv) {
    const auto rendering = forge_configure_rendering();
    if (argc > 1 && (std::string_view(argv[1]) == "--suggestion-demo" || std::string_view(argv[1]) == "--suggestion-smoke")) return forge_business_suggestion(std::string_view(argv[1]) == "--suggestion-smoke");
    if (argc > 1 && (std::string_view(argv[1]) == "--dialog-demo" || std::string_view(argv[1]) == "--dialog-smoke")) return forge_business_dialog(std::string_view(argv[1]) == "--dialog-smoke");
    if (argc > 1 && std::string_view(argv[1]) == "--grids-smoke") return forge_example_grids_smoke();
    if (argc > 1 && std::string_view(argv[1]) == "--charts-smoke") return forge_example_charts_smoke();
    if (argc > 1 && std::string_view(argv[1]) == "--widgets-smoke") return forge_example_widgets_smoke();
    auto window = MinimalWindow::create();
    forge_bind_capture_service<ForgeCaptureService, ForgeCaptureResult>(window);
    window->on_export_capture([](slint::Image image) -> slint::SharedString {
        const auto pixels = image.to_rgba8();
        if (!pixels) return "无法读取截图像素";
        std::error_code directory_error;
        const auto directory = std::filesystem::absolute("build/captures", directory_error);
        if (!directory_error) std::filesystem::create_directories(directory, directory_error);
        if (directory_error) return ("无法创建截图目录: " + directory_error.message()).c_str();
        const auto stamp = std::chrono::system_clock::now().time_since_epoch().count();
        const auto path = directory / ("consumer-cpp-" + std::to_string(stamp) + ".png");
        const auto error = forge_save_capture_png(*pixels, path);
        const auto bytes = path.u8string();
        return error.empty() ? std::string(bytes.begin(), bytes.end()).c_str() : error.c_str();
    });
    slint::Timer timer;
    if (argc > 1 && std::string_view(argv[1]) == "--capture-smoke") {
        const slint::ComponentWeakHandle<MinimalWindow> weak(window);
        timer.start(slint::TimerMode::SingleShot, std::chrono::milliseconds(250), [weak]() {
            const auto ui = *weak.lock();
            if (!ui->invoke_qa_capture_window().ok) throw std::runtime_error("消费者整窗截图失败");
            const auto result = ui->invoke_qa_capture_page();
            if (!result.ok) throw std::runtime_error(std::string(result.error));
            const auto path = ui->invoke_export_capture(result.image);
            if (!std::filesystem::exists(std::filesystem::u8path(path.data()))) throw std::runtime_error(std::string(path));
            std::cout << "CONSUMER_CAPTURE_OK host=cpp path=" << path << '\n';
            slint::quit_event_loop();
        });
    }
    window->run();
    return 0;
}
