// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

use crate::ProfessionalPlotWindow;
use slint::{ComponentHandle, Model};
use std::time::Duration;

// 验证真实窗口中两个信息框的区域与位置，并保留布局元数据和截图。
pub fn start(ui: &ProfessionalPlotWindow, timer: &slint::Timer) {
    let directory = crate::performance::directory();
    std::fs::create_dir_all(&directory).unwrap();
    let weak = ui.as_weak();
    let mut phase = 0;
    let mut manifest = String::from("name,legend_area,measure_area,position,panel,rect_x,rect_y,rect_width,rect_height,legend_x,legend_y,legend_width,legend_height,measure_x,measure_y,measure_width,measure_height,offset_x,offset_y\n");
    let mut overlay = String::from("name,legend_position,measure_position,rect_x,rect_y,rect_width,rect_height,legend_x,legend_y,legend_width,legend_height,measure_x,measure_y,measure_width,measure_height,offset_x,offset_y\n");
    timer.start(slint::TimerMode::Repeated, Duration::from_millis(150), move || {
        let ui = weak.upgrade().unwrap();
        if phase == 0 { ui.set_dark_mode(true); phase = 1; return; }
        let theme = if phase == 1 { "dark" } else { "light" };
        // 左上/左下交叉放置，开关测量框时保持相同波形，验证透明底色。
        ui.set_mode(0);ui.invoke_reset();ui.set_measurement_series(0);
        ui.set_legend_placement_index(0);ui.set_measurement_placement_index(0);ui.set_legend_style_index(1);
        for position in [6, 7] {
            let measure_position=if position==6 {7} else {6};
            ui.set_legend_position_index(position);ui.set_measurement_position_index(measure_position);ui.set_measure(true);
            let scene=ui.get_scene();assert_eq!(scene.status,0,"{}",scene.error);
            let rect=scene.rects.row_data(0).unwrap();let l=scene.legend_rect;let m=scene.measurement_rect;
            let name=format!("{theme}-corner-{position}");
            std::fs::write(directory.join(format!("{name}.svg")),scene.svg.as_bytes()).unwrap();
            let image=slintforge::capture_window(ui.window()).unwrap();slintforge::save_capture_png(&image,&directory.join(format!("{name}-on.png"))).unwrap();
            ui.set_measure(false);assert_eq!(ui.get_scene().svg,scene.svg,"透明框对比必须使用同一波形");
            let image=slintforge::capture_window(ui.window()).unwrap();slintforge::save_capture_png(&image,&directory.join(format!("{name}-off.png"))).unwrap();
            overlay+=&format!("{name},{position},{measure_position},{},{},{},{},{},{},{},{},{},{},{},{},{},{}\n",rect.x,rect.y,rect.width,rect.height,l.x,l.y,l.width,l.height,m.x,m.y,m.width,m.height,ui.get_plot_offset_x(),ui.get_plot_offset_y());
        }
        ui.set_measure(true);
        for mode in [0, 9] {
            ui.set_mode(mode);ui.invoke_reset();ui.set_measurement_series(if mode == 9 {1} else {0});
            for legend_area in 0..2 {for measure_area in 0..2 {for position in 0..8 {
                if mode == 9 && (legend_area != 1 || measure_area != 1 || position < 2 || position > 3) {continue;}
                ui.set_legend_placement_index(legend_area);ui.set_measurement_placement_index(measure_area);
                ui.set_legend_position_index(position);ui.set_measurement_position_index(position);ui.set_legend_style_index(position % 3);
                let scene = ui.get_scene();assert_eq!(scene.status, 0, "{}", scene.error);
                if mode == 0 {assert!(!scene.svg.contains("data:image/png"), "默认示例仍有红色图片");}
                let rect = scene.rects.row_data(if mode == 9 {1} else {0}).unwrap();let l=scene.legend_rect;let m=scene.measurement_rect;
                assert_eq!(m.id, rect.id);
                let name=format!("{theme}-{mode}-{legend_area}-{measure_area}-{position}");
                std::fs::write(directory.join(format!("{name}.svg")), scene.svg.as_bytes()).unwrap();
                let image=slintforge::capture_window(ui.window()).unwrap();slintforge::save_capture_png(&image,&directory.join(format!("{name}.png"))).unwrap();
                manifest+=&format!("{name},{legend_area},{measure_area},{position},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{}\n",rect.id,rect.x,rect.y,rect.width,rect.height,l.x,l.y,l.width,l.height,m.x,m.y,m.width,m.height,ui.get_plot_offset_x(),ui.get_plot_offset_y());
            }}}
        }
        if phase == 1 {ui.set_dark_mode(false);phase=2;return;}
        std::fs::write(directory.join("placement.csv"),manifest.as_bytes()).unwrap();
        std::fs::write(directory.join("overlay.csv"),overlay.as_bytes()).unwrap();
        ui.set_mode(0);ui.invoke_reset();ui.set_measurement_series(0);ui.set_legend_placement_index(1);ui.set_measurement_placement_index(1);ui.set_legend_position_index(0);ui.set_measurement_position_index(0);
        crate::streaming_qa::performance(&ui,&directory);
        println!("PLOT_PLACEMENT_WINDOW_OK host=rust themes=2 positions=8 placements=4 screenshots=76 transparent_corners=4");
        slint::quit_event_loop().unwrap();
    });
}
