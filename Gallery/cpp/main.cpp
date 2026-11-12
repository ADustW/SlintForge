// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#include "gallery.h"
#include "ForgeFlowBindings.hpp"
#include "../../examples/flow-cpp/FlowCardExport.hpp"
#include <cstdlib>
#include "ForgeSdk.hpp"
#include "ForgeStaggeredLayout.hpp"
#include "ForgeIconCatalog.hpp"
#include "ForgeWindowCorners.hpp"
#include "ForgeRendering.hpp"
#include "ForgeGalleryNavigation.hpp"
#include "ForgeGallerySettings.hpp"
#include "ForgeGalleryCapture.hpp"
#include "ForgeGalleryDemos.hpp"
#include "ForgeGalleryComponentQa.hpp"
#include "ForgeGalleryGrids.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <cmath>
#include <chrono>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <vector>
#include "ForgeGalleryTitlebarQa.hpp"

struct ForgeGalleryState {
    ForgeIconCatalog icons;
    std::string icon_query;
    std::size_t icon_page = 0;
    ForgeRouter router;
    ForgeNavTree tree;
    std::vector<ForgeGalleryEntry> entries;
    std::string search_query;
    ForgeNavPlacement placement = ForgeNavPlacement::Left;
    ForgePaneState pane_state = ForgePaneState::Expanded;
    std::string language = "zh";
    std::string font_family;
    float font_scale = 1.0f;
    bool pinned = false;
    bool rounded_corners = true;
    bool dark_mode = true;
    bool persist = true;
    std::shared_ptr<ForgeGallerySettingsWriter> settings_writer = std::make_shared<ForgeGallerySettingsWriter>();
};

static ForgeGallerySettings forge_gallery_saved_settings(const ForgeGalleryState& state) {
    ForgeGallerySettings settings;
    settings.language = state.language;
    settings.font_family = state.font_family;
    settings.font_scale = state.font_scale;
    settings.pinned = state.pinned;
    settings.rounded_corners = state.rounded_corners;
    settings.dark_mode = state.dark_mode;
    settings.placement = state.placement == ForgeNavPlacement::Right ? "right" : state.placement == ForgeNavPlacement::Top ? "top" : "left";
    settings.pane_state = state.pane_state == ForgePaneState::Compact ? "compact" : state.pane_state == ForgePaneState::Hidden ? "hidden" : "expanded";
    return settings;
}

static void forge_gallery_persist(const ForgeGalleryState& state) {
    if (state.persist) state.settings_writer->request(forge_gallery_saved_settings(state));
}

static ForgeNavItemKind forge_gallery_ui_kind(ForgeTreeItemKind kind) {
    switch (kind) {
    case ForgeTreeItemKind::Page: return ForgeNavItemKind::Page;
    case ForgeTreeItemKind::Group: return ForgeNavItemKind::Group;
    case ForgeTreeItemKind::Header: return ForgeNavItemKind::Header;
    case ForgeTreeItemKind::Separator: return ForgeNavItemKind::Separator;
    }
    return ForgeNavItemKind::Page;
}

static ForgeNavigationItem forge_gallery_ui_row(const ForgeNavItem& item, const std::string& title, int depth, bool expanded, bool has_children) {
    ForgeNavigationItem row;
    row.id = item.id.c_str();
    row.parent_id = item.parent_id.c_str();
    row.title = title.c_str();
    row.icon = item.icon.c_str();
    row.kind = forge_gallery_ui_kind(item.kind);
    row.depth = depth;
    row.expanded = expanded;
    row.has_children = has_children;
    row.child_count = 0;
    row.enabled = item.enabled;
    return row;
}

static std::string forge_gallery_active_top_id(const std::vector<ForgeGalleryEntry>& entries, std::string_view route) {
    std::string current(route);
    while (true) {
        const auto found = std::find_if(entries.begin(), entries.end(), [&](const auto& entry) {
            return entry.item.id == current;
        });
        if (found == entries.end() || found->item.parent_id.empty()) return current;
        current = found->item.parent_id;
    }
}

static std::string forge_gallery_lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
    return value;
}

static void forge_gallery_expand_route_ancestors(ForgeGalleryState& state, const std::string& id) {
    auto found = std::find_if(state.entries.begin(), state.entries.end(), [&](const auto& entry) {
        return entry.item.id == id;
    });
    std::string parent_id = found == state.entries.end() ? "" : found->item.parent_id;
    while (!parent_id.empty()) {
        const auto result = state.tree.set_expanded(parent_id, true);
        if (!result.ok()) throw std::runtime_error(result.message);
        found = std::find_if(state.entries.begin(), state.entries.end(), [&](const auto& entry) {
            return entry.item.id == parent_id;
        });
        parent_id = found == state.entries.end() ? "" : found->item.parent_id;
    }
}

// 模型内容未变化时保留原实例，避免一次导航重建所有列表与图标行。
template<typename T, typename Setter>
static void forge_gallery_update_model(const std::shared_ptr<slint::Model<T>>& current,
                                      std::vector<T> rows, Setter set) {
    bool equal = current && current->row_count() == rows.size();
    for (std::size_t index = 0; equal && index < rows.size(); ++index)
        equal = current->row_data(index) == std::optional<T>(rows[index]);
    if (!equal) set(std::make_shared<slint::VectorModel<T>>(std::move(rows)));
}

static void forge_gallery_apply_state(const slint::ComponentHandle<GalleryWindow>& ui, const ForgeGalleryState& state) {
    const auto icon_total = state.icons.count(state.icon_query);
    const auto icon_page = state.icons.search(state.icon_query, state.icon_page * 60, 60);
    std::vector<ForgeIconEntry> icons;
    icons.reserve(icon_page.size());
    for (const auto& item : icon_page) {
        ForgeIconEntry entry;
        entry.name = item.name.c_str();
        entry.codepoint = item.codepoint_hex().c_str();
        entry.glyph = item.glyph_utf8().c_str();
        icons.push_back(std::move(entry));
    }
    forge_gallery_update_model(ui->get_icon_items(), std::move(icons), [&](auto rows) { ui->set_icon_items(rows); });
    ui->set_icon_query(state.icon_query.c_str());
    ui->set_icon_total(static_cast<int>(icon_total));
    ui->set_icon_page(static_cast<int>(state.icon_page));
    ui->set_icon_page_count(static_cast<int>((icon_total + 59) / 60));
    std::vector<ForgeNavigationItem> rows;
    const auto query = forge_gallery_lower(state.search_query);
    if (query.empty()) {
        for (const auto& visible : state.tree.visible_items()) {
            const auto found = std::find_if(state.entries.begin(), state.entries.end(), [&](const auto& entry) {
                return entry.item.id == visible.item.id && entry.area == ForgeGalleryArea::Main;
            });
            if (found != state.entries.end())
                rows.push_back(forge_gallery_ui_row(visible.item, forge_gallery_title(*found, state.language), static_cast<int>(visible.depth), visible.expanded, visible.has_children));
        }
    } else {
        for (const auto& entry : state.entries) {
            if (entry.area == ForgeGalleryArea::Main && entry.item.kind == ForgeTreeItemKind::Page &&
                (forge_gallery_lower(entry.item.title).find(query) != std::string::npos ||
                 entry.zh_title.find(query) != std::string::npos ||
                 forge_gallery_lower(entry.item.id).find(query) != std::string::npos)) {
                rows.push_back(forge_gallery_ui_row(entry.item, forge_gallery_title(entry, state.language), 0, false, false));
            }
        }
    }
    std::vector<ForgeNavigationItem> footer;
    for (const auto& entry : state.entries) {
        if (entry.area == ForgeGalleryArea::Footer)
            footer.push_back(forge_gallery_ui_row(entry.item, forge_gallery_title(entry, state.language), 0, false, false));
    }
    std::vector<ForgeNavigationItem> all_rows;
    for (const auto& entry : state.entries) {
        if (entry.area != ForgeGalleryArea::Main ||
            (entry.item.kind != ForgeTreeItemKind::Page && entry.item.kind != ForgeTreeItemKind::Group)) continue;
        const auto child_count = static_cast<int>(std::count_if(state.entries.begin(), state.entries.end(), [&](const auto& child) {
            return child.area == ForgeGalleryArea::Main && child.item.parent_id == entry.item.id;
        }));
        auto row = forge_gallery_ui_row(entry.item, forge_gallery_title(entry, state.language), 0, false, child_count > 0);
        row.child_count = child_count;
        all_rows.push_back(std::move(row));
    }
    forge_gallery_update_model(ui->get_navigation_items(), std::move(rows), [&](auto model) { ui->set_navigation_items(model); });
    forge_gallery_update_model(ui->get_all_navigation_items(), std::move(all_rows), [&](auto model) { ui->set_all_navigation_items(model); });
    forge_gallery_update_model(ui->get_footer_items(), std::move(footer), [&](auto rows) { ui->set_footer_items(rows); });
    ui->set_active_route_id(state.router.current().id.c_str());
    ui->set_active_top_id(forge_gallery_active_top_id(state.entries, state.router.current().id).c_str());
    ui->set_search_query(state.search_query.c_str());
    ui->set_can_go_back(state.router.can_go_back());
    ui->set_placement(state.placement);
    ui->set_pane_state(state.pane_state);
    ui->set_language(state.language.c_str());
    ui->set_font_family_name(state.font_family.c_str());
    ui->set_font_scale(state.font_scale);
    ui->set_window_pinned(state.pinned);
    ui->set_window_rounded(state.rounded_corners);
    ui->set_theme_dark(state.dark_mode);
    std::vector<GalleryPendingItem> pending;
    for (const auto& entry : state.entries) {
        if (entry.item.kind != ForgeTreeItemKind::Page || !forge_gallery_is_deferred(entry.item.id)) continue;
        GalleryPendingItem item;
        item.id = entry.item.id.c_str();
        item.title = forge_gallery_title(entry, state.language).c_str();
        item.category = forge_gallery_category(state.entries, entry.item.id, state.language).c_str();
        pending.push_back(std::move(item));
    }
    forge_gallery_update_model(ui->get_pending_items(), std::move(pending), [&](auto rows) { ui->set_pending_items(rows); });
    std::string summary;
    for (const auto& param : state.router.current().params) {
        if (!summary.empty()) summary += ", ";
        summary += param.key + "=" + param.value;
    }
    ui->set_route_param_summary(summary.empty() ? (state.language == "zh" ? "无" : "None") : summary.c_str());
}

static void forge_gallery_apply_staggered_layout(const slint::ComponentHandle<GalleryWindow>& ui, int columns) {
    constexpr std::array<float, 9> heights{235.0f, 125.0f, 115.0f, 240.0f, 175.0f, 160.0f, 210.0f, 135.0f, 190.0f};
    constexpr std::array<std::array<unsigned char, 3>, 9> colors{{
        {0x37, 0x62, 0xa8}, {0x87, 0x56, 0xaa}, {0x24, 0x7b, 0x6f},
        {0xa0, 0x53, 0x32}, {0x2c, 0x79, 0x94}, {0xa2, 0x4d, 0x70},
        {0x43, 0x68, 0xa3}, {0x69, 0x70, 0x43}, {0x84, 0x54, 0x9c},
    }};
    std::vector<ForgeStaggeredInput> inputs;
    inputs.reserve(heights.size());
    for (std::size_t index = 0; index < heights.size(); ++index)
        inputs.push_back({"tile-" + std::to_string(index + 1), heights[index]});
    const auto layout = ForgeStaggeredLayout::arrange(inputs, static_cast<std::size_t>(std::max(1, columns)), 12.0f, 12.0f);
    std::vector<ForgeStaggeredItem> items;
    items.reserve(heights.size());
    for (std::size_t index = 0; index < heights.size(); ++index) {
        ForgeStaggeredItem item;
        item.id = layout.placements[index].id.c_str();
        const auto title = (index < 9 ? "0" : "") + std::to_string(index + 1);
        item.title = title.c_str();
        item.description = (std::to_string(static_cast<int>(heights[index])) + " px").c_str();
        item.tile_height = heights[index];
        item.accent = slint::Color::from_rgb_uint8(colors[index][0], colors[index][1], colors[index][2]);
        item.column = static_cast<int>(layout.placements[index].column);
        item.top = layout.placements[index].top;
        items.push_back(std::move(item));
    }
    ui->set_staggered_items(std::make_shared<slint::VectorModel<ForgeStaggeredItem>>(std::move(items)));
    ui->set_staggered_content_height(layout.content_height);
}

int main(int argc, char** argv) {
    const auto rendering = forge_configure_rendering();
    auto ui = GalleryWindow::create();
    forge_bind_flow_services<ForgeFlowPointData, ForgeFlowPortData, ForgeFlowNodeData, ForgeFlowEdgeData, ForgeFlowViewport, ForgeFlowRect, ForgeFlowAction, ForgeFlowState, ForgeFlowServices>(ui);
    ui->global<ForgeFlowServices>().on_export_card([](slint::SharedString node){return slint::SharedString(forge_flow_demo_export_card(std::string(node)).c_str());});
    const bool smoke = argc > 1 && std::string_view(argv[1]) == "--smoke";
    const bool capture_smoke = std::any_of(argv + 1, argv + argc, [](const char* arg) { return std::string_view(arg) == "--capture-smoke"; });
    const bool capture_preview = std::any_of(argv + 1, argv + argc, [](const char* arg) { return std::string_view(arg) == "--capture-preview"; });
    const bool components_smoke = std::any_of(argv + 1, argv + argc, [](const char* arg) { return std::string_view(arg) == "--components-smoke"; });
    const bool titlebar_smoke = std::any_of(argv + 1, argv + argc, [](const char* arg) { return std::string_view(arg) == "--titlebar-smoke"; });
    const auto saved = smoke || capture_smoke || capture_preview || components_smoke || titlebar_smoke ? ForgeGallerySettings{} : forge_gallery_load_settings();
    if (!slint::select_bundled_translation(saved.language == "en" ? "en" : ""))
        throw std::runtime_error("translation unavailable");
    ForgeGalleryState state;
    state.language = saved.language;
    state.font_family = saved.font_family;
    state.font_scale = saved.font_scale;
    state.pinned = saved.pinned;
    state.rounded_corners = saved.rounded_corners;
    state.dark_mode = saved.dark_mode;
    state.placement = saved.placement == "right" ? ForgeNavPlacement::Right : saved.placement == "top" ? ForgeNavPlacement::Top : ForgeNavPlacement::Left;
    state.pane_state = saved.pane_state == "compact" ? ForgePaneState::Compact : saved.pane_state == "hidden" ? ForgePaneState::Hidden : ForgePaneState::Expanded;
    state.persist = !smoke && !capture_smoke && !capture_preview && !components_smoke && !titlebar_smoke;
    state.entries = forge_gallery_entries();
    std::vector<std::string> routes;
    std::vector<ForgeNavItem> tree_items;
    for (const auto& entry : state.entries) {
        tree_items.push_back(entry.item);
        if (entry.item.kind != ForgeTreeItemKind::Page) continue;
        routes.push_back(entry.item.id);
    }
    auto status = ForgeRouter::create(routes, "home", state.router);
    if (!status.ok()) throw std::runtime_error(status.message);
    status = ForgeNavTree::create(std::move(tree_items), state.tree);
    if (!status.ok()) throw std::runtime_error(status.message);
    if (argc > 1 && std::string_view(argv[1]) == "--preview-top") state.placement = ForgeNavPlacement::Top;
    if (argc > 1 && std::string_view(argv[1]) == "--preview-right") state.placement = ForgeNavPlacement::Right;
    for (int arg = 1; arg < argc; ++arg) {
        const std::string_view value(argv[arg]);
        constexpr std::string_view prefix = "--preview-route=";
        if (value.starts_with(prefix)) {
            const auto route = std::string(value.substr(prefix.size()));
            status = state.router.go(route, {});
            if (!status.ok()) throw std::runtime_error(status.message);
            forge_gallery_expand_route_ancestors(state, route);
        }
    }
    if (capture_smoke) {
        status = state.router.go("surfaces-page", {});
        if (!status.ok()) throw std::runtime_error(status.message);
        forge_gallery_expand_route_ancestors(state, "surfaces-page");
    }
    forge_gallery_apply_state(ui, state);
    forge_gallery_connect_capture(ui);
    forge_gallery_connect_demos(ui);
    ui->on_staggered_layout_requested([&](int columns) {
        forge_gallery_apply_staggered_layout(ui, columns);
    });
    forge_gallery_apply_staggered_layout(ui, ui->get_qa_staggered_columns());

    ui->on_navigate_requested([&](slint::SharedString id, std::shared_ptr<slint::Model<ForgeRouteParam>> params) {
        std::vector<ForgeRouteArgument> args;
        for (std::size_t i = 0; i < params->row_count(); ++i) {
            if (auto param = params->row_data(i)) {
                args.push_back({std::string(param->key.data()), std::string(param->value.data())});
            }
        }
        const auto result = state.router.go(std::string(id.data()), std::move(args));
        if (!result.ok()) std::cerr << result.message << '\n';
        else {
            forge_gallery_expand_route_ancestors(state, std::string(id.data()));
            state.search_query.clear();
        }
        forge_gallery_apply_state(ui, state);
    });
    ui->on_group_toggle_requested([&](slint::SharedString id) {
        const auto result = state.tree.toggle(std::string(id.data()));
        if (!result.ok()) std::cerr << result.message << '\n';
        forge_gallery_apply_state(ui, state);
    });
    ui->on_search_requested([&](slint::SharedString query) {
        state.search_query = std::string(query.data());
        forge_gallery_apply_state(ui, state);
    });
    ui->on_icon_search_requested([&](slint::SharedString query) {
        state.icon_query = std::string(query.data());
        state.icon_page = 0;
        forge_gallery_apply_state(ui, state);
    });
    ui->on_icon_page_requested([&](int page) {
        const auto count = state.icons.count(state.icon_query);
        if (page < 0 || static_cast<std::size_t>(page) >= (count + 59) / 60) return;
        state.icon_page = static_cast<std::size_t>(page);
        forge_gallery_apply_state(ui, state);
    });
    ui->on_pane_state_requested([&](ForgePaneState pane) {
        if (state.pane_state == pane) return;
        state.pane_state = pane;
        // 侧栏切换不重建图标、导航、页脚和页面模型。
        ui->set_pane_state(pane);
        forge_gallery_persist(state);
    });
    ui->on_placement_requested([&](ForgeNavPlacement placement) {
        state.placement = placement;
        forge_gallery_persist(state);
        forge_gallery_apply_state(ui, state);
    });
    ui->on_back_requested([&]() {
        const auto result = state.router.back();
        if (!result.ok()) std::cerr << result.message << '\n';
        forge_gallery_apply_state(ui, state);
    });
    ui->on_language_requested([&](slint::SharedString language) {
        const std::string value(language.data());
        if (value != "zh" && value != "en") return;
        if (!slint::select_bundled_translation(value == "zh" ? "" : "en")) return;
        state.language = value;
        forge_gallery_persist(state);
        forge_gallery_apply_state(ui, state);
    });
    ui->on_font_family_requested([&](slint::SharedString family) {
        const std::string value(family.data());
        if (value != "" && value != "Microsoft YaHei UI" && value != "Segoe UI" && value != "Consolas") return;
        state.font_family = value;
        forge_gallery_persist(state);
        forge_gallery_apply_state(ui, state);
    });
    ui->on_font_scale_requested([&](float scale) {
        if (std::fabs(scale - 0.9f) > 0.001f && std::fabs(scale - 1.0f) > 0.001f &&
            std::fabs(scale - 1.1f) > 0.001f && std::fabs(scale - 1.25f) > 0.001f) return;
        state.font_scale = scale;
        forge_gallery_persist(state);
        forge_gallery_apply_state(ui, state);
    });
    ui->on_pinned_requested([&](bool pinned) {
        state.pinned = pinned;
        forge_gallery_persist(state);
        forge_gallery_apply_state(ui, state);
    });
    ui->on_corner_style_requested([&](bool rounded) {
        state.rounded_corners = rounded;
        forge_gallery_persist(state);
        forge_gallery_apply_state(ui, state);
        forge_apply_window_corners(ui->window(), rounded);
    });
    ui->on_theme_setting_changed([&](bool dark) {
        state.dark_mode = dark;
        forge_gallery_persist(state);
        forge_gallery_apply_state(ui, state);
    });

    // 后台线程持续采样；页面切换仅改变显示状态。
    std::atomic<int> samples{0};
    std::jthread worker([&](std::stop_token stop) {
        while (!stop.stop_requested()) {
            samples.fetch_add(1, std::memory_order_relaxed);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    slint::Timer timer;
    timer.start(slint::TimerMode::Repeated, std::chrono::milliseconds(100), [&]() {
        ui->set_sample_counter(samples.load(std::memory_order_relaxed));
    });
    if (argc > 1 && std::string_view(argv[1]) == "--smoke") {
        forge_gallery_verify_grids(ui);
        if (ui->get_icon_total() != 9767 || ui->get_icon_items()->row_count() != 60)
            throw std::runtime_error("full icon catalog");
        ui->invoke_icon_search_requested("arrow");
        if (ui->get_icon_total() <= 60) throw std::runtime_error("icon search");
        ui->invoke_icon_page_requested(1);
        if (ui->get_icon_page() != 1) throw std::runtime_error("icon paging");
        ui->invoke_icon_search_requested("home");
        if (ui->get_icon_page() != 0 || ui->get_icon_total() <= 0)
            throw std::runtime_error("icon search reset");
        if (std::string_view(ui->get_active_route_id()) != "home") throw std::runtime_error("initial route");
        if (ui->get_pending_items()->row_count() != 0) throw std::runtime_error("pending pages");
        constexpr std::array<std::string_view, 13> implemented_pages{
            "layouts-split", "layouts-staggered", "layouts-flow",
            "layouts-adaptive-grid", "layouts-dock", "layouts-page-stack", "layouts-form-row",
            "surfaces-page", "surfaces-round-image", "surfaces-icon-label",
            "surfaces-expander", "surfaces-watermark", "surfaces-marquee"
        };
        for (const auto id : implemented_pages) {
            for (std::size_t index = 0; index < ui->get_pending_items()->row_count(); ++index) {
                const auto row = ui->get_pending_items()->row_data(index);
                if (row && std::string_view(row->id) == id) throw std::runtime_error("implemented page is pending");
            }
        }
        const auto expect_navigation_glyph = [&](std::string_view name) {
            const auto* glyph = state.icons.find(name);
            if (!glyph || std::string_view(ui->get_qa_navigation_glyph()) != glyph->glyph_utf8())
                throw std::runtime_error("titlebar navigation glyph");
        };
        expect_navigation_glyph("navigation_20_regular");
        ui->invoke_qa_toggle_navigation();
        if (ui->get_pane_state() != ForgePaneState::Compact) throw std::runtime_error("titlebar compact");
        expect_navigation_glyph("navigation_20_regular");
        ui->invoke_qa_toggle_navigation();
        if (ui->get_pane_state() != ForgePaneState::Expanded) throw std::runtime_error("titlebar restore");
        expect_navigation_glyph("navigation_20_regular");
        if (std::string_view(ui->get_qa_localized_settings_label()) != "设置") throw std::runtime_error("default Chinese");
        ui->invoke_staggered_layout_requested(3);
        if (ui->get_staggered_items()->row_count() != 9) throw std::runtime_error("staggered count");
        const auto first_tile = ui->get_staggered_items()->row_data(0);
        const auto fourth_tile = ui->get_staggered_items()->row_data(3);
        if (!first_tile || std::string_view(first_tile->id) != "tile-1" || !fourth_tile || fourth_tile->column != 2)
            throw std::runtime_error("staggered placement");
        ui->invoke_qa_set_stack_draft("retained-input");
        ui->invoke_qa_switch_stack("second");
        ui->invoke_qa_switch_stack("first");
        if (std::string_view(ui->get_qa_stack_draft()) != "retained-input")
            throw std::runtime_error("stack draft");
        ui->invoke_open_settings_requested();
        if (std::string_view(ui->get_active_route_id()) != "settings") throw std::runtime_error("settings route");
        ui->invoke_navigate_requested("theme-typography", std::make_shared<slint::VectorModel<ForgeRouteParam>>());
        if (std::string_view(ui->get_active_route_id()) != "theme-typography") throw std::runtime_error("typography route");
        ui->invoke_navigate_requested("theme-icons", std::make_shared<slint::VectorModel<ForgeRouteParam>>());
        if (std::string_view(ui->get_active_route_id()) != "theme-icons") throw std::runtime_error("icons route");
        ui->invoke_back_requested();
        ui->invoke_back_requested();
        if (std::string_view(ui->get_active_route_id()) != "settings") throw std::runtime_error("font page back");
        ui->invoke_font_family_requested("Segoe UI");
        if (std::string_view(ui->get_font_family_name()) != "Segoe UI") throw std::runtime_error("font family");
        ui->invoke_font_scale_requested(1.1f);
        if (std::fabs(ui->get_font_scale() - 1.1f) > 0.001f) throw std::runtime_error("font scale");
        ui->invoke_pinned_requested(true);
        if (!ui->get_window_pinned()) throw std::runtime_error("always on top");
        ui->invoke_corner_style_requested(false);
        if (ui->get_window_rounded()) throw std::runtime_error("square corners");
        ui->invoke_corner_style_requested(true);
        if (!ui->get_window_rounded()) throw std::runtime_error("rounded corners");
        // 检查完整树、已展开的侧栏和页脚，覆盖嵌套子项与搜索结果的动态切换。
        const auto verify_navigation_language = [&](std::string_view language) {
            for (const auto& model : {ui->get_navigation_items(), ui->get_all_navigation_items(), ui->get_footer_items()}) {
                for (std::size_t index = 0; index < model->row_count(); ++index) {
                    const auto row = model->row_data(index);
                    if (!row) throw std::runtime_error("导航条目不存在");
                    if (row->kind == ForgeNavItemKind::Separator) continue;
                    const auto entry = std::find_if(state.entries.begin(), state.entries.end(), [&](const auto& item) { return item.item.id == std::string_view(row->id); });
                    if (entry == state.entries.end() || std::string_view(row->title) != forge_gallery_title(*entry, language))
                        throw std::runtime_error("导航语言未同步: " + std::string(row->id));
                    if (language == "zh" && (entry->zh_title.empty() || entry->zh_title == entry->item.title))
                        throw std::runtime_error("导航仍为英文: " + entry->item.id);
                }
            }
        };
        const auto navigation_structure = [&]() {
            std::string value;
            for (std::size_t index = 0; index < ui->get_navigation_items()->row_count(); ++index) {
                const auto row = ui->get_navigation_items()->row_data(index);
                if (!row) throw std::runtime_error("导航条目不存在");
                value += std::string(row->id) + "|" + std::string(row->parent_id) + "|" + std::to_string(row->depth) + "|" + (row->expanded ? "1\n" : "0\n");
            }
            return value;
        };
        const auto route_before_language_switch = ui->get_active_route_id();
        const auto structure_before_language_switch = navigation_structure();
        verify_navigation_language("zh");
        ui->invoke_language_requested("en");
        verify_navigation_language("en");
        if (std::string_view(ui->get_active_route_id()) != std::string_view(route_before_language_switch) || navigation_structure() != structure_before_language_switch)
            throw std::runtime_error("语言切换改变了当前路由或展开状态");
        if (std::string_view(ui->get_language()) != "en" ||
            std::string_view(ui->get_qa_localized_settings_label()) != "Settings") throw std::runtime_error("English translation");
        bool translated_settings = false;
        for (std::size_t index = 0; index < ui->get_footer_items()->row_count(); ++index) {
            const auto row = ui->get_footer_items()->row_data(index);
            if (row && std::string_view(row->id) == "settings" && std::string_view(row->title) == "Settings") translated_settings = true;
        }
        if (!translated_settings) throw std::runtime_error("English navigation");
        ui->invoke_language_requested("zh");
        verify_navigation_language("zh");
        if (navigation_structure() != structure_before_language_switch) throw std::runtime_error("中文切换改变了导航结构");
        ui->invoke_search_requested("日期选择器");
        const auto date_search = ui->get_navigation_items()->row_data(0);
        if (ui->get_navigation_items()->row_count() != 1 || !date_search || std::string_view(date_search->id) != "form-date-picker")
            throw std::runtime_error("中文导航搜索");
        ui->invoke_language_requested("en");
        verify_navigation_language("en");
        if (std::string_view(ui->get_search_query()) != "日期选择器" || std::string_view(ui->get_navigation_items()->row_data(0)->title) != "DatePicker")
            throw std::runtime_error("英文搜索结果未同步");
        ui->invoke_language_requested("zh");
        verify_navigation_language("zh");
        ui->invoke_search_requested("");
        if (std::string_view(ui->get_active_route_id()) != std::string_view(route_before_language_switch) || navigation_structure() != structure_before_language_switch)
            throw std::runtime_error("语言切换未保留导航状态");
        std::cout << "NAV_LOCALIZATION_OK host=cpp languages=2 nested_items=localized search=localized route=retained expansion=retained\n";
        if (std::string_view(ui->get_qa_localized_settings_label()) != "设置") throw std::runtime_error("Chinese translation");
        ui->invoke_pinned_requested(false);
        if (ui->get_window_pinned()) throw std::runtime_error("unpin");
        ui->invoke_qa_set_lifecycle_draft("retained-input");
        ForgeRouteParam param;
        param.key = "source";
        param.value = "smoke";
        ui->invoke_navigate_requested("lifecycle", std::make_shared<slint::VectorModel<ForgeRouteParam>>(
            std::vector<ForgeRouteParam>{param}));
        if (std::string_view(ui->get_route_param_summary()) != "source=smoke") throw std::runtime_error("params");
        ui->invoke_navigate_requested("tabbar", std::make_shared<slint::VectorModel<ForgeRouteParam>>());
        bool tabbar_visible = false;
        for (std::size_t index = 0; index < ui->get_navigation_items()->row_count(); ++index) {
            const auto row = ui->get_navigation_items()->row_data(index);
            if (row && std::string_view(row->id) == "tabbar") tabbar_visible = true;
        }
        if (!tabbar_visible) throw std::runtime_error("active route in navigation");
        ui->invoke_placement_requested(ForgeNavPlacement::Right);
        if (ui->get_placement() != ForgeNavPlacement::Right) throw std::runtime_error("right");
        expect_navigation_glyph("navigation_20_regular");
        ui->invoke_qa_toggle_navigation();
        if (ui->get_pane_state() != ForgePaneState::Compact) throw std::runtime_error("right compact");
        expect_navigation_glyph("navigation_20_regular");
        ui->invoke_qa_toggle_navigation();
        if (ui->get_pane_state() != ForgePaneState::Expanded) throw std::runtime_error("right restore");
        ui->invoke_placement_requested(ForgeNavPlacement::Top);
        if (ui->get_placement() != ForgeNavPlacement::Top) throw std::runtime_error("top");
        expect_navigation_glyph("navigation_20_regular");
        ui->invoke_qa_toggle_navigation();
        if (ui->get_pane_state() != ForgePaneState::Compact) throw std::runtime_error("top compact");
        expect_navigation_glyph("navigation_20_regular");
        ui->invoke_qa_toggle_navigation();
        if (ui->get_pane_state() != ForgePaneState::Expanded) throw std::runtime_error("top restore");
        if (std::string_view(ui->get_active_top_id()) != "navigation-group") throw std::runtime_error("top active group");
        if (ui->get_all_navigation_items()->row_count() <= 90) throw std::runtime_error("top tree");
        for (std::size_t index = 0; index < ui->get_all_navigation_items()->row_count(); ++index) {
            const auto row = ui->get_all_navigation_items()->row_data(index);
            if (!row || std::string_view(row->icon).size() <= 1) throw std::runtime_error("navigation item icon");
        }
        bool nested_top_group = false;
        for (std::size_t index = 0; index < ui->get_all_navigation_items()->row_count(); ++index) {
            const auto row = ui->get_all_navigation_items()->row_data(index);
            if (row && std::string_view(row->id) == "inputs-button" &&
                std::string_view(row->parent_id) == "inputs" && row->child_count > 0) nested_top_group = true;
        }
        if (!nested_top_group) throw std::runtime_error("top nested group");
        ui->invoke_pane_state_requested(ForgePaneState::Compact);
        if (ui->get_pane_state() != ForgePaneState::Compact) throw std::runtime_error("compact");
        ui->invoke_pane_state_requested(ForgePaneState::Hidden);
        if (ui->get_pane_state() != ForgePaneState::Hidden) throw std::runtime_error("hidden");
        ui->invoke_pane_state_requested(ForgePaneState::Expanded);
        if (ui->get_pane_state() != ForgePaneState::Expanded) throw std::runtime_error("expanded");
        const auto row_count = ui->get_navigation_items()->row_count();
        ui->invoke_group_toggle_requested("inputs");
        if (ui->get_navigation_items()->row_count() <= row_count) throw std::runtime_error("expand");
        ui->invoke_group_toggle_requested("inputs");
        if (ui->get_navigation_items()->row_count() != row_count) throw std::runtime_error("expand");
        ui->invoke_search_requested("DataGrid");
        if (ui->get_navigation_items()->row_count() < 2) throw std::runtime_error("search");
        ui->invoke_search_requested("");
        if (ui->get_navigation_items()->row_count() != row_count) throw std::runtime_error("clear search");
        std::this_thread::sleep_for(std::chrono::milliseconds(350));
        const auto count = samples.load(std::memory_order_relaxed);
        ui->set_sample_counter(count);
        if (count < 2) throw std::runtime_error("samples");
        ui->invoke_back_requested();
        if (std::string_view(ui->get_active_route_id()) != "lifecycle") throw std::runtime_error("back");
        if (std::string_view(ui->get_lifecycle_draft()) != "retained-input") throw std::runtime_error("retained input");
        ui->invoke_navigate_requested("missing", std::make_shared<slint::VectorModel<ForgeRouteParam>>());
        if (std::string_view(ui->get_active_route_id()) != "lifecycle") throw std::runtime_error("invalid route");
        for (const auto id : implemented_pages) {
            ui->invoke_navigate_requested(id.data(), std::make_shared<slint::VectorModel<ForgeRouteParam>>());
            if (std::string_view(ui->get_active_route_id()) != id) throw std::runtime_error("implemented route");
        }
        for (const auto id : implemented_pages) {
            (void)id;
            ui->invoke_back_requested();
        }
        if (std::string_view(ui->get_active_route_id()) != "lifecycle") throw std::runtime_error("implemented route back");
        worker.request_stop();
        worker.join();
        std::cout << "SMOKE_OK route=lifecycle pending=none search=matched draft=retained-input placements=3 pane_states=3 samples=positive\n";
        return 0;
    }
    const bool run_smoke = argc > 1 && std::string_view(argv[1]) == "--run-smoke";
    slint::Timer quit_timer;
    if (run_smoke) {
        quit_timer.start(slint::TimerMode::SingleShot, std::chrono::milliseconds(850), []() {
            slint::quit_event_loop();
        });
    }
    slint::Timer corner_timer;
    corner_timer.start(slint::TimerMode::SingleShot, std::chrono::milliseconds(50), [&]() {
        forge_apply_window_corners(ui->window(), state.rounded_corners);
    });
    slint::Timer capture_timer;
    if (capture_smoke || capture_preview) forge_gallery_start_capture_verification(ui, capture_timer, capture_preview);
    slint::Timer component_timer;
    if (components_smoke) forge_gallery_start_component_qa(ui, component_timer);
    slint::Timer titlebar_timer;
    if (titlebar_smoke) forge_gallery_start_titlebar_qa(ui, titlebar_timer);
    ui->run();
    worker.request_stop();
    worker.join();
    if (run_smoke) {
        if (ui->get_sample_counter() < 2) throw std::runtime_error("event loop samples");
        std::cout << "RUN_OK samples=positive\n";
    }
    return 0;
}
