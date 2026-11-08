// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

use crate::ProfessionalPlotWindow;
use slint::{ComponentHandle, Model};
use std::time::Duration;

// 明暗主题下遍历全部图例位置和样式，捕获真实窗口与测量覆盖层。
pub fn start(ui: &ProfessionalPlotWindow, timer: &slint::Timer) {
    let directory = crate::performance::directory();
    std::fs::create_dir_all(&directory).unwrap();
    let weak = ui.as_weak();
    let mut phase = 0;
    let mut manifest = String::from(
        "theme,position,style,rect_x,rect_y,rect_width,rect_height,offset_x,offset_y\n",
    );
    timer.start(
        slint::TimerMode::Repeated,
        Duration::from_millis(150),
        move || {
            let ui = weak.upgrade().unwrap();
            if phase == 0 {
                ui.set_dark_mode(true);
                phase = 1;
                return;
            }
            let theme = if phase == 1 { "dark" } else { "light" };
            for position in 0..8 {
                for style in 0..3 {
                    ui.set_measure(false);
                    ui.set_legend_position_index(position);
                    ui.set_legend_style_index(style);
                    let scene = ui.get_scene();
                    assert_eq!(scene.status, 0, "{}", scene.error);
                    let rect = scene.rects.row_data(0).unwrap();
                    let name = format!("{theme}-legend-{position}-{style}");
                    std::fs::write(directory.join(format!("{name}.svg")), scene.svg.as_bytes())
                        .unwrap();
                    let image = slintforge::capture_window(ui.window()).unwrap();
                    slintforge::save_capture_png(&image, &directory.join(format!("{name}.png")))
                        .unwrap();
                    manifest += &format!(
                        "{theme},{position},{style},{},{},{},{},{},{}\n",
                        rect.x,
                        rect.y,
                        rect.width,
                        rect.height,
                        ui.get_plot_offset_x(),
                        ui.get_plot_offset_y()
                    );
                }
            }
            ui.set_legend_position_index(4);
            ui.set_legend_style_index(2);
            let image = slintforge::capture_window(ui.window()).unwrap();
            slintforge::save_capture_png(
                &image,
                &directory.join(format!("measure-{theme}-off.png")),
            )
            .unwrap();
            ui.set_measure(true);
            let image = slintforge::capture_window(ui.window()).unwrap();
            slintforge::save_capture_png(&image, &directory.join(format!("measure-{theme}.png")))
                .unwrap();
            ui.set_legend_position_index(0);
            let image = slintforge::capture_window(ui.window()).unwrap();
            slintforge::save_capture_png(
                &image,
                &directory.join(format!("measure-{theme}-bottom.png")),
            )
            .unwrap();
            if phase == 1 {
                ui.set_dark_mode(false);
                phase = 2;
                return;
            }
            std::fs::write(directory.join("appearance.csv"), &manifest).unwrap();
            println!("PLOT_APPEARANCE_OK host=rust themes=2 positions=8 styles=3 screenshots=54");
            slint::quit_event_loop().unwrap();
        },
    );
}
