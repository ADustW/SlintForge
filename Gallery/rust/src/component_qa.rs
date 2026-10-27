//! 实际渲染器逐页截图验证；仅在显式命令行检查模式启用。
use crate::GalleryWindow;
use slint::{ComponentHandle, LogicalSize, Model, ModelRc, VecModel};
use std::cell::Cell;
use std::rc::Rc;
use std::time::Duration;

pub fn verification_timer(ui: &GalleryWindow, enabled: bool) -> slint::Timer {
    let timer = slint::Timer::default();
    if !enabled {
        return timer;
    }
    assert_eq!(ui.get_pending_items().row_count(), 0);
    let rows = include_str!("../../data/component-status.txt")
        .lines()
        .filter(|line| !line.starts_with('#'))
        .map(|line| line.split('|').collect::<Vec<_>>())
        .collect::<Vec<_>>();
    let mut frames = Vec::new();
    for row in &rows {
        frames.push((row[0].to_owned(), false, false, false, -1));
    }
    for row in &rows {
        frames.push((row[0].to_owned(), false, true, false, -1));
    }
    for row in &rows {
        frames.push((row[0].to_owned(), false, false, true, -1));
    }
    for route in [
        "form-date-picker",
        "form-time-picker",
        "form-auto-suggest",
        "inputs-progress-button",
        "inputs-dropdown-button",
        "form-multi-select-combo",
        "form-multi-tree-combo",
        "popups-content-dialog",
        "popups-drawer",
        "other-tour",
        "chart-bar",
        "chart-line",
        "chart-pie",
        "chart-polar",
        "chart-bubble",
        "chart-scatter",
        "chart-radar",
        "chart-professional",
        "data-grid",
        "data-tree-grid",
    ] {
        frames.push((route.into(), true, false, false, -1));
    }
    // 本地化弹层覆盖中英文、放大字号、窄窗口和 12/24 小时时钟/数字模式。
    for route in [
        "form-date-picker",
        "form-calendar-picker",
        "form-time-picker",
    ] {
        if route == "form-calendar-picker" {
            frames.push((route.into(), true, false, false, -1));
        }
        frames.push((route.into(), true, true, false, -1));
        frames.push((route.into(), true, false, true, -1));
    }
    for mode in [-2, -3, -4] {
        for (light, narrow) in [(false, false), (true, false), (false, true)] {
            frames.push(("form-time-picker".into(), true, light, narrow, mode));
        }
    }
    // 两种日历弹层分别验收，避免仅检查紧凑按钮而遗漏时间栏布局。
    for (light, narrow) in [(false, false), (true, false), (false, true)] {
        frames.push(("form-calendar-picker".into(), true, light, narrow, -2));
    }
    for route in [
        "form-color-picker",
        "form-shortcut-picker",
        "inputs-delay-button",
    ] {
        for (light, narrow) in [(false, false), (true, false), (false, true)] {
            frames.push((route.into(), true, light, narrow, -1));
        }
    }
    // 新增示例与原有 308 个状态分别截图；典型复杂页面再检查字号和窄窗口。
    let responsive = [
        "inputs-standard-button",
        "form-textbox",
        "form-color-picker",
        "form-auto-suggest",
        "feedback-result",
        "feedback-skeleton",
        "chart-bar",
        "chart-line",
        "chart-professional",
        "data-grid",
        "data-tree-grid",
        "popups-content-dialog",
    ];
    for line in include_str!("../../data/showcase-routes.inc").lines() {
        let Some((route, count)) = line.split_once('|') else {
            continue;
        };
        for tab in 0..count.parse::<i32>().expect("示例标签数量") {
            frames.push((
                route.into(),
                route == "popups-content-dialog",
                false,
                false,
                tab,
            ));
            if responsive.contains(&route) {
                frames.push((
                    route.into(),
                    route == "popups-content-dialog",
                    true,
                    false,
                    tab,
                ));
                frames.push((
                    route.into(),
                    route == "popups-content-dialog",
                    false,
                    true,
                    tab,
                ));
            }
        }
    }
    if let Ok(filter) = std::env::var("SLINTFORGE_COMPONENT_QA_FILTER") {
        frames.retain(|frame| frame.0.starts_with(&filter));
    }
    if let Ok(exclude) = std::env::var("SLINTFORGE_COMPONENT_QA_EXCLUDE") {
        frames.retain(|frame| !frame.0.starts_with(&exclude));
    }
    assert!(!frames.is_empty());
    let directory = std::env::var_os("SLINTFORGE_COMPONENT_QA_DIR")
        .map(std::path::PathBuf::from)
        .unwrap_or_else(|| "build/component-qa/rust".into());
    std::fs::create_dir_all(&directory).expect("创建逐页截图目录");
    let phase = Rc::new(Cell::new(0usize));
    let retries = Rc::new(Cell::new(0usize));
    let inject_minimize = std::env::var_os("SLINTFORGE_COMPONENT_QA_MINIMIZE_ONCE").is_some();
    let minimized_once = Rc::new(Cell::new(false));
    let weak = ui.as_weak();
    timer.start(
        slint::TimerMode::Repeated,
        Duration::from_millis(180),
        move || {
            let ui = weak.upgrade().expect("检查期间窗口存在");
            let count = phase.get();
            let index = count / 3;
            let stage = count % 3;
            let (route, open, light, narrow, showcase) = &frames[index];
            match stage {
                0 => {
                    ui.window().set_minimized(false);
                    ui.invoke_qa_showcase("".into(), -1);
                    ui.invoke_qa_prepare_component(false);
                    ui.invoke_theme_setting_changed(!*light);
                    ui.set_theme_dark(!*light);
                    ui.invoke_language_requested(if *light { "en" } else { "zh" }.into());
                    ui.invoke_font_scale_requested(if *light { 1.25 } else { 1.0 });
                    ui.window().set_size(if *narrow {
                        LogicalSize::new(960., 720.)
                    } else if *light {
                        LogicalSize::new(1100., 800.)
                    } else {
                        LogicalSize::new(1280., 900.)
                    });
                    ui.invoke_navigate_requested(
                        route.as_str().into(),
                        ModelRc::new(VecModel::from(Vec::new())),
                    );
                    assert_eq!(ui.get_active_route_id().as_str(), route);
                }
                1 => {
                    if route.starts_with("flow-") {
                        let before = ui.invoke_qa_flow_state(route.as_str().into());
                        assert_eq!(
                            before.node_count,
                            if route == "flow-mindmap" { 9 } else { 6 }
                        );
                        if *light || *narrow {
                            assert_eq!(before.nodes.row_data(0).unwrap().title, "页面返回后保留");
                        } else {
                            ui.invoke_qa_flow_edit(
                                route.as_str().into(),
                                crate::ForgeFlowAction {
                                    op: "title".into(),
                                    id: "node-1".into(),
                                    text: "页面返回后保留".into(),
                                    ..Default::default()
                                },
                            );
                        }
                        ui.invoke_search_requested(if *light {
                            "Custom Cards".into()
                        } else {
                            "自定义卡片".into()
                        });
                        let items = ui.get_navigation_items();
                        assert!(
                            (0..items.row_count())
                                .any(|i| items.row_data(i).unwrap().id == "flow-custom"),
                            "节点页面导航搜索未命中"
                        );
                        ui.invoke_search_requested("".into());
                    }
                    if route == "form-time-picker" {
                        ui.invoke_qa_time_picker(
                            *showcase == -3,
                            *showcase != -3 && *showcase != -4,
                        );
                    }
                    if route == "form-calendar-picker" {
                        ui.invoke_qa_calendar_picker(*showcase == -2);
                    }
                    if *showcase >= 0 {
                        ui.invoke_qa_showcase(route.as_str().into(), *showcase);
                    }
                    ui.invoke_qa_prepare_component(*open);
                    if inject_minimize && !minimized_once.replace(true) {
                        ui.window().set_minimized(true);
                    }
                }
                _ => {
                    let size = ui.window().size();
                    if ui.window().is_minimized() || size.width < 960 || size.height < 600 {
                        assert!(retries.get() < 2, "验收窗口恢复后仍未完成布局: {route}");
                        retries.set(retries.get() + 1);
                        // 原生最小化可能未同步到 Slint 属性，切换意图以确保恢复请求生效。
                        ui.window().set_minimized(true);
                        ui.window().set_minimized(false);
                        println!("QA_WINDOW_RESTORED host=rust route={route}");
                        phase.set(index * 3);
                        return;
                    }
                    retries.set(0);
                    if *open && (route == "data-grid" || route == "data-tree-grid") {
                        assert!(ui.get_qa_grid_editing(), "表格操作截图必须包含编辑器");
                    }
                    if route == "chart-professional" && (*showcase < 0 || *showcase == 2) {
                        assert_eq!(
                            ui.get_qa_plot_scene().status,
                            0,
                            "{}",
                            ui.get_qa_plot_scene().error
                        );
                    }
                    if route == "chart-professional" {
                        assert!(ui.get_qa_showcase_ready(), "图表总览场景必须全部完成渲染");
                    }
                    let pixels = slintforge::capture_window(ui.window()).expect("实际窗口截图成功");
                    assert!(
                        pixels.width() >= 960 && pixels.height() >= 600,
                        "整窗截图尺寸不正确: {route}"
                    );
                    let file = directory.join(format!(
                        "{index:03}-{route}-{}{}{}.png",
                        if *narrow {
                            "narrow-zh"
                        } else if *light {
                            "light-en"
                        } else {
                            "dark-zh"
                        },
                        if *open { "-open" } else { "" },
                        if *showcase >= 0 {
                            format!("-showcase{showcase}")
                        } else {
                            String::new()
                        }
                    ));
                    slintforge::save_capture_png(&pixels, &file).expect("保存实际页面截图");
                    if index + 1 == frames.len() {
                        ui.invoke_qa_prepare_component(false);
                        println!("COMPONENTS_RENDER_OK deferred=0 frames={}", frames.len());
                        slint::quit_event_loop().unwrap();
                    }
                }
            }
            phase.set(count + 1);
        },
    );
    timer
}
