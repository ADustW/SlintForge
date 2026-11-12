// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

use crate::{ForgePlotPoint, ForgePlotServices, ProfessionalPlotWindow};
use slint::platform::{PointerEventButton, WindowEvent};
use slint::{ComponentHandle, Model};
use std::time::Duration;

// 每个拖动步骤之间追加数据，覆盖场景重建、顶部标签与两条光标交叉。
pub fn start(ui: &ProfessionalPlotWindow, timer: &slint::Timer) {
    let directory = crate::performance::directory();
    std::fs::create_dir_all(&directory).unwrap();
    let weak = ui.as_weak();
    let mut phase = 0;
    let mut start_x = 0.;
    let mut y = 0.;
    let mut csv =
        String::from("cursor,step,maximum,target_pixel,actual_pixel,actual_value,other_pixel\n");
    timer.start(slint::TimerMode::Repeated,Duration::from_millis(100),move||{
        let ui=weak.upgrade().unwrap();let save=|name:&str|{let image=slintforge::capture_window(ui.window()).unwrap();slintforge::save_capture_png(&image,&directory.join(format!("{name}.png"))).unwrap();};
        if phase==0 {ui.set_dark_mode(true);ui.set_mode(0);ui.set_streaming(true);ui.set_window_mode(2);ui.set_measure(false);ui.set_measurement_placement_index(0);ui.set_measurement_position_index(5);}
        else if phase==1 {
            for id in 0..2 {let points=(0..=13000).map(|i|{let x=i as f32*0.01;ForgePlotPoint{x,y:if id==0{(x*3.).sin()*2.+5.}else{(x*1.5).sin()+2.},..Default::default()}}).collect::<Vec<_>>();assert!(ui.invoke_append_points(id,slint::ModelRc::new(slint::VecModel::from(points))));}ui.invoke_flush();ui.set_measure(true);
        }else {
            let cursor=(phase-2)/11;let stage=(phase-2)%11;
            if stage==0 {let scene=ui.get_scene();let rect=scene.rects.row_data(0).unwrap();start_x=if cursor==0{ui.get_cursor_a_pixel()}else{ui.get_cursor_b_pixel()};y=rect.y+if cursor==0{rect.height*0.4}else{9.};save(&format!("cursor-{cursor}-before"));let position=slint::LogicalPosition::new(ui.get_plot_offset_x()+start_x,ui.get_plot_offset_y()+y);ui.window().dispatch_event(WindowEvent::PointerMoved{position});ui.window().dispatch_event(WindowEvent::PointerPressed{position,button:PointerEventButton::Left});}
            else if stage%2==1 {let time=ui.get_scene().window_range.maximum+0.1;for id in 0..2 {assert!(ui.invoke_append_points(id,slint::ModelRc::new(slint::VecModel::from(vec![ForgePlotPoint{x:time,y:if id==0{(time*3.).sin()*2.+5.}else{(time*1.5).sin()+2.},..Default::default()}]))));}ui.invoke_flush();}
            else {let step=stage/2;let target=start_x+step as f32*if cursor==0{80.}else{-70.};let position=slint::LogicalPosition::new(ui.get_plot_offset_x()+target,ui.get_plot_offset_y()+y);ui.window().dispatch_event(WindowEvent::PointerMoved{position});let scene=ui.get_scene();let actual=if cursor==0{ui.get_cursor_a()}else{ui.get_cursor_b()};let pixel=if cursor==0{ui.get_cursor_a_pixel()}else{ui.get_cursor_b_pixel()};let other=if cursor==0{ui.get_cursor_b_pixel()}else{ui.get_cursor_a_pixel()};let expected=ui.global::<ForgePlotServices>().invoke_coordinate(scene.token,scene.measurement_axis,target,true);
                assert!((actual-expected).abs()<0.002,"实时刷新中拖动失败 cursor={cursor} step={step} actual={actual} expected={expected}");assert!((pixel-target).abs()<0.02);assert!((scene.window_range.maximum-(130.+cursor as f32*0.5+step as f32*0.1)).abs()<0.002,"光标拖动不应平移坐标轴");csv+=&format!("{cursor},{step},{},{target},{pixel},{actual},{other}\n",scene.window_range.maximum);
                if stage==10 {ui.window().dispatch_event(WindowEvent::PointerReleased{position,button:PointerEventButton::Left});save(&format!("cursor-{cursor}-after"));if cursor==1 {std::fs::write(directory.join("drag.csv"),&csv).unwrap();println!("PLOT_CURSOR_DRAG_OK host=rust cursors=2 steps=10 updates=10");slint::quit_event_loop().unwrap();}}
            }
        }
        phase+=1;
    });
}
