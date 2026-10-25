use slint::platform::{Key, WindowEvent};
use slint::{ComponentHandle, SharedString};
use std::time::Duration;

slint::slint! {
    import { ForgeNumberBox } from "../../../ui/slintforge.slint";
    export component BindingRegressionWindow inherits Window {
        width: 600px; height: 200px;
        in-out property<float> number <=> number-box.value;
        in-out property<int> number-events: 0;
        public function focus-number() { number-box.focus(); }
        public function focus-other() { other.focus(); }
        number-box := ForgeNumberBox {
            x: 10px; y: 10px; width: 260px; minimum: -10; maximum: 10;
            value-changed(value) => { root.number-events += 1; }
        }
        other := FocusScope { x: 300px; y: 10px; width: 20px; height: 20px; }
    }
}

fn settle() {
    i_slint_backend_testing::mock_elapsed_time(Duration::from_millis(1));
}

fn key(ui: &BindingRegressionWindow, text: SharedString) {
    ui.window().dispatch_event(WindowEvent::KeyPressed { text: text.clone() });
    ui.window().dispatch_event(WindowEvent::KeyReleased { text });
}

// 全选后输入替换文本，不提交，只留在输入框里。
fn type_pending(ui: &BindingRegressionWindow, text: &str) {
    ui.invoke_focus_number();
    let control: SharedString = Key::Control.into();
    ui.window().dispatch_event(WindowEvent::KeyPressed { text: control.clone() });
    key(ui, "a".into());
    ui.window().dispatch_event(WindowEvent::KeyReleased { text: control });
    for ch in text.chars() {
        key(ui, ch.to_string().into());
    }
    settle();
}

#[test]
fn number_box_commits_pending_text_when_focus_leaves() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = BindingRegressionWindow::new().unwrap();
    ui.show().unwrap();
    settle();
    type_pending(&ui, "25");
    assert_eq!(ui.get_number(), 0.);
    ui.invoke_focus_other();
    settle();
    assert_eq!(ui.get_number(), 10.);
    assert_eq!(ui.get_number_events(), 1);
    // 非法文本在失焦后恢复为当前值，不触发回调。
    type_pending(&ui, "abc");
    ui.invoke_focus_other();
    settle();
    assert_eq!(ui.get_number(), 10.);
    assert_eq!(ui.get_number_events(), 1);
    // 未修改的文本失焦不产生回调。
    ui.invoke_focus_number();
    ui.invoke_focus_other();
    settle();
    assert_eq!(ui.get_number_events(), 1);
    type_pending(&ui, "-3.5");
    ui.invoke_focus_other();
    settle();
    assert_eq!(ui.get_number(), -3.5);
    assert_eq!(ui.get_number_events(), 2);
}
