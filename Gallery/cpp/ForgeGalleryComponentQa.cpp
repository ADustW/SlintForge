// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#include "ForgeGalleryComponentQa.hpp"
#include "ForgeGalleryNavigation.hpp"
#include "ForgeCapture.hpp"
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

struct ForgeGalleryQaFrame { std::string route; bool open = false; bool light = false; bool narrow = false; int showcase = -1; };

void forge_gallery_start_component_qa(const slint::ComponentHandle<GalleryWindow>& ui, slint::Timer& timer) {
    assert(ui->get_pending_items()->row_count() == 0);
    auto frames = std::make_shared<std::vector<ForgeGalleryQaFrame>>();
    const auto entries = forge_gallery_entries();
    for (const auto& entry : entries) if (entry.item.kind == ForgeTreeItemKind::Page) frames->push_back({entry.item.id});
    for (const auto& entry : entries) if (entry.item.kind == ForgeTreeItemKind::Page) frames->push_back({entry.item.id, false, true});
    for (const auto& entry : entries) if (entry.item.kind == ForgeTreeItemKind::Page) frames->push_back({entry.item.id, false, false, true});
    for (auto route : {"form-date-picker", "form-time-picker", "form-auto-suggest", "inputs-progress-button", "inputs-dropdown-button", "form-multi-select-combo", "form-multi-tree-combo", "popups-content-dialog", "popups-drawer", "other-tour", "chart-bar", "chart-line", "chart-pie", "chart-polar", "chart-bubble", "chart-scatter", "chart-radar", "chart-professional", "data-grid", "data-tree-grid"}) frames->push_back({route, true, false});
    assert(!frames->empty());
    // 浅色弹层同时检查遮罩、卡片边界和正文与操作区分隔线。
    for (auto route : {"popups-content-dialog", "popups-drawer"}) frames->push_back({route, true, true});
    // 本地化弹层覆盖中英文、放大字号、窄窗口和 12/24 小时时钟/数字模式。
    for (auto route : {"form-date-picker", "form-calendar-picker", "form-time-picker"}) {
        if (std::string_view(route) == "form-calendar-picker") frames->push_back({route, true, false});
        frames->push_back({route, true, true});
        frames->push_back({route, true, false, true});
    }
    for (int mode : {-2, -3, -4}) {
        frames->push_back({"form-time-picker", true, false, false, mode});
        frames->push_back({"form-time-picker", true, true, false, mode});
        frames->push_back({"form-time-picker", true, false, true, mode});
    }
    // 两种日历弹层分别验收，覆盖中英文、放大字号和窄窗口。
    frames->push_back({"form-calendar-picker", true, false, false, -2});
    frames->push_back({"form-calendar-picker", true, true, false, -2});
    frames->push_back({"form-calendar-picker", true, false, true, -2});
    for (auto route : {"form-color-picker", "form-shortcut-picker", "inputs-delay-button"}) {
        frames->push_back({route, true, false}); frames->push_back({route, true, true}); frames->push_back({route, true, false, true});
    }
    const std::vector<std::string> responsive = {"inputs-standard-button", "form-textbox", "form-color-picker", "form-auto-suggest", "feedback-result", "feedback-skeleton", "chart-bar", "chart-line", "chart-professional", "data-grid", "data-tree-grid", "popups-content-dialog"};
    std::istringstream showcase_routes(
#include "../data/showcase-routes.inc"
    );
    for (std::string line; std::getline(showcase_routes, line);) {
        const auto divider = line.find('|'); if (divider == std::string::npos) continue;
        const auto route = line.substr(0, divider); const auto count = std::stoi(line.substr(divider + 1));
        for (int tab = 0; tab < count; ++tab) {
            frames->push_back({route, route == "popups-content-dialog", false, false, tab});
            if (std::find(responsive.begin(), responsive.end(), route) != responsive.end()) {
                frames->push_back({route, route == "popups-content-dialog", true, false, tab});
                frames->push_back({route, route == "popups-content-dialog", false, true, tab});
            }
        }
    }
    if (const char* filter = std::getenv("SLINTFORGE_COMPONENT_QA_FILTER")) {
        std::istringstream input(filter); std::vector<std::string> selected;
        for (std::string route; std::getline(input, route, ',');) if (!route.empty()) selected.push_back(route);
        std::erase_if(*frames, [&selected](const auto& frame) { return std::none_of(selected.begin(), selected.end(), [&](const auto& prefix) { return frame.route.starts_with(prefix); }); });
    }
    if (std::getenv("SLINTFORGE_COMPONENT_QA_BASE_ONLY")) std::erase_if(*frames, [](const auto& frame) {return frame.showcase != -1;});
    if (const char* exclude = std::getenv("SLINTFORGE_COMPONENT_QA_EXCLUDE")) std::erase_if(*frames, [exclude](const auto& frame) { return frame.route.starts_with(exclude); });
    assert(!frames->empty());
    const char* custom_directory = std::getenv("SLINTFORGE_COMPONENT_QA_DIR");
    const auto directory = std::filesystem::path(custom_directory ? custom_directory : "build/component-qa/cpp");
    std::filesystem::create_directories(directory);
    auto phase = std::make_shared<std::size_t>(0);
    auto retries = std::make_shared<int>(0);
    auto minimized_once = std::make_shared<bool>(false);
    const bool background = std::getenv("SLINTFORGE_COMPONENT_QA_BACKGROUND") != nullptr;
    const bool inject_minimize = !background && std::getenv("SLINTFORGE_COMPONENT_QA_MINIMIZE_ONCE") != nullptr;
    slint::LogicalSize forced_size{{0, 0}};
    if (const char* value = std::getenv("SLINTFORGE_COMPONENT_QA_SIZE")) {
        std::istringstream input(value); char separator = 0;
        input >> forced_size.width >> separator >> forced_size.height;
        if (!input || separator != 'x' || forced_size.width < 960 || forced_size.height < 600) throw std::runtime_error("验收尺寸须为至少 960x600 的宽x高");
    }
    const slint::ComponentWeakHandle<GalleryWindow> weak(ui);
    timer.start(slint::TimerMode::Repeated, std::chrono::milliseconds(180), [weak, phase, frames, directory, retries, minimized_once, inject_minimize, forced_size, background]() {
        auto locked = weak.lock(); if (!locked) throw std::runtime_error("component QA window lost");
        auto ui = *locked; const auto index = *phase / 3, stage = *phase % 3; const auto& frame = frames->at(index);
        if (stage == 0) {
            if (!background) ui->window().set_minimized(false);
            ui->invoke_qa_showcase("", -1);
            ui->invoke_qa_prepare_component(false);
            ui->invoke_theme_setting_changed(!frame.light); ui->set_theme_dark(!frame.light);
            ui->invoke_language_requested(frame.light ? "en" : "zh");
            ui->invoke_font_scale_requested(frame.light ? 1.25f : 1.f);
            ui->window().set_size(forced_size.width > 0 ? forced_size : frame.narrow ? slint::LogicalSize{{960, 600}} : frame.light ? slint::LogicalSize{{1100, 800}} : slint::LogicalSize{{1280, 900}});
            ui->invoke_navigate_requested(frame.route.c_str(), std::make_shared<slint::VectorModel<ForgeRouteParam>>());
            assert(std::string_view(ui->get_active_route_id()) == frame.route);
        } else if (stage == 1) {
            if (frame.route.starts_with("flow-")) {
                const auto before = ui->invoke_qa_flow_state(frame.route.c_str());
                if (before.node_count != (frame.route == "flow-mindmap" ? 9 : 6)) throw std::runtime_error("节点页面未初始化");
                if (frame.light || frame.narrow) {
                    if (before.nodes->row_data(0)->title != "页面返回后保留") throw std::runtime_error("节点页面返回后丢失编辑状态");
                } else {
                    ForgeFlowAction action{}; action.op = "title"; action.id = "node-1"; action.text = "页面返回后保留";
                    ui->invoke_qa_flow_edit(frame.route.c_str(), action);
                }
                ui->invoke_search_requested(frame.light ? "Custom Cards" : "自定义卡片");
                bool found = false; const auto items = ui->get_navigation_items();
                for (std::size_t i = 0; i < items->row_count(); ++i) if (items->row_data(i)->id == "flow-custom") found = true;
                if (!found) throw std::runtime_error("节点页面导航搜索未命中");
                ui->invoke_search_requested("");
            }
            if (frame.route == "form-time-picker") ui->invoke_qa_time_picker(frame.showcase == -3, frame.showcase != -3 && frame.showcase != -4);
            if (frame.route == "form-calendar-picker") ui->invoke_qa_calendar_picker(frame.showcase == -2);
            if (frame.showcase >= 0) ui->invoke_qa_showcase(frame.route.c_str(), frame.showcase);
            ui->invoke_qa_prepare_component(frame.open);
            if (inject_minimize && !*minimized_once) { *minimized_once = true; ui->window().set_minimized(true); }
        }
        else {
            const auto size = ui->window().size();
            if (ui->window().is_minimized() || size.width < 960 || size.height < 600) {
                if (background) throw std::runtime_error("后台测试窗口尺寸无效，不执行前台恢复");
                if (++*retries > 2) throw std::runtime_error("验收窗口恢复后仍未完成布局");
                // 确保原生最小化和 Slint 属性都收到恢复请求，再从布局准备阶段重试。
                ui->window().set_minimized(true); ui->window().set_minimized(false);
                std::cout << "QA_WINDOW_RESTORED host=cpp route=" << frame.route << '\n';
                *phase = index * 3; return;
            }
            *retries = 0;
            if (frame.open && (frame.route == "data-grid" || frame.route == "data-tree-grid") && !ui->get_qa_grid_editing())
                throw std::runtime_error("grid operation capture must include editor");
            if (frame.route == "chart-professional" && (frame.showcase < 0 || frame.showcase == 2) && ui->get_qa_plot_scene().status != 0) throw std::runtime_error(std::string(ui->get_qa_plot_scene().error));
            if (frame.route == "chart-professional" && !ui->get_qa_showcase_ready()) throw std::runtime_error("图表总览场景必须全部完成渲染");
            auto capture = background ? ForgeCapturedImage{ui->window().take_snapshot(), {}} : forge_capture_window(ui->window()); if (!capture.ok()) throw std::runtime_error(capture.error);
            if (capture.pixels->width() < 960 || capture.pixels->height() < 600) throw std::runtime_error("整窗截图尺寸不正确");
            std::ostringstream name; name << std::setfill('0') << std::setw(3) << index << '-' << frame.route << '-' << (frame.narrow ? "narrow-zh" : frame.light ? "light-en" : "dark-zh") << (frame.open ? "-open" : "");
            if (frame.showcase >= 0) name << "-showcase" << frame.showcase;
            name << ".png";
            const auto error = forge_save_capture_png(*capture.pixels, directory / name.str()); if (!error.empty()) throw std::runtime_error(error);
            if (index + 1 == frames->size()) {
                ui->invoke_qa_prepare_component(false);
                std::cout << "COMPONENTS_RENDER_OK deferred=0 frames=" << frames->size() << '\n';
                slint::quit_event_loop();
            }
        }
        ++*phase;
    });
}
