// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

slint::include_modules!();
use slint::ComponentHandle;

mod business;
#[cfg(test)]
mod business_tests;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    slintforge::configure_rendering(Default::default())?;
    if std::env::args().any(|arg| arg == "--suggestion-demo" || arg == "--suggestion-smoke") {
        return business::suggestion(std::env::args().any(|arg| arg == "--suggestion-smoke"));
    }
    if std::env::args().any(|arg| arg == "--dialog-demo" || arg == "--dialog-smoke") {
        return business::dialog(std::env::args().any(|arg| arg == "--dialog-smoke"));
    }
    if std::env::args().any(|arg| arg == "--grids-smoke") {
        return grids_smoke();
    }
    if std::env::args().any(|arg| arg == "--charts-smoke") {
        return charts_smoke();
    }
    if std::env::args().any(|arg| arg == "--widgets-smoke") {
        return widgets_smoke();
    }
    let ui = MinimalWindow::new()?;
    slintforge::bind_capture_service!(ui, ForgeCaptureService, ForgeCaptureResult);
    ui.on_export_capture(|image| {
        let result = (|| {
            let pixels = image.to_rgba8().ok_or("无法读取截图像素")?;
            let directory = std::env::current_dir()?.join("build/captures");
            std::fs::create_dir_all(&directory)?;
            let stamp = std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH)?
                .as_nanos();
            let path = directory.join(format!("consumer-rust-{stamp}.png"));
            slintforge::save_capture_png(&pixels, &path)?;
            Ok::<_, Box<dyn std::error::Error>>(path.display().to_string())
        })();
        result.unwrap_or_else(|error| error.to_string()).into()
    });
    let timer = slint::Timer::default();
    if std::env::args().any(|arg| arg == "--capture-smoke") {
        let weak = ui.as_weak();
        timer.start(
            slint::TimerMode::SingleShot,
            std::time::Duration::from_millis(250),
            move || {
                let ui = weak.upgrade().unwrap();
                assert!(ui.invoke_qa_capture_window().ok);
                let result = ui.invoke_qa_capture_page();
                assert!(result.ok, "{}", result.error);
                let path = ui.invoke_export_capture(result.image);
                assert!(std::path::Path::new(path.as_str()).exists());
                println!("CONSUMER_CAPTURE_OK host=rust path={path}");
                slint::quit_event_loop().unwrap();
            },
        );
    }
    ui.run()?;
    Ok(())
}

fn charts_smoke() -> Result<(), Box<dyn std::error::Error>> {
    let ui = ChartsWindow::new()?;
    slintforge::bind_plot_services!(ui, ForgePlotServices, ForgePlotScene, ForgePlotAxis, ForgePlotRange, ForgePlotRect, ForgePlotHit, ForgePlotSelection, ForgePlotExportResult);
    let directory = std::path::Path::new("build/charts-qa");
    std::fs::create_dir_all(directory)?;
    let timer = slint::Timer::default();
    let weak = ui.as_weak();
    let phase = std::rc::Rc::new(std::cell::Cell::new(0));
    timer.start(
        slint::TimerMode::Repeated,
        std::time::Duration::from_millis(120),
        move || {
            let ui = weak.upgrade().unwrap();
            let index = phase.get();
            if index % 2 == 0 {
                ui.set_kind(index / 2);
            } else {
                let kind = index / 2;
                assert_eq!(ui.get_status(), 0);
                assert_eq!(
                    ui.get_mark_count(),
                    if kind == 2 || kind == 3 { 2 } else { 6 }
                );
                slintforge::save_capture_png(
                    &slintforge::capture_window(ui.window()).unwrap(),
                    directory.join(format!("consumer-rust-{kind}.png")),
                )
                .unwrap();
                if kind == 6 {
                    println!("CONSUMER_CHARTS_OK host=rust kinds=7");
                    slint::quit_event_loop().unwrap();
                }
            }
            phase.set(index + 1);
        },
    );
    ui.run()?;
    Ok(())
}

fn grids_smoke() -> Result<(), Box<dyn std::error::Error>> {
    let ui = GridsWindow::new()?;
    slintforge::bind_grid_services!(
        ui,
        ForgeGridServices,
        ForgeGridScene,
        ForgeGridViewRow,
        ForgeGridViewColumn,
        ForgeGridRow,
        ForgeGridCell,
        ForgeGridColumn,
        ForgeGridEditResult
    );
    let directory = std::path::Path::new("build/grids-qa");
    std::fs::create_dir_all(directory)?;
    let timer = slint::Timer::default();
    let weak = ui.as_weak();
    let phase = std::rc::Rc::new(std::cell::Cell::new(0));
    timer.start(
        slint::TimerMode::Repeated,
        std::time::Duration::from_millis(140),
        move || {
            let ui = weak.upgrade().unwrap();
            let index = phase.get();
            let mode = index / 2;
            if index % 2 == 0 {
                ui.invoke_prepare(mode);
            } else {
                assert_eq!(ui.get_status(), if mode == 2 { 1 } else { 0 });
                assert_eq!(
                    ui.get_visible_count(),
                    if mode == 2 {
                        0
                    } else if mode == 6 {
                        2
                    } else {
                        3
                    }
                );
                if mode == 3 {
                    assert_eq!(ui.get_selected_count(), 0);
                }
                if mode == 4 {
                    assert_eq!(ui.get_selected_count(), 1);
                }
                if mode == 5 {
                    assert!(ui.get_editing());
                }
                slintforge::save_capture_png(
                    &slintforge::capture_window(ui.window()).unwrap(),
                    directory.join(format!("consumer-rust-{mode}.png")),
                )
                .unwrap();
                if mode == 6 {
                    println!("CONSUMER_GRIDS_OK host=rust states=7");
                    slint::quit_event_loop().unwrap();
                }
            }
            phase.set(index + 1);
        },
    );
    ui.run()?;
    Ok(())
}

fn widgets_smoke() -> Result<(), Box<dyn std::error::Error>> {
    let ui = WidgetsWindow::new()?;
    slintforge::bind_widget_services!(ui, ForgeWidgetServices, ForgeQRCodeResult, ForgeSyntaxLine);
    let mut pixels = slint::SharedPixelBuffer::<slint::Rgba8Pixel>::new(64, 48);
    for y in 0..48 {
        for x in 0..64 {
            pixels.make_mut_slice()[(y * 64 + x) as usize] = slint::Rgba8Pixel {
                r: (x * 3) as u8,
                g: (y * 4) as u8,
                b: 160,
                a: 255,
            };
        }
    }
    ui.set_source(slint::Image::from_rgba8(pixels));
    let timer = slint::Timer::default();
    let weak = ui.as_weak();
    timer.start(
        slint::TimerMode::SingleShot,
        std::time::Duration::from_millis(350),
        move || {
            let ui = weak.upgrade().unwrap();
            assert!(ui.get_qr_result().ok);
            assert_eq!(ui.get_syntax_line_count(), 4);
            let size = ui.get_processed().size();
            assert_eq!((size.width, size.height), (64, 48));
            let output = std::path::Path::new("build/component-qa");
            std::fs::create_dir_all(output).unwrap();
            slintforge::save_capture_png(
                &slintforge::capture_window(ui.window()).unwrap(),
                output.join("consumer-rust.png"),
            )
            .unwrap();
            println!("CONSUMER_WIDGETS_OK host=rust qr=encoded syntax=4 backdrop=64x48");
            slint::quit_event_loop().unwrap();
        },
    );
    ui.run()?;
    Ok(())
}
