use slint::ComponentHandle;
use std::time::Duration;

slint::slint! {
    import { ForgeFlowLayout } from "../../../ui/layouts/flow-layout.slint";

    export component FlowLayoutTestWindow inherits Window {
        width: 500px;
        height: 260px;
        in-out property<bool> narrow: true;
        in-out property<int> extra-items: 0;
        out property<bool> scrollbar-visible: flow.vertical-scrollbar-visible;
        out property<length> measured-content-height: flow.content-height;
        out property<length> first-y: first.y;
        out property<length> second-y: second.y;

        flow := ForgeFlowLayout {
            width: root.narrow ? 340px : 460px;
            height: 200px;
            spacing: 10px;
            content-padding: 10px;
            first := Rectangle { width: 170px; height: 60px; background: blue; }
            second := Rectangle { width: 170px; height: 60px; background: red; }
            for index in root.extra-items : Rectangle {
                width: 100px;
                height: 55px;
                background: green;
            }
        }
    }
}

#[test]
fn flow_wraps_and_scrollbar_tracks_overflow() {
    i_slint_backend_testing::init_no_event_loop();
    let window = FlowLayoutTestWindow::new().unwrap();
    window.show().unwrap();
    i_slint_backend_testing::mock_elapsed_time(Duration::from_millis(1));

    assert!(window.get_second_y() > window.get_first_y());
    assert!(!window.get_scrollbar_visible());

    window.set_narrow(false);
    i_slint_backend_testing::mock_elapsed_time(Duration::from_millis(1));
    assert!((window.get_second_y() - window.get_first_y()).abs() < 0.01);

    window.set_extra_items(24);
    i_slint_backend_testing::mock_elapsed_time(Duration::from_millis(1));
    assert!(window.get_measured_content_height() > 200.0);
    assert!(window.get_scrollbar_visible());

    window.set_extra_items(0);
    i_slint_backend_testing::mock_elapsed_time(Duration::from_millis(1));
    assert!(!window.get_scrollbar_visible());
}
