use slint::platform::WindowEvent;
use slint::{ComponentHandle, LogicalPosition};
use std::time::Duration;

slint::slint! {
    import { ForgeNumberBox, ForgeDial, ForgeFormRow, ForgeSplitLayout, ForgeSplitOrientation, ForgeAutoSuggestBox, ForgeRatingControl } from "../../../ui/slintforge.slint";
    export component ComponentReviewWindow inherits Window {
        width: 900px; height: 700px;
        in-out property<float> number <=> number-box.value;
        in-out property<float> number-min: -10;
        in-out property<float> number-max: 10;
        in-out property<int> number-events: 0;
        out property<float> initial-number: initial-number-box.value;
        out property<float> initial-dial: initial-dial.value;
        out property<int> initial-rating: initial-rating.value;
        out property<bool> initially-disabled-open: initial-suggest.open;
        in-out property<float> dial-value <=> dial.value;
        in-out property<float> dial-step: 5;
        in-out property<int> dial-events: 0;
        in-out property<bool> suggestions-enabled: true;
        in-out property<bool> suggestions-open <=> suggest.open;
        in-out property<float> split-ratio <=> split.ratio;
        in-out property<length> divider-size: 8px;
        in-out property<bool> vertical: false;
        out property<length> first-size: root.vertical ? split.first-height : split.first-width;
        out property<length> second-origin: root.vertical ? split.second-y : split.second-x;
        out property<length> second-size: root.vertical ? split.second-height : split.second-width;
        out property<length> form-height: form.height;
        out property<length> control-bottom: form.control-origin-y + control.y + control.height;
        number-box := ForgeNumberBox {
            x: 10px; y: 10px; width: 260px; minimum: root.number-min; maximum: root.number-max;
            value-changed(value) => { root.number-events += 1; }
        }
        initial-number-box := ForgeNumberBox { x: 500px; y: 10px; width: 260px; minimum: 5; maximum: 10; value: 25; }
        initial-dial := ForgeDial { x: 600px; y: 70px; width: 120px; height: 120px; minimum: 5; maximum: 10; value: -2; }
        initial-rating := ForgeRatingControl { x: 600px; y: 220px; value: 9; maximum: 3; }
        initial-suggest := ForgeAutoSuggestBox { x: 600px; y: 270px; width: 220px; enabled: false; open: true; suggestions: ["Slint"]; }
        dial := ForgeDial {
            x: 10px; y: 80px; width: 180px; height: 180px; value: 40; step: root.dial-step;
            value-changed(value) => { root.dial-events += 1; }
        }
        suggest := ForgeAutoSuggestBox {
            x: 10px; y: 300px; width: 260px; enabled: root.suggestions-enabled; suggestions: ["Slint", "Rust"];
        }
        form := ForgeFormRow {
            x: 300px; y: 10px; width: 190px; height: 230px; title: "说明";
            description: "窄窗口中的说明文字需要换行。内容较长时，输入控件仍然应当完整显示在表单行内部，底部不能被裁切。增加字体大小后也应保持这个约束。";
            control := Rectangle { min-height: 32px; vertical-stretch: 1; background: blue; }
        }
        split := ForgeSplitLayout {
            x: 300px; y: 400px; width: 500px; height: 220px;
            divider-size: root.divider-size;
            orientation: root.vertical ? ForgeSplitOrientation.vertical : ForgeSplitOrientation.horizontal;
        }
    }
}

fn settle() {
    i_slint_backend_testing::mock_elapsed_time(Duration::from_millis(1));
}

#[test]
fn external_number_values_stay_in_range_without_user_callbacks() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = ComponentReviewWindow::new().unwrap();
    ui.show().unwrap();
    settle();
    assert_eq!(ui.get_initial_number(), 10.);
    assert_eq!(ui.get_initial_dial(), 5.);
    assert_eq!(ui.get_initial_rating(), 3);
    assert!(!ui.get_initially_disabled_open());
    for (value, expected) in [(25., 10.), (-25., -10.), (2.5, 2.5)] {
        ui.set_number(value);
        settle();
        assert_eq!(ui.get_number(), expected);
    }
    ui.set_number_min(4.);
    settle();
    assert_eq!(ui.get_number(), 4.);
    ui.set_number_max(2.);
    settle();
    assert_eq!(ui.get_number(), 4.);
    assert_eq!(ui.get_number_events(), 0);
}

#[test]
fn dial_ignores_horizontal_scroll_and_nonpositive_steps() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = ComponentReviewWindow::new().unwrap();
    ui.show().unwrap();
    let scroll = |dx, dy| {
        ui.window().dispatch_event(WindowEvent::PointerScrolled {
            position: LogicalPosition::new(100., 170.),
            delta_x: dx,
            delta_y: dy,
        });
        settle();
    };
    scroll(20., 0.);
    assert_eq!(ui.get_dial_value(), 40.);
    assert_eq!(ui.get_dial_events(), 0);
    scroll(0., 20.);
    assert_eq!(ui.get_dial_value(), 45.);
    scroll(0., -20.);
    assert_eq!(ui.get_dial_value(), 40.);
    for step in [0., -5.] {
        ui.set_dial_step(step);
        scroll(0., 20.);
        assert_eq!(ui.get_dial_value(), 40.);
    }
    assert_eq!(ui.get_dial_events(), 2);
}

#[test]
fn wrapped_form_controls_fit_their_row() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = ComponentReviewWindow::new().unwrap();
    ui.show().unwrap();
    assert!(
        ui.get_control_bottom() <= ui.get_form_height() + 0.1,
        "控件底部={}，表单行高度={}",
        ui.get_control_bottom(),
        ui.get_form_height()
    );
}

#[test]
fn split_geometry_stays_inside_its_bounds() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = ComponentReviewWindow::new().unwrap();
    ui.show().unwrap();
    for vertical in [false, true] {
        ui.set_vertical(vertical);
        let extent = if vertical { 220. } else { 500. };
        for ratio in [-0.5, 0., 0.5, 1., 1.5] {
            ui.set_split_ratio(ratio);
            for divider in [-8., 0., 8., 1000.] {
                ui.set_divider_size(divider);
                settle();
                assert!((0. ..=extent).contains(&ui.get_first_size()));
                assert!((0. ..=extent).contains(&ui.get_second_origin()));
                assert!((0. ..=extent).contains(&ui.get_second_size()));
                assert!((ui.get_second_origin() + ui.get_second_size() - extent).abs() < 0.1);
            }
        }
    }
}

#[test]
fn disabled_suggestions_close_and_cannot_reopen() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = ComponentReviewWindow::new().unwrap();
    ui.show().unwrap();
    ui.set_suggestions_open(true);
    settle();
    assert!(ui.get_suggestions_open());
    ui.set_suggestions_enabled(false);
    settle();
    assert!(!ui.get_suggestions_open());
    ui.set_suggestions_open(true);
    settle();
    assert!(!ui.get_suggestions_open());
}
