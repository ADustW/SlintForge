// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

use crate::{
    capture_scene, ForgeCaptureResult, ForgeCaptureSceneWindow, ForgeCaptureService, GalleryWindow,
};
use slint::ComponentHandle;
use std::time::{Duration, SystemTime, UNIX_EPOCH};

pub fn connect(ui: &GalleryWindow) {
    slintforge::bind_capture_service!(ui, ForgeCaptureService, ForgeCaptureResult);
    ui.on_export_capture(|image| {
        let result = (|| {
            let pixels = image
                .to_rgba8()
                .ok_or_else(|| "无法读取截图像素".to_string())?;
            let directory = std::env::current_dir()
                .map_err(|e| e.to_string())?
                .join("build/captures");
            std::fs::create_dir_all(&directory).map_err(|e| e.to_string())?;
            let stamp = SystemTime::now()
                .duration_since(UNIX_EPOCH)
                .unwrap_or_default()
                .as_nanos();
            let path = directory.join(format!("capture-{stamp}.png"));
            slintforge::save_capture_png(&pixels, &path).map_err(|e| e.to_string())?;
            Ok::<_, String>(path.display().to_string())
        })();
        result
            .unwrap_or_else(|error| format!("PNG 导出失败: {error}"))
            .into()
    });
}

fn check_window(
    ui: &GalleryWindow,
    path: &std::path::Path,
) -> slint::SharedPixelBuffer<slint::Rgba8Pixel> {
    let result = ui.invoke_qa_capture_window();
    assert!(result.ok, "{}", result.error);
    let size = ui.window().size();
    assert_eq!(
        (result.pixel_width as u32, result.pixel_height as u32),
        (size.width, size.height)
    );
    let pixels = result.image.to_rgba8().unwrap();
    slintforge::save_capture_png(&pixels, path).unwrap();
    let decoded = slint::Image::load_from_path(path).unwrap();
    assert_eq!(decoded.size(), result.image.size());
    pixels
}

pub fn verification_timer(ui: &GalleryWindow, enabled: bool, preview: bool) -> slint::Timer {
    let timer = slint::Timer::default();
    if !enabled {
        return timer;
    }
    let stamp = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap_or_default()
        .as_millis();
    let directory = std::env::current_dir()
        .unwrap()
        .join(format!("build/captures/rust-{stamp}"));
    std::fs::create_dir_all(&directory).unwrap();
    let weak = ui.as_weak();
    let step = std::cell::Cell::new(0);
    let scene = std::cell::RefCell::new(None::<ForgeCaptureSceneWindow>);
    timer.start(slint::TimerMode::Repeated, Duration::from_millis(350), move || {
        let ui = weak.upgrade().unwrap();
        if preview {
            check_window(&ui, &directory.join("preview.png"));
            println!("CAPTURE_PREVIEW_OK host=rust path={}", directory.display());
            slint::quit_event_loop().unwrap();
            return;
        }
        match step.get() {
            0 => {
                let window_pixels = check_window(&ui, &directory.join("window.png"));
                let result = ui.invoke_qa_capture_page();
                assert!(result.ok, "{}", result.error);
                let rect = ui.get_qa_page_rect();
                let scale = f64::from(ui.window().scale_factor());
                assert_eq!(result.pixel_width as f64, ((f64::from(rect.x) + f64::from(rect.width)) * scale).ceil() - (f64::from(rect.x) * scale).floor());
                assert_eq!(result.pixel_height as f64, ((f64::from(rect.y) + f64::from(rect.height)) * scale).ceil() - (f64::from(rect.y) * scale).floor());
                let pixels = result.image.to_rgba8().unwrap();
                let expected = slintforge::crop_capture(&window_pixels, slintforge::CaptureRect {
                    x: rect.x, y: rect.y, width: rect.width, height: rect.height,
                }, ui.window().scale_factor()).unwrap();
                assert_eq!(pixels.as_bytes(), expected.as_bytes());
                let color_row = pixels.height() - (34.0 * scale) as u32;
                for (column, color) in [(1, (0xe0, 0x50, 0x40)), (3, (0x40, 0xb0, 0x80)), (5, (0x40, 0x80, 0xd0))] {
                    let pixel = pixels.as_slice()[(color_row * pixels.width() + column * pixels.width() / 6) as usize];
                    assert_eq!((pixel.r, pixel.g, pixel.b), color);
                }
                let corner = pixels.as_slice()[pixels.as_slice().len() - 3];
                assert_eq!((corner.r, corner.g, corner.b), (0x20, 0x40, 0x60));
                slintforge::save_capture_png(&pixels, directory.join("page.png")).unwrap();
                ui.invoke_qa_set_page_active(false);
                assert!(!ui.invoke_qa_capture_page().ok);
                ui.invoke_qa_set_page_active(true);
                ui.global::<ForgeCaptureService>().set_connected(false);
                assert!(!ui.invoke_qa_capture_window().ok);
                ui.global::<ForgeCaptureService>().set_connected(true);
                assert!(ui.invoke_qa_capture_page().ok);
                let exported = ui.invoke_qa_export_page();
                let decoded = slint::Image::load_from_path(std::path::Path::new(exported.as_str())).unwrap();
                assert_eq!(decoded.size(), result.image.size());
                let hidden = GalleryWindow::new().unwrap();
                assert!(!hidden.global::<ForgeCaptureService>().get_connected());
                assert!(slintforge::capture_window(hidden.window()).is_err());
                ui.window().set_size(slint::LogicalSize::new(1100.0, 740.0));
                *scene.borrow_mut() = Some(capture_scene::create());
            }
            1 => {
                check_window(&ui, &directory.join("resized.png"));
                let scene = scene.borrow();
                let scene = scene.as_ref().unwrap();
                capture_scene::check(scene, 0, &directory);
                scene.set_scroll_offset(64.0);
                scene.set_fit_index(0);
                ui.invoke_language_requested("en".into());
                ui.invoke_font_scale_requested(1.5);
                ui.invoke_qa_set_maximized(true);
            }
            2 => {
                check_window(&ui, &directory.join("maximized.png"));
                let scene = scene.borrow();
                let scene = scene.as_ref().unwrap();
                capture_scene::check(scene, 1, &directory);
                scene.set_image_empty(true);
            }
            3 => {
                let scene = scene.borrow();
                let scene = scene.as_ref().unwrap();
                capture_scene::check(scene, 2, &directory);
                scene.set_image_empty(false);
                scene.set_fit_index(2);
                scene.set_image_width(180.0);
                scene.set_image_height(80.0);
            }
            4 => {
                let scene = scene.borrow();
                let scene = scene.as_ref().unwrap();
                capture_scene::check(scene, 3, &directory);
                scene.hide().unwrap();
                println!("CAPTURE_OK host=rust scale={} window=matched page=matched resized=matched maximized=matched scroll=matched overlay=matched image-fit=checked export=checked errors=checked path={}", ui.window().scale_factor(), directory.display());
                ui.invoke_qa_set_maximized(false);
                slint::quit_event_loop().unwrap();
            }
            _ => {}
        }
        step.set(step.get() + 1);
    });
    timer
}
