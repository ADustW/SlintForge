use crate::*;
use serde_json::{json, Value};
use slint::{
    platform::{PointerEventButton, WindowEvent},
    ComponentHandle, LogicalPosition, PhysicalSize,
};
use std::{
    cell::{Cell, RefCell},
    collections::VecDeque,
    rc::Rc,
    time::Duration,
};

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
fn step(
    ui: &ForgeFlowWindow,
    v: &Value,
    events: &RefCell<VecDeque<(String, String)>>,
) -> Result<(), Box<dyn std::error::Error>> {
    let window = ui.window();
    let kind = v["kind"].as_str().unwrap();
    let position = LogicalPosition::new(
        v["x"].as_f64().unwrap_or_default() as f32,
        v["y"].as_f64().unwrap_or_default() as f32,
    );
    let button = if v["button"] == "middle" {
        PointerEventButton::Middle
    } else if v["button"] == "right" {
        PointerEventButton::Right
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
        "block-edit" => ui.set_blocked_operation(v["op"].as_str().unwrap_or_default().into()),
        "event" => {
            if ui.get_last_interaction().as_str() != v["value"].as_str().unwrap()
                || ui.get_last_interaction_id().as_str() != v["id"].as_str().unwrap_or_default()
            {
                return Err(format!("事件不符: {}", ui.get_last_interaction()).into());
            }
        }
        "clear-events" => events.borrow_mut().clear(),
        "observed-event" => {
            if !events.borrow().contains(&(
                v["value"].as_str().unwrap().into(),
                v["id"].as_str().unwrap_or_default().into(),
            )) {
                return Err(format!("未收到事件: {}", v["value"]).into());
            }
        }
        "preview" => {
            if !ui
                .get_connection_path()
                .as_str()
                .contains(v["contains"].as_str().unwrap())
            {
                return Err("连接预览线型不符".into());
            }
        }
        "error" => {
            if !ui
                .get_flow_error()
                .as_str()
                .contains(v["contains"].as_str().unwrap())
            {
                return Err("缺少预期错误".into());
            }
        }
        "readonly" => ui.set_read_only(v["value"].as_bool().unwrap()),
        "animations" => ui.set_animations_enabled(v["value"].as_bool().unwrap()),
        "file-path" | "image-path" => {
            let base=std::env::var_os("SLINTFORGE_FLOW_QA_DIR").map(std::path::PathBuf::from).unwrap_or_else(||"build/flow-qa/rust".into());
            let path=base.join(v["name"].as_str().unwrap()).to_string_lossy().into_owned();
            if kind=="file-path" {ui.set_file_path(path.into());} else {ui.set_image_path(path.into());}
        }
        "save" => ui.invoke_save_document(),
        "focus-canvas" => ui.invoke_focus_canvas(),
        "dispose" => ui.invoke_dispose(),
        "reopen" => ui.invoke_reopen(),
        "controllers" => {if ui.global::<ForgeFlowServices>().invoke_active_documents()!=v["value"].as_i64().unwrap() as i32 {return Err("文档控制器数量不符".into());}},
        "lifecycle-marker" => println!("FLOW_LIFECYCLE_PHASE {}",v["name"].as_str().unwrap()),
        "lifecycle" => {
            let service=ui.global::<ForgeFlowServices>();let active=service.invoke_active_documents();let preserved=service.invoke_export_json(ui.get_state().token);
            for cycle in 0..64 {let state=service.invoke_create("graph".into());service.invoke_execute(state.token,action(&json!({"op":"demo","number":cycle%4,"count":12})));service.invoke_release(state.token);service.invoke_release(state.token);if service.invoke_execute(state.token,action(&json!({"op":"refresh"}))).ok {return Err("已释放句柄仍可编辑".into());}}
            if service.invoke_active_documents()!=active || service.invoke_export_json(ui.get_state().token)!=preserved {return Err("多实例释放泄漏控制器或影响当前文档".into());}
        }
        "export" => ui.invoke_export_image(v["selected"].as_bool().unwrap_or(false)),
        "ui-state" => {
            let state=json!({"dirty":ui.get_state().dirty,"save_count":ui.get_save_count(),"export_count":ui.get_export_count()});
            for (key,value) in v["value"].as_object().unwrap() {if &state[key]!=value {return Err(format!("界面状态不符: {state}").into());}}
        }
        "press" => window.dispatch_event(WindowEvent::PointerPressed { position, button }),
        "release" => window.dispatch_event(WindowEvent::PointerReleased { position, button }),
        "move" => window.dispatch_event(WindowEvent::PointerMoved { position }),
        "click" => {
            window.dispatch_event(WindowEvent::PointerMoved { position });
            window.dispatch_event(WindowEvent::PointerPressed { position, button });
            window.dispatch_event(WindowEvent::PointerReleased { position, button });
        }
        "double-click" => {
            window.dispatch_event(WindowEvent::PointerMoved { position });
            for _ in 0..2 {
                window.dispatch_event(WindowEvent::PointerPressed { position, button });
                window.dispatch_event(WindowEvent::PointerReleased { position, button });
            }
        }
        "wheel" => {
            window.dispatch_event(WindowEvent::PointerMoved { position });
            window.dispatch_event(WindowEvent::PointerScrolled {
                position,
                delta_x: v["delta_x"].as_f64().unwrap_or(0.) as f32,
                delta_y: v["delta"].as_f64().unwrap() as f32,
            });
        }
        "pinch" => {
            // 合成后端手势验证真实分发路径，不代表实体触摸设备验收。
            let phase = match v["phase"].as_str().unwrap() {
                "start" => i_slint_core::input::TouchPhase::Started,
                "move" => i_slint_core::input::TouchPhase::Moved,
                "end" => i_slint_core::input::TouchPhase::Ended,
                _ => i_slint_core::input::TouchPhase::Cancelled,
            };
            window.dispatch_event(WindowEvent::internal(
                i_slint_core::input::BackendMouseEvent::PinchGesture {
                    position: i_slint_core::lengths::LogicalPoint::new(position.x, position.y),
                    delta: v["delta"].as_f64().unwrap_or(0.) as f32,
                    phase,
                },
            ));
        }
        "expect-range" => {
            let doc: serde_json::Value = serde_json::from_str(ui.invoke_document_json().as_str())
                .map_err(|e| e.to_string())?;
            let value = doc
                .pointer(v["path"].as_str().unwrap())
                .and_then(|v| v.as_f64())
                .ok_or("范围断言字段不是数值")?;
            if value < v["min"].as_f64().unwrap() || value > v["max"].as_f64().unwrap() {
                return Err(format!("{} 超出范围: {}", v["path"], value).into());
            }
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
        "expect" | "query-expect" => {
            let text = if kind == "query-expect" {
                ui.global::<ForgeFlowServices>()
                    .invoke_query(ui.get_state().token, v["request"].to_string().into())
            } else {
                ui.invoke_document_json()
            };
            let doc: Value = serde_json::from_str(text.as_str())?;
            if kind == "query-expect" && doc["ok"] != true {
                return Err(format!("查询失败: {}", doc["error"]).into());
            }
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
    let events = Rc::new(RefCell::new(VecDeque::new()));
    let observed = events.clone();
    ui.on_interaction(move |name, a| {
        let mut events = observed.borrow_mut();
        if events.len() == 64 {
            events.pop_front();
        }
        events.push_back((name.to_string(), a.id.to_string()));
    });
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
            if let Err(error) = step(&ui, v, &events) {
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
