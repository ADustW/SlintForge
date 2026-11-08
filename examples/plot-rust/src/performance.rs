// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

use crate::{ForgePlotPoint, ProfessionalPlotWindow};
use slint::platform::{PointerEventButton, WindowEvent};
use slint::{ComponentHandle, LogicalPosition, Model, ModelRc, VecModel};
use std::time::Instant;

pub fn directory() -> std::path::PathBuf {
    std::env::var_os("SLINTFORGE_PLOT_QA_DIR")
        .map(std::path::PathBuf::from)
        .unwrap_or_else(|| std::path::PathBuf::from("build/plot-qa/rust-consumer"))
}

// 使用真实窗口事件与完整截图，覆盖鼠标悬停、拖动、滚轮和实时模型更新。
pub fn interaction(ui: &ProfessionalPlotWindow) -> Result<(), Box<dyn std::error::Error>> {
    let directory = directory();
    std::fs::create_dir_all(&directory)?;
    let mut csv = String::from("points,operation,frame,event_ms,prepare_ms,capture_ms,total_ms\n");
    for count in [12, 2000] {
        ui.set_live_points(ModelRc::default());
        if count > 12 {
            ui.set_live_points(points(count, 0));
        }
        ui.invoke_reset();
        let scene = ui.get_scene();
        assert_eq!(scene.status, 0, "{}", scene.error);
        let stage = Instant::now();
        let vector = slint::Image::load_from_svg_data(scene.svg.as_bytes())?;
        let parse_ms = stage.elapsed().as_secs_f64() * 1000.;
        let stage = Instant::now();
        let _ = vector.to_rgba8_premultiplied().ok_or("图像栅格化失败")?;
        println!(
            "PLOT_RASTER_STAGE points={count} svg_bytes={} parse_ms={parse_ms:.3} raster_ms={:.3}",
            scene.svg_bytes,
            stage.elapsed().as_secs_f64() * 1000.
        );
        let _ = slintforge::capture_window(ui.window())?;
        let rect = scene.rects.row_data(0).unwrap();
        let x = ui.get_plot_offset_x() + rect.x + rect.width * 0.5;
        let y = ui.get_plot_offset_y() + rect.y + rect.height * 0.5;
        for operation in ["hover", "drag", "wheel", "live"] {
            let mut totals = Vec::new();
            if operation == "drag" {
                ui.window().dispatch_event(WindowEvent::PointerPressed {
                    position: LogicalPosition::new(x, y),
                    button: PointerEventButton::Left,
                });
            }
            for frame in 0..24 {
                let start = Instant::now();
                let position = LogicalPosition::new(x + frame as f32 * 0.8, y);
                match operation {
                    "hover" | "drag" => {
                        ui.window()
                            .dispatch_event(WindowEvent::PointerMoved { position });
                    }
                    "wheel" => ui.window().dispatch_event(WindowEvent::PointerScrolled {
                        position,
                        delta_x: 0.,
                        delta_y: if frame % 2 == 0 { 50. } else { -50. },
                    }),
                    _ => {
                        ui.set_live_points(points(count, frame));
                    }
                }
                let event_ms = start.elapsed().as_secs_f64() * 1000.;
                let start_prepare = Instant::now();
                let scene = ui.get_scene();
                assert_eq!(scene.status, 0, "{}", scene.error);
                let prepare_ms = start_prepare.elapsed().as_secs_f64() * 1000.;
                let start_capture = Instant::now();
                let pixels = slintforge::capture_window(ui.window())?;
                let capture_ms = start_capture.elapsed().as_secs_f64() * 1000.;
                let total_ms = start.elapsed().as_secs_f64() * 1000.;
                totals.push(total_ms);
                csv += &format!("{count},{operation},{frame},{event_ms:.4},{prepare_ms:.4},{capture_ms:.4},{total_ms:.4}\n");
                if frame == 23 {
                    slintforge::save_capture_png(
                        &pixels,
                        &directory.join(format!("interaction-{count}-{operation}.png")),
                    )?;
                }
            }
            if operation == "drag" {
                ui.window().dispatch_event(WindowEvent::PointerReleased {
                    position: LogicalPosition::new(x + 23. * 0.8, y),
                    button: PointerEventButton::Left,
                });
            }
            ui.invoke_reset();
            totals.sort_by(f64::total_cmp);
            println!("PLOT_INTERACTION_PERF host=rust points={count} operation={operation} median_ms={:.3} p95_ms={:.3}", totals[12], totals[22]);
        }
    }
    std::fs::write(directory.join("interaction-performance.csv"), csv)?;
    println!("PLOT_INTERACTION_BENCHMARK_OK host=rust");
    Ok(())
}

fn points(count: usize, frame: usize) -> ModelRc<ForgePlotPoint> {
    ModelRc::new(VecModel::from(
        (0..count)
            .map(|index| ForgePlotPoint {
                x: index as f32 * 0.01,
                y: ((index + frame) as f32 * 0.03).sin() * 2. + 5.,
                ..Default::default()
            })
            .collect::<Vec<_>>(),
    ))
}
