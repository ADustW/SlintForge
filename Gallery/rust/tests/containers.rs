// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

include!(concat!(env!("OUT_DIR"), "/slintforge_style.rs"));
use slint::platform::WindowEvent;
use slint::{ComponentHandle, LogicalPosition};
use std::time::Duration;

slintforge_ui! {

    import { LineEdit } from "std-widgets.slint";
    import { ForgeExpander } from "../../../ui/surfaces/expander.slint";
    import { ForgeWatermark } from "../../../ui/surfaces/watermark.slint";
    import { ForgeMarquee } from "../../../ui/surfaces/marquee.slint";
    import { ForgeIconLabel } from "../../../ui/surfaces/icon-label.slint";
    import { ForgeCard } from "../../../ui/basic/card.slint";
    import { ForgePage } from "../../../ui/surfaces/page.slint";
    import { ForgeTypography } from "../../../ui/typography/typography.slint";
    import { ForgeCaptureResult } from "../../../ui/types/capture.slint";
    export component ContainerTestWindow inherits Window {
        width: 900px; height: 800px;
        in-out property<bool> expanded <=> expander.expanded;
        in-out property<bool> expander-enabled: true;
        in-out property<length> explicit-height: 0px;
        out property<length> expander-height: expander.height;
        out property<length> natural-height: expander.natural-content-height;
        out property<string> draft: edit.text;
        callback set-draft(string);
        set-draft(value) => { edit.text = value; }
        in-out property<int> toggles: 0;
        in-out property<int> clicks: 0;
        in-out property<length> water-width: 370px;
        out property<int> water-columns: watermark.columns;
        out property<int> water-rows: watermark.rows;
        in-out property<string> marquee-text: "A very long notification repeated many times to ensure horizontal overflow in the small marquee viewport.";
        in-out property<bool> running: true;
        in-out property<length> marquee-width: 180px;
        in-out property<int> cycle: 8;
        out property<length> marquee-x: marquee.text-position;
        out property<bool> overflow: marquee.overflowing;
        in-out property<bool> vertical: false;
        in-out property<string> card-title: "标题";
        out property<length> card-height: card.height;
        out property<length> icon-height: icon-label.height;
        in-out property<float> scale <=> ForgeTypography.scale;
        in-out property<bool> page-active: true;
        out property<string> page-draft: page-edit.text;
        callback set-page-draft(string);
        set-page-draft(value) => { page-edit.text = value; }
        callback page-capture() -> ForgeCaptureResult;
        page-capture => { return page.capture(); }

        expander := ForgeExpander {
            x: 0px; y: 0px; width: 280px;
            title: "高级设置";
            enabled: root.expander-enabled;
            content-height: root.explicit-height;
            toggled(value) => { root.toggles += 1; }
            VerticalLayout {
                spacing: 12px;
                edit := LineEdit { text: ""; }
                Text { text: "自然高度内容"; height: 34px; }
            }
        }
        watermark := ForgeWatermark {
            x: 0px; y: 260px; width: root.water-width; height: 250px;
            mark: "测试水印"; mark-opacity: 2;
            TouchArea { clicked => { root.clicks += 1; } }
        }
        marquee := ForgeMarquee {
            x: 0px; y: 540px; width: root.marquee-width; height: 40px;
            text: root.marquee-text;
            cycle-duration: root.cycle * 1s;
            running: root.running;
        }
        card := ForgeCard {
            x: 430px; y: 250px; width: 180px; height: self.preferred-height;
            title: root.card-title;
            Text { text: "内容"; height: 24px; }
        }
        icon-label := ForgeIconLabel {
            x: 430px; y: 400px; width: 180px; height: self.preferred-height;
            label: "Long label that wraps onto several lines";
            description: "An explanation that is longer than the available horizontal space.";
            vertical: root.vertical;
        }
        page := ForgePage {
            x: 430px; y: 20px; width: 240px; height: 140px;
            active: root.page-active;
            page-edit := LineEdit { x: 12px; y: 12px; text: ""; }
        }
    }
}

fn tick(milliseconds: u64) {
    i_slint_backend_testing::mock_elapsed_time(Duration::from_millis(milliseconds));
}

fn click(ui: &ContainerTestWindow, position: LogicalPosition) {
    use i_slint_core::items::PointerEventButton;
    ui.window()
        .dispatch_event(WindowEvent::PointerMoved { position });
    ui.window().dispatch_event(WindowEvent::PointerPressed {
        position,
        button: PointerEventButton::Left,
    });
    ui.window().dispatch_event(WindowEvent::PointerReleased {
        position,
        button: PointerEventButton::Left,
    });
    tick(1);
}

#[test]
fn containers_keep_state_handle_input_and_resize_content() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = ContainerTestWindow::new().unwrap();
    ui.show().unwrap();
    tick(1);
    ui.invoke_set_draft("保留输入".into());
    let collapsed_height = ui.get_expander_height();
    click(&ui, LogicalPosition::new(20.0, 20.0));
    assert!(ui.get_expanded());
    assert!(ui.get_natural_height() >= 70.0);
    assert!(ui.get_expander_height() >= collapsed_height + 70.0);
    ui.window()
        .dispatch_event(WindowEvent::KeyPressed { text: " ".into() });
    ui.window()
        .dispatch_event(WindowEvent::KeyReleased { text: " ".into() });
    tick(1);
    assert!(!ui.get_expanded());
    ui.window()
        .dispatch_event(WindowEvent::KeyPressed { text: "\n".into() });
    ui.window()
        .dispatch_event(WindowEvent::KeyReleased { text: "\n".into() });
    tick(1);
    assert!(ui.get_expanded());
    assert_eq!(ui.get_draft().as_str(), "保留输入");
    click(&ui, LogicalPosition::new(20.0, 20.0));
    assert!(!ui.get_expanded());
    click(&ui, LogicalPosition::new(20.0, 20.0));
    assert!(ui.get_expanded());
    assert_eq!(ui.get_toggles(), 5);
    ui.set_explicit_height(160.0);
    assert!((ui.get_expander_height() - collapsed_height - 160.0).abs() < 0.1);
    ui.set_expander_enabled(false);
    click(&ui, LogicalPosition::new(20.0, 20.0));
    assert_eq!(ui.get_toggles(), 5);

    assert_eq!((ui.get_water_columns(), ui.get_water_rows()), (3, 3));
    click(&ui, LogicalPosition::new(100.0, 300.0));
    assert_eq!(ui.get_clicks(), 1);
    ui.set_water_width(720.0);
    assert_eq!(ui.get_water_columns(), 4);

    assert!(ui.get_overflow());
    let start = ui.get_marquee_x();
    tick(200);
    assert!(ui.get_marquee_x() < start);
    ui.set_running(false);
    tick(1);
    let paused = ui.get_marquee_x();
    tick(500);
    assert!((ui.get_marquee_x() - paused).abs() < 0.01);
    ui.set_running(true);
    // changed 回调在下一次事件循环处理；先完成恢复，再推进动画时间。
    tick(1);
    assert!((ui.get_marquee_x() - paused).abs() < 0.01);
    tick(200);
    assert!(
        ui.get_marquee_x() < paused,
        "resume={} paused={} start={}",
        ui.get_marquee_x(),
        paused,
        start
    );
    for cycle in [0, -1] {
        ui.set_cycle(cycle);
        tick(1);
        assert_eq!(ui.get_marquee_x(), 12.0);
    }
    ui.set_cycle(8);
    ui.set_marquee_width(2000.0);
    tick(1);
    assert!(!ui.get_overflow());
    assert_eq!(ui.get_marquee_x(), 12.0);
    ui.set_marquee_width(180.0);
    tick(1);
    assert!(ui.get_overflow());
    assert_eq!(ui.get_marquee_x(), 12.0);
    ui.set_marquee_text("短文本".into());
    tick(1);
    assert!(!ui.get_overflow());
    assert_eq!(ui.get_marquee_x(), 12.0);

    let short = ui.get_card_height();
    ui.set_card_title("这是一个需要在窄卡片中换行显示的很长的标题".into());
    tick(1);
    assert!(ui.get_card_height() > short);
    ui.set_card_title("".into());
    tick(1);
    assert!(ui.get_card_height() < short);
    let small_icon = ui.get_icon_height();
    ui.set_scale(1.5);
    tick(1);
    assert!(ui.get_icon_height() > small_icon);
    ui.set_vertical(true);
    tick(1);
    assert!(ui.get_icon_height() > 60.0);

    ui.invoke_set_page_draft("页面输入".into());
    assert!(!ui.invoke_page_capture().ok);
    assert_eq!(ui.invoke_page_capture().error.as_str(), "截图服务未接入");
    ui.set_page_active(false);
    assert_eq!(ui.invoke_page_capture().error.as_str(), "页面未激活");
    ui.set_page_active(true);
    assert_eq!(ui.get_page_draft().as_str(), "页面输入");
}
