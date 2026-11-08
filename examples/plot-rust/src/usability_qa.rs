// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

use crate::{ForgePlotPoint, ProfessionalPlotWindow};
use slint::{ComponentHandle, Model};
use std::time::Duration;

// 实际展开并点击菜单；用累计采样验证超过 130 秒后的光标位置。
pub fn start(ui: &ProfessionalPlotWindow, timer: &slint::Timer) {
    let directory = crate::performance::directory();
    std::fs::create_dir_all(&directory).unwrap();
    let weak = ui.as_weak();
    let mut phase = 0;
    let mut fraction = 0.;
    let mut csv = String::from("time,minimum,maximum,cursor_a,cursor_b,pixel_a,pixel_b,rect_x,rect_width,measure_width,measure_height,input_points\n");
    timer.start(
        slint::TimerMode::Repeated,
        Duration::from_millis(150),
        move || {
            let ui = weak.upgrade().unwrap();
            let save = |name: &str| {
                let pixels = slintforge::capture_window(ui.window()).unwrap();
                slintforge::save_capture_png(&pixels, &directory.join(format!("{name}.png")))
                    .unwrap();
            };
            let click = |legend: bool, index: i32| {
                assert!(if legend {
                    ui.get_legend_menu_open()
                } else {
                    ui.get_measurement_menu_open()
                });
                let (x, y) = if legend {
                    (ui.get_legend_picker_x(), ui.get_legend_picker_y())
                } else {
                    (ui.get_measurement_picker_x(), ui.get_measurement_picker_y())
                };
                let position =
                    slint::LogicalPosition::new(x + 30., y + 42. + index as f32 * 32. + 16.);
                ui.window()
                    .dispatch_event(slint::platform::WindowEvent::PointerMoved { position });
                ui.window()
                    .dispatch_event(slint::platform::WindowEvent::PointerPressed {
                        position,
                        button: slint::platform::PointerEventButton::Left,
                    });
                ui.window()
                    .dispatch_event(slint::platform::WindowEvent::PointerReleased {
                        position,
                        button: slint::platform::PointerEventButton::Left,
                    });
            };
            let append = |first: i32, last: i32| {
                for id in 0..2 {
                    let points = (first..=last)
                        .map(|i| {
                            let x = i as f32 * 0.01;
                            ForgePlotPoint {
                                x,
                                y: if id == 0 {
                                    (x * 3.).sin() * 2. + 5.
                                } else {
                                    (x * 1.5).sin() + 2.
                                },
                                ..Default::default()
                            }
                        })
                        .collect::<Vec<_>>();
                    assert!(ui.invoke_append_points(
                        id,
                        slint::ModelRc::new(slint::VecModel::from(points))
                    ));
                }
                ui.invoke_flush();
            };
            match phase {
                0 => {
                    ui.set_dark_mode(true);
                    ui.set_measure(true);
                }
                1 => ui.invoke_show_legend_positions(),
                2 => {
                    save("legend-menu-left-top");
                    click(true, 6);
                }
                3 => {
                    assert_eq!(ui.get_legend_position_index(), 6);
                    save("legend-left-top");
                    ui.invoke_show_legend_positions();
                }
                4 => {
                    save("legend-menu-left-bottom");
                    click(true, 7);
                }
                5 => {
                    assert_eq!(ui.get_legend_position_index(), 7);
                    save("legend-left-bottom");
                    ui.invoke_show_measurement_positions();
                }
                6 => {
                    save("measure-menu-left-top");
                    click(false, 6);
                }
                7 => {
                    assert_eq!(ui.get_measurement_position_index(), 6);
                    save("measure-left-top");
                    ui.invoke_show_measurement_positions();
                }
                8 => {
                    save("measure-menu-left-bottom");
                    click(false, 7);
                }
                9 => {
                    assert_eq!(ui.get_measurement_position_index(), 7);
                    assert!(!ui.get_legend_menu_open() && !ui.get_measurement_menu_open());
                    save("measure-left-bottom");
                    ui.set_measure(false);
                    ui.set_mode(0);
                    ui.invoke_reset();
                    ui.set_streaming(true);
                    ui.set_window_mode(2);
                    ui.set_legend_placement_index(1);
                    ui.set_legend_position_index(1);
                    ui.set_measurement_placement_index(0);
                    ui.set_measurement_position_index(5);
                }
                10 => {
                    append(0, 13000);
                    ui.set_measure(true);
                }
                11..=13 => {
                    let time = [130., 140., 170.][(phase - 11) as usize];
                    let scene = ui.get_scene();
                    let rect = scene.rects.row_data(0).unwrap();
                    let range = scene.measurement_range;
                    assert_eq!(scene.status, 0, "{}", scene.error);
                    assert!((range.maximum - time).abs() < 0.001);
                    assert!((range.maximum - range.minimum - 10.).abs() < 0.001);
                    for (value, pixel) in [
                        (ui.get_cursor_a(), ui.get_cursor_a_pixel()),
                        (ui.get_cursor_b(), ui.get_cursor_b_pixel()),
                    ] {
                        assert!(value > range.minimum && value < range.maximum);
                        assert!(pixel > rect.x && pixel < rect.x + rect.width);
                    }
                    let current =
                        (ui.get_cursor_a() - range.minimum) / (range.maximum - range.minimum);
                    if phase == 11 {
                        fraction = current;
                    } else {
                        assert!((current - fraction).abs() < 0.001);
                    }
                    assert!(
                        scene.measurement_rect.width <= 260.
                            && scene.measurement_rect.height <= 80.
                    );
                    assert_eq!(scene.input_points, (time as i32 * 100 + 1) * 2);
                    csv += &format!(
                        "{time},{},{},{},{},{},{},{},{},{},{},{}\n",
                        range.minimum,
                        range.maximum,
                        ui.get_cursor_a(),
                        ui.get_cursor_b(),
                        ui.get_cursor_a_pixel(),
                        ui.get_cursor_b_pixel(),
                        rect.x,
                        rect.width,
                        scene.measurement_rect.width,
                        scene.measurement_rect.height,
                        scene.input_points
                    );
                    save(&format!("live-{time}"));
                    if phase == 11 {
                        append(13001, 14000);
                    } else if phase == 12 {
                        append(14001, 17000);
                    } else {
                        std::fs::write(directory.join("usability.csv"), &csv).unwrap();
                        println!(
                            "PLOT_USABILITY_OK host=rust menu_clicks=4 time=170 points={}",
                            scene.input_points
                        );
                        slint::quit_event_loop().unwrap();
                    }
                }
                _ => unreachable!(),
            }
            phase += 1;
        },
    );
}
