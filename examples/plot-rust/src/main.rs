// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

slint::include_modules!();
mod appearance_qa;
mod drag_qa;
mod performance;
mod placement_qa;
mod streaming_qa;
mod usability_qa;
use slint::{ComponentHandle, Model};
use std::time::{Duration, Instant};
fn benchmark(ui: &ProfessionalPlotWindow) -> Result<(), Box<dyn std::error::Error>> {
    use std::time::Instant;
    let directory = performance::directory();
    std::fs::create_dir_all(&directory)?;
    let mut csv=String::from("points,model_ms,prepare_ms,capture_ms,pan_prepare_ms,zoom_prepare_ms,zoom_capture_ms,rendered,svg_bytes\n");
    for count in [100000, 1000000] {
        let start = Instant::now();
        let points = (0..count)
            .map(|index| ForgePlotPoint {
                x: index as f32,
                y: if index == count / 3 {
                    1000.
                } else {
                    (index as f32 * 0.003).sin()
                },
                ..Default::default()
            })
            .collect::<Vec<_>>();
        ui.set_benchmark_points(slint::ModelRc::new(slint::VecModel::from(points)));
        let model_ms = start.elapsed().as_secs_f64() * 1000.;
        let start = Instant::now();
        ui.invoke_refresh();
        let scene = ui.get_scene();
        let prepare_ms = start.elapsed().as_secs_f64() * 1000.;
        assert_eq!(scene.status, 0, "{}", scene.error);
        assert_eq!(scene.input_points, count);
        assert!(scene.rendered_points < 5000);
        let start = Instant::now();
        let capture = slintforge::capture_window(ui.window())?;
        let capture_ms = start.elapsed().as_secs_f64() * 1000.;
        slintforge::save_capture_png(&capture, &directory.join(format!("benchmark-{count}.png")))?;
        let start = Instant::now();
        ui.invoke_benchmark_pan(12., 0.);
        let _ = ui.get_scene();
        let pan_ms = start.elapsed().as_secs_f64() * 1000.;
        let start = Instant::now();
        ui.invoke_benchmark_zoom();
        let _ = ui.get_scene();
        let zoom_ms = start.elapsed().as_secs_f64() * 1000.;
        let start = Instant::now();
        let _ = slintforge::capture_window(ui.window())?;
        let zoom_capture_ms = start.elapsed().as_secs_f64() * 1000.;
        csv+=&format!("{count},{model_ms},{prepare_ms},{capture_ms},{pan_ms},{zoom_ms},{zoom_capture_ms},{},{}\n",scene.rendered_points,scene.svg_bytes);
        println!("PLOT_WINDOW_PERF host=rust points={count} prepare_ms={prepare_ms:.3} capture_ms={capture_ms:.3} pan_ms={pan_ms:.3} zoom_ms={zoom_ms:.3}");
        ui.set_benchmark_points(Default::default());
        ui.invoke_reset();
        let _ = ui.get_scene();
    }
    std::fs::write(directory.join("window-performance.csv"), csv)?;
    println!("PLOT_WINDOW_BENCHMARK_OK host=rust");
    Ok(())
}
fn main() -> Result<(), Box<dyn std::error::Error>> {
    slintforge::configure_rendering(Default::default())?;
    let ui = ProfessionalPlotWindow::new()?;
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
    let sampler = std::rc::Rc::new(slint::Timer::default());
    let timer = sampler.clone();
    let clock = std::rc::Rc::new(std::cell::Cell::new(0f64));
    let weak = ui.as_weak();
    ui.on_sample_running(move |running| {
        if !running {
            timer.stop();
            return;
        }
        let clock = clock.clone();
        let base = clock.get();
        let started = Instant::now();
        let weak = weak.clone();
        timer.start(
            slint::TimerMode::Repeated,
            Duration::from_millis(33),
            move || {
                if let Some(ui) = weak.upgrade() {
                    let target = base + started.elapsed().as_secs_f64();
                    let mut time = clock.get();
                    let mut measured = Vec::new();
                    let mut reference = Vec::new();
                    while time + 0.01 <= target {
                        time += 0.01;
                        measured.push(ForgePlotPoint {
                            x: time as f32,
                            y: ((time * 3.).sin() * 2. + 5.) as f32,
                            ..Default::default()
                        });
                        reference.push(ForgePlotPoint {
                            x: time as f32,
                            y: ((time * 1.5).sin() + 2.) as f32,
                            ..Default::default()
                        });
                    }
                    if !measured.is_empty() {
                        ui.invoke_append_points(
                            0,
                            slint::ModelRc::new(slint::VecModel::from(measured)),
                        );
                        ui.invoke_append_points(
                            1,
                            slint::ModelRc::new(slint::VecModel::from(reference)),
                        );
                        clock.set(time);
                    }
                }
            },
        );
    });
    let smoke = slint::Timer::default();
    if std::env::args().any(|a| a == "--appearance-smoke") {
        appearance_qa::start(&ui, &smoke);
    }
    if std::env::args().any(|a| a == "--placement-smoke") {
        placement_qa::start(&ui, &smoke);
    }
    if std::env::args().any(|a| a == "--usability-smoke") {
        usability_qa::start(&ui, &smoke);
    }
    if std::env::args().any(|a| a == "--cursor-drag-smoke") {
        drag_qa::start(&ui, &smoke);
    }
    if std::env::args().any(|a| a == "--streaming-smoke") {
        streaming_qa::start(&ui, &smoke);
    }
    if std::env::args().any(|a| a == "--interaction-benchmark") {
        let weak = ui.as_weak();
        smoke.start(
            slint::TimerMode::SingleShot,
            Duration::from_millis(100),
            move || {
                performance::interaction(&weak.upgrade().unwrap()).unwrap();
                slint::quit_event_loop().unwrap();
            },
        );
    }
    if std::env::args().any(|a| a == "--benchmark") {
        let weak = ui.as_weak();
        smoke.start(
            slint::TimerMode::SingleShot,
            Duration::from_millis(100),
            move || {
                benchmark(&weak.upgrade().unwrap()).unwrap();
                slint::quit_event_loop().unwrap();
            },
        );
    }
    if std::env::args().any(|a| a == "--smoke") {
        let directory = performance::directory();
        std::fs::create_dir_all(&directory)?;
        let weak = ui.as_weak();
        let mut mode = 0;
        smoke.start(
            slint::TimerMode::Repeated,
            Duration::from_millis(200),
            move || {
                let ui = weak.upgrade().unwrap();
                let scene = ui.get_scene();
                assert_eq!(scene.status, 0, "mode={mode} {}", scene.error);
                assert!(scene.input_points > 0);
                assert!(scene.rects.row_count() > 0);
                let pixels = slintforge::capture_window(ui.window()).unwrap();
                slintforge::save_capture_png(&pixels, &directory.join(format!("mode-{mode}.png")))
                    .unwrap();
                if mode == 0 {
                    for format in ["png", "jpeg", "bmp", "svg", "pdf"] {
                        let path = directory.join(format!("专业图表.{format}"));
                        let result = ui.invoke_export_file(
                            path.to_string_lossy().as_ref().into(),
                            format.into(),
                        );
                        assert!(result.ok, "{}", result.error);
                    }
                }
                for format in ["svg", "pdf"] {
                    let path = directory.join(format!("mode-{mode}.{format}"));
                    let result = ui
                        .invoke_export_file(path.to_string_lossy().as_ref().into(), format.into());
                    assert!(result.ok, "{}", result.error);
                }
                println!(
                    "PLOT_CONSUMER_FRAME host=rust mode={mode} points={} rendered={}",
                    scene.input_points, scene.rendered_points
                );
                ui.invoke_benchmark_pan(12., 0.);
                ui.invoke_benchmark_zoom();
                mode += 1;
                if mode == 10 {
                    println!("PLOT_CONSUMER_OK host=rust");
                    slint::quit_event_loop().unwrap();
                } else {
                    ui.set_mode(mode);
                }
            },
        );
    }
    ui.run()?;
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    fn tick() {
        i_slint_backend_testing::mock_elapsed_time(Duration::from_millis(20));
        slint::platform::update_timers_and_animations();
    }
    #[test]
    fn position_menu_last_two_items_can_be_clicked_and_panel_axis_is_correct() {
        use slint::platform::{PointerEventButton, WindowEvent};
        i_slint_backend_testing::init_no_event_loop();
        let ui = ProfessionalPlotWindow::new().unwrap();
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
        ui.set_measure(true);
        tick();
        for menu in 0..2 {
            for position in [6, 7] {
                let (x, y) = if menu == 0 {
                    ui.invoke_show_legend_positions();
                    (ui.get_legend_picker_x(), ui.get_legend_picker_y())
                } else {
                    ui.invoke_show_measurement_positions();
                    (ui.get_measurement_picker_x(), ui.get_measurement_picker_y())
                };
                tick();
                assert!(if menu == 0 {
                    ui.get_legend_menu_open()
                } else {
                    ui.get_measurement_menu_open()
                });
                let target = slint::LogicalPosition::new(
                    x + 30.,
                    y + 34. + 4. + 4. + position as f32 * 32. + 16.,
                );
                ui.window()
                    .dispatch_event(WindowEvent::PointerMoved { position: target });
                ui.window().dispatch_event(WindowEvent::PointerPressed {
                    position: target,
                    button: PointerEventButton::Left,
                });
                ui.window().dispatch_event(WindowEvent::PointerReleased {
                    position: target,
                    button: PointerEventButton::Left,
                });
                tick();
                assert_eq!(
                    if menu == 0 {
                        ui.get_legend_position_index()
                    } else {
                        ui.get_measurement_position_index()
                    },
                    position,
                    "必须点击实际弹出菜单的最后两行"
                );
                assert!(!ui.get_legend_menu_open() && !ui.get_measurement_menu_open());
            }
        }
        ui.set_mode(9);
        ui.invoke_reset();
        ui.set_measurement_series(1);
        tick();
        let scene = ui.get_scene();
        assert_eq!(scene.measurement_axis, 2);
        ui.invoke_benchmark_pan(40., 0.);
        tick();
        let scene = ui.get_scene();
        let service = ui.global::<ForgePlotServices>();
        assert!(
            (ui.get_cursor_a_pixel()
                - service.invoke_coordinate(scene.token, 2, ui.get_cursor_a(), false))
            .abs()
                < 0.001,
            "光标应使用所选曲线的坐标轴"
        );
        assert!(
            (ui.get_cursor_a_pixel()
                - service.invoke_coordinate(scene.token, 0, ui.get_cursor_a(), false))
            .abs()
                > 1.,
            "两个绘图区的独立视图应可区分"
        );
    }
    #[test]
    fn incremental_history_survives_view_and_legend_changes() {
        i_slint_backend_testing::init_no_event_loop();
        let ui = ProfessionalPlotWindow::new().unwrap();
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
        ui.set_streaming(true);
        ui.set_window_mode(1);
        tick();
        for id in 0..2 {
            let points = (0..10000)
                .map(|i| ForgePlotPoint {
                    x: i as f32 * 0.01,
                    y: if id == 0 {
                        5. + (i as f32 * 0.03).sin()
                    } else {
                        2.
                    },
                    ..Default::default()
                })
                .collect::<Vec<_>>();
            assert!(ui.invoke_append_points(id, slint::ModelRc::new(slint::VecModel::from(points))));
        }
        ui.invoke_flush();
        let scene = ui.get_scene();
        assert_eq!(scene.status, 0, "{}", scene.error);
        assert_eq!(scene.input_points, 20000);
        assert!((scene.window_range.maximum - scene.window_range.minimum - 10.).abs() < 0.001);
        assert!(scene.history_range.maximum > 99.);
        ui.set_legend_position_index(4);
        ui.set_legend_style_index(1);
        tick();
        assert_eq!(ui.get_scene().input_points, 20000, "图例配置不应清空历史");
        ui.set_measure(true);
        ui.set_legend_placement_index(1);
        ui.set_measurement_position_index(2);
        tick();
        assert_eq!(
            ui.get_scene().input_points,
            20000,
            "区域内外布局切换不能清空追加历史"
        );
        ui.set_measurement_placement_index(0);
        tick();
        assert_eq!(ui.get_scene().input_points, 20000);
        ui.set_measure(false);
        ui.invoke_scroll_history(0.);
        let before = ui.get_scene();
        assert!(before.window_range.minimum.abs() < 0.001);
        assert!(ui.invoke_append_points(
            0,
            slint::ModelRc::new(slint::VecModel::from(vec![ForgePlotPoint {
                x: 100.,
                y: 6.,
                ..Default::default()
            }]))
        ));
        ui.invoke_flush();
        let after = ui.get_scene();
        assert_eq!(after.input_points, 20001);
        assert!(
            (after.window_range.minimum - before.window_range.minimum).abs() < 0.001,
            "查看历史时追加不应强制跟随"
        );
        ui.invoke_reset();
        tick();
        let scene = ui.get_scene();
        assert_eq!(scene.input_points, 20001);
        assert!((scene.window_range.maximum - 100.).abs() < 0.001);
        let measure = ui
            .global::<ForgePlotServices>()
            .invoke_measure(scene.token, 0, 0.01);
        assert_eq!(measure.index, 1);
        assert!((measure.x - 0.01).abs() < 0.00001, "测量必须使用原始点");
    }
    #[test]
    fn demo_preserves_legend_on_data_updates_and_resets_axes_on_mode_switch() {
        i_slint_backend_testing::init_no_event_loop();
        let ui = ProfessionalPlotWindow::new().unwrap();
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
        let scene = ui.get_scene();
        assert_eq!(scene.status, 0);
        let svg = scene.svg.as_str();
        let end = svg.find(">测量 / 不对称误差").unwrap();
        let start = svg[..end].rfind("<text ").unwrap();
        let attributes = &svg[start..end];
        let x = attributes
            .split("x=\"")
            .nth(1)
            .unwrap()
            .split('"')
            .next()
            .unwrap()
            .parse::<f32>()
            .unwrap();
        let y = attributes
            .split("y=\"")
            .nth(1)
            .unwrap()
            .split('"')
            .next()
            .unwrap()
            .parse::<f32>()
            .unwrap();
        let position = slint::LogicalPosition::new(
            ui.get_plot_offset_x() + x + 4.,
            ui.get_plot_offset_y() + y - 4.,
        );
        for event in [
            slint::platform::WindowEvent::PointerMoved { position },
            slint::platform::WindowEvent::PointerPressed {
                position,
                button: slint::platform::PointerEventButton::Left,
            },
            slint::platform::WindowEvent::PointerReleased {
                position,
                button: slint::platform::PointerEventButton::Left,
            },
        ] {
            ui.window().dispatch_event(event);
        }
        tick();
        assert_eq!(ui.get_scene().rendered_points, 6, "图例应隐藏第一个系列");
        ui.set_live_points(slint::ModelRc::new(slint::VecModel::from(
            (0..20)
                .map(|i| ForgePlotPoint {
                    x: i as f32,
                    y: 5.,
                    ..Default::default()
                })
                .collect::<Vec<_>>(),
        )));
        tick();
        ui.invoke_refresh();
        tick();
        assert_eq!(ui.get_scene().input_points, 26);
        assert_eq!(
            ui.get_scene().rendered_points,
            6,
            "实时更新应保留图例隐藏状态"
        );
        ui.invoke_benchmark_zoom();
        tick();
        ui.set_mode(7);
        tick();
        assert_eq!(ui.get_scene().status, 0);
        assert_eq!(
            ui.get_scene().ranges.row_count(),
            3,
            "模式切换应恢复多轴配置"
        );
        ui.invoke_benchmark_zoom();
        tick();
        ui.set_mode(9);
        tick();
        assert_eq!(ui.get_scene().status, 0);
        assert_eq!(ui.get_scene().rects.row_count(), 2);
        assert_eq!(ui.get_scene().ranges.row_count(), 4);
    }
}
