use super::*;
use slint::platform::{Key, WindowEvent};
use std::time::Duration;

fn key(window: &slint::Window, text: impl Into<slint::SharedString>) {
    let text = text.into();
    window.dispatch_event(WindowEvent::KeyPressed { text: text.clone() });
    window.dispatch_event(WindowEvent::KeyReleased { text });
}

fn save(window: &slint::Window, name: &str) {
    let directory = std::env::var_os("SLINTFORGE_INTERACTION_QA_DIR")
        .map(std::path::PathBuf::from).unwrap_or_else(|| "build/business-qa/rust".into());
    std::fs::create_dir_all(&directory).unwrap();
    slintforge::save_capture_png(&slintforge::capture_window(window).unwrap(), directory.join(name)).unwrap();
}

pub fn suggestion(smoke: bool) -> Result<(), Box<dyn std::error::Error>> {
    let ui = SuggestionWindow::new()?;
    slintforge::bind_widget_services!(ui, ForgeWidgetServices, ForgeQRCodeResult, ForgeSyntaxLine);
    let timer = slint::Timer::default();
    if smoke {
        let weak = ui.as_weak();
        let phase = std::rc::Rc::new(std::cell::Cell::new(0));
        timer.start(slint::TimerMode::Repeated, Duration::from_millis(180), move || {
            let ui = weak.upgrade().unwrap();
            match phase.get() {
                0 => { ui.set_query("sl".into()); ui.invoke_focus_input(); }
                1 => { key(ui.window(), Key::DownArrow); key(ui.window(), Key::DownArrow); }
                2 => { assert_eq!(ui.get_candidate_index(), 1); save(ui.window(), "suggestion-open.png"); key(ui.window(), Key::Return); }
                3 => {
                    assert_eq!(ui.get_picked().as_str(), "Slider"); assert_eq!(ui.get_selections(), 1);
                    assert!(!ui.get_suggestions_open()); ui.set_input_enabled(false); ui.invoke_move_candidate(1);
                }
                _ => { assert!(!ui.get_suggestions_open()); println!("BUSINESS_SUGGESTION_OK host=rust"); slint::quit_event_loop().unwrap(); }
            }
            phase.set(phase.get() + 1);
        });
    }
    ui.run()?;
    Ok(())
}

pub fn dialog(smoke: bool) -> Result<(), Box<dyn std::error::Error>> {
    let ui = BusinessDialogWindow::new()?;
    let timer = slint::Timer::default();
    if smoke {
        let weak = ui.as_weak();
        let phase = std::rc::Rc::new(std::cell::Cell::new(0));
        timer.start(slint::TimerMode::Repeated, Duration::from_millis(180), move || {
            let ui = weak.upgrade().unwrap();
            match phase.get() {
                0 => ui.invoke_open_dialog(),
                1 => { assert_eq!(ui.get_focus_position(), 0); key(ui.window(), Key::Tab); }
                2 => { assert_eq!(ui.get_focus_position(), 2); key(ui.window(), Key::Tab); }
                3 => {
                    assert_eq!(ui.get_focus_position(), 3);
                    ui.window().dispatch_event(WindowEvent::KeyPressed { text: Key::Shift.into() }); key(ui.window(), Key::Tab);
                    ui.window().dispatch_event(WindowEvent::KeyReleased { text: Key::Shift.into() });
                }
                4 => {
                    assert_eq!(ui.get_focus_position(), 2); ui.set_device_name("设备 A".into()); ui.set_directory("D:/记录 数据".into());
                    save(ui.window(), "dialog-open.png"); key(ui.window(), Key::Escape);
                }
                5 => { assert!(ui.get_launch_focused()); assert_eq!(ui.get_canceled_count(), 1); ui.invoke_open_dialog(); }
                6 => { assert_eq!(ui.get_device_name().as_str(), "设备 A"); assert_eq!(ui.get_directory().as_str(), "D:/记录 数据"); ui.invoke_focus_target(3); key(ui.window(), Key::Return); }
                _ => { assert_eq!(ui.get_accepted_count(), 1); assert!(ui.get_launch_focused()); println!("BUSINESS_DIALOG_OK host=rust"); slint::quit_event_loop().unwrap(); }
            }
            phase.set(phase.get() + 1);
        });
    }
    ui.run()?;
    Ok(())
}
