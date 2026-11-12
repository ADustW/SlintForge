// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#include "ForgePlot.hpp"
#include "plot-live-zoom.h"
#include <slint-platform.h>
#include <cmath>
#include <iostream>
#include <stdexcept>

static void forge_plot_live_require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}

static void forge_plot_live_append(const slint::ComponentHandle<ForgePlotLiveZoomTestWindow>& ui, float x) {
    forge_plot_live_require(ui->invoke_append(x), "追加实时数据失败");
    slint::platform::update_timers_and_animations();
    forge_plot_live_require(ui->get_scene().status == 0, "实时场景计算失败");
}

int main() {
    try {
        for (auto mode : {ForgePlotXWindowMode::Fixed, ForgePlotXWindowMode::Unlimited}) {
            const auto ui = ForgePlotLiveZoomTestWindow::create();
            forge_bind_plot_services<ForgePlotServices, ForgePlotScene, ForgePlotAxis,
                ForgePlotRange, ForgePlotRect, ForgePlotHit, ForgePlotSelection, ForgePlotExportResult>(ui);
            ui->set_window_mode(mode);
            slint::platform::update_timers_and_animations();
            forge_plot_live_append(ui, 130);
            float latest = 130, span = 10;
            for (float factor : {0.5f, 2.4f, 0.75f}) {
                ui->invoke_zoom(factor); span *= factor;
                slint::platform::update_timers_and_animations();
                for (int step = 0; step < 3; ++step) {
                    const auto before = ui->get_scene();
                    latest += 0.5f; forge_plot_live_append(ui, latest);
                    const auto scene = ui->get_scene();
                    forge_plot_live_require(std::abs(scene.window_range.maximum - latest) < 0.001f,
                        "缩放后实时窗口没有跟随新数据");
                    forge_plot_live_require(std::abs(scene.window_range.maximum - scene.window_range.minimum - span) < 0.001f,
                        "实时追加改变了缩放跨度");
                    forge_plot_live_require(scene.input_points == before.input_points + 1 && scene.svg != before.svg,
                        "新数据没有进入更新后的可见场景");
                }
            }
            ui->invoke_history(0.25f); ui->invoke_zoom(0.5f); span *= 0.5f;
            const auto history = ui->get_scene().window_range;
            forge_plot_live_append(ui, 150);
            forge_plot_live_require(ui->get_scene().window_range.minimum == history.minimum
                && ui->get_scene().window_range.maximum == history.maximum, "历史定位被实时追加打断");
            ui->invoke_follow();
            const auto followed = ui->get_scene().window_range;
            forge_plot_live_require(std::abs(followed.maximum - 150) < 0.001f
                && std::abs(followed.maximum - followed.minimum - span) < 0.001f, "恢复跟随丢失缩放比例");
            ui->invoke_reset();
            const auto reset = ui->get_scene().window_range;
            forge_plot_live_require(std::abs(reset.maximum - 150) < 0.001f
                && std::abs(reset.maximum - reset.minimum - 10) < 0.001f, "复位没有恢复配置跨度");
        }
        for (int direction : {1, 2, 3}) {
            const auto ui = ForgePlotLiveZoomTestWindow::create();
            forge_bind_plot_services<ForgePlotServices, ForgePlotScene, ForgePlotAxis,
                ForgePlotRange, ForgePlotRect, ForgePlotHit, ForgePlotSelection, ForgePlotExportResult>(ui);
            forge_plot_live_append(ui, 130);
            const auto before = ui->get_scene();
            ui->invoke_zoom_direction(direction);
            ui->invoke_zoom(0.5f);
            const auto zoomed = ui->get_scene();
            for (int axis = 0; axis < 2; ++axis) {
                const auto old = before.ranges->row_data(axis).value();
                const auto next = zoomed.ranges->row_data(axis).value();
                const bool enabled = (direction & (1 << axis)) != 0;
                forge_plot_live_require(std::abs(next.maximum - next.minimum
                    - (old.maximum - old.minimum) * (enabled ? 0.5f : 1.f)) < 0.001f, "单轴缩放改变了错误方向");
                if (!enabled) forge_plot_live_require(next.minimum == old.minimum && next.maximum == old.maximum,
                    "未启用缩放的坐标轴范围发生变化");
            }
            forge_plot_live_append(ui, 131);
            const auto live = ui->get_scene();
            forge_plot_live_require(std::abs(live.window_range.maximum - 131) < 0.001f
                && std::abs(live.window_range.maximum - live.window_range.minimum - ((direction & 1) ? 5.f : 10.f)) < 0.001f,
                "单轴缩放后实时跟随或跨度错误");
            ui->invoke_pan(24, 18);
            const auto panned = ui->get_scene();
            for (int axis = 0; axis < 2; ++axis) {
                forge_plot_live_require(std::abs(panned.ranges->row_data(axis)->minimum
                    - live.ranges->row_data(axis)->minimum) > 0.001f, "缩放方向错误地限制了平移方向");
            }
        }
        std::cout << "PLOT_LIVE_ZOOM_OK modes=2 zooms=6 live_appends=18 history=preserved reset=passed zoom_directions=3 pan=independent\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
