// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

use crate::GalleryWindow;
use slint::{ComponentHandle, LogicalSize, RenderingState};
use std::cell::RefCell;
use std::rc::Rc;
use std::time::{Duration, Instant};

#[derive(Default)]
struct Measurements {
    next: usize,
    pending: Option<(&'static str, Instant)>,
    frame_started: Option<Instant>,
    responses: Vec<serde_json::Value>,
    callbacks: Vec<serde_json::Value>,
    frame_ms: Vec<f64>,
    finished: Option<Instant>,
    idle_frames: usize,
}

// 定向运行真实渲染循环，覆盖导航、搜索、缩放及返回后的状态；普通启动不启用。
pub fn verification_timer(ui: &GalleryWindow, enabled: bool) -> slint::Timer {
    let timer = slint::Timer::default();
    if !enabled {
        return timer;
    }
    let measurements = Rc::new(RefCell::new(Measurements::default()));
    let rendered = measurements.clone();
    ui.window()
        .set_rendering_notifier(move |event, _| {
            let mut data = rendered.borrow_mut();
            match event {
                RenderingState::BeforeRendering => data.frame_started = Some(Instant::now()),
                RenderingState::AfterRendering => {
                    if let Some(started) = data.frame_started.take() {
                        data.frame_ms.push(started.elapsed().as_secs_f64() * 1000.);
                    }
                    if let Some((action, started)) = data.pending.take() {
                        data.responses.push(serde_json::json!({
                            "action": action, "response_ms": started.elapsed().as_secs_f64() * 1000.
                        }));
                    }
                    if data
                        .finished
                        .is_some_and(|time| time.elapsed() > Duration::from_millis(400))
                    {
                        data.idle_frames += 1;
                    }
                }
                _ => {}
            }
        })
        .expect("交互性能验证需要渲染通知");
    let icons = ui.get_icon_items();
    let all_navigation = ui.get_all_navigation_items();
    let footer = ui.get_footer_items();
    ui.invoke_qa_set_lifecycle_draft("性能回归保留草稿".into());
    let weak = ui.as_weak();
    let started = Instant::now();
    timer.start(
        slint::TimerMode::Repeated,
        Duration::from_millis(180),
        move || {
            let ui = weak.upgrade().expect("验证期间窗口存在");
            if started.elapsed() < Duration::from_secs(2) {
                return;
            }
            let mut data = measurements.borrow_mut();
            if let Some((action, pending)) = data.pending {
                assert!(
                    pending.elapsed() < Duration::from_secs(5),
                    "操作未完成渲染: {action}"
                );
                return;
            }
            if let Some(finished) = data.finished {
                if finished.elapsed() < Duration::from_millis(1400) {
                    return;
                }
                assert_eq!(ui.get_lifecycle_draft().as_str(), "性能回归保留草稿");
                assert_eq!(ui.get_icon_items(), icons, "无关交互不能重建图标模型");
                assert_eq!(
                    ui.get_all_navigation_items(),
                    all_navigation,
                    "无关交互不能重建完整导航模型"
                );
                assert_eq!(ui.get_footer_items(), footer, "无关交互不能重建页脚模型");
                assert!(
                    data.idle_frames <= 4,
                    "隐藏页面仍在持续驱动重绘: {}",
                    data.idle_frames
                );
                let directory = std::env::var("SLINTFORGE_INTERACTION_QA_DIR")
                    .unwrap_or_else(|_| "build/gallery-performance/interaction".into());
                std::fs::create_dir_all(&directory).unwrap();
                let report = serde_json::json!({
                    "passed": true, "actions": data.responses, "callbacks": data.callbacks,
                    "frame_ms": data.frame_ms, "idle_frames_in_final_second": data.idle_frames,
                    "draft_retained": true, "unchanged_models_retained": true
                });
                std::fs::write(
                    std::path::Path::new(&directory).join("result.json"),
                    serde_json::to_vec_pretty(&report).unwrap(),
                )
                .unwrap();
                println!(
                    "INTERACTION_OK actions={} idle_frames={}",
                    data.responses.len(),
                    data.idle_frames
                );
                slint::quit_event_loop().unwrap();
                return;
            }
            if data.next == 48 {
                data.finished = Some(Instant::now());
                return;
            }
            let step = data.next % 16;
            let name = match step {
                0..=3 => "search",
                4..=5 => "expand",
                6..=7 => "pane",
                8..=11 => "navigate",
                12..=13 => "theme",
                _ => "resize",
            };
            data.next += 1;
            data.pending = Some((name, Instant::now()));
            drop(data);
            let callback_started = Instant::now();
            match step {
                0 => ui.invoke_search_requested("D".into()),
                1 => ui.invoke_search_requested("Data".into()),
                2 => ui.invoke_search_requested("DataGrid".into()),
                3 => ui.invoke_search_requested("".into()),
                4..=5 => ui.invoke_group_toggle_requested("inputs".into()),
                6..=7 => ui.invoke_qa_toggle_navigation(),
                8 => ui.invoke_navigate_requested("lifecycle".into(), Default::default()),
                9 => ui
                    .invoke_navigate_requested("inputs-standard-button".into(), Default::default()),
                10 => ui.invoke_back_requested(),
                11 => ui.invoke_navigate_requested("home".into(), Default::default()),
                12 => ui.invoke_theme_setting_changed(false),
                13 => ui.invoke_theme_setting_changed(true),
                14 => ui.window().set_size(LogicalSize::new(960., 600.)),
                _ => ui.window().set_size(LogicalSize::new(1920., 1080.)),
            }
            measurements.borrow_mut().callbacks.push(serde_json::json!({
                "action": name, "callback_ms": callback_started.elapsed().as_secs_f64() * 1000.
            }));
        },
    );
    timer
}
