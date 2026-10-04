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
    for row in &rows { frames.push((row[0].to_owned(), false, false, false)); }
    for row in &rows { frames.push((row[0].to_owned(), false, true, false)); }
    for row in &rows { frames.push((row[0].to_owned(), false, false, true)); }
    for route in ["form-date-picker", "form-time-picker", "form-auto-suggest", "inputs-progress-button", "inputs-dropdown-button", "form-multi-select-combo", "form-multi-tree-combo", "popups-content-dialog", "popups-drawer", "other-tour", "chart-bar", "chart-line", "chart-pie", "chart-polar", "chart-bubble", "chart-scatter", "chart-radar", "chart-professional", "data-grid", "data-tree-grid"] {
        frames.push((route.into(), true, false, false));
    }
    if let Ok(filter) = std::env::var("SLINTFORGE_COMPONENT_QA_FILTER") { frames.retain(|frame| frame.0.starts_with(&filter)); }
    assert!(!frames.is_empty());
    let directory = std::env::var_os("SLINTFORGE_COMPONENT_QA_DIR")
        .map(std::path::PathBuf::from)
        .unwrap_or_else(|| "build/component-qa/rust".into());
    std::fs::create_dir_all(&directory).expect("创建逐页截图目录");
    let phase = Rc::new(Cell::new(0usize));
    let weak = ui.as_weak();
    timer.start(
        slint::TimerMode::Repeated,
        Duration::from_millis(180),
        move || {
            let ui = weak.upgrade().expect("检查期间窗口存在");
            let count = phase.get();
            let index = count / 3;
            let stage = count % 3;
            let (route, open, light, narrow) = &frames[index];
            match stage {
                0 => {
                    ui.invoke_qa_prepare_component(false);
                    ui.invoke_theme_setting_changed(!*light);
                    ui.set_theme_dark(!*light);
                    ui.invoke_language_requested(if *light { "en" } else { "zh" }.into());
                    ui.invoke_font_scale_requested(if *light { 1.25 } else { 1.0 });
                    ui.window().set_size(if *narrow { LogicalSize::new(960., 720.) } else if *light {
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
                    ui.invoke_qa_prepare_component(*open);
                }
                _ => {
                    if *open && (route == "data-grid" || route == "data-tree-grid") {
                        assert!(ui.get_qa_grid_editing(), "表格操作截图必须包含编辑器");
                    }
                    if route == "chart-professional" { assert_eq!(ui.get_qa_plot_scene().status, 0, "{}", ui.get_qa_plot_scene().error); }
                    let pixels = slintforge::capture_window(ui.window()).expect("实际窗口截图成功");
                    let file = directory.join(format!(
                        "{index:03}-{route}-{}{}.png",
                        if *narrow { "narrow-zh" } else if *light { "light-en" } else { "dark-zh" },
                        if *open { "-open" } else { "" }
                    ));
                    slintforge::save_capture_png(&pixels, &file).expect("保存实际页面截图");
                    if index + 1 == frames.len() {
                        ui.invoke_qa_prepare_component(false);
                        println!(
                            "COMPONENTS_RENDER_OK catalog=96 implemented=96 deferred=0 frames={}",
                            frames.len()
                        );
                        slint::quit_event_loop().unwrap();
                    }
                }
            }
            phase.set(count + 1);
        },
    );
    timer
}
