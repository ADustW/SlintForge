// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

include!(concat!(env!("OUT_DIR"), "/slintforge_style.rs"));
use i_slint_core::items::PointerEventButton;
use slint::platform::WindowEvent;
use slint::{ComponentHandle, LogicalPosition};

slintforge_ui! {

    import { ForgeSplitLayout, ForgeSplitOrientation } from "../../../ui/layouts/split-layout.slint";

    export component SplitDragTestWindow inherits Window {
        width: 600px;
        height: 400px;
        in property<bool> vertical: false;
        out property<float> split-ratio: split.ratio;
        split := ForgeSplitLayout {
            width: parent.width;
            height: parent.height;
            orientation: root.vertical ? ForgeSplitOrientation.vertical : ForgeSplitOrientation.horizontal;
        }
    }
}

fn pointer(window: &SplitDragTestWindow, event: WindowEvent) {
    window.window().dispatch_event(event);
}

fn check_drag(vertical: bool, direction: f32) {
    let window = SplitDragTestWindow::new().unwrap();
    window.set_vertical(vertical);
    window.show().unwrap();
    let start = LogicalPosition::new(300.0, 200.0);
    pointer(&window, WindowEvent::PointerMoved { position: start });
    pointer(
        &window,
        WindowEvent::PointerPressed {
            position: start,
            button: PointerEventButton::Left,
        },
    );

    let mut previous = window.get_split_ratio();
    assert!((previous - 0.5).abs() < 0.001);
    for step in 1..=5 {
        let position = if vertical {
            LogicalPosition::new(300.0, 200.0 + direction * 12.0 * step as f32)
        } else {
            LogicalPosition::new(300.0 + direction * 12.0 * step as f32, 200.0)
        };
        pointer(&window, WindowEvent::PointerMoved { position });
        let current = window.get_split_ratio();
        assert!(
            direction * (current - previous) > 0.01,
            "vertical={vertical}, direction={direction}, step={step}, previous={previous}, current={current}"
        );
        previous = current;
    }

    let end = if vertical {
        LogicalPosition::new(300.0, 200.0 + direction * 60.0)
    } else {
        LogicalPosition::new(300.0 + direction * 60.0, 200.0)
    };
    pointer(
        &window,
        WindowEvent::PointerReleased {
            position: end,
            button: PointerEventButton::Left,
        },
    );
    let expected = 0.5 + direction * if vertical { 0.15 } else { 0.10 };
    assert!((window.get_split_ratio() - expected).abs() < 0.002);
}

#[test]
fn dragging_a_moving_divider_has_monotonic_ratio() {
    i_slint_backend_testing::init_no_event_loop();
    check_drag(false, 1.0);
    check_drag(false, -1.0);
    check_drag(true, 1.0);
    check_drag(true, -1.0);
}
