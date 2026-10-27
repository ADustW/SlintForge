use slint::platform::{Key, PointerEventButton, WindowEvent};
use slint::{ComponentHandle, SharedString};
use std::time::Duration;

slint::slint! {
    import { ForgeNumberBox } from "../../../ui/slintforge.slint";
    export component BindingRegressionWindow inherits Window {
        width: 600px; height: 200px;
        in-out property<float> number <=> number-box.value;
        in-out property<int> number-events: 0;
        in-out property<float> number-minimum: -10;
        in-out property<float> number-maximum: 10;
        in-out property<float> number-step: 0.1;
        in-out property<bool> number-enabled: true;
        public function focus-number() { number-box.focus(); }
        public function focus-other() { other.focus(); }
        number-box := ForgeNumberBox {
            x: 10px; y: 10px; width: 260px; height: 36px;
            minimum: root.number-minimum; maximum: root.number-maximum;
            step: root.number-step; enabled: root.number-enabled;
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

fn click(ui: &BindingRegressionWindow, x: f32, y: f32) {
    let position = slint::LogicalPosition::new(x, y);
    ui.window().dispatch_event(WindowEvent::PointerMoved { position });
    ui.window().dispatch_event(WindowEvent::PointerPressed {
        position,
        button: PointerEventButton::Left,
    });
    settle();
    ui.window().dispatch_event(WindowEvent::PointerReleased {
        position,
        button: PointerEventButton::Left,
    });
    settle();
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

#[test]
fn number_box_steps_from_pending_text() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = BindingRegressionWindow::new().unwrap();
    ui.set_number_minimum(-100.);
    ui.set_number_maximum(100.);
    ui.set_number_step(1.);
    ui.show().unwrap();
    settle();
    // 实际鼠标点击不保证失焦；旧值在上下限时也要按当前输入决定按钮是否可用。
    for (initial, text, x, expected) in [
        (0., "50", 250., 51.),
        (0., "50", 20., 49.),
        (100., "50", 250., 51.),
        (-100., "-50", 20., -51.),
        (0., "125", 20., 99.),
        (0., "-125", 250., -99.),
        (0., "abc", 250., 1.),
        (0., "abc", 20., -1.),
    ] {
        ui.set_number(initial);
        settle();
        ui.set_number_events(0);
        type_pending(&ui, text);
        click(&ui, x, 28.);
        assert_eq!(ui.get_number(), expected, "initial={initial}, text={text}, x={x}");
        assert_eq!(ui.get_number_events(), 1);
        ui.invoke_focus_other();
        settle();
        assert_eq!(ui.get_number_events(), 1, "步进后失焦不应再次提交");
    }
    for (text, arrow, expected) in [("25", Key::UpArrow, 26.), ("-25", Key::DownArrow, -26.)] {
        ui.set_number_events(0);
        type_pending(&ui, text);
        key(&ui, arrow.into());
        settle();
        assert_eq!(ui.get_number(), expected);
        assert_eq!(ui.get_number_events(), 1);
    }
    ui.set_number(100.);
    settle();
    ui.set_number_events(0);
    type_pending(&ui, "125");
    key(&ui, Key::UpArrow.into());
    ui.invoke_focus_other();
    settle();
    assert_eq!(ui.get_number(), 100.);
    assert_eq!(ui.get_number_events(), 1, "边界步进也应清除未提交文本");
}
