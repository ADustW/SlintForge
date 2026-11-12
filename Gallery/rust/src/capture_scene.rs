// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

use crate::{ForgeCaptureResult, ForgeCaptureSceneWindow, ForgeCaptureService};
use slint::ComponentHandle;

pub fn create() -> ForgeCaptureSceneWindow {
    let ui = ForgeCaptureSceneWindow::new().unwrap();
    slintforge::bind_capture_service!(&ui, ForgeCaptureService, ForgeCaptureResult);
    let mut pixels = slint::SharedPixelBuffer::<slint::Rgba8Pixel>::new(100, 50);
    for (index, pixel) in pixels.make_mut_slice().iter_mut().enumerate() {
        *pixel = if index % 100 < 50 {
            slint::Rgba8Pixel {
                r: 224,
                g: 80,
                b: 64,
                a: 255,
            }
        } else {
            slint::Rgba8Pixel {
                r: 64,
                g: 128,
                b: 208,
                a: 255,
            }
        };
    }
    ui.set_test_image(slint::Image::from_rgba8(pixels));
    ui.show().unwrap();
    ui
}

pub fn check(ui: &ForgeCaptureSceneWindow, stage: u32, directory: &std::path::Path) {
    let pixels = slintforge::capture_window(ui.window()).unwrap();
    let scale = ui.window().scale_factor();
    slintforge::save_capture_png(&pixels, directory.join(format!("scene-{stage}.png"))).unwrap();
    assert_eq!(
        (pixels.width(), pixels.height()),
        (ui.window().size().width, ui.window().size().height)
    );
    let page = ui.invoke_capture_page();
    assert!(page.ok, "{}", page.error);
    let rect = ui.get_page_rect();
    let expected = slintforge::crop_capture(
        &pixels,
        slintforge::CaptureRect {
            x: rect.x,
            y: rect.y,
            width: rect.width,
            height: rect.height,
        },
        scale,
    )
    .unwrap();
    let page_pixels = page.image.to_rgba8().unwrap();
    assert_eq!(page_pixels.as_bytes(), expected.as_bytes());
    let color_at = |x: f32, y: f32| {
        let pixel = pixels.as_slice()
            [((y * scale).floor() as u32 * pixels.width() + (x * scale).floor() as u32) as usize];
        (pixel.r, pixel.g, pixel.b)
    };
    assert_eq!(color_at(320.0, 108.0), (240, 192, 32));
    assert_eq!(
        color_at(90.0, 110.0),
        if stage == 0 {
            (224, 80, 64)
        } else {
            (64, 176, 128)
        }
    );
    assert_eq!(color_at(90.0, 260.0), (16, 16, 16));
    assert_eq!(color_at(500.0, 40.0), (16, 16, 16));
    assert_eq!(color_at(560.0, 42.0), (0, 0, 0));
    match stage {
        0 => {
            assert_eq!(color_at(520.0, 54.0), (96, 96, 96));
            assert_eq!(color_at(520.0, 90.0), (224, 80, 64));
            assert_eq!(color_at(600.0, 90.0), (64, 128, 208));
        }
        1 => assert_eq!(color_at(520.0, 54.0), (224, 80, 64)),
        2 => assert_eq!(color_at(520.0, 90.0), (96, 96, 96)),
        3 => {
            assert_eq!(color_at(520.0, 80.0), (224, 80, 64));
            assert_eq!(color_at(650.0, 80.0), (64, 128, 208));
            assert_eq!(color_at(676.0, 80.0), (0, 0, 0));
        }
        _ => unreachable!(),
    }
    if stage < 2 {
        slintforge::save_capture_png(
            &page_pixels,
            directory.join(format!("scrolled-page-{stage}.png")),
        )
        .unwrap();
    }
}
