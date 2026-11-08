// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

use slint::{
    platform::{Key, PointerEventButton, WindowEvent},
    ComponentHandle, LogicalPosition,
};
slint::include_modules!();
fn click(window: &slint::Window, x: f32, y: f32) {
    let position = LogicalPosition::new(x, y);
    window.dispatch_event(WindowEvent::PointerMoved { position });
    window.dispatch_event(WindowEvent::PointerPressed {
        position,
        button: PointerEventButton::Left,
    });
    window.dispatch_event(WindowEvent::PointerReleased {
        position,
        button: PointerEventButton::Left,
    });
}
fn main() -> Result<(), Box<dyn std::error::Error>> {
    slint::BackendSelector::new()
        .backend_name("winit".into())
        .renderer_name(
            std::env::var("SLINT_BACKEND")
                .unwrap_or("winit-skia".into())
                .trim_start_matches("winit-")
                .into(),
        )
        .select()?;
    let ui = ForgeFlowTransformWindow::new()?;
    let weak = ui.as_weak();
    let phase = std::rc::Rc::new(std::cell::Cell::new(0));
    let timer = slint::Timer::default();
    timer.start(
        slint::TimerMode::Repeated,
        std::time::Duration::from_millis(180),
        move || {
            let ui = weak.unwrap();
            let window = ui.window();
            let z = ui.get_zoom();
            let p = phase.get();
            phase.set(p + 1);
            match p {
                0 => {
                    click(window, 60. + 40. * z, 50. + 70. * z);
                    window.dispatch_event(WindowEvent::KeyPressed { text: "x".into() });
                    window.dispatch_event(WindowEvent::KeyReleased { text: "x".into() });
                }
                1 => {
                    assert_eq!(ui.get_draft(), "x");
                    click(window, 60. + 50. * z, 50. + 120. * z);
                }
                2 => {
                    assert_eq!(ui.get_clicks(), 1);
                    click(window, 60. + 80. * z, 50. + 178. * z);
                }
                3 => {
                    std::fs::create_dir_all("build/flow-qa").unwrap();
                    let pixels = slintforge::capture_window(window).unwrap();
                    slintforge::save_capture_png(&pixels, "build/flow-qa/probe-rust.png").unwrap();
                    for key in [Key::DownArrow, Key::Return] {
                        window.dispatch_event(WindowEvent::KeyPressed { text: key.into() });
                        window.dispatch_event(WindowEvent::KeyReleased { text: key.into() });
                    }
                }
                _ => {
                    assert_eq!(ui.get_choice(), 1);
                    println!("FLOW_TRANSFORM_OK host=rust");
                    slint::quit_event_loop().unwrap();
                }
            }
        },
    );
    ui.run()?;
    Ok(())
}
