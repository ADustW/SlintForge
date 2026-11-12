// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

include!(concat!(env!("OUT_DIR"), "/slintforge_style.rs"));
use slint::platform::{Key, PointerEventButton, WindowEvent};
use slint::{ComponentHandle, LogicalPosition, Model};
use std::time::Duration;

slintforge_ui! {

    import {ForgePlot} from "../../../ui/charts/plot.slint";
    import {ForgePlotAxis,ForgePlotPanel,ForgePlotScene,ForgePlotSelection,ForgePlotExportResult,ForgePlotPoint,ForgePlotXWindowMode} from "../../../ui/types/plot.slint";
    export {ForgePlotServices,ForgePlotScene,ForgePlotAxis,ForgePlotRange,ForgePlotRect,ForgePlotHit,ForgePlotSelection,ForgePlotExportResult} from "../../../ui/types/plot.slint";
    export component PlotTestWindow inherits Window {
        width: 780px; height: 520px;
        in-out property<bool> enabled: true;
        in-out property<bool> allow-pan: true;
        in-out property<bool> allow-zoom: true;
        in-out property<bool> bad-binding: false;
        in-out property<bool> measure-enabled: false;
        in-out property<ForgePlotXWindowMode> window-mode: fit;
        in-out property<float> window-size: 10;
        in-out property<int> directions: 3;
        in-out property<int> window-axis: 0;
        in-out property<int> zoom-panel: 0;
        in-out property<[ForgePlotAxis]> axes: [{id:0,side:bottom},{id:1,side:left}];
        in-out property<[ForgePlotPanel]> panels;
        in-out property<bool> follow-measurement: false;
        out property<float> cursor-a-pixel:plot.cursor-a-pixel;
        out property<float> cursor-b-pixel:plot.cursor-b-pixel;
        in-out property<float> cursor-a: 0.5;
        in-out property<float> cursor-b: 1.5;
        private property<[ForgePlotPoint]> data-points: [{x:0,y:1},{x:1,y:3},{x:2,y:2}];
        out property<ForgePlotScene> scene: plot.scene;
        out property<[ForgePlotSelection]> selection: plot.selection;
        in-out property<int> clicks: 0;
        in-out property<int> legend-clicks: 0;
        callback zoom(float,float,float);zoom(x,y,f)=>{plot.zoom-at(x,y,f,root.zoom-panel);}
        callback zoom-direction(int);zoom-direction(direction)=>{plot.zoom-directions=direction;}
        callback reset();reset=>{plot.reset-view();}
        callback refresh();refresh=>{plot.refresh-immediate();}
        callback append-point(ForgePlotPoint)->bool;append-point(point)=>{return plot.append-point(7,point);}
        callback flush();flush=>{plot.flush();}
        callback pan(float,float);pan(x,y)=>{plot.pan-by(x,y,0);}
        callback history(float);history(position)=>{plot.scroll-history(position);}
        callback follow();follow=>{plot.follow-latest();}
        callback export-file(string,string)->ForgePlotExportResult;export-file(p,f)=>{return plot.export-file(p,f);}
        plot:=ForgePlot {
            width: 780px; height: 520px;
            enabled: root.enabled;
            allow-pan:root.allow-pan;allow-zoom:root.allow-zoom;
            measure-enabled:root.measure-enabled;cursor-a<=>root.cursor-a;cursor-b<=>root.cursor-b;measurement-series:7;
            axes <=> root.axes; panels: root.panels;
            x-window-mode:root.window-mode;x-window-size:root.window-size;x-window-axis:root.window-axis;
            directions:root.directions;measurement-follow-view:root.follow-measurement;
            series: [{id: 7,name:"测量",x-axis:root.window-axis,y-axis:root.bad-binding?99:1,points:root.data-points}];
            point-selected(hit)=>{root.clicks+=1;}
            legend-clicked(id)=>{root.legend-clicks+=1;}
        }
    }
}
fn tick() {
    i_slint_backend_testing::mock_elapsed_time(Duration::from_millis(20));
    slint::platform::update_timers_and_animations();
}

fn live_zoom_window(mode: ForgePlotXWindowMode) -> PlotTestWindow {
    let ui = PlotTestWindow::new().unwrap();
    slintforge::bind_plot_services!(ui, ForgePlotServices, ForgePlotScene, ForgePlotAxis,
        ForgePlotRange, ForgePlotRect, ForgePlotHit, ForgePlotSelection, ForgePlotExportResult);
    ui.set_window_mode(mode);
    append_live_sample(&ui, 130.);
    ui
}

fn append_live_sample(ui: &PlotTestWindow, x: f32) {
    assert!(ui.invoke_append_point(ForgePlotPoint { x, y: x.sin(), ..Default::default() }));
    ui.invoke_flush();
    tick();
    assert_eq!(ui.get_scene().status, 0, "{}", ui.get_scene().error);
}

#[test]
fn realtime_zoom_keeps_following_latest_at_selected_scale() {
    i_slint_backend_testing::init_no_event_loop();
    for mode in [ForgePlotXWindowMode::Fixed, ForgePlotXWindowMode::Unlimited] {
        let ui = live_zoom_window(mode);
        ui.set_measure_enabled(true);
        ui.set_follow_measurement(true);
        tick();
        let mut latest = 130.;
        let mut span = 10.;
        for factor in [0.5, 2.4, 0.75] {
            let before = ui.get_scene();
            let rect = before.rects.row_data(0).unwrap();
            ui.invoke_zoom(rect.x + rect.width * 0.35, rect.y + rect.height * 0.6, factor);
            span *= factor;
            tick();
            for _ in 0..3 {
                let previous = ui.get_scene();
                latest += 0.5;
                append_live_sample(&ui, latest);
                let scene = ui.get_scene();
                assert!((scene.window_range.maximum - latest).abs() < 0.001,
                    "实时缩放后须继续跟随最新数据：actual={}, latest={latest}", scene.window_range.maximum);
                assert!((scene.window_range.maximum - scene.window_range.minimum - span).abs() < 0.001,
                    "实时追加必须保持缩放后的跨度");
                assert_eq!(scene.input_points, previous.input_points + 1);
                assert_ne!(scene.svg, previous.svg, "新数据必须进入更新后的可见场景");
                assert!(ui.get_cursor_a_pixel() >= rect.x && ui.get_cursor_a_pixel() <= rect.x + rect.width,
                    "缩放后实时测量光标仍须留在可见范围内");
            }
        }
    }
}

#[test]
fn realtime_history_zoom_stays_paused_and_follow_preserves_scale() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = live_zoom_window(ForgePlotXWindowMode::Unlimited);
    ui.invoke_history(0.25);
    tick();
    let rect = ui.get_scene().rects.row_data(0).unwrap();
    ui.invoke_zoom(rect.x + rect.width * 0.5, rect.y + rect.height * 0.5, 0.5);
    tick();
    let history = ui.get_scene().window_range;
    append_live_sample(&ui, 140.);
    assert_eq!(ui.get_scene().window_range, history, "查看历史时不能被实时追加拉走");
    ui.invoke_follow();
    tick();
    let followed = ui.get_scene().window_range;
    assert!((followed.maximum - 140.).abs() < 0.001);
    assert!((followed.maximum - followed.minimum - 5.).abs() < 0.001,
        "恢复跟随应保留当前缩放比例");
    ui.invoke_pan(24., 0.);
    tick();
    let panned = ui.get_scene().window_range;
    append_live_sample(&ui, 150.);
    assert_eq!(ui.get_scene().window_range, panned, "平移查看历史时保持定位");
    ui.invoke_reset();
    tick();
    assert!((ui.get_scene().window_range.maximum - 150.).abs() < 0.001);
    assert!((ui.get_scene().window_range.maximum - ui.get_scene().window_range.minimum - 10.).abs() < 0.001);
}

#[test]
fn realtime_wheel_keyboard_and_box_zoom_keep_following() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = live_zoom_window(ForgePlotXWindowMode::Unlimited);
    ui.show().unwrap();
    let rect = ui.get_scene().rects.row_data(0).unwrap();
    let center = LogicalPosition::new(rect.x + rect.width * 0.5, rect.y + rect.height * 0.5);
    ui.window().dispatch_event(WindowEvent::PointerScrolled {position:center,delta_x:0.,delta_y:50.});
    tick();
    append_live_sample(&ui, 131.);
    assert!((ui.get_scene().window_range.maximum - 131.).abs() < 0.001);
    assert!((ui.get_scene().window_range.maximum - ui.get_scene().window_range.minimum - 8.5).abs() < 0.001);
    click(&ui, center.x, center.y);
    let plus: slint::SharedString = "+".into();
    ui.window().dispatch_event(WindowEvent::KeyPressed {text:plus.clone()});
    ui.window().dispatch_event(WindowEvent::KeyReleased {text:plus});
    tick();
    append_live_sample(&ui, 132.);
    assert!((ui.get_scene().window_range.maximum - 132.).abs() < 0.001);
    assert!((ui.get_scene().window_range.maximum - ui.get_scene().window_range.minimum - 6.8).abs() < 0.001);
    key(&ui, Key::Shift, true);
    drag(&ui, (rect.x + rect.width * 0.25, rect.y + rect.height * 0.25),
        (rect.x + rect.width * 0.75, rect.y + rect.height * 0.75));
    key(&ui, Key::Shift, false);
    append_live_sample(&ui, 133.);
    assert!((ui.get_scene().window_range.maximum - 133.).abs() < 0.001);
    assert!((ui.get_scene().window_range.maximum - ui.get_scene().window_range.minimum - 3.4).abs() < 0.001);
    ui.set_directions(2);
    ui.invoke_zoom(center.x, center.y, 0.5);
    tick();
    append_live_sample(&ui, 134.);
    assert!((ui.get_scene().window_range.maximum - ui.get_scene().window_range.minimum - 3.4).abs() < 0.001,
        "仅缩放 Y 轴时不能改变实时 X 跨度");
    ui.set_window_size(12.);
    tick();
    assert!((ui.get_scene().window_range.maximum - ui.get_scene().window_range.minimum - 12.).abs() < 0.001,
        "应用更新配置跨度后应清除交互缩放跨度");
}

#[test]
fn zoom_directions_apply_to_all_inputs_and_keep_live_following() {
    i_slint_backend_testing::init_no_event_loop();
    for mode in [ForgePlotXWindowMode::Fixed, ForgePlotXWindowMode::Unlimited] {
        for direction in [1, 2, 3] {
            for input in 0..4 {
                let ui = live_zoom_window(mode);
                ui.invoke_zoom_direction(direction);
                // 测试后端只向组件投递事件，不创建可见桌面窗口。
                ui.show().unwrap();
                let before = ui.get_scene();
                let rect = before.rects.row_data(0).unwrap();
                let center = LogicalPosition::new(rect.x + rect.width * 0.5, rect.y + rect.height * 0.5);
                let factor = match input {
                    0 => { ui.invoke_zoom(center.x, center.y, 0.5); 0.5 }
                    1 => {
                        ui.window().dispatch_event(WindowEvent::PointerScrolled {
                            position: center, delta_x: 0., delta_y: 50.,
                        });
                        0.85
                    }
                    2 => {
                        click(&ui, center.x, center.y);
                        let text: slint::SharedString = "+".into();
                        ui.window().dispatch_event(WindowEvent::KeyPressed { text: text.clone() });
                        ui.window().dispatch_event(WindowEvent::KeyReleased { text });
                        0.8
                    }
                    _ => {
                        key(&ui, Key::Shift, true);
                        drag(&ui, (rect.x + rect.width * 0.25, rect.y + rect.height * 0.25),
                            (rect.x + rect.width * 0.75, rect.y + rect.height * 0.75));
                        key(&ui, Key::Shift, false);
                        0.5
                    }
                };
                tick();
                let zoomed = ui.get_scene();
                for axis in 0..2 {
                    let old = before.ranges.row_data(axis).unwrap();
                    let new = zoomed.ranges.row_data(axis).unwrap();
                    if direction & (1 << axis) == 0 {
                        assert_eq!(new, old, "未启用的轴范围必须保持原样：direction={direction}, input={input}");
                    } else {
                        assert!(((new.maximum - new.minimum) - (old.maximum - old.minimum) * factor).abs() < 0.001,
                            "指定方向未按比例缩放：direction={direction}, input={input}, axis={axis}");
                    }
                }
                append_live_sample(&ui, 131.);
                let live = ui.get_scene();
                assert!((live.window_range.maximum - 131.).abs() < 0.001);
                let expected_span = if direction & 1 != 0 { 10. * factor } else { 10. };
                assert!((live.window_range.maximum - live.window_range.minimum - expected_span).abs() < 0.001);
                if direction & 2 != 0 {
                    assert_eq!(live.ranges.row_data(1), zoomed.ranges.row_data(1), "实时更新须保留 Y 轴缩放范围");
                }
                ui.invoke_zoom_direction(if direction == 1 { 2 } else { 1 });
                tick();
                assert_eq!(ui.get_scene().window_range, live.window_range, "切换缩放方向不能复位已有视图");
                assert_eq!(ui.get_scene().ranges.row_data(1), live.ranges.row_data(1));
            }
        }
    }
}

#[test]
fn explicit_zoom_direction_does_not_restrict_pan() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = live_zoom_window(ForgePlotXWindowMode::Fit);
    ui.invoke_zoom_direction(1);
    let before = ui.get_scene();
    ui.invoke_pan(24., 18.);
    tick();
    let after = ui.get_scene();
    for axis in 0..2 {
        let old = before.ranges.row_data(axis).unwrap();
        let new = after.ranges.row_data(axis).unwrap();
        assert!((new.minimum - old.minimum).abs() > 0.001, "仅缩放 X 轴不能禁止 Y 轴平移");
        assert!(((new.maximum - new.minimum) - (old.maximum - old.minimum)).abs() < 0.001);
    }
}

#[test]
fn realtime_zoom_uses_configured_axis_id_and_panel() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = live_zoom_window(ForgePlotXWindowMode::Fixed);
    let mut x = ui.get_axes().row_data(0).unwrap();
    let mut y = ui.get_axes().row_data(1).unwrap();
    x.id = 42; x.panel = 4; x.reversed = true; y.panel = 4;
    let mut other_x = x.clone(); other_x.id = 20; other_x.panel = 8;
    let mut other_y = y.clone(); other_y.id = 21; other_y.panel = 8;
    ui.set_axes(slint::ModelRc::new(slint::VecModel::from(vec![other_x, y, x, other_y])));
    ui.set_panels(slint::ModelRc::new(slint::VecModel::from(vec![
        ForgePlotPanel {id:4,x:0.,y:0.,width:0.5,height:1.,..Default::default()},
        ForgePlotPanel {id:8,x:0.5,y:0.,width:0.5,height:1.,..Default::default()},
    ])));
    ui.set_window_axis(42);
    tick();
    ui.set_zoom_panel(8);
    let other = ui.get_scene().rects.iter().find(|r| r.id == 8).unwrap();
    ui.invoke_zoom(other.x + other.width * 0.5, other.y + other.height * 0.5, 0.5);
    tick();
    append_live_sample(&ui, 131.);
    assert!((ui.get_scene().window_range.maximum - ui.get_scene().window_range.minimum - 10.).abs() < 0.001);
    ui.set_zoom_panel(4);
    let main = ui.get_scene().rects.iter().find(|r| r.id == 4).unwrap();
    ui.invoke_zoom(main.x + main.width * 0.5, main.y + main.height * 0.5, 0.5);
    tick();
    append_live_sample(&ui, 132.);
    assert!((ui.get_scene().window_range.maximum - 132.).abs() < 0.001);
    assert!((ui.get_scene().window_range.maximum - ui.get_scene().window_range.minimum - 5.).abs() < 0.001);
    assert!(!ui.get_axes().row_data(2).unwrap().manual_range);
}
#[test]
fn enabling_measurement_after_live_window_moves_keeps_cursors_visible() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = PlotTestWindow::new().unwrap();
    slintforge::bind_plot_services!(
        ui,
        ForgePlotServices,
        ForgePlotScene,
        ForgePlotAxis,
        ForgePlotRange,
        ForgePlotRect,
        ForgePlotHit,
        ForgePlotSelection,
        ForgePlotExportResult
    );
    ui.set_window_mode(ForgePlotXWindowMode::Fixed);
    ui.set_follow_measurement(true);
    assert!(ui.invoke_append_point(ForgePlotPoint {
        x: 130.,
        y: 4.,
        ..Default::default()
    }));
    ui.invoke_flush();
    tick();
    ui.set_measure_enabled(true);
    tick();
    let scene = ui.get_scene();
    let rect = scene.rects.row_data(0).unwrap();
    assert!(ui.get_cursor_a() > 120. && ui.get_cursor_b() < 130.);
    assert!(ui.get_cursor_a_pixel() > rect.x && ui.get_cursor_b_pixel() < rect.x + rect.width);
    let relative = (ui.get_cursor_a() - scene.measurement_range.minimum) / 10.;
    assert!(ui.invoke_append_point(ForgePlotPoint {
        x: 160.,
        y: 5.,
        ..Default::default()
    }));
    ui.invoke_flush();
    tick();
    let scene = ui.get_scene();
    assert!((ui.get_cursor_a() - scene.measurement_range.minimum - relative * 10.).abs() < 0.001);
    assert!(
        ui.get_cursor_a() > 150. && ui.get_cursor_b() < 160.,
        "滚动到新的实时范围后光标仍须可见"
    );
    let before = ui.get_cursor_a();
    ui.set_follow_measurement(false);
    assert!(ui.invoke_append_point(ForgePlotPoint {
        x: 200.,
        y: 6.,
        ..Default::default()
    }));
    ui.invoke_flush();
    tick();
    assert_eq!(ui.get_cursor_a(), before, "关闭跟随后应保留数据坐标");
}
#[test]
fn temporary_axis_configuration_error_preserves_appended_history() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = PlotTestWindow::new().unwrap();
    slintforge::bind_plot_services!(
        ui,
        ForgePlotServices,
        ForgePlotScene,
        ForgePlotAxis,
        ForgePlotRange,
        ForgePlotRect,
        ForgePlotHit,
        ForgePlotSelection,
        ForgePlotExportResult
    );
    let _ = ui.get_scene();
    assert!(ui.invoke_append_point(ForgePlotPoint {
        x: 3.,
        y: 4.,
        ..Default::default()
    }));
    ui.invoke_flush();
    assert_eq!(ui.get_scene().input_points, 4);
    ui.set_bad_binding(true);
    tick();
    assert_eq!(ui.get_scene().status, 2);
    ui.set_bad_binding(false);
    tick();
    let scene = ui.get_scene();
    assert_eq!(scene.status, 0, "{}", scene.error);
    assert_eq!(scene.input_points, 4, "临时配置错误不能丢弃已追加的数据");
}
#[test]
fn realtime_updates_during_cursor_drag_keep_pointer_capture_and_view_unchanged() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = PlotTestWindow::new().unwrap();
    slintforge::bind_plot_services!(
        ui,
        ForgePlotServices,
        ForgePlotScene,
        ForgePlotAxis,
        ForgePlotRange,
        ForgePlotRect,
        ForgePlotHit,
        ForgePlotSelection,
        ForgePlotExportResult
    );
    ui.show().unwrap();
    ui.set_window_mode(ForgePlotXWindowMode::Fixed);
    ui.set_follow_measurement(true);
    ui.invoke_append_point(ForgePlotPoint {
        x: 130.,
        y: 4.,
        ..Default::default()
    });
    ui.invoke_flush();
    ui.set_measure_enabled(true);
    tick();
    for cursor in 0..2 {
        let scene = ui.get_scene();
        let base = scene.window_range.maximum;
        let rect = scene.rects.row_data(0).unwrap();
        let start_x = if cursor == 0 {
            ui.get_cursor_a_pixel()
        } else {
            ui.get_cursor_b_pixel()
        };
        let start = LogicalPosition::new(start_x, rect.y + rect.height * 0.4);
        ui.window()
            .dispatch_event(WindowEvent::PointerMoved { position: start });
        ui.window().dispatch_event(WindowEvent::PointerPressed {
            position: start,
            button: PointerEventButton::Left,
        });
        tick();
        for step in 1..=5 {
            // 按住期间刷新场景，复现实时绘图导致重复项重建、丢失鼠标捕获的问题。
            ui.invoke_append_point(ForgePlotPoint {
                x: base + step as f32 * 0.1,
                y: 4.,
                ..Default::default()
            });
            ui.invoke_flush();
            tick();
            let pixel = start_x + step as f32 * 15.;
            let position = LogicalPosition::new(pixel, start.y);
            ui.window()
                .dispatch_event(WindowEvent::PointerMoved { position });
            tick();
            let scene = ui.get_scene();
            let expected = ui.global::<ForgePlotServices>().invoke_coordinate(
                scene.token,
                scene.measurement_axis,
                pixel,
                true,
            );
            let actual = if cursor == 0 {
                ui.get_cursor_a()
            } else {
                ui.get_cursor_b()
            };
            assert!((actual-expected).abs()<0.002,"按住刷新后第 {step} 步光标 {cursor} 未跟随鼠标：actual={actual}, expected={expected}");
            assert!(
                (scene.window_range.maximum - (base + step as f32 * 0.1)).abs() < 0.002,
                "拖动光标不应平移视图"
            );
        }
        ui.window().dispatch_event(WindowEvent::PointerReleased {
            position: LogicalPosition::new(start_x + 75., start.y),
            button: PointerEventButton::Left,
        });
        tick();
    }
}
#[test]
fn point_appends_are_coalesced_and_measurement_cursor_can_be_dragged() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = PlotTestWindow::new().unwrap();
    slintforge::bind_plot_services!(
        ui,
        ForgePlotServices,
        ForgePlotScene,
        ForgePlotAxis,
        ForgePlotRange,
        ForgePlotRect,
        ForgePlotHit,
        ForgePlotSelection,
        ForgePlotExportResult
    );
    ui.show().unwrap();
    tick();
    let before = ui.get_scene();
    for index in 0..1000 {
        assert!(ui.invoke_append_point(ForgePlotPoint {
            x: index as f32 + 3.,
            y: 2.,
            ..Default::default()
        }));
    }
    assert_eq!(
        ui.get_scene().input_points,
        before.input_points,
        "同一帧的逐点追加不应逐点重绘"
    );
    let service = ui.global::<ForgePlotServices>();
    assert_eq!(
        service.invoke_measure(before.token, 7, 1002.).index,
        1002,
        "逐点追加必须立即保留原始数据"
    );
    slint::platform::update_timers_and_animations();
    tick();
    tick();
    assert_eq!(ui.get_scene().input_points, 1003);
    // 静态视图使用真实窗口事件拖动 A 光标，独立验证覆盖层的输入路由。
    ui.invoke_refresh();
    ui.set_measure_enabled(true);
    tick();
    let scene = ui.get_scene();
    let rect = scene.rects.row_data(0).unwrap();
    let service = ui.global::<ForgePlotServices>();
    let y = rect.y + rect.height * 0.4;
    let start =
        slint::LogicalPosition::new(service.invoke_coordinate(scene.token, 0, 0.5, false), y);
    let end = slint::LogicalPosition::new(service.invoke_coordinate(scene.token, 0, 1.6, false), y);
    ui.window()
        .dispatch_event(WindowEvent::PointerMoved { position: start });
    ui.window().dispatch_event(WindowEvent::PointerPressed {
        position: start,
        button: PointerEventButton::Left,
    });
    ui.window()
        .dispatch_event(WindowEvent::PointerMoved { position: end });
    ui.window().dispatch_event(WindowEvent::PointerReleased {
        position: end,
        button: PointerEventButton::Left,
    });
    tick();
    assert!(
        (ui.get_cursor_a() - 1.6).abs() < 0.02,
        "光标拖动未更新数据坐标"
    );
    let sample = service.invoke_measure(scene.token, 7, ui.get_cursor_a());
    assert_eq!(sample.index, 2);
    assert_eq!(sample.value, 2.);
}
fn click(ui: &PlotTestWindow, x: f32, y: f32) {
    let position = LogicalPosition::new(x, y);
    ui.window()
        .dispatch_event(WindowEvent::PointerMoved { position });
    ui.window().dispatch_event(WindowEvent::PointerPressed {
        position,
        button: PointerEventButton::Left,
    });
    ui.window().dispatch_event(WindowEvent::PointerReleased {
        position,
        button: PointerEventButton::Left,
    });
    tick();
}
fn key(ui: &PlotTestWindow, key: Key, pressed: bool) {
    let text: slint::SharedString = key.into();
    ui.window().dispatch_event(if pressed {
        WindowEvent::KeyPressed { text }
    } else {
        WindowEvent::KeyReleased { text }
    });
    tick();
}
fn drag(ui: &PlotTestWindow, from: (f32, f32), to: (f32, f32)) {
    let start = LogicalPosition::new(from.0, from.1);
    let end = LogicalPosition::new(to.0, to.1);
    ui.window()
        .dispatch_event(WindowEvent::PointerMoved { position: start });
    ui.window().dispatch_event(WindowEvent::PointerPressed {
        position: start,
        button: PointerEventButton::Left,
    });
    ui.window()
        .dispatch_event(WindowEvent::PointerMoved { position: end });
    ui.window().dispatch_event(WindowEvent::PointerReleased {
        position: end,
        button: PointerEventButton::Left,
    });
    tick();
}
#[test]
fn public_plot_binds_selects_zooms_and_disables() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = PlotTestWindow::new().unwrap();
    assert_eq!(ui.get_scene().status, 3);
    slintforge::bind_plot_services!(
        ui,
        ForgePlotServices,
        ForgePlotScene,
        ForgePlotAxis,
        ForgePlotRange,
        ForgePlotRect,
        ForgePlotHit,
        ForgePlotSelection,
        ForgePlotExportResult
    );
    tick();
    let initial = ui.get_scene();
    assert_eq!(initial.status, 0, "{}", initial.error);
    assert_eq!(initial.input_points, 3);
    assert_eq!(initial.rects.row_count(), 1);
    assert_eq!(
        (initial.image.size().width, initial.image.size().height),
        (780, 520)
    );
    for index in 0..20 {
        ui.window().dispatch_event(WindowEvent::PointerMoved {
            position: LogicalPosition::new(200. + index as f32, 200.),
        });
        tick();
        assert_eq!(
            ui.get_scene().image,
            initial.image,
            "悬停应复用同一幅场景图像"
        );
        assert_eq!(
            ui.get_scene().svg,
            initial.svg,
            "悬停覆盖层不能重建矢量场景"
        );
    }
    ui.window()
        .dispatch_event(WindowEvent::ScaleFactorChanged { scale_factor: 2. });
    tick();
    assert_eq!(
        (
            ui.get_scene().image.size().width,
            ui.get_scene().image.size().height
        ),
        (1560, 1040),
        "DPI 变化后应按物理尺寸重建缓存"
    );
    assert_eq!(ui.get_scene().svg, initial.svg, "DPI 不能改变矢量导出尺寸");
    ui.window()
        .dispatch_event(WindowEvent::ScaleFactorChanged { scale_factor: 1. });
    tick();
    let x_range = initial.ranges.row_data(0).unwrap();
    let y_range = initial.ranges.row_data(1).unwrap();
    let r = initial.rects.row_data(0).unwrap();
    let x = r.x + (1. - x_range.minimum) / (x_range.maximum - x_range.minimum) * r.width;
    let y = r.y + (1. - (3. - y_range.minimum) / (y_range.maximum - y_range.minimum)) * r.height;
    click(&ui, x, y);
    assert_eq!(ui.get_clicks(), 1);
    assert_eq!(ui.get_selection().row_data(0).unwrap().id, 7);
    assert_eq!(ui.get_selection().row_data(0).unwrap().first, 1);
    assert_ne!(ui.get_scene().svg, initial.svg, "选择应改变可见图形");
    // 从实际命中区域定位图例，点击文字部分验证显示与输入区域一致。
    let service = ui.global::<ForgePlotServices>();
    let legend = (0..520)
        .step_by(4)
        .find_map(|y| {
            (0..780).step_by(4).find_map(|x| {
                let hit = service.invoke_hit_test(initial.token, x as f32, y as f32, 0.);
                (hit.kind == 2 && hit.id == 7).then_some((hit.pixel_x + 32., hit.pixel_y + 6.))
            })
        })
        .expect("图例应具有可点击区域");
    click(&ui, legend.0, legend.1);
    assert_eq!(ui.get_legend_clicks(), 1);
    assert_eq!(ui.get_scene().status, 1);
    click(&ui, legend.0, legend.1);
    assert_eq!(ui.get_legend_clicks(), 2);
    assert_eq!(ui.get_scene().status, 0);
    ui.invoke_zoom(x, y, 0.5);
    tick();
    let zoomed = ui.get_scene().ranges.row_data(0).unwrap();
    assert!(
        (zoomed.maximum - zoomed.minimum - (x_range.maximum - x_range.minimum) * 0.5).abs() < 1e-5
    );
    ui.invoke_reset();
    tick();
    assert!((ui.get_scene().ranges.row_data(0).unwrap().maximum - x_range.maximum).abs() < 1e-5);
    key(&ui, Key::Control, true);
    drag(
        &ui,
        (r.x + 1., r.y + 1.),
        (r.x + r.width - 1., r.y + r.height - 1.),
    );
    key(&ui, Key::Control, false);
    let selected = ui.get_selection();
    assert_eq!(selected.row_count(), 1);
    assert_eq!(selected.row_data(0).unwrap().first, 0);
    assert_eq!(
        selected.row_data(0).unwrap().last,
        2,
        "真实框选应保留全部原始索引"
    );
    key(&ui, Key::Shift, true);
    drag(
        &ui,
        (r.x + r.width * 0.25, r.y + r.height * 0.25),
        (r.x + r.width * 0.75, r.y + r.height * 0.75),
    );
    key(&ui, Key::Shift, false);
    assert!(
        (ui.get_scene().ranges.row_data(0).unwrap().maximum
            - ui.get_scene().ranges.row_data(0).unwrap().minimum
            - (x_range.maximum - x_range.minimum) * 0.5)
            .abs()
            < 1e-4
    );
    ui.invoke_reset();
    tick();
    drag(&ui, (x, y), (x + 20., y));
    assert!(ui.get_scene().ranges.row_data(0).unwrap().minimum < x_range.minimum);
    ui.invoke_reset();
    tick();
    ui.window().dispatch_event(WindowEvent::PointerScrolled {
        position: LogicalPosition::new(x, y),
        delta_x: 0.,
        delta_y: 50.,
    });
    tick();
    assert!(
        ui.get_scene().ranges.row_data(0).unwrap().maximum
            - ui.get_scene().ranges.row_data(0).unwrap().minimum
            < x_range.maximum - x_range.minimum
    );
    ui.invoke_reset();
    tick();
    ui.set_enabled(false);
    tick();
    click(&ui, x, y);
    assert_eq!(ui.get_clicks(), 1);
    ui.invoke_zoom(x, y, 0.5);
    assert!((ui.get_scene().ranges.row_data(0).unwrap().maximum - x_range.maximum).abs() < 1e-5);
    ui.set_enabled(true);
    tick();
    click(&ui, x, y);
    let text: slint::SharedString = Key::Escape.into();
    ui.window()
        .dispatch_event(WindowEvent::KeyPressed { text: text.clone() });
    ui.window()
        .dispatch_event(WindowEvent::KeyReleased { text });
    tick();
    assert_eq!(ui.get_selection().row_count(), 0);
    ui.invoke_pan(24., 0.);
    tick();
    assert!(ui.get_scene().ranges.row_data(0).unwrap().minimum < x_range.minimum);
    ui.invoke_reset();
    tick();
    ui.set_allow_pan(false);
    ui.set_allow_zoom(false);
    tick();
    ui.invoke_pan(24., 0.);
    ui.invoke_zoom(x, y, 0.5);
    tick();
    assert!((ui.get_scene().ranges.row_data(0).unwrap().minimum - x_range.minimum).abs() < 1e-5);
    for key in [Key::RightArrow, Key::Home] {
        let text: slint::SharedString = key.into();
        ui.window()
            .dispatch_event(WindowEvent::KeyPressed { text: text.clone() });
        ui.window()
            .dispatch_event(WindowEvent::KeyReleased { text });
        tick();
    }
    assert!(
        (ui.get_scene().ranges.row_data(0).unwrap().minimum - x_range.minimum).abs() < 1e-5,
        "交互开关应同时约束键盘"
    );
    let directory = std::env::var_os("SLINTFORGE_PLOT_QA_DIR")
        .map(std::path::PathBuf::from)
        .unwrap_or_else(|| {
            std::env::current_dir()
                .unwrap()
                .join("../../build/plot-qa/interaction-export")
        });
    std::fs::create_dir_all(&directory).unwrap();
    let result = ui.invoke_export_file(
        directory
            .join("pointer.svg")
            .to_string_lossy()
            .as_ref()
            .into(),
        "svg".into(),
    );
    assert!(result.ok, "{}", result.error);
    let result = ui.invoke_export_file(
        directory
            .join("pointer.invalid")
            .to_string_lossy()
            .as_ref()
            .into(),
        "invalid".into(),
    );
    assert!(!result.ok);
}
