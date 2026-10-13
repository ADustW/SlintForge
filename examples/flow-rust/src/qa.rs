use crate::*;
use serde_json::{json, Value};
use slint::{
    platform::{PointerEventButton, WindowEvent},
    ComponentHandle, LogicalPosition, PhysicalSize,
};
use std::{cell::Cell, rc::Rc, time::Duration};

fn action(v: &Value) -> ForgeFlowAction {
    ForgeFlowAction {
        op: v["op"].as_str().unwrap_or_default().into(),
        id: v["id"].as_str().unwrap_or_default().into(),
        target: v["target"].as_str().unwrap_or_default().into(),
        port: v["port"].as_str().unwrap_or_default().into(),
        target_port: v["target_port"].as_str().unwrap_or_default().into(),
        text: v["text"].as_str().unwrap_or_default().into(),
        x: v["x"].as_f64().unwrap_or_default() as f32,
        y: v["y"].as_f64().unwrap_or_default() as f32,
        width: v["width"].as_f64().unwrap_or_default() as f32,
        height: v["height"].as_f64().unwrap_or_default() as f32,
        zoom: v["zoom"].as_f64().unwrap_or(1.) as f32,
        number: v["number"].as_i64().unwrap_or_default() as i32,
        count: v["count"].as_i64().unwrap_or_default() as i32,
        flag: v["flag"].as_bool().unwrap_or(false),
        additive: v["additive"].as_bool().unwrap_or(false),
    }
}
fn screenshot(ui: &ForgeFlowWindow, name: &str) -> Result<(), Box<dyn std::error::Error>> {
    let path = std::path::PathBuf::from(
        std::env::var("SLINTFORGE_FLOW_QA_DIR").unwrap_or_else(|_| "build/flow-qa/rust".into()),
    );
    std::fs::create_dir_all(&path)?;
    slintforge::save_capture_png(
        &slintforge::capture_window(ui.window())?,
        path.join(format!("{name}.png")),
    )?;
    Ok(())
}
fn step(ui: &ForgeFlowWindow, v: &Value) -> Result<(), Box<dyn std::error::Error>> {
    let window = ui.window();
    let kind = v["kind"].as_str().unwrap();
    let position = LogicalPosition::new(
        v["x"].as_f64().unwrap_or_default() as f32,
        v["y"].as_f64().unwrap_or_default() as f32,
    );
    let button = if v["button"] == "middle" {
        PointerEventButton::Middle
    } else {
        PointerEventButton::Left
    };
    match kind {
        "view" => ui.invoke_set_view(position.x, position.y, v["zoom"].as_f64().unwrap() as f32),
        "edit" => {
            if v["op"] == "demo" {
                ui.set_variant(v["number"].as_i64().unwrap_or_default() as i32);
            }
            ui.invoke_edit(action(v));
            if !ui.get_state().ok {
                return Err(ui.get_state().error.to_string().into());
            }
        }
        "block" => ui.set_blocked_target(v["target"].as_str().unwrap_or_default().into()),
        "readonly" => ui.set_read_only(v["value"].as_bool().unwrap()),
        "press" => window.dispatch_event(WindowEvent::PointerPressed { position, button }),
        "release" => window.dispatch_event(WindowEvent::PointerReleased { position, button }),
        "move" => window.dispatch_event(WindowEvent::PointerMoved { position }),
        "click" => {
            window.dispatch_event(WindowEvent::PointerMoved { position });
            window.dispatch_event(WindowEvent::PointerPressed { position, button });
            window.dispatch_event(WindowEvent::PointerReleased { position, button });
        }
        "wheel" => {
            window.dispatch_event(WindowEvent::PointerMoved { position });
            window.dispatch_event(WindowEvent::PointerScrolled {
                position,
                delta_x: 0.,
                delta_y: v["delta"].as_f64().unwrap() as f32,
            });
        }
        "key" | "key-down" | "key-up" => {
            let text: slint::SharedString = v["text"].as_str().unwrap().into();
            if kind != "key-up" {
                window.dispatch_event(WindowEvent::KeyPressed { text: text.clone() });
            }
            if kind != "key-down" {
                window.dispatch_event(WindowEvent::KeyReleased { text });
            }
        }
        "theme" => ui.set_dark(v["dark"].as_bool().unwrap()),
        // 保持指定的逻辑尺寸，不让应用缩放与系统 DPI 的差异改变控件落点。
        "size" => window.set_size(PhysicalSize::new(
            (v["width"].as_f64().unwrap() as f32 * window.scale_factor()).round() as u32,
            (v["height"].as_f64().unwrap() as f32 * window.scale_factor()).round() as u32,
        )),
        "fit" => ui.invoke_fit(),
        "snapshot" => screenshot(ui, v["name"].as_str().unwrap())?,
        "expect" => {
            let doc: Value = serde_json::from_str(ui.invoke_document_json().as_str())?;
            let actual = doc
                .pointer(v["path"].as_str().unwrap())
                .ok_or("字段不存在")?;
            let expected = &v["value"];
            let equal = match (actual.as_f64(), expected.as_f64()) {
                (Some(a), Some(b)) => (a - b).abs() < 0.02,
                _ => actual == expected,
            };
            if !equal {
                return Err(
                    format!("{} expected={} actual={}", v["path"], expected, actual).into(),
                );
            }
        }
        "count" => {
            let s = ui.get_state();
            if json!(s.node_count) != v["nodes"] || json!(s.edge_count) != v["edges"] {
                return Err(format!(
                    "节点或连线数量不符 {}/{} {}",
                    s.node_count, s.edge_count, s.error
                )
                .into());
            }
        }
        "save-load" => {
            let service = ui.global::<ForgeFlowServices>();
            let token = ui.get_state().token;
            let before = service.invoke_export_json(token);
            let error = service.invoke_save_file(token, "build/flow-qa/rust-roundtrip.json".into());
            if !error.is_empty() {
                return Err(error.to_string().into());
            }
            let loaded =
                service.invoke_load_file(token, "build/flow-qa/rust-roundtrip.json".into());
            if !loaded.ok || service.invoke_export_json(token) != before {
                return Err("文件保存加载不一致".into());
            }
        }
        _ => return Err(format!("未知验收步骤 {kind}").into()),
    }
    Ok(())
}
pub fn start(ui: &ForgeFlowWindow) -> (slint::Timer, Rc<Cell<bool>>) {
    let failed = Rc::new(Cell::new(false));
    let failure = failed.clone();
    let weak = ui.as_weak();
    let timer = slint::Timer::default();
    let scenario = std::env::var("SLINTFORGE_FLOW_QA_SCENARIO")
        .map(|path| std::fs::read_to_string(path).expect("无法读取 Flow 验收场景"))
        .unwrap_or_else(|_| include_str!("../../ui/flow-qa.json").to_owned());
    let steps: Vec<Value> = serde_json::from_str(&scenario).unwrap();
    let mut index: i32 = -3;
    timer.start(
        slint::TimerMode::Repeated,
        Duration::from_millis(100),
        move || {
            if index < 0 {
                index += 1;
                return;
            }
            let ui = weak.unwrap();
            let v = &steps[index as usize];
            if v["kind"] == "finish" {
                println!("FLOW_WINDOW_OK host=rust steps={}", steps.len());
                slint::quit_event_loop().unwrap();
                return;
            }
            if let Err(error) = step(&ui, v) {
                eprintln!("步骤 {index}: {error}");
                failure.set(true);
                let _ = screenshot(&ui, "failure");
                slint::quit_event_loop().unwrap();
            }
            index += 1;
        },
    );
    (timer, failed)
}
