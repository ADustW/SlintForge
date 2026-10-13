#include "flow-transform.h"
#include "ForgeCapture.hpp"
#include <iostream>
#include <stdexcept>
#include <cstdlib>

static void forge_flow_probe_click(slint::Window& window, float x, float y) {
    window.dispatch_pointer_move_event(slint::LogicalPosition{{x, y}});
    window.dispatch_pointer_press_event(slint::LogicalPosition{{x, y}}, slint::PointerEventButton::Left);
    window.dispatch_pointer_release_event(slint::LogicalPosition{{x, y}}, slint::PointerEventButton::Left);
}
int main() {
    auto ui = ForgeFlowTransformWindow::create();
    auto phase = std::make_shared<int>(0);
    auto failure = std::make_shared<std::string>();
    slint::Timer timer;
    timer.start(slint::TimerMode::Repeated, std::chrono::milliseconds(180), [=]() {
        try {
            const auto scale = ui->get_zoom();
            switch ((*phase)++) {
            case 0: forge_flow_probe_click(ui->window(), 60 + 40 * scale, 50 + 70 * scale); ui->window().dispatch_key_press_event("x"); ui->window().dispatch_key_release_event("x"); break;
            case 1: if (ui->get_draft() != "x") throw std::runtime_error("缩放后输入命中失败"); forge_flow_probe_click(ui->window(), 60 + 50 * scale, 50 + 120 * scale); break;
            case 2: if (ui->get_clicks() != 1) throw std::runtime_error("缩放后按钮命中失败"); forge_flow_probe_click(ui->window(), 60 + 80 * scale, 50 + 178 * scale); break;
            case 3: {
                const auto capture = forge_capture_window(ui->window());
                if (!capture.ok()) throw std::runtime_error(capture.error);
                const auto path = std::filesystem::path("build/flow-qa/probe-cpp.png");
                std::filesystem::create_directories(path.parent_path());
                auto error = forge_save_capture_png(*capture.pixels, path);
                if (!error.empty()) throw std::runtime_error(error);
                ui->window().dispatch_key_press_event(slint::platform::key_codes::DownArrow);
                ui->window().dispatch_key_release_event(slint::platform::key_codes::DownArrow);
                ui->window().dispatch_key_press_event(slint::platform::key_codes::Return);
                ui->window().dispatch_key_release_event(slint::platform::key_codes::Return);
                break;
            }
            default: if (ui->get_choice() != 1) throw std::runtime_error("缩放后弹层选择失败"); std::cout << "FLOW_TRANSFORM_OK host=cpp renderer=" << std::getenv("SLINT_BACKEND") << '\n'; slint::quit_event_loop();
            }
        } catch (const std::exception& e) { *failure = e.what(); slint::quit_event_loop(); }
    });
    ui->run();
    if (!failure->empty()) { std::cerr << *failure << '\n'; return 1; }
}
