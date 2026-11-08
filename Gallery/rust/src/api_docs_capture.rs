// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

//! 由 docs/api/components.json 驱动的双语 API 文档截图模式。

use crate::GalleryWindow;
use serde::Deserialize;
use slint::{ComponentHandle, LogicalSize, ModelRc, VecModel};
use std::cell::Cell;
use std::path::PathBuf;
use std::rc::Rc;
use std::time::Duration;

#[derive(Deserialize)]
struct ApiDocsEntry {
    name: String,
    slug: String,
    screenshot: ApiDocsScene,
}

#[derive(Deserialize)]
struct ApiDocsScene {
    route: String,
    open: bool,
    focus: String,
}

pub fn verification_timer(ui: &GalleryWindow, enabled: bool) -> slint::Timer {
    let timer = slint::Timer::default();
    if !enabled {
        return timer;
    }
    let entries: Vec<ApiDocsEntry> =
        serde_json::from_str(include_str!("../../../docs/api/components.json"))
            .expect("API 文档组件清单有效");
    assert_eq!(entries.len(), 79, "API 文档截图必须覆盖 79 个组件");
    let directory = std::env::var_os("SLINTFORGE_API_DOCS_DIR")
        .map(PathBuf::from)
        .unwrap_or_else(|| {
            PathBuf::from(env!("CARGO_MANIFEST_DIR"))
                .join("../../docs/site/public/images/components")
        });
    for locale in ["zh", "en"] {
        std::fs::create_dir_all(directory.join(locale)).expect("创建 API 文档截图目录");
    }
    let phase = Rc::new(Cell::new(0usize));
    let weak = ui.as_weak();
    timer.start(
        slint::TimerMode::Repeated,
        Duration::from_millis(180),
        move || {
            let ui = weak.upgrade().expect("截图期间窗口存在");
            let count = phase.get();
            let frame = count / 3;
            let stage = count % 3;
            let entry_index = frame / 2;
            let locale = if frame % 2 == 0 { "zh" } else { "en" };
            let entry = &entries[entry_index];
            match stage {
                0 => {
                    ui.window().set_minimized(false);
                    ui.window().set_size(LogicalSize::new(1280., 900.));
                    ui.invoke_qa_showcase("".into(), -1);
                    ui.invoke_qa_prepare_component(false);
                    ui.invoke_theme_setting_changed(false);
                    ui.set_theme_dark(false);
                    ui.invoke_language_requested(locale.into());
                    ui.set_api_doc_focus(entry.screenshot.focus.as_str().into());
                    ui.invoke_navigate_requested(
                        entry.screenshot.route.as_str().into(),
                        ModelRc::new(VecModel::from(Vec::new())),
                    );
                    assert_eq!(
                        ui.get_active_route_id().as_str(),
                        entry.screenshot.route,
                        "API 文档截图路由"
                    );
                }
                1 => {
                    ui.invoke_qa_prepare_component(entry.screenshot.open);
                }
                _ => {
                    let pixels = slintforge::capture_window(ui.window())
                        .expect("API 文档实际窗口截图成功");
                    assert_eq!(
                        (pixels.width(), pixels.height()),
                        (1280, 900),
                        "API 文档截图固定为 1280x900；请清除外部缩放覆盖"
                    );
                    let path = directory
                        .join(locale)
                        .join(format!("{}.png", entry.slug));
                    slintforge::save_capture_png(&pixels, &path)
                        .expect("保存 API 文档截图");
                    println!(
                        "API_DOC_CAPTURE locale={locale} component={} route={} file={}",
                        entry.name,
                        entry.screenshot.route,
                        path.display()
                    );
                    if entry_index + 1 == entries.len() && locale == "en" {
                        ui.set_api_doc_focus("".into());
                        ui.invoke_qa_prepare_component(false);
                        println!("API_DOCS_CAPTURE_OK components=79 images=158 size=1280x900");
                        slint::quit_event_loop().expect("结束 API 文档截图事件循环");
                    }
                }
            }
            phase.set(count + 1);
        },
    );
    timer
}
