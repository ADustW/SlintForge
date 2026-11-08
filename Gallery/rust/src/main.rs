// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#![cfg_attr(target_os = "windows", windows_subsystem = "windows")]

slint::include_modules!();

mod api_docs_capture;
mod capture;
mod capture_scene;
mod component_qa;
mod demos;
#[path = "../../../examples/flow-rust/src/card_export.rs"]
mod flow_card_export;
mod grids_demo;
mod interaction_qa;
mod navigation;
mod plot_demo;
mod settings;

use navigation::{GalleryArea, GalleryEntry};
use settings::GallerySettings;
use slint::{Model, ModelRc, VecModel};
use slintforge::{
    apply_window_corners, IconCatalog, NavItem, NavItemKind, NavTree, RouteParam, Router,
    StaggeredInput, StaggeredLayout,
};
use std::cell::RefCell;
use std::rc::Rc;
use std::sync::atomic::{AtomicBool, AtomicU32, Ordering};
use std::sync::Arc;
use std::time::Duration;

struct GalleryState {
    icons: IconCatalog,
    icon_query: String,
    icon_page: usize,
    router: Router,
    tree: NavTree,
    entries: Vec<GalleryEntry>,
    search_query: String,
    placement: ForgeNavPlacement,
    pane_state: ForgePaneState,
    language: String,
    font_family: String,
    font_scale: f32,
    pinned: bool,
    rounded_corners: bool,
    dark_mode: bool,
    persist: bool,
    settings_writer: settings::SettingsWriter,
}

fn saved_settings(state: &GalleryState) -> GallerySettings {
    GallerySettings {
        language: state.language.clone(),
        font_family: state.font_family.clone(),
        font_scale: state.font_scale,
        pinned: state.pinned,
        rounded_corners: state.rounded_corners,
        dark_mode: state.dark_mode,
        placement: match state.placement {
            ForgeNavPlacement::Left => "left",
            ForgeNavPlacement::Right => "right",
            ForgeNavPlacement::Top => "top",
        }
        .into(),
        pane_state: match state.pane_state {
            ForgePaneState::Expanded => "expanded",
            ForgePaneState::Compact => "compact",
            ForgePaneState::Hidden => "hidden",
        }
        .into(),
    }
}

fn persist_settings(state: &GalleryState) {
    if state.persist {
        state.settings_writer.request(saved_settings(state));
    }
}

fn apply_staggered_layout(ui: &GalleryWindow, columns: i32) {
    let heights = [
        235.0_f32, 125.0, 115.0, 240.0, 175.0, 160.0, 210.0, 135.0, 190.0,
    ];
    let colors = [
        (0x37, 0x62, 0xa8),
        (0x87, 0x56, 0xaa),
        (0x24, 0x7b, 0x6f),
        (0xa0, 0x53, 0x32),
        (0x2c, 0x79, 0x94),
        (0xa2, 0x4d, 0x70),
        (0x43, 0x68, 0xa3),
        (0x69, 0x70, 0x43),
        (0x84, 0x54, 0x9c),
    ];
    let inputs: Vec<_> = heights
        .iter()
        .enumerate()
        .map(|(index, height)| StaggeredInput {
            id: format!("tile-{}", index + 1),
            height: *height,
        })
        .collect();
    let layout = StaggeredLayout::arrange(&inputs, columns.max(1) as usize, 12.0, 12.0);
    let items = layout
        .placements
        .iter()
        .enumerate()
        .map(|(index, position)| {
            let (red, green, blue) = colors[index];
            ForgeStaggeredItem {
                id: position.id.as_str().into(),
                title: format!("{:02}", index + 1).into(),
                description: format!("{} px", heights[index] as i32).into(),
                tile_height: heights[index],
                accent: slint::Color::from_rgb_u8(red, green, blue),
                column: position.column as i32,
                top: position.top,
            }
        })
        .collect::<Vec<_>>();
    ui.set_staggered_items(ModelRc::new(VecModel::from(items)));
    ui.set_staggered_content_height(layout.content_height);
}

fn ui_kind(kind: NavItemKind) -> ForgeNavItemKind {
    match kind {
        NavItemKind::Page => ForgeNavItemKind::Page,
        NavItemKind::Group => ForgeNavItemKind::Group,
        NavItemKind::Header => ForgeNavItemKind::Header,
        NavItemKind::Separator => ForgeNavItemKind::Separator,
    }
}

fn ui_row(
    item: &NavItem,
    title: &str,
    depth: i32,
    expanded: bool,
    has_children: bool,
) -> ForgeNavigationItem {
    ForgeNavigationItem {
        id: item.id.as_str().into(),
        parent_id: item.parent_id.as_deref().unwrap_or("").into(),
        title: title.into(),
        icon: item.icon.as_str().into(),
        kind: ui_kind(item.kind),
        depth,
        expanded,
        has_children,
        child_count: 0,
        enabled: item.enabled,
    }
}

fn active_top_id(entries: &[GalleryEntry], route: &str) -> String {
    let mut current = route;
    loop {
        let Some(entry) = entries.iter().find(|entry| entry.item.id == current) else {
            return current.to_owned();
        };
        match entry.item.parent_id.as_deref() {
            Some(parent) if !parent.is_empty() => current = parent,
            _ => return current.to_owned(),
        }
    }
}

fn expand_route_ancestors(state: &mut GalleryState, id: &str) {
    let mut parent_id = state
        .entries
        .iter()
        .find(|entry| entry.item.id == id)
        .and_then(|entry| entry.item.parent_id.clone());
    while let Some(parent) = parent_id {
        state
            .tree
            .set_expanded(&parent, true)
            .expect("导航目录已校验");
        parent_id = state
            .entries
            .iter()
            .find(|entry| entry.item.id == parent)
            .and_then(|entry| entry.item.parent_id.clone());
    }
}

// 仅替换实际变化的模型，避免搜索、主题和导航操作重建无关重复器并丢失行状态。
fn update_model<T: Clone + PartialEq + 'static>(
    current: ModelRc<T>,
    rows: Vec<T>,
    set: impl FnOnce(ModelRc<T>),
) {
    if current.row_count() == rows.len()
        && rows
            .iter()
            .enumerate()
            .all(|(index, row)| current.row_data(index).as_ref() == Some(row))
    {
        return;
    }
    set(ModelRc::new(VecModel::from(rows)));
}

fn apply_state(ui: &GalleryWindow, state: &GalleryState) {
    let (icons, icon_total) = state
        .icons
        .search(&state.icon_query, state.icon_page * 60, 60);
    update_model(
        ui.get_icon_items(),
        icons
            .into_iter()
            .map(|icon| {
                let codepoint = icon.codepoint_hex();
                let glyph = icon.glyph();
                ForgeIconEntry {
                    name: icon.name.into(),
                    codepoint: codepoint.into(),
                    glyph: glyph.into(),
                }
            })
            .collect::<Vec<_>>(),
        |rows| ui.set_icon_items(rows),
    );
    ui.set_icon_query(state.icon_query.as_str().into());
    ui.set_icon_total(icon_total as i32);
    ui.set_icon_page(state.icon_page as i32);
    ui.set_icon_page_count(icon_total.div_ceil(60) as i32);
    let query = state.search_query.trim().to_lowercase();
    let items: Vec<_> = if query.is_empty() {
        state
            .tree
            .visible_items()
            .iter()
            .filter(|row| {
                state
                    .entries
                    .iter()
                    .any(|entry| entry.item.id == row.item.id && entry.area == GalleryArea::Main)
            })
            .map(|row| {
                let entry = state
                    .entries
                    .iter()
                    .find(|entry| entry.item.id == row.item.id)
                    .expect("导航目录已校验");
                ui_row(
                    &row.item,
                    navigation::title(entry, &state.language),
                    row.depth as i32,
                    row.expanded,
                    row.has_children,
                )
            })
            .collect()
    } else {
        state
            .entries
            .iter()
            .filter(|entry| {
                entry.area == GalleryArea::Main
                    && entry.item.kind == NavItemKind::Page
                    && (entry.item.title.to_lowercase().contains(&query)
                        || entry.zh_title.contains(&query)
                        || entry.item.id.to_lowercase().contains(&query))
            })
            .map(|entry| {
                ui_row(
                    &entry.item,
                    navigation::title(entry, &state.language),
                    0,
                    false,
                    false,
                )
            })
            .collect()
    };
    let footer: Vec<_> = state
        .entries
        .iter()
        .filter(|entry| entry.area == GalleryArea::Footer)
        .map(|entry| {
            ui_row(
                &entry.item,
                navigation::title(entry, &state.language),
                0,
                false,
                false,
            )
        })
        .collect();
    let all_items: Vec<_> = state
        .entries
        .iter()
        .filter(|entry| {
            entry.area == GalleryArea::Main
                && matches!(entry.item.kind, NavItemKind::Page | NavItemKind::Group)
        })
        .map(|entry| {
            let child_count = state
                .entries
                .iter()
                .filter(|child| {
                    child.area == GalleryArea::Main
                        && child.item.parent_id.as_deref() == Some(entry.item.id.as_str())
                })
                .count() as i32;
            let mut row = ui_row(
                &entry.item,
                navigation::title(entry, &state.language),
                0,
                false,
                child_count > 0,
            );
            row.child_count = child_count;
            row
        })
        .collect();
    update_model(ui.get_navigation_items(), items, |rows| {
        ui.set_navigation_items(rows)
    });
    update_model(ui.get_all_navigation_items(), all_items, |rows| {
        ui.set_all_navigation_items(rows)
    });
    update_model(ui.get_footer_items(), footer, |rows| {
        ui.set_footer_items(rows)
    });
    ui.set_active_route_id(state.router.current().id.as_str().into());
    ui.set_active_top_id(active_top_id(&state.entries, &state.router.current().id).into());
    ui.set_search_query(state.search_query.as_str().into());
    ui.set_can_go_back(state.router.can_go_back());
    ui.set_placement(state.placement);
    ui.set_pane_state(state.pane_state);
    ui.set_language(state.language.as_str().into());
    ui.set_font_family_name(state.font_family.as_str().into());
    ui.set_font_scale(state.font_scale);
    ui.set_window_pinned(state.pinned);
    ui.set_window_rounded(state.rounded_corners);
    ui.set_theme_dark(state.dark_mode);
    let pending: Vec<_> = state
        .entries
        .iter()
        .filter(|entry| {
            entry.item.kind == NavItemKind::Page && navigation::is_deferred(&entry.item.id)
        })
        .map(|entry| GalleryPendingItem {
            id: entry.item.id.as_str().into(),
            title: navigation::title(entry, &state.language).into(),
            category: navigation::category(&state.entries, &entry.item.id, &state.language).into(),
        })
        .collect();
    update_model(ui.get_pending_items(), pending, |rows| {
        ui.set_pending_items(rows)
    });
    let summary = state
        .router
        .current()
        .params
        .iter()
        .map(|p| format!("{}={}", p.key, p.value))
        .collect::<Vec<_>>()
        .join(", ");
    ui.set_route_param_summary(if summary.is_empty() {
        (if state.language == "zh" {
            "无"
        } else {
            "None"
        })
        .into()
    } else {
        summary.into()
    });
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let interaction_smoke = std::env::args().any(|arg| arg == "--interaction-smoke");
    let api_docs_capture = std::env::args().any(|arg| arg == "--api-docs-capture");
    if api_docs_capture && std::env::var_os("SLINT_SCALE_FACTOR").is_none() {
        std::env::set_var("SLINT_SCALE_FACTOR", "1");
    }
    slintforge::configure_rendering(Default::default())?;
    let ui = GalleryWindow::new()?;
    slintforge::bind_flow_services!(
        ui,
        ForgeFlowServices,
        ForgeFlowPointData,
        ForgeFlowPortData,
        ForgeFlowNodeData,
        ForgeFlowEdgeData,
        ForgeFlowViewport,
        ForgeFlowRect,
        ForgeFlowAction,
        ForgeFlowState
    );
    ui.global::<ForgeFlowServices>()
        .on_export_card(|node| flow_card_export::content(node.as_str()).into());
    let smoke = std::env::args().any(|arg| arg == "--smoke");
    let capture_smoke = std::env::args().any(|arg| arg == "--capture-smoke");
    let capture_preview = std::env::args().any(|arg| arg == "--capture-preview");
    let components_smoke = std::env::args().any(|arg| arg == "--components-smoke");
    let saved = if smoke
        || capture_smoke
        || capture_preview
        || components_smoke
        || api_docs_capture
        || interaction_smoke
    {
        GallerySettings::default()
    } else {
        settings::load()
    };
    slint::select_bundled_translation(if saved.language == "en" { "en" } else { "" })?;
    let entries = navigation::entries();
    let tree = NavTree::new(entries.iter().map(|entry| entry.item.clone()).collect())?;
    let routes = entries
        .iter()
        .filter(|entry| entry.item.kind == NavItemKind::Page)
        .map(|entry| entry.item.id.clone());
    let state = Rc::new(RefCell::new(GalleryState {
        icons: IconCatalog::new(),
        icon_query: String::new(),
        icon_page: 0,
        router: Router::new(routes, "home")?,
        tree,
        entries,
        search_query: String::new(),
        placement: match saved.placement.as_str() {
            "right" => ForgeNavPlacement::Right,
            "top" => ForgeNavPlacement::Top,
            _ => ForgeNavPlacement::Left,
        },
        pane_state: match saved.pane_state.as_str() {
            "compact" => ForgePaneState::Compact,
            "hidden" => ForgePaneState::Hidden,
            _ => ForgePaneState::Expanded,
        },
        language: saved.language,
        font_family: saved.font_family,
        font_scale: saved.font_scale,
        pinned: saved.pinned,
        rounded_corners: saved.rounded_corners,
        dark_mode: saved.dark_mode,
        settings_writer: settings::SettingsWriter::new(),
        persist: !smoke
            && !capture_smoke
            && !capture_preview
            && !components_smoke
            && !api_docs_capture
            && !interaction_smoke,
    }));
    if std::env::args().any(|arg| arg == "--preview-top") {
        state.borrow_mut().placement = ForgeNavPlacement::Top;
    } else if std::env::args().any(|arg| arg == "--preview-right") {
        state.borrow_mut().placement = ForgeNavPlacement::Right;
    }
    if let Some(route) =
        std::env::args().find_map(|arg| arg.strip_prefix("--preview-route=").map(str::to_owned))
    {
        state.borrow_mut().router.go(&route, Vec::new())?;
        expand_route_ancestors(&mut state.borrow_mut(), &route);
    }
    if capture_smoke {
        state.borrow_mut().router.go("surfaces-page", Vec::new())?;
        expand_route_ancestors(&mut state.borrow_mut(), "surfaces-page");
    }
    apply_state(&ui, &state.borrow());
    capture::connect(&ui);
    demos::connect(&ui);
    {
        let weak = ui.as_weak();
        ui.on_staggered_layout_requested(move |columns| {
            if let Some(ui) = weak.upgrade() {
                apply_staggered_layout(&ui, columns);
            }
        });
    }
    apply_staggered_layout(&ui, ui.get_qa_staggered_columns());

    {
        let state = state.clone();
        let weak = ui.as_weak();
        ui.on_navigate_requested(move |id, params| {
            let params = params
                .iter()
                .map(|p| RouteParam::new(p.key.as_str(), p.value.as_str()))
                .collect();
            let mut state = state.borrow_mut();
            match state.router.go(id.as_str(), params) {
                Ok(_) => {
                    expand_route_ancestors(&mut state, id.as_str());
                    state.search_query.clear();
                }
                Err(error) => eprintln!("{error}"),
            }
            if let Some(ui) = weak.upgrade() {
                apply_state(&ui, &state);
            }
        });
    }
    {
        let state = state.clone();
        let weak = ui.as_weak();
        ui.on_group_toggle_requested(move |id| {
            let mut state = state.borrow_mut();
            if let Err(error) = state.tree.toggle(id.as_str()) {
                eprintln!("{error}");
            }
            if let Some(ui) = weak.upgrade() {
                apply_state(&ui, &state);
            }
        });
    }
    {
        let state = state.clone();
        let weak = ui.as_weak();
        ui.on_search_requested(move |query| {
            let mut state = state.borrow_mut();
            state.search_query = query.to_string();
            if let Some(ui) = weak.upgrade() {
                apply_state(&ui, &state);
            }
        });
    }
    {
        let state = state.clone();
        let weak = ui.as_weak();
        ui.on_icon_search_requested(move |query| {
            let mut state = state.borrow_mut();
            state.icon_query = query.to_string();
            state.icon_page = 0;
            if let Some(ui) = weak.upgrade() {
                apply_state(&ui, &state);
            }
        });
    }
    {
        let state = state.clone();
        let weak = ui.as_weak();
        ui.on_icon_page_requested(move |page| {
            let mut state = state.borrow_mut();
            let count = state.icons.search(&state.icon_query, 0, 0).1;
            if page < 0 || page as usize >= count.div_ceil(60) {
                return;
            }
            state.icon_page = page as usize;
            if let Some(ui) = weak.upgrade() {
                apply_state(&ui, &state);
            }
        });
    }
    {
        let state = state.clone();
        let weak = ui.as_weak();
        ui.on_pane_state_requested(move |pane| {
            let mut state = state.borrow_mut();
            if state.pane_state == pane { return; }
            state.pane_state = pane;
            if let Some(ui) = weak.upgrade() {
                // 宽度切换只更新导航状态，不重新搜索图标或生成全部页面模型。
                ui.set_pane_state(pane);
            }
            persist_settings(&state);
        });
    }
    {
        let state = state.clone();
        let weak = ui.as_weak();
        ui.on_placement_requested(move |placement| {
            let mut state = state.borrow_mut();
            state.placement = placement;
            persist_settings(&state);
            if let Some(ui) = weak.upgrade() {
                apply_state(&ui, &state);
            }
        });
    }
    {
        let state = state.clone();
        let weak = ui.as_weak();
        ui.on_back_requested(move || {
            let mut state = state.borrow_mut();
            if let Err(error) = state.router.back() {
                eprintln!("{error}");
            }
            if let Some(ui) = weak.upgrade() {
                apply_state(&ui, &state);
            }
        });
    }
    {
        let state = state.clone();
        let weak = ui.as_weak();
        ui.on_language_requested(move |language| {
            let language = language.as_str();
            if language != "zh" && language != "en" {
                return;
            }
            if let Err(error) =
                slint::select_bundled_translation(if language == "zh" { "" } else { "en" })
            {
                eprintln!("{error}");
                return;
            }
            let mut state = state.borrow_mut();
            state.language = language.to_owned();
            persist_settings(&state);
            if let Some(ui) = weak.upgrade() {
                apply_state(&ui, &state);
            }
        });
    }
    {
        let state = state.clone();
        let weak = ui.as_weak();
        ui.on_font_family_requested(move |family| {
            let family = family.as_str();
            if !matches!(family, "" | "Microsoft YaHei UI" | "Segoe UI" | "Consolas") {
                return;
            }
            let mut state = state.borrow_mut();
            state.font_family = family.to_owned();
            persist_settings(&state);
            if let Some(ui) = weak.upgrade() {
                apply_state(&ui, &state);
            }
        });
    }
    {
        let state = state.clone();
        let weak = ui.as_weak();
        ui.on_font_scale_requested(move |scale| {
            if ![0.9_f32, 1.0, 1.1, 1.25]
                .iter()
                .any(|choice| (scale - choice).abs() < 0.001)
            {
                return;
            }
            let mut state = state.borrow_mut();
            state.font_scale = scale;
            persist_settings(&state);
            if let Some(ui) = weak.upgrade() {
                apply_state(&ui, &state);
            }
        });
    }
    {
        let state = state.clone();
        let weak = ui.as_weak();
        ui.on_pinned_requested(move |pinned| {
            let mut state = state.borrow_mut();
            state.pinned = pinned;
            persist_settings(&state);
            if let Some(ui) = weak.upgrade() {
                apply_state(&ui, &state);
            }
        });
    }
    {
        let state = state.clone();
        let weak = ui.as_weak();
        ui.on_corner_style_requested(move |rounded| {
            let mut state = state.borrow_mut();
            state.rounded_corners = rounded;
            persist_settings(&state);
            if let Some(ui) = weak.upgrade() {
                apply_state(&ui, &state);
                apply_window_corners(ui.window(), rounded);
            }
        });
    }
    {
        let state = state.clone();
        let weak = ui.as_weak();
        ui.on_theme_setting_changed(move |dark| {
            let mut state = state.borrow_mut();
            state.dark_mode = dark;
            persist_settings(&state);
            if let Some(ui) = weak.upgrade() {
                apply_state(&ui, &state);
            }
        });
    }

    // 采样器归宿主所有，页面隐藏不会中断采样；定时器只负责把最新值送到 UI。
    let samples = Arc::new(AtomicU32::new(0));
    let running = Arc::new(AtomicBool::new(true));
    let worker_samples = samples.clone();
    let worker_running = running.clone();
    let worker = std::thread::spawn(move || {
        while worker_running.load(Ordering::Relaxed) {
            worker_samples.fetch_add(1, Ordering::Relaxed);
            std::thread::sleep(Duration::from_millis(100));
        }
    });
    let weak = ui.as_weak();
    let smoke_samples = samples.clone();
    let timer = slint::Timer::default();
    timer.start(
        slint::TimerMode::Repeated,
        Duration::from_millis(100),
        move || {
            if let Some(ui) = weak.upgrade() {
                ui.set_sample_counter(samples.load(Ordering::Relaxed) as i32);
            }
        },
    );
    if smoke {
        grids_demo::verify(&ui);
        assert_eq!(ui.get_icon_total(), 9767);
        assert_eq!(ui.get_icon_items().row_count(), 60);
        ui.invoke_icon_search_requested("arrow".into());
        assert!(ui.get_icon_total() > 60);
        ui.invoke_icon_page_requested(1);
        assert_eq!(ui.get_icon_page(), 1);
        ui.invoke_icon_search_requested("home".into());
        assert_eq!(ui.get_icon_page(), 0);
        assert!(ui.get_icon_total() > 0);
        assert_eq!(ui.get_active_route_id().as_str(), "home");
        assert_eq!(ui.get_pending_items().row_count(), 0);
        let implemented_pages = [
            "layouts-split",
            "layouts-staggered",
            "layouts-flow",
            "layouts-adaptive-grid",
            "layouts-dock",
            "layouts-page-stack",
            "layouts-form-row",
            "surfaces-page",
            "surfaces-round-image",
            "surfaces-icon-label",
            "surfaces-expander",
            "surfaces-watermark",
            "surfaces-marquee",
        ];
        for id in implemented_pages {
            assert!(!(0..ui.get_pending_items().row_count())
                .filter_map(|index| ui.get_pending_items().row_data(index))
                .any(|row| row.id == id));
        }
        assert_eq!(ui.get_qa_navigation_glyph().as_str(), "\u{f560}");
        ui.invoke_qa_toggle_navigation();
        assert_eq!(ui.get_pane_state(), ForgePaneState::Compact);
        assert_eq!(ui.get_qa_navigation_glyph().as_str(), "\u{f560}");
        ui.invoke_qa_toggle_navigation();
        assert_eq!(ui.get_pane_state(), ForgePaneState::Expanded);
        assert_eq!(ui.get_qa_navigation_glyph().as_str(), "\u{f560}");
        assert_eq!(ui.get_qa_localized_settings_label().as_str(), "设置");
        ui.invoke_staggered_layout_requested(3);
        assert_eq!(ui.get_staggered_items().row_count(), 9);
        assert_eq!(
            ui.get_staggered_items().row_data(0).unwrap().id.as_str(),
            "tile-1"
        );
        assert_eq!(ui.get_staggered_items().row_data(3).unwrap().column, 2);
        ui.invoke_qa_set_stack_draft("保持输入".into());
        ui.invoke_qa_switch_stack("second".into());
        ui.invoke_qa_switch_stack("first".into());
        assert_eq!(ui.get_qa_stack_draft().as_str(), "保持输入");
        ui.invoke_open_settings_requested();
        assert_eq!(ui.get_active_route_id().as_str(), "settings");
        ui.invoke_navigate_requested(
            "theme-typography".into(),
            ModelRc::new(VecModel::from(vec![])),
        );
        assert_eq!(ui.get_active_route_id().as_str(), "theme-typography");
        ui.invoke_navigate_requested("theme-icons".into(), ModelRc::new(VecModel::from(vec![])));
        assert_eq!(ui.get_active_route_id().as_str(), "theme-icons");
        ui.invoke_back_requested();
        ui.invoke_back_requested();
        assert_eq!(ui.get_active_route_id().as_str(), "settings");
        ui.invoke_font_family_requested("Segoe UI".into());
        assert_eq!(ui.get_font_family_name().as_str(), "Segoe UI");
        ui.invoke_font_scale_requested(1.1);
        assert!((ui.get_font_scale() - 1.1).abs() < 0.001);
        ui.invoke_pinned_requested(true);
        assert!(ui.get_window_pinned());
        ui.invoke_corner_style_requested(false);
        assert!(!ui.get_window_rounded());
        ui.invoke_corner_style_requested(true);
        assert!(ui.get_window_rounded());
        // 检查完整树、已展开的侧栏和页脚，覆盖嵌套子项与搜索结果的动态切换。
        let verify_navigation_language = |language: &str| {
            let current = state.borrow();
            for model in [
                ui.get_navigation_items(),
                ui.get_all_navigation_items(),
                ui.get_footer_items(),
            ] {
                for index in 0..model.row_count() {
                    let row = model.row_data(index).unwrap();
                    if row.kind == ForgeNavItemKind::Separator {
                        continue;
                    }
                    let entry = current
                        .entries
                        .iter()
                        .find(|entry| entry.item.id.as_str() == row.id.as_str())
                        .unwrap();
                    assert_eq!(
                        row.title.as_str(),
                        navigation::title(entry, language),
                        "导航名称: {}",
                        row.id
                    );
                    if language == "zh" {
                        assert!(
                            row.title
                                .chars()
                                .any(|c| ('\u{4e00}'..='\u{9fff}').contains(&c)),
                            "导航仍为英文: {}",
                            row.id
                        );
                    }
                }
            }
        };
        let navigation_structure = || {
            (0..ui.get_navigation_items().row_count())
                .filter_map(|index| ui.get_navigation_items().row_data(index))
                .map(|row| (row.id, row.parent_id, row.depth, row.expanded))
                .collect::<Vec<_>>()
        };
        let route_before_language_switch = ui.get_active_route_id();
        let structure_before_language_switch = navigation_structure();
        verify_navigation_language("zh");
        ui.invoke_language_requested("en".into());
        verify_navigation_language("en");
        assert_eq!(ui.get_active_route_id(), route_before_language_switch);
        assert_eq!(navigation_structure(), structure_before_language_switch);
        assert_eq!(ui.get_language().as_str(), "en");
        assert_eq!(ui.get_qa_localized_settings_label().as_str(), "Settings");
        assert!((0..ui.get_footer_items().row_count())
            .filter_map(|index| ui.get_footer_items().row_data(index))
            .any(|row| row.id == "settings" && row.title == "Settings"));
        ui.invoke_language_requested("zh".into());
        verify_navigation_language("zh");
        assert_eq!(navigation_structure(), structure_before_language_switch);
        ui.invoke_search_requested("日期选择器".into());
        assert_eq!(ui.get_navigation_items().row_count(), 1);
        assert_eq!(
            ui.get_navigation_items().row_data(0).unwrap().id,
            "form-date-picker"
        );
        ui.invoke_language_requested("en".into());
        verify_navigation_language("en");
        assert_eq!(ui.get_search_query(), "日期选择器");
        assert_eq!(
            ui.get_navigation_items().row_data(0).unwrap().title,
            "DatePicker"
        );
        ui.invoke_language_requested("zh".into());
        verify_navigation_language("zh");
        ui.invoke_search_requested("".into());
        assert_eq!(ui.get_active_route_id(), route_before_language_switch);
        assert_eq!(navigation_structure(), structure_before_language_switch);
        println!("NAV_LOCALIZATION_OK host=rust languages=2 nested_items=localized search=localized route=retained expansion=retained");
        assert_eq!(ui.get_qa_localized_settings_label().as_str(), "设置");
        ui.invoke_pinned_requested(false);
        assert!(!ui.get_window_pinned());
        ui.invoke_qa_set_lifecycle_draft("retained-input".into());
        let params = ModelRc::new(VecModel::from(vec![ForgeRouteParam {
            key: "source".into(),
            value: "smoke".into(),
        }]));
        ui.invoke_navigate_requested("lifecycle".into(), params);
        assert_eq!(ui.get_route_param_summary().as_str(), "source=smoke");
        ui.invoke_navigate_requested("tabbar".into(), ModelRc::new(VecModel::from(vec![])));
        assert!((0..ui.get_navigation_items().row_count())
            .filter_map(|index| ui.get_navigation_items().row_data(index))
            .any(|row| row.id == "tabbar"));
        ui.invoke_placement_requested(ForgeNavPlacement::Right);
        assert_eq!(ui.get_placement(), ForgeNavPlacement::Right);
        assert_eq!(ui.get_qa_navigation_glyph().as_str(), "\u{f560}");
        ui.invoke_qa_toggle_navigation();
        assert_eq!(ui.get_pane_state(), ForgePaneState::Compact);
        assert_eq!(ui.get_qa_navigation_glyph().as_str(), "\u{f560}");
        ui.invoke_qa_toggle_navigation();
        assert_eq!(ui.get_pane_state(), ForgePaneState::Expanded);
        ui.invoke_placement_requested(ForgeNavPlacement::Top);
        assert_eq!(ui.get_placement(), ForgeNavPlacement::Top);
        assert_eq!(ui.get_qa_navigation_glyph().as_str(), "\u{f560}");
        ui.invoke_qa_toggle_navigation();
        assert_eq!(ui.get_pane_state(), ForgePaneState::Compact);
        assert_eq!(ui.get_qa_navigation_glyph().as_str(), "\u{f560}");
        ui.invoke_qa_toggle_navigation();
        assert_eq!(ui.get_pane_state(), ForgePaneState::Expanded);
        assert_eq!(ui.get_active_top_id().as_str(), "navigation-group");
        assert!(ui.get_all_navigation_items().row_count() > 90);
        assert!((0..ui.get_all_navigation_items().row_count())
            .filter_map(|index| ui.get_all_navigation_items().row_data(index))
            .all(|row| row.icon.as_str().len() > 1));
        assert!((0..ui.get_all_navigation_items().row_count())
            .filter_map(|index| ui.get_all_navigation_items().row_data(index))
            .any(|row| row.id == "inputs-button"
                && row.parent_id == "inputs"
                && row.child_count > 0));
        ui.invoke_pane_state_requested(ForgePaneState::Compact);
        assert_eq!(ui.get_pane_state(), ForgePaneState::Compact);
        ui.invoke_pane_state_requested(ForgePaneState::Hidden);
        assert_eq!(ui.get_pane_state(), ForgePaneState::Hidden);
        ui.invoke_pane_state_requested(ForgePaneState::Expanded);
        assert_eq!(ui.get_pane_state(), ForgePaneState::Expanded);
        let row_count = ui.get_navigation_items().row_count();
        ui.invoke_group_toggle_requested("inputs".into());
        assert!(ui.get_navigation_items().row_count() > row_count);
        ui.invoke_group_toggle_requested("inputs".into());
        assert_eq!(ui.get_navigation_items().row_count(), row_count);
        ui.invoke_search_requested("DataGrid".into());
        assert!(ui.get_navigation_items().row_count() >= 2);
        ui.invoke_search_requested("".into());
        assert_eq!(ui.get_navigation_items().row_count(), row_count);
        std::thread::sleep(Duration::from_millis(350));
        let count = smoke_samples.load(Ordering::Relaxed);
        ui.set_sample_counter(count as i32);
        assert!(count >= 2);
        ui.invoke_back_requested();
        assert_eq!(ui.get_active_route_id().as_str(), "lifecycle");
        assert_eq!(ui.get_lifecycle_draft().as_str(), "retained-input");
        ui.invoke_navigate_requested("missing".into(), ModelRc::new(VecModel::from(vec![])));
        assert_eq!(ui.get_active_route_id().as_str(), "lifecycle");
        for id in implemented_pages {
            ui.invoke_navigate_requested(id.into(), ModelRc::new(VecModel::from(vec![])));
            assert_eq!(ui.get_active_route_id().as_str(), id);
        }
        for _ in implemented_pages {
            ui.invoke_back_requested();
        }
        assert_eq!(ui.get_active_route_id().as_str(), "lifecycle");
        running.store(false, Ordering::Relaxed);
        worker.join().expect("采样线程正常结束");
        println!("SMOKE_OK route=lifecycle pending=none search=matched draft=retained-input placements=3 pane_states=3 samples=positive");
        return Ok(());
    }
    let run_smoke = std::env::args().any(|arg| arg == "--run-smoke");
    let quit_timer = slint::Timer::default();
    if run_smoke {
        quit_timer.start(
            slint::TimerMode::SingleShot,
            Duration::from_millis(850),
            || {
                slint::quit_event_loop().expect("事件循环正常退出");
            },
        );
    }
    let corner_timer = slint::Timer::default();
    let corner_weak = ui.as_weak();
    let corner_state = state.clone();
    corner_timer.start(
        slint::TimerMode::SingleShot,
        Duration::from_millis(50),
        move || {
            if let Some(ui) = corner_weak.upgrade() {
                apply_window_corners(ui.window(), corner_state.borrow().rounded_corners);
            }
        },
    );
    let _capture_timer =
        capture::verification_timer(&ui, capture_smoke || capture_preview, capture_preview);
    let _component_timer = component_qa::verification_timer(&ui, components_smoke);
    let _api_docs_timer = api_docs_capture::verification_timer(&ui, api_docs_capture);
    let _interaction_timer = interaction_qa::verification_timer(&ui, interaction_smoke);
    let result = ui.run();
    running.store(false, Ordering::Relaxed);
    worker.join().expect("采样线程正常结束");
    result?;
    if run_smoke {
        assert!(ui.get_sample_counter() >= 2);
        println!("RUN_OK samples=positive");
    }
    Ok(())
}
