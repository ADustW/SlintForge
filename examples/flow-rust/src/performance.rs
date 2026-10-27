use crate::*;
use serde_json::json;
use slint::{
    platform::{PointerEventButton, WindowEvent},
    ComponentHandle, LogicalPosition, RenderingState,
};
use std::{
    cell::RefCell,
    rc::Rc,
    time::{Duration, Instant},
};

#[derive(Default)]
struct Metrics {
    active: bool,
    input: Option<Instant>,
    render: Option<Instant>,
    last: Option<Instant>,
    renders: Vec<f64>,
    frames: Vec<f64>,
    feedback: Vec<f64>,
    commands: Vec<f64>,
    phase: usize,
    phase_commands: [Vec<f64>; 3],
    phase_frames: [Vec<f64>; 3],
    node_dragged: bool,
    viewport_panned: bool,
    viewport_zoomed: bool,
}
fn p95(values: &[f64]) -> f64 {
    let mut v = values.to_vec();
    v.sort_by(f64::total_cmp);
    if v.is_empty() {
        0.
    } else {
        v[((v.len() - 1) as f64 * 0.95) as usize]
    }
}
pub fn start(
    ui: &ForgeFlowWindow,
    complex: bool,
) -> Result<slint::Timer, Box<dyn std::error::Error>> {
    // 关闭绘制层只用于定位开销，诊断结果不允许计入正式性能达标。
    let diagnostic = std::env::var("SLINTFORGE_FLOW_PROFILE_LAYER").unwrap_or_default();
    match diagnostic.as_str() {
        "" => {}
        "background" => ui.set_diagnostic_background(false),
        "minimap" => ui.set_diagnostic_minimap(false),
        "background+minimap" => {
            ui.set_diagnostic_background(false);
            ui.set_diagnostic_minimap(false);
        }
        _ => return Err("未知的 Flow 绘制层诊断选项".into()),
    }
    let metrics = Rc::new(RefCell::new(Metrics::default()));
    let m = metrics.clone();
    ui.window()
        .set_rendering_notifier(move |state, _| {
            let mut m = m.borrow_mut();
            if !m.active {
                return;
            }
            match state {
                RenderingState::BeforeRendering => m.render = Some(Instant::now()),
                RenderingState::AfterRendering => {
                    if let Some(t) = m.render {
                        m.renders.push(t.elapsed().as_secs_f64() * 1000.);
                    }
                    if let Some(t) = m.last {
                        m.frames.push(t.elapsed().as_secs_f64() * 1000.);
                        let phase = m.phase;
                        m.phase_frames[phase].push(t.elapsed().as_secs_f64() * 1000.);
                    }
                    m.last = Some(Instant::now());
                    if let Some(t) = m.input.take() {
                        m.feedback.push(t.elapsed().as_secs_f64() * 1000.);
                    }
                }
                _ => {}
            }
        })
        .map_err(|e| format!("当前渲染器不提供帧回调: {e:?}"))?;
    let weak = ui.as_weak();
    let timer = slint::Timer::default();
    let mut frame = -30;
    timer.start(slint::TimerMode::Repeated,Duration::from_millis(16),move||{
        let ui=weak.unwrap();let w=ui.window();let i=frame;frame+=1;
        if i==-30{ui.invoke_edit(ForgeFlowAction{op:"demo".into(),number:if complex{1}else{0},count:500,..Default::default()});ui.invoke_set_view(40.,40.,1.);return;}
        if i<0 || i>303{return;}
        if i==0{w.dispatch_event(WindowEvent::PointerMoved{position:LogicalPosition::new(100.,156.)});w.dispatch_event(WindowEvent::PointerPressed{position:LogicalPosition::new(100.,156.),button:PointerEventButton::Left});metrics.borrow_mut().active=true;return;}
        if i==101{w.dispatch_event(WindowEvent::PointerReleased{position:LogicalPosition::new(199.,180.),button:PointerEventButton::Left});let doc:serde_json::Value=serde_json::from_str(&ui.invoke_document_json()).unwrap();metrics.borrow_mut().node_dragged=doc["nodes"][0]["x"].as_f64().unwrap()>90.;w.dispatch_event(WindowEvent::PointerPressed{position:LogicalPosition::new(10.,110.),button:PointerEventButton::Middle});return;}
        if i==202{w.dispatch_event(WindowEvent::PointerReleased{position:LogicalPosition::new(109.,140.),button:PointerEventButton::Middle});metrics.borrow_mut().viewport_panned=ui.get_state().viewport.x>100.;return;}
        if i>302{
            let mut m=metrics.borrow_mut();m.active=false;let frame_p95=p95(&m.frames);let feedback=p95(&m.feedback);let size=w.size();let measured=m.frames.len()>=150&&m.feedback.len()>=150&&m.node_dragged&&m.viewport_panned&&m.viewport_zoomed;
            let report=json!({"host":"rust","build":"Release","machine":std::env::var("COMPUTERNAME").unwrap_or_default(),"renderer":std::env::var("SLINT_BACKEND").unwrap_or_else(|_|"winit-skia".into()),"scenario":if complex{"mixed-cards"}else{"standard"},"nodes":500,"edges":1000,"physical_width":size.width,"physical_height":size.height,"scale_factor":w.scale_factor(),"frames":m.frames.len(),"render_p95_ms":measured.then(||p95(&m.renders)),"frame_p95_ms":measured.then_some(frame_p95),"feedback_p95_ms":measured.then_some(feedback),"command_p95_ms":p95(&m.commands),"target_met":diagnostic.is_empty()&&measured&&frame_p95<=33.&&feedback<=100.,"measurement_complete":measured,"acceptance_eligible":diagnostic.is_empty(),"disabled_diagnostic_layers":diagnostic});
            let mut report=report;report["node_dragged"]=json!(m.node_dragged);report["viewport_panned"]=json!(m.viewport_panned);report["viewport_zoomed"]=json!(m.viewport_zoomed);for (phase,name) in ["drag","pan","zoom"].iter().enumerate(){report["phases"][name]=json!({"command_p95_ms":p95(&m.phase_commands[phase]),"frame_p95_ms":p95(&m.phase_frames[phase])});}drop(m);
            let path=std::path::PathBuf::from(std::env::var("SLINTFORGE_FLOW_QA_DIR").unwrap_or_else(|_|"build/flow-qa/rust".into()));std::fs::create_dir_all(&path).unwrap();
            std::fs::write(path.join(if complex{"performance-mixed.json"}else{"performance-standard.json"}),serde_json::to_string_pretty(&report).unwrap()).unwrap();
            println!("FLOW_PERFORMANCE {report}");
            let image=slintforge::capture_window(w).unwrap();slintforge::save_capture_png(&image,path.join(if complex{"performance-mixed.png"}else{"performance-standard.png"})).unwrap();slint::quit_event_loop().unwrap();return;
        }
        {let mut m=metrics.borrow_mut();if m.input.is_none(){m.input=Some(Instant::now());}}
        let started=Instant::now();
        if i<=100{w.dispatch_event(WindowEvent::PointerMoved{position:LogicalPosition::new(100.+i as f32,156.+(i%25) as f32)});}
        else if i<=201{w.dispatch_event(WindowEvent::PointerMoved{position:LogicalPosition::new(10.+(i-102) as f32,110.+((i-102)%30) as f32)});}
        else {let position=LogicalPosition::new(1070.,600.);w.dispatch_event(WindowEvent::PointerMoved{position});w.dispatch_event(WindowEvent::PointerScrolled{position,delta_x:0.,delta_y:if i%10<5{20.}else{-20.}});let mut m=metrics.borrow_mut();m.viewport_zoomed=m.viewport_zoomed||(ui.get_state().viewport.zoom-1.).abs()>0.01;}
        let elapsed=started.elapsed().as_secs_f64()*1000.;let mut m=metrics.borrow_mut();m.phase=if i<=100{0}else if i<=201{1}else{2};let phase=m.phase;m.commands.push(elapsed);m.phase_commands[phase].push(elapsed);
    });
    Ok(timer)
}
