use slint::platform::{Key, WindowEvent};
use slint::ComponentHandle;
use std::time::Duration;

slint::slint! { export { ForgePickerTestWindow, ForgeWidgetServices, ForgeDelayButtonMode } from "../../../tests/ui/picker-interactions.slint"; }

fn tick(ms: u64) {
    i_slint_backend_testing::mock_elapsed_time(Duration::from_millis(ms));
    slint::platform::update_timers_and_animations();
}

#[test]
fn delay_modes_confirm_once_and_cancel_without_reusing_progress() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = ForgePickerTestWindow::new().unwrap();
    ui.show().unwrap();
    ui.invoke_begin();
    tick(1);
    tick(50);
    assert!(ui.get_progress() > 0.);
    ui.invoke_finish();
    assert_eq!(ui.get_confirmed(), 0);
    assert_eq!(ui.get_progress(), 0.);
    ui.invoke_begin();
    tick(1);
    tick(120);
    assert_eq!(ui.get_confirmed(), 0);
    ui.invoke_finish();
    assert_eq!(ui.get_confirmed(), 1);
    ui.set_mode(ForgeDelayButtonMode::Hold);
    tick(1);
    ui.invoke_begin();
    tick(1);
    tick(120);
    assert_eq!(ui.get_confirmed(), 2);
    tick(120);
    ui.invoke_finish();
    assert_eq!(ui.get_confirmed(), 2);
    ui.set_mode(ForgeDelayButtonMode::Click);
    tick(1);
    ui.invoke_begin();
    tick(1);
    tick(50);
    ui.invoke_begin();
    tick(120);
    assert_eq!(ui.get_confirmed(), 2);
    ui.invoke_begin();
    tick(1);
    tick(120);
    assert_eq!(ui.get_confirmed(), 3);
    assert!(!ui.get_running());
    ui.invoke_begin();
    tick(1);
    ui.set_enabled(false);
    tick(120);
    assert_eq!(ui.get_confirmed(), 3);
    assert!(!ui.get_running());
}

#[test]
fn date_wheels_clamp_leap_days_and_ranges_and_commit_only_on_confirm() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = ForgePickerTestWindow::new().unwrap();
    ui.show().unwrap();
    ui.invoke_open_date();
    ui.invoke_set_day(29);
    assert_eq!(ui.get_date().day, 28);
    assert_eq!(ui.get_draft_date().day, 29);
    ui.invoke_set_year(2025);
    assert_eq!(ui.get_draft_date().day, 28);
    ui.invoke_cancel_date();
    assert_eq!(ui.get_date().year, 2024);
    ui.invoke_open_date();
    ui.invoke_set_year(3000);
    assert_eq!(
        ui.get_draft_date(),
        Date {
            year: 2025,
            month: 3,
            day: 20
        }
    );
    ui.invoke_confirm_date();
    assert_eq!(
        ui.get_date(),
        Date {
            year: 2025,
            month: 3,
            day: 20
        }
    );
    ui.invoke_open_date();
    ui.set_enabled(false);
    tick(1);
    assert!(!ui.get_date_open());
}

#[test]
fn color_hex_edits_are_drafts_and_invalid_values_cannot_commit() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = ForgePickerTestWindow::new().unwrap();
    let service = ui.global::<ForgeWidgetServices>();
    service.on_color_to_hex(|color| slintforge::color_to_hex(color).into());
    service.on_color_hex_valid(|text| slintforge::color_from_hex(text.as_str()).is_some());
    service.on_color_from_hex(|text| slintforge::color_from_hex(text.as_str()).unwrap_or_default());
    service.set_connected(true);
    ui.show().unwrap();
    ui.invoke_open_color();
    ui.invoke_edit_hex("#FF000080".into());
    assert_eq!(
        ui.get_selected_color(),
        slint::Color::from_rgb_u8(0, 120, 212)
    );
    assert_eq!(ui.get_draft_color().red(), 255);
    ui.invoke_cancel_color();
    ui.invoke_open_color();
    ui.invoke_edit_hex("bad".into());
    ui.invoke_confirm_color();
    assert_eq!(
        ui.get_selected_color(),
        slint::Color::from_rgb_u8(0, 120, 212)
    );
    ui.invoke_edit_hex("#12345680".into());
    ui.invoke_confirm_color();
    assert_eq!(
        ui.get_selected_color(),
        slint::Color::from_argb_u8(128, 18, 52, 86)
    );
}

#[test]
fn shortcut_keyboard_capture_preserves_the_committed_value_until_saved() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = ForgePickerTestWindow::new().unwrap();
    ui.show().unwrap();
    ui.invoke_open_shortcut();
    ui.window()
        .dispatch_event(WindowEvent::KeyPressed { text: "k".into() });
    ui.window()
        .dispatch_event(WindowEvent::KeyReleased { text: "k".into() });
    tick(1);
    assert_eq!(ui.get_draft_shortcut(), "K");
    assert_eq!(ui.get_shortcut(), "Alt+A");
    ui.invoke_reset_shortcut();
    assert_eq!(ui.get_draft_shortcut(), "Alt+A");
    ui.window().dispatch_event(WindowEvent::KeyPressed {
        text: Key::Return.into(),
    });
    tick(1);
    assert_eq!(ui.get_draft_shortcut(), "Enter");
    ui.invoke_confirm_shortcut();
    assert_eq!(ui.get_shortcut(), "Enter");
    ui.invoke_open_shortcut();
    ui.window().dispatch_event(WindowEvent::KeyPressed {
        text: Key::Escape.into(),
    });
    tick(1);
    assert_eq!(ui.get_shortcut(), "Enter");
}

#[test]
fn calendar_date_and_time_commit_before_the_accepted_callback() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = ForgePickerTestWindow::new().unwrap();
    ui.show().unwrap();
    ui.invoke_open_calendar();
    ui.invoke_set_calendar_time(Time {
        hour: 19,
        minute: 59,
        second: 23,
    });
    assert_eq!(ui.get_calendar_time().hour, 9);
    ui.invoke_cancel_calendar();
    assert_eq!(ui.get_calendar_time().hour, 9);
    ui.invoke_open_calendar();
    ui.invoke_set_calendar_time(Time {
        hour: 19,
        minute: 59,
        second: 23,
    });
    ui.invoke_confirm_calendar();
    assert_eq!(
        ui.get_calendar_time(),
        Time {
            hour: 19,
            minute: 55,
            second: 23
        }
    );
    assert_eq!(ui.get_calendar_observed_hour(), 19);
}

#[test]
fn wheel_keeps_the_owner_binding_when_resetting_or_reopening() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = ForgePickerTestWindow::new().unwrap();
    ui.show().unwrap();
    ui.invoke_change_wheel(1);
    assert_eq!(ui.get_wheel_value(), 15);
    ui.set_wheel_model(20);
    assert_eq!(ui.get_wheel_value(), 20);
    ui.set_wheel_model(55);
    ui.invoke_change_wheel(1);
    assert_eq!(ui.get_wheel_value(), 0);
    ui.invoke_change_wheel(-1);
    assert_eq!(ui.get_wheel_value(), 55);
}
