// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#include "ForgeGalleryDemos.hpp"
#include "ForgeWidgets.hpp"
#include "ForgeGalleryGrids.hpp"
#include "ForgeGalleryPlot.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <optional>
#include <tuple>

static std::vector<ForgeTreeItem> forge_gallery_demo_tree() {
    const std::array<std::tuple<const char*, const char*, const char*, bool, bool, bool>, 7> definitions{{
        {"project", "", "SlintForge", true, true, true},
        {"project/sdk", "project", "SDK", true, true, true},
        {"project/sdk/rust", "project/sdk", "Rust", false, false, true},
        {"project/sdk/cpp", "project/sdk", "C++", false, false, true},
        {"project/gallery", "project", "Gallery", true, false, true},
        {"project/gallery/main", "project/gallery", "main-screen.slint", false, false, true},
        {"project/readonly", "project", "readonly.txt", false, false, false},
    }};
    std::vector<ForgeTreeItem> items;
    for (const auto& [id, parent, label, folder, expanded, enabled] : definitions) {
        ForgeTreeItem item{}; item.id = id; item.parent_id = parent; item.label = label;
        item.folder = folder; item.expanded = expanded; item.enabled = enabled; item.visible = true;
        items.push_back(std::move(item));
    }
    return items;
}

static void forge_gallery_refresh_demo_tree(const slint::ComponentHandle<GalleryWindow>& ui, std::vector<ForgeTreeItem>& items) {
    const auto source = items;
    std::string summary;
    for (auto& item : items) {
        item.depth = 0; item.visible = true; auto parent = item.parent_id;
        while (!parent.empty()) {
            const auto node = std::find_if(source.begin(), source.end(), [&](const auto& node) { return node.id == parent; });
            if (node == source.end()) break;
            ++item.depth; item.visible = item.visible && node->expanded; parent = node->parent_id;
        }
        if (item.selected && !item.folder) { if (!summary.empty()) summary += ", "; summary += std::string_view(item.label); }
    }
    const auto& global = ui->global<GalleryDemoModels>();
    global.set_tree_selection(summary.c_str());
    global.set_tree_items(std::make_shared<slint::VectorModel<ForgeTreeItem>>(items));
}

void forge_gallery_connect_demos(const slint::ComponentHandle<GalleryWindow>& ui) {
    forge_gallery_connect_plot(ui);
    forge_gallery_connect_grids(ui);
    auto model = std::make_shared<std::vector<ForgeTreeItem>>(forge_gallery_demo_tree());
    forge_gallery_refresh_demo_tree(ui, *model);
    slint::SharedPixelBuffer<slint::Rgba8Pixel> pixels(600, 360);
    for (std::uint32_t y = 0; y < 360; ++y) for (std::uint32_t x = 0; x < 600; ++x) {
        const auto checker = (x / 45 + y / 45) % 2;
        pixels.begin()[y * 600 + x] = {static_cast<std::uint8_t>(40 + x * 150 / 600), static_cast<std::uint8_t>(70 + y * 100 / 360), static_cast<std::uint8_t>(100 + checker * 100), 255};
    }
    ui->global<GalleryDemoModels>().set_backdrop_image(slint::Image(pixels));
    forge_bind_widget_services<ForgeWidgetServices, ForgeQRCodeResult, ForgeSyntaxLine>(ui);
    const slint::ComponentWeakHandle<GalleryWindow> weak(ui);
    const auto& global = ui->global<GalleryDemoModels>();
    global.on_add_tag([weak](slint::SharedString value) {
        const auto ui = weak.lock(); if (!ui) return;
        std::string text(value);
        const auto first = text.find_first_not_of(" \t\r\n"), last = text.find_last_not_of(" \t\r\n");
        if (first == std::string::npos) return; text = text.substr(first, last - first + 1);
        const auto& global = (*ui)->global<GalleryDemoModels>(); const auto model = global.get_tags();
        std::vector<slint::SharedString> tags;
        for (std::size_t index = 0; index < model->row_count(); ++index) if (const auto row = model->row_data(index)) tags.push_back(*row);
        if (tags.size() < 20 && std::none_of(tags.begin(), tags.end(), [&](const auto& tag) { return std::string_view(tag) == text; })) tags.emplace_back(text.c_str());
        global.set_tags(std::make_shared<slint::VectorModel<slint::SharedString>>(std::move(tags)));
    });
    global.on_remove_tag([weak](int index) {
        const auto ui = weak.lock(); if (!ui) return;
        const auto& global = (*ui)->global<GalleryDemoModels>(); const auto model = global.get_tags();
        std::vector<slint::SharedString> tags;
        for (std::size_t row = 0; row < model->row_count(); ++row) if (static_cast<int>(row) != index) if (const auto value = model->row_data(row)) tags.push_back(*value);
        global.set_tags(std::make_shared<slint::VectorModel<slint::SharedString>>(std::move(tags)));
    });
    global.on_expand_tree([weak, model](slint::SharedString id, bool value) {
        const auto ui = weak.lock(); if (!ui) return;
        for (auto& item : *model) if (item.id == id && item.enabled && item.folder) item.expanded = value;
        forge_gallery_refresh_demo_tree(*ui, *model);
    });
    global.on_select_tree([weak, model](slint::SharedString id, bool selected, bool multi) {
        const auto ui = weak.lock(); if (!ui) return;
        if (std::none_of(model->begin(), model->end(), [&](const auto& item) { return item.id == id && item.enabled; })) return;
        const auto prefix = std::string(id) + "/";
        for (auto& item : *model) {
            if (multi) { if (item.enabled && (item.id == id || std::string_view(item.id).starts_with(prefix))) item.selected = selected; }
            else item.selected = item.enabled && item.id == id && selected;
        }
        if (multi) for (std::size_t index = model->size(); index > 0; --index) {
            auto& item = (*model)[index - 1]; if (!item.folder) continue;
            bool any = false, all = true;
            for (const auto& child : *model) if (child.parent_id == item.id && child.enabled) { any = true; all = all && child.selected; }
            item.selected = any && all;
        }
        forge_gallery_refresh_demo_tree(*ui, *model);
    });
    auto windows = std::make_shared<std::pair<std::optional<slint::ComponentHandle<GalleryDemoWindow>>, std::optional<slint::ComponentHandle<GalleryDemoDialog>>>>();
    global.on_show_window([windows](bool dialog) {
        if (dialog) {
            if (!windows->second) {
                auto window = GalleryDemoDialog::create();
                const slint::ComponentWeakHandle<GalleryDemoDialog> weak(window);
                window->on_ok_clicked([weak]() { if (auto window = weak.lock()) (*window)->hide(); });
                window->on_cancel_clicked([weak]() { if (auto window = weak.lock()) (*window)->hide(); });
                windows->second = window;
            }
            (*windows->second)->show();
        } else {
            if (!windows->first) {
                auto window = GalleryDemoWindow::create();
                const slint::ComponentWeakHandle<GalleryDemoWindow> weak(window);
                window->on_dismissed([weak]() { if (auto window = weak.lock()) (*window)->hide(); });
                windows->first = window;
            }
            (*windows->first)->show();
        }
    });
    global.on_close_windows([windows]() { if (windows->first) (*windows->first)->hide(); if (windows->second) (*windows->second)->hide(); });
    global.on_export_image([weak](slint::Image image) { const auto ui = weak.lock(); return ui ? (*ui)->invoke_export_capture(image) : slint::SharedString{}; });
}
