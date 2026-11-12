// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "ForgeCapture.hpp"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>

// 只向 Slint 场景发送事件，不移动系统鼠标、不发送系统按键或激活窗口。
static void forge_gallery_titlebar_click(const slint::ComponentHandle<GalleryWindow>& ui, float x) {
    const slint::LogicalPosition position{{x, 18.f}};
    const auto start = std::chrono::steady_clock::now();
    ui->window().dispatch_pointer_move_event(position);
    ui->window().dispatch_pointer_press_event(position, slint::PointerEventButton::Left);
    ui->window().dispatch_pointer_release_event(position, slint::PointerEventButton::Left);
    std::cout << "TITLEBAR_INPUT_MS " << std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count() << '\n';
}

static void forge_gallery_start_titlebar_qa(const slint::ComponentHandle<GalleryWindow>& ui, slint::Timer& timer) {
    const char* custom = std::getenv("SLINTFORGE_TITLEBAR_QA_DIR");
    const auto directory = custom ? std::filesystem::path(custom) : std::filesystem::path("build/titlebar/cpp-background");
    std::filesystem::create_directories(directory);
    auto phase = std::make_shared<int>(0);
    const slint::ComponentWeakHandle<GalleryWindow> weak(ui);
    timer.start(slint::TimerMode::Repeated, std::chrono::milliseconds(200), [weak, phase, directory] {
        auto locked = weak.lock(); if (!locked) throw std::runtime_error("标题栏测试窗口丢失");
        const auto ui = *locked;
        const int step = (*phase)++;
        if (step == 0) {
            ui->window().set_size(slint::LogicalSize{{1380, 860}});
            ui->invoke_theme_setting_changed(false);
            return;
        }
        if (step % 2 == 0) {
            if (step == 2 || step == 4) {
                const float width = ui->window().size().width / ui->window().scale_factor();
                forge_gallery_titlebar_click(ui, width - 198);
                if (ui->get_theme_dark() != (step == 2)) throw std::runtime_error("主题按钮未响应");
            } else if (step == 6 || step == 8 || step == 12 || step == 14) {
                forge_gallery_titlebar_click(ui, 60);
                const auto expected = step == 6 || step == 12 ? ForgePaneState::Compact : ForgePaneState::Expanded;
                if (ui->get_pane_state() != expected) throw std::runtime_error("菜单按钮必须切换完整菜单与图标栏");
            } else if (step == 10) {
                ui->window().set_size(slint::LogicalSize{{2560, 1440}});
                ui->invoke_language_requested("en");
                ui->invoke_font_scale_requested(1.25f);
                ui->invoke_theme_setting_changed(true);
            } else if (step == 16) {
                ui->window().set_size(slint::LogicalSize{{960, 600}});
                ui->invoke_language_requested("zh");
                ui->invoke_font_scale_requested(1.f);
                ui->invoke_theme_setting_changed(false);
            } else if (step == 26) {
                ui->window().set_size(slint::LogicalSize{{1920, 1080}});
                ui->invoke_navigate_requested("home", std::make_shared<slint::VectorModel<ForgeRouteParam>>());
            } else if (step == 18 || step == 20 || step == 22 || step == 24) {
                if (step == 22) {
                    ForgeFlowAction action{}; action.op = "title"; action.id = "node-1"; action.text = "后台验证草稿";
                    ui->invoke_qa_flow_edit("flow-custom", action);
                }
                const char* route = step == 18 ? "chart-professional" : step == 22 ? "data-grid" : "flow-custom";
                ui->invoke_navigate_requested(route, std::make_shared<slint::VectorModel<ForgeRouteParam>>());
            }
            return;
        }
        auto pixels = ui->window().take_snapshot();
        if (!pixels || pixels->width() < 960 || pixels->height() < 600) throw std::runtime_error("后台窗口截图无效");
        const auto path = directory / (std::to_string(step) + "-" + (ui->get_theme_dark() ? "dark" : "light") + "-" + (ui->get_pane_state() == ForgePaneState::Compact ? "compact" : "expanded") + ".png");
        const auto error = forge_save_capture_png(*pixels, path);
        if (!error.empty()) throw std::runtime_error(error);
        if (step == 19 && ui->get_qa_plot_scene().status != 0) throw std::runtime_error("图表重新显示后未恢复有效场景");
        if (step == 27) {
            std::cout << "TITLEBAR_BACKGROUND_OK captures=14 scene_pointer_events=true heavy_pages_resumed=true\n";
            slint::quit_event_loop();
        }
        if (step == 25) {
            const auto state = ui->invoke_qa_flow_state("flow-custom");
            if (state.node_count != 6 || state.nodes->row_data(0)->title != "后台验证草稿") throw std::runtime_error("隐藏页面后丢失节点编辑状态");

        }
    });
}
