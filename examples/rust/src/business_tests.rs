// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

use super::*;
use slint::platform::{Key, WindowEvent};
use slint::{ModelRc, VecModel};

fn settle() {
    i_slint_backend_testing::mock_elapsed_time(std::time::Duration::from_millis(20));
    slint::platform::update_timers_and_animations();
}

fn key(window: &slint::Window, value: impl Into<slint::SharedString>) {
    let text = value.into();
    window.dispatch_event(WindowEvent::KeyPressed { text: text.clone() });
    window.dispatch_event(WindowEvent::KeyReleased { text });
    settle();
}

#[test]
fn suggestions_use_filtered_source_indices_and_submit_with_keyboard() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = SuggestionWindow::new().unwrap();
    slintforge::bind_widget_services!(ui, ForgeWidgetServices, ForgeQRCodeResult, ForgeSyntaxLine);
    ui.show().unwrap();
    ui.set_query("sl".into());
    settle();
    ui.invoke_focus_input();
    key(ui.window(), Key::DownArrow);
    assert_eq!(ui.get_candidate_index(), 0);
    key(ui.window(), Key::DownArrow);
    assert_eq!(ui.get_candidate_index(), 1);
    key(ui.window(), Key::DownArrow);
    assert_eq!(ui.get_candidate_index(), 3);
    key(ui.window(), Key::DownArrow);
    assert_eq!(ui.get_candidate_index(), 3);
    key(ui.window(), Key::UpArrow);
    assert_eq!(ui.get_candidate_index(), 1);
    key(ui.window(), Key::Return);
    assert_eq!(ui.get_picked().as_str(), "Slider");
    assert_eq!(ui.get_selections(), 1);
    assert!(!ui.get_suggestions_open());
}

#[test]
fn suggestion_changes_empty_results_escape_and_disabled_state_are_safe() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = SuggestionWindow::new().unwrap();
    slintforge::bind_widget_services!(ui, ForgeWidgetServices, ForgeQRCodeResult, ForgeSyntaxLine);
    ui.show().unwrap();
    ui.invoke_focus_input();
    key(ui.window(), Key::DownArrow);
    ui.set_candidates(ModelRc::new(VecModel::from(vec![
        "中文".into(),
        "另一个".into(),
    ])));
    settle();
    assert_eq!(ui.get_candidate_index(), -1);
    ui.set_query("中文".into());
    settle();
    key(ui.window(), Key::UpArrow);
    assert_eq!(ui.get_candidate_index(), 0);
    key(ui.window(), Key::Escape);
    assert_eq!(ui.get_selections(), 0);
    assert_eq!(ui.get_query().as_str(), "中文");
    ui.set_query("free text".into());
    ui.set_suggestions_open(true);
    settle();
    key(ui.window(), Key::DownArrow);
    assert_eq!(ui.get_candidate_index(), -1);
    key(ui.window(), Key::Return);
    assert_eq!(ui.get_picked().as_str(), "free text");
    ui.set_input_enabled(false);
    ui.invoke_move_candidate(1);
    ui.invoke_submit();
    settle();
    assert!(!ui.get_suggestions_open());
    assert_eq!(ui.get_selections(), 1);
}

#[test]
fn business_dialog_cycles_content_skips_disabled_and_restores_focus() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = BusinessDialogWindow::new().unwrap();
    ui.show().unwrap();
    ui.invoke_open_dialog();
    settle();
    assert_eq!(ui.get_focus_position(), 0);
    key(ui.window(), Key::Tab);
    assert_eq!(ui.get_focus_position(), 2);
    key(ui.window(), Key::Tab);
    assert_eq!(ui.get_focus_position(), 3);
    key(ui.window(), Key::Tab);
    assert_eq!(ui.get_focus_position(), 4);
    key(ui.window(), Key::Tab);
    assert_eq!(ui.get_focus_position(), 0);
    // 使用真实修饰键事件检查反向遍历，而非直接调用焦点函数。
    ui.window().dispatch_event(WindowEvent::KeyPressed {
        text: Key::Shift.into(),
    });
    key(ui.window(), Key::Tab);
    ui.window().dispatch_event(WindowEvent::KeyReleased {
        text: Key::Shift.into(),
    });
    assert_eq!(ui.get_focus_position(), 4);
    ui.set_device_name("设备 A".into());
    ui.set_directory("D:/记录 数据".into());
    key(ui.window(), Key::Escape);
    assert!(!ui.get_dialog_open());
    assert!(ui.get_launch_focused());
    assert_eq!(ui.get_canceled_count(), 1);
    ui.invoke_open_dialog();
    settle();
    assert_eq!(ui.get_device_name().as_str(), "设备 A");
    assert_eq!(ui.get_directory().as_str(), "D:/记录 数据");
    ui.set_primary_enabled(false);
    key(ui.window(), Key::Tab);
    key(ui.window(), Key::Tab);
    assert_eq!(ui.get_focus_position(), 4);
    ui.set_dialog_open(false);
    settle();
    assert!(ui.get_launch_focused());
    assert_eq!(ui.get_canceled_count(), 1);
    assert_eq!(ui.get_accepted_count(), 0);
}
