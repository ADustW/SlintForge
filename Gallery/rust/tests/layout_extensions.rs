// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

include!(concat!(env!("OUT_DIR"), "/slintforge_style.rs"));
use i_slint_core::items::PointerEventButton;
use slint::platform::WindowEvent;
use slint::{ComponentHandle, LogicalPosition};
use std::cell::RefCell;
use std::rc::Rc;
use std::time::Duration;

slintforge_ui! {

    import { LineEdit } from "std-widgets.slint";
    import { ForgeAdaptiveGrid } from "../../../ui/layouts/adaptive-grid.slint";
    import { ForgeDockLayout } from "../../../ui/layouts/dock-layout.slint";
    import { ForgePageStack, ForgeStackPage } from "../../../ui/layouts/page-stack.slint";
    import { ForgeFormRow } from "../../../ui/layouts/form-row.slint";
    import { ForgeTypography } from "../../../ui/typography/typography.slint";
    import { ForgeStaggeredGridView, ForgeStaggeredItem } from "../../../ui/layouts/staggered-grid-view.slint";

    export component LayoutExtensionTestWindow inherits Window {
        width: 1000px; height: 800px;
        in-out property<length> adaptive-width: 650px;
        in-out property<int> adaptive-max-columns: 0;
        in-out property<int> extra-cards: 0;
        out property<int> adaptive-columns: adaptive.column-count;
        out property<length> adaptive-item-width: adaptive.item-width;
        out property<bool> adaptive-scroll: adaptive.vertical-scrollbar-visible;
        out property<length> card-one-y: card-one.y;
        out property<length> card-four-y: card-four.y;
        in-out property<length> dock-width: 800px;
        in-out property<length> dock-height: 500px;
        in-out property<bool> left-visible: true;
        in-out property<bool> top-visible: true;
        in-out property<bool> bottom-visible: true;
        in-out property<bool> right-visible: true;
        out property<length> left-size: dock.left-rect.width;
        out property<length> center-size: dock.center-rect.width;
        out property<length> center-height: dock.center-rect.height;
        out property<length> right-size: dock.right-rect.width;
        out property<length> top-size: dock.top-rect.height;
        out property<length> bottom-size: dock.bottom-rect.height;
        in-out property<string> active-page: "one";
        out property<bool> first-visible: first-page.visible;
        out property<bool> second-visible: second-page.visible;
        out property<string> draft: input.text;
        callback set-draft(string);
        set-draft(value) => { input.text = value; }
        in-out property<length> form-width: 700px;
        out property<bool> form-stacked: form.stacked-layout;
        out property<length> form-control-x: form.control-origin-x + control.x;
        out property<length> form-control-y: form.control-origin-y + control.y;
        out property<length> long-form-control-y: long-form.control-origin-y;
        out property<length> long-form-height: long-form.height;
        out property<length> long-description-height: long-description.preferred-height;
        in-out property<length> tall-form-width: 200px;
        out property<length> tall-form-control-y: tall-form.control-origin-y;
        out property<length> tall-form-height: tall-form.height;
        out property<length> tall-control-height: tall-control.height;
        in-out property<length> staggered-width: 750px;
        in-out property<length> staggered-content-height: 144px;
        out property<int> staggered-columns: staggered.column-count;
        out property<bool> staggered-scroll: staggered.vertical-scrollbar-visible;
        callback reflow-requested(int);
        callback item-selected(string);

        adaptive := ForgeAdaptiveGrid {
            x: 0px; y: 0px;
            width: root.adaptive-width; height: 170px;
            max-columns: root.adaptive-max-columns;
            card-one := Rectangle { width: adaptive.item-width; height: 60px; background: red; }
            Rectangle { width: adaptive.item-width; height: 90px; background: blue; }
            Rectangle { width: adaptive.item-width; height: 40px; background: green; }
            card-four := Rectangle { width: adaptive.item-width; height: 50px; background: yellow; }
            for index in root.extra-cards : Rectangle {
                width: adaptive.item-width; height: 55px; background: gray;
            }
        }
        dock := ForgeDockLayout {
            x: 0px; y: 190px;
            width: root.dock-width; height: root.dock-height;
            top-height: 80px; bottom-height: 90px;
            left-width: 150px; right-width: 170px;
            show-left: root.left-visible;
            show-top: root.top-visible;
            show-bottom: root.bottom-visible;
            show-right: root.right-visible;
        }
        stack := ForgePageStack {
            x: 810px; y: 0px; width: 180px; height: 170px;
            active-id: root.active-page;
            first-page := ForgeStackPage {
                width: stack.width; height: stack.height;
                page-id: "one"; active-id: stack.active-id;
                input := LineEdit { x: 10px; y: 10px; width: 120px; }
            }
            second-page := ForgeStackPage {
                width: stack.width; height: stack.height;
                page-id: "two"; active-id: stack.active-id;
                Rectangle { width: 100px; height: 80px; background: red; }
            }
        }
        form := ForgeFormRow {
            x: 0px; y: 700px; width: root.form-width; height: 90px;
            title: "Name"; description: "Description";
            control := Rectangle { width: 120px; height: 32px; background: blue; }
        }
        long-form := ForgeFormRow {
            x: 0px; y: 810px; width: 200px;
            title: "Long description";
            description: "A longer description must remain above its control even when it wraps across many lines in a narrow form row.";
            Rectangle { width: 120px; height: 32px; background: green; }
        }
        long-description := Text {
            x: 0px; y: 950px; width: 200px;
            text: "A longer description must remain above its control even when it wraps across many lines in a narrow form row.";
            font-size: 12px * ForgeTypography.scale;
            wrap: word-wrap;
        }
        tall-form := ForgeFormRow {
            x: 0px; y: 1020px; width: root.tall-form-width;
            title: "Tall control";
            description: "A custom control can be taller than a standard input.";
            tall-control := Rectangle { width: 120px; height: 100px; background: green; }
        }
        staggered := ForgeStaggeredGridView {
            x: 810px; y: 200px; width: root.staggered-width; height: 200px;
            content-height: root.staggered-content-height;
            items: [
                {
                    id: "stable-one", title: "01", description: "Card",
                    tile-height: 120px, accent: blue, column: 0, top: 12px,
                },
                {
                    id: "stable-two", title: "02", description: "Card",
                    tile-height: 100px, accent: green, column: 0, top: 300px,
                },
            ];
            layout-requested(columns) => { root.reflow-requested(columns); }
            selected(id) => { root.item-selected(id); }
        }
    }
}

#[test]
fn layout_geometry_responsive_state_and_scroll() {
    i_slint_backend_testing::init_no_event_loop();
    let window = LayoutExtensionTestWindow::new().unwrap();
    let calls = Rc::new(RefCell::new(Vec::<i32>::new()));
    let observed = calls.clone();
    window.on_reflow_requested(move |columns| observed.borrow_mut().push(columns));
    let selected = Rc::new(RefCell::new(String::new()));
    let selection = selected.clone();
    window.on_item_selected(move |id| *selection.borrow_mut() = id.to_string());
    window.show().unwrap();

    assert_eq!(window.get_adaptive_columns(), 3);
    assert!((window.get_adaptive_item_width() - 196.0).abs() < 0.1);
    assert!(window.get_card_four_y() > window.get_card_one_y());
    window.set_adaptive_max_columns(2);
    assert_eq!(window.get_adaptive_columns(), 2);
    window.set_adaptive_max_columns(0);
    assert_eq!(window.get_adaptive_columns(), 3);
    window.set_adaptive_width(440.0);
    assert_eq!(window.get_adaptive_columns(), 2);
    window.set_adaptive_width(190.0);
    assert_eq!(window.get_adaptive_columns(), 1);
    window.set_extra_cards(20);
    assert!(window.get_adaptive_scroll());
    window.set_extra_cards(0);
    window.set_adaptive_width(850.0);
    assert!(!window.get_adaptive_scroll());

    assert_eq!(window.get_left_size(), 150.0);
    assert_eq!(window.get_right_size(), 170.0);
    assert_eq!(window.get_top_size(), 80.0);
    assert_eq!(window.get_bottom_size(), 90.0);
    assert_eq!(window.get_center_size(), 464.0);
    assert_eq!(window.get_center_height(), 314.0);
    window.set_left_visible(false);
    assert_eq!(window.get_left_size(), 0.0);
    assert_eq!(window.get_center_size(), 622.0);
    window.set_left_visible(true);
    assert_eq!(window.get_left_size(), 150.0);
    window.set_top_visible(false);
    assert_eq!(window.get_top_size(), 0.0);
    window.set_top_visible(true);
    assert_eq!(window.get_top_size(), 80.0);
    window.set_bottom_visible(false);
    assert_eq!(window.get_bottom_size(), 0.0);
    window.set_bottom_visible(true);
    assert_eq!(window.get_bottom_size(), 90.0);
    window.set_right_visible(false);
    assert_eq!(window.get_right_size(), 0.0);
    window.set_right_visible(true);
    assert_eq!(window.get_right_size(), 170.0);
    window.set_dock_height(330.0);
    assert_eq!(window.get_center_height(), 144.0);
    window.set_dock_height(500.0);
    assert_eq!(window.get_top_size(), 80.0);
    assert_eq!(window.get_bottom_size(), 90.0);
    window.set_dock_width(430.0);
    assert_eq!(window.get_center_size(), 160.0);
    window.set_dock_width(800.0);
    assert_eq!(window.get_left_size(), 150.0);
    assert_eq!(window.get_right_size(), 170.0);

    window.invoke_set_draft("retained".into());
    window.set_active_page("two".into());
    assert!(!window.get_first_visible());
    assert!(window.get_second_visible());
    window.set_active_page("missing".into());
    assert!(!window.get_first_visible());
    assert!(!window.get_second_visible());
    window.set_active_page("one".into());
    assert_eq!(window.get_draft().as_str(), "retained");

    assert!(!window.get_form_stacked());
    let wide_x = window.get_form_control_x();
    assert!(wide_x > 550.0);
    window.set_form_width(430.0);
    assert!(window.get_form_stacked());
    assert!(
        window.get_form_control_x() < 20.0,
        "x={}",
        window.get_form_control_x()
    );
    assert!(window.get_form_control_y() > 40.0);
    assert!(
        window.get_long_form_control_y() >= 29.0 + window.get_long_description_height() + 12.0,
        "control-y={} description-height={}",
        window.get_long_form_control_y(),
        window.get_long_description_height()
    );
    assert!(window.get_long_form_height() >= window.get_long_form_control_y() + 32.0);
    assert!(
        window.get_tall_form_height()
            >= window.get_tall_form_control_y() + window.get_tall_control_height(),
        "row-height={} control-y={} control-height={}",
        window.get_tall_form_height(),
        window.get_tall_form_control_y(),
        window.get_tall_control_height()
    );
    window.set_tall_form_width(700.0);
    assert!(
        window.get_tall_form_height()
            >= window.get_tall_form_control_y() + window.get_tall_control_height()
    );
    assert_eq!(window.get_staggered_columns(), 3);
    assert!(!window.get_staggered_scroll());
    window.set_staggered_content_height(450.0);
    i_slint_backend_testing::mock_elapsed_time(Duration::from_millis(1));
    assert!(window.get_staggered_scroll());
    window
        .window()
        .dispatch_event(WindowEvent::PointerScrolled {
            position: LogicalPosition::new(840.0, 300.0),
            delta_x: 0.0,
            delta_y: -250.0,
        });
    i_slint_backend_testing::mock_elapsed_time(Duration::from_millis(1));
    let scrolled_tile = LogicalPosition::new(840.0, 300.0);
    window.window().dispatch_event(WindowEvent::PointerMoved {
        position: scrolled_tile,
    });
    window.window().dispatch_event(WindowEvent::PointerPressed {
        position: scrolled_tile,
        button: PointerEventButton::Left,
    });
    window
        .window()
        .dispatch_event(WindowEvent::PointerReleased {
            position: scrolled_tile,
            button: PointerEventButton::Left,
        });
    assert_eq!(selected.borrow().as_str(), "stable-two");
    window.set_staggered_content_height(144.0);
    assert!(!window.get_staggered_scroll());
    window.set_staggered_width(430.0);
    assert_eq!(window.get_staggered_columns(), 1);
    i_slint_backend_testing::mock_elapsed_time(Duration::from_millis(1));
    assert_eq!(calls.borrow().last(), Some(&1));
    let tile_position = LogicalPosition::new(840.0, 230.0);
    window.window().dispatch_event(WindowEvent::PointerMoved {
        position: tile_position,
    });
    window.window().dispatch_event(WindowEvent::PointerPressed {
        position: tile_position,
        button: PointerEventButton::Left,
    });
    window
        .window()
        .dispatch_event(WindowEvent::PointerReleased {
            position: tile_position,
            button: PointerEventButton::Left,
        });
    assert_eq!(selected.borrow().as_str(), "stable-one");
}

#[test]
fn dock_drag_tracks_pointer_and_keeps_center_minimum() {
    i_slint_backend_testing::init_no_event_loop();
    let window = LayoutExtensionTestWindow::new().unwrap();
    window.show().unwrap();
    let start = LogicalPosition::new(154.0, 400.0);
    window
        .window()
        .dispatch_event(WindowEvent::PointerMoved { position: start });
    window.window().dispatch_event(WindowEvent::PointerPressed {
        position: start,
        button: PointerEventButton::Left,
    });
    for step in 1..=5 {
        window.window().dispatch_event(WindowEvent::PointerMoved {
            position: LogicalPosition::new(154.0 + step as f32 * 20.0, 400.0),
        });
        assert!((window.get_left_size() - (150.0 + step as f32 * 20.0)).abs() < 1.0);
    }
    window
        .window()
        .dispatch_event(WindowEvent::PointerReleased {
            position: LogicalPosition::new(254.0, 400.0),
            button: PointerEventButton::Left,
        });
    assert_eq!(window.get_left_size(), 250.0);
    assert!(window.get_center_size() >= 160.0);

    let top_start = LogicalPosition::new(400.0, 274.0);
    window.window().dispatch_event(WindowEvent::PointerMoved {
        position: top_start,
    });
    window.window().dispatch_event(WindowEvent::PointerPressed {
        position: top_start,
        button: PointerEventButton::Left,
    });
    window.window().dispatch_event(WindowEvent::PointerMoved {
        position: LogicalPosition::new(400.0, 314.0),
    });
    window
        .window()
        .dispatch_event(WindowEvent::PointerReleased {
            position: LogicalPosition::new(400.0, 314.0),
            button: PointerEventButton::Left,
        });
    assert_eq!(window.get_top_size(), 120.0);

    let bottom_start = LogicalPosition::new(400.0, 596.0);
    window.window().dispatch_event(WindowEvent::PointerMoved {
        position: bottom_start,
    });
    window.window().dispatch_event(WindowEvent::PointerPressed {
        position: bottom_start,
        button: PointerEventButton::Left,
    });
    window.window().dispatch_event(WindowEvent::PointerMoved {
        position: LogicalPosition::new(400.0, 556.0),
    });
    window
        .window()
        .dispatch_event(WindowEvent::PointerReleased {
            position: LogicalPosition::new(400.0, 556.0),
            button: PointerEventButton::Left,
        });
    assert_eq!(window.get_bottom_size(), 130.0);

    let right_start = LogicalPosition::new(626.0, 400.0);
    window.window().dispatch_event(WindowEvent::PointerMoved {
        position: right_start,
    });
    window.window().dispatch_event(WindowEvent::PointerPressed {
        position: right_start,
        button: PointerEventButton::Left,
    });
    window.window().dispatch_event(WindowEvent::PointerMoved {
        position: LogicalPosition::new(586.0, 400.0),
    });
    window
        .window()
        .dispatch_event(WindowEvent::PointerReleased {
            position: LogicalPosition::new(586.0, 400.0),
            button: PointerEventButton::Left,
        });
    assert_eq!(window.get_right_size(), 210.0);

    let left_start = LogicalPosition::new(254.0, 400.0);
    window.window().dispatch_event(WindowEvent::PointerMoved {
        position: left_start,
    });
    window.window().dispatch_event(WindowEvent::PointerPressed {
        position: left_start,
        button: PointerEventButton::Left,
    });
    let left_limit = LogicalPosition::new(754.0, 400.0);
    window.window().dispatch_event(WindowEvent::PointerMoved {
        position: left_limit,
    });
    window
        .window()
        .dispatch_event(WindowEvent::PointerReleased {
            position: left_limit,
            button: PointerEventButton::Left,
        });
    assert_eq!(window.get_left_size(), 414.0);
    assert_eq!(window.get_center_size(), 160.0);
    window.set_left_visible(false);
    window.set_left_visible(true);
    assert_eq!(window.get_left_size(), 414.0);

    let top_limit_start = LogicalPosition::new(400.0, 314.0);
    window.window().dispatch_event(WindowEvent::PointerMoved {
        position: top_limit_start,
    });
    window.window().dispatch_event(WindowEvent::PointerPressed {
        position: top_limit_start,
        button: PointerEventButton::Left,
    });
    let top_limit = LogicalPosition::new(400.0, 714.0);
    window.window().dispatch_event(WindowEvent::PointerMoved {
        position: top_limit,
    });
    window
        .window()
        .dispatch_event(WindowEvent::PointerReleased {
            position: top_limit,
            button: PointerEventButton::Left,
        });
    assert_eq!(window.get_top_size(), 234.0);
    assert_eq!(window.get_center_height(), 120.0);
}
