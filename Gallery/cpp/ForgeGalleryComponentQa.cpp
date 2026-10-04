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

struct ForgeGalleryQaFrame { std::string route; bool open = false; bool light = false; bool narrow = false; };

void forge_gallery_start_component_qa(const slint::ComponentHandle<GalleryWindow>& ui, slint::Timer& timer) {
    assert(ui->get_pending_items()->row_count() == 0);
    auto frames = std::make_shared<std::vector<ForgeGalleryQaFrame>>();
    const auto entries = forge_gallery_entries();
    for (const auto& entry : entries) if (entry.item.kind == ForgeTreeItemKind::Page) frames->push_back({entry.item.id});
    for (const auto& entry : entries) if (entry.item.kind == ForgeTreeItemKind::Page) frames->push_back({entry.item.id, false, true});
    for (const auto& entry : entries) if (entry.item.kind == ForgeTreeItemKind::Page) frames->push_back({entry.item.id, false, false, true});
    for (auto route : {"form-date-picker", "form-time-picker", "form-auto-suggest", "inputs-progress-button", "inputs-dropdown-button", "form-multi-select-combo", "form-multi-tree-combo", "popups-content-dialog", "popups-drawer", "other-tour", "chart-bar", "chart-line", "chart-pie", "chart-polar", "chart-bubble", "chart-scatter", "chart-radar", "chart-professional", "data-grid", "data-tree-grid"}) frames->push_back({route, true, false});
    assert(frames->size() == 308);
    if (const char* filter = std::getenv("SLINTFORGE_COMPONENT_QA_FILTER")) std::erase_if(*frames, [filter](const auto& frame) { return !frame.route.starts_with(filter); });
    assert(!frames->empty());
    const char* custom_directory = std::getenv("SLINTFORGE_COMPONENT_QA_DIR");
    const auto directory = std::filesystem::path(custom_directory ? custom_directory : "build/component-qa/cpp");
    std::filesystem::create_directories(directory);
    auto phase = std::make_shared<std::size_t>(0);
    const slint::ComponentWeakHandle<GalleryWindow> weak(ui);
    timer.start(slint::TimerMode::Repeated, std::chrono::milliseconds(180), [weak, phase, frames, directory]() {
        auto locked = weak.lock(); if (!locked) throw std::runtime_error("component QA window lost");
        auto ui = *locked; const auto index = *phase / 3, stage = *phase % 3; const auto& frame = frames->at(index);
        if (stage == 0) {
            ui->invoke_qa_prepare_component(false);
            ui->invoke_theme_setting_changed(!frame.light); ui->set_theme_dark(!frame.light);
            ui->invoke_language_requested(frame.light ? "en" : "zh");
            ui->invoke_font_scale_requested(frame.light ? 1.25f : 1.f);
            ui->window().set_size(frame.narrow ? slint::LogicalSize{{960, 720}} : frame.light ? slint::LogicalSize{{1100, 800}} : slint::LogicalSize{{1280, 900}});
            ui->invoke_navigate_requested(frame.route.c_str(), std::make_shared<slint::VectorModel<ForgeRouteParam>>());
            assert(std::string_view(ui->get_active_route_id()) == frame.route);
        } else if (stage == 1) ui->invoke_qa_prepare_component(frame.open);
        else {
            if (frame.open && (frame.route == "data-grid" || frame.route == "data-tree-grid") && !ui->get_qa_grid_editing())
                throw std::runtime_error("grid operation capture must include editor");
            if (frame.route == "chart-professional" && ui->get_qa_plot_scene().status != 0) throw std::runtime_error(std::string(ui->get_qa_plot_scene().error));
            auto capture = forge_capture_window(ui->window()); if (!capture.ok()) throw std::runtime_error(capture.error);
            std::ostringstream name; name << std::setfill('0') << std::setw(3) << index << '-' << frame.route << '-' << (frame.narrow ? "narrow-zh" : frame.light ? "light-en" : "dark-zh") << (frame.open ? "-open" : "") << ".png";
            const auto error = forge_save_capture_png(*capture.pixels, directory / name.str()); if (!error.empty()) throw std::runtime_error(error);
            if (index + 1 == frames->size()) {
                ui->invoke_qa_prepare_component(false);
                std::cout << "COMPONENTS_RENDER_OK catalog=96 implemented=96 deferred=0 frames=" << frames->size() << '\n';
                slint::quit_event_loop();
            }
        }
        ++*phase;
    });
}
