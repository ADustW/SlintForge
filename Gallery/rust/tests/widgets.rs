// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

include!(concat!(env!("OUT_DIR"), "/slintforge_style.rs"));
use slint::platform::{Key, WindowEvent};
use slint::{ComponentHandle, LogicalPosition, Model, ModelRc, VecModel};
use std::time::Duration;

slintforge_ui! {

    import { LineEdit } from "std-widgets.slint";
    import { ForgePillButton, ForgeDelayButton, ForgeProgressButton, ForgeDropDownButton } from "../../../ui/inputs/action-buttons.slint";
    import { ForgeInputValidation } from "../../../ui/inputs/validation.slint";
    import { ForgeTagInputBox } from "../../../ui/inputs/tags.slint";
    import { ForgeNumberBox } from "../../../ui/form/number-box.slint";
    import { ForgeShortcutPicker } from "../../../ui/form/shortcut-picker.slint";
    import { ForgeAutoSuggestBox } from "../../../ui/form/auto-suggest.slint";
    import { ForgeMultiSelectComboBox } from "../../../ui/form/multi-select.slint";
    import { ForgeSegmentedControl } from "../../../ui/navigation/segmented-control.slint";
    import { ForgePagination } from "../../../ui/navigation/pagination.slint";
    import { ForgeRatingControl } from "../../../ui/feedback/rating.slint";
    import { ForgeShimmer, ForgeSkeletonView } from "../../../ui/feedback/shimmer.slint";
    import { ForgeDial } from "../../../ui/other/dial.slint";
    import { ForgeContentDialog } from "../../../ui/popups/content-dialog.slint";
    import { ForgeDrawer } from "../../../ui/popups/drawer.slint";
    import { ForgeTour } from "../../../ui/other/tour.slint";
    import { ForgeChoiceItem } from "../../../ui/types/widgets.slint";
    import { ForgeTreeItem } from "../../../ui/types/widgets.slint";
    import { ForgeColorPicker } from "../../../ui/form/color-picker.slint";
    import { ForgeFileTree } from "../../../ui/data/file-tree.slint";
    export component WidgetTestWindow inherits Window {
        width: 1080px; height: 850px;
        in-out property<bool> enabled: true;
        in-out property<int> pill-clicks: 0;
        in-out property<int> delay-clicks: 0;
        in-out property<int> progress-clicks: 0;
        in-out property<bool> busy: false;
        out property<float> hold-progress: delay.progress;
        in-out property<string> input <=> validation.text;
        in-out property<string> remote-error;
        out property<bool> valid: validation.valid;
        in-out property<int> accepted-inputs: 0;
        in-out property<float> number <=> number-box.value;
        in-out property<float> number-min: -10;
        in-out property<float> number-max: 10;
        in-out property<[string]> tags: ["Slint"];
        in-out property<string> draft <=> tags-box.draft;
        in-out property<string> tag-request;
        callback submit-tag(); submit-tag => { tags-box.submit(); }
        callback set-number(float); set-number(value) => { number-box.set-value(value); }
        callback commit-number(); commit-number => { number-box.commit(); }
        out property<bool> recording: shortcut.recording;
        out property<string> shortcut-value: shortcut.shortcut;
        callback record(); record => { shortcut.start-recording(); }
        callback clear-shortcut(); clear-shortcut => { shortcut.clear(); }
        callback confirm-shortcut(); confirm-shortcut => { shortcut.confirm(); }
        in-out property<string> suggest-text <=> suggest.text;
        in-out property<bool> suggestions-open <=> suggest.open;
        in-out property<string> suggestion-picked;
        in-out property<[ForgeChoiceItem]> choices <=> multi.items;
        out property<bool> multi-open: multi.open;
        in-out property<int> selection-count: 0;
        in-out property<string> segment <=> segments.selected-id;
        in-out property<int> total-items: 237;
        in-out property<int> page-size: 10;
        in-out property<int> page <=> pages.current-page;
        out property<int> page-count: pages.page-count;
        callback go(int); go(value) => { pages.go-to(value); }
        in-out property<int> rating <=> stars.value;
        in-out property<bool> rating-readonly: false;
        in-out property<int> rating-maximum: 5;
        callback rate(int); rate(value) => { stars.set-value(value); }
        in-out property<float> dial-value <=> dial.value;
        in-out property<bool> shimmer-running: true;
        in-out property<int> shimmer-cycle: 1600;
        out property<float> phase: shimmer.phase;
        in-out property<bool> loading <=> skeleton.loading;
        out property<string> retained: retained-input.text;
        callback set-retained(string); set-retained(value) => { retained-input.text = value; }
        in-out property<bool> dialog-open <=> dialog.open;
        in-out property<bool> drawer-open <=> drawer.open;
        in-out property<bool> primary-enabled: true;
        in-out property<int> dialog-accepted: 0;
        in-out property<int> dialog-canceled: 0;
        in-out property<int> behind-clicks: 0;
        in-out property<bool> tour-active <=> tour.active;
        out property<int> tour-step: tour.current-step;
        in-out property<int> tours-finished: 0;
        in-out property<int> tours-dismissed: 0;
        callback next-step(); next-step => { tour.next(); }
        callback previous-step(); previous-step => { tour.previous(); }
        callback start-tour(); start-tour => { tour.start(); }
        in-out property<string> dropdown-picked;
        out property<bool> dropdown-open: dropdown.open;
        in-out property<int> color-red <=> colors.red;
        in-out property<int> color-alpha <=> colors.alpha;
        out property<color> color-value: colors.value;
        in-out property<int> color-events: 0;
        in-out property<[ForgeTreeItem]> tree;
        in-out property<bool> tree-multi: false;
        in-out property<string> tree-expanded-id;
        in-out property<string> tree-selected-id;

        pill := ForgePillButton { x: 10px; y: 10px; width: 220px; height: 36px; text: "Pill"; enabled: root.enabled; clicked => { root.pill-clicks += 1; } }
        delay := ForgeDelayButton { x: 10px; y: 60px; width: 220px; height: 36px; delay: 200ms; enabled: root.enabled; clicked => { root.delay-clicks += 1; } }
        ForgeProgressButton { x: 10px; y: 110px; width: 220px; height: 36px; busy: root.busy; enabled: root.enabled; clicked => { root.progress-clicks += 1; } }
        validation := ForgeInputValidation { x: 10px; y: 160px; width: 240px; height: 72px; minimum-length: 3; maximum-length: 4; error-message: root.remote-error; enabled: root.enabled; accepted(value) => { root.accepted-inputs += 1; } }
        number-box := ForgeNumberBox { x: 10px; y: 245px; width: 250px; height: 36px; minimum: root.number-min; maximum: root.number-max; step: 0.25; enabled: root.enabled; }
        tags-box := ForgeTagInputBox { x: 10px; y: 295px; width: 270px; height: 100px; tags: root.tags; maximum-tags: 2; enabled: root.enabled; add-requested(value) => { root.tag-request = value; } }
        shortcut := ForgeShortcutPicker { x: 10px; y: 425px; width: 300px; height: 36px; shortcut: "Ctrl+K"; enabled: root.enabled; }
        suggest := ForgeAutoSuggestBox { x: 330px; y: 10px; width: 260px; height: self.preferred-height; suggestions: ["Slint", "Slider", "Rust"]; enabled: root.enabled; selected(value) => { root.suggestion-picked = value; } }
        multi := ForgeMultiSelectComboBox { x: 330px; y: 220px; width: 260px; height: 36px; enabled: root.enabled; selection-changed(id, checked) => { root.selection-count += 1; } }
        segments := ForgeSegmentedControl { x: 330px; y: 280px; width: 300px; height: 36px; items: [{id: "a", label: "A", enabled: true}, {id: "b", label: "B", enabled: false}, {id: "c", label: "C", enabled: true}]; selected-id: "a"; }
        pages := ForgePagination { x: 330px; y: 340px; width: 720px; height: 36px; total-items: root.total-items; page-size: root.page-size; enabled: root.enabled; }
        stars := ForgeRatingControl { x: 330px; y: 400px; width: 350px; height: 38px; maximum: root.rating-maximum; read-only: root.rating-readonly; enabled: root.enabled; }
        dial := ForgeDial { x: 330px; y: 480px; width: 180px; height: 180px; value: 40; minimum: 0; maximum: 100; step: 5; enabled: root.enabled; }
        shimmer := ForgeShimmer { x: 10px; y: 500px; width: 280px; height: 30px; running: root.shimmer-running; cycle: root.shimmer-cycle * 1ms; }
        skeleton := ForgeSkeletonView { x: 10px; y: 550px; width: 280px; height: 190px; retained-input := LineEdit { text: "before"; } }
        dropdown := ForgeDropDownButton { x: 650px; y: 10px; width: 240px; height: 36px; items: [{id: "one", label: "One", enabled: true}, {id: "two", label: "Disabled", enabled: false}]; selected(id) => { root.dropdown-picked = id; } }
        ForgeFileTree { x: 650px; y: 60px; width: 410px; height: 145px; items: root.tree; multi-select: root.tree-multi; expansion-changed(id, value) => { root.tree-expanded-id = id; } selection-changed(id, value) => { root.tree-selected-id = id; } }
        colors := ForgeColorPicker { x: 720px; y: 480px; width: 320px; height: 280px; color-changed(value) => { root.color-events += 1; } }
        TouchArea { x: 950px; y: 760px; width: 100px; height: 60px; clicked => { root.behind-clicks += 1; } }
        dialog := ForgeContentDialog { open: false; title: "Confirm"; message: "Continue?"; primary-enabled: root.primary-enabled; accepted => { root.dialog-accepted += 1; } canceled => { root.dialog-canceled += 1; } }
        drawer := ForgeDrawer { title: "Settings"; panel-width: 300px; LineEdit { text: "drawer"; } }
        tour := ForgeTour { steps: [{title: "Input", description: "First", x: 10px, y: 160px, width: 240px, height: 36px}, {title: "Dial", description: "Second", x: 330px, y: 480px, width: 180px, height: 180px}]; finished => { root.tours-finished += 1; } dismissed => { root.tours-dismissed += 1; } }
    }
}

fn tick(ms: u64) {
    i_slint_backend_testing::mock_elapsed_time(Duration::from_millis(ms));
}
fn pointer(ui: &WidgetTestWindow, x: f32, y: f32, press: bool) {
    use i_slint_core::items::PointerEventButton;
    let position = LogicalPosition::new(x, y);
    ui.window()
        .dispatch_event(WindowEvent::PointerMoved { position });
    ui.window().dispatch_event(if press {
        WindowEvent::PointerPressed {
            position,
            button: PointerEventButton::Left,
        }
    } else {
        WindowEvent::PointerReleased {
            position,
            button: PointerEventButton::Left,
        }
    });
    tick(1);
}
fn click(ui: &WidgetTestWindow, x: f32, y: f32) {
    pointer(ui, x, y, true);
    pointer(ui, x, y, false);
}
fn key(ui: &WidgetTestWindow, key: impl Into<slint::SharedString>) {
    let text = key.into();
    ui.window()
        .dispatch_event(WindowEvent::KeyPressed { text: text.clone() });
    ui.window()
        .dispatch_event(WindowEvent::KeyReleased { text });
    tick(1);
}

#[test]
fn widgets_handle_pointer_keyboard_models_and_edge_values() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = WidgetTestWindow::new().unwrap();
    ui.show().unwrap();
    tick(1);
    click(&ui, 40., 26.);
    assert_eq!(ui.get_pill_clicks(), 1);
    key(&ui, " ");
    assert_eq!(ui.get_pill_clicks(), 2);
    ui.set_enabled(false);
    key(&ui, " ");
    click(&ui, 40., 26.);
    assert_eq!(ui.get_pill_clicks(), 2);
    ui.set_enabled(true);
    pointer(&ui, 40., 76., true);
    tick(80);
    pointer(&ui, 40., 76., false);
    assert_eq!(ui.get_delay_clicks(), 0);
    pointer(&ui, 40., 76., true);
    for _ in 0..10 {
        tick(25);
    }
    assert!(ui.get_hold_progress() >= 1.);
    pointer(&ui, 40., 76., false);
    assert_eq!(ui.get_delay_clicks(), 1);
    // 已获得焦点的长按按钮也通过键盘累计时间。
    ui.window()
        .dispatch_event(WindowEvent::KeyPressed { text: " ".into() });
    tick(1);
    for _ in 0..10 {
        tick(25);
    }
    key(&ui, " ");
    assert_eq!(ui.get_delay_clicks(), 2);
    // 焦点离开或拖出后，下一次短按不得继承前一次的完成状态。
    ui.window()
        .dispatch_event(WindowEvent::KeyPressed { text: " ".into() });
    tick(25);
    click(&ui, 40., 26.);
    tick(250);
    ui.window()
        .dispatch_event(WindowEvent::KeyReleased { text: " ".into() });
    assert_eq!(ui.get_delay_clicks(), 2);
    assert_eq!(ui.get_hold_progress(), 0.);
    pointer(&ui, 40., 76., true);
    for _ in 0..10 {
        tick(25);
    }
    pointer(&ui, 290., 76., false);
    assert_eq!(ui.get_delay_clicks(), 2);
    click(&ui, 40., 76.);
    assert_eq!(ui.get_delay_clicks(), 2);
    pointer(&ui, 40., 76., true);
    tick(25);
    ui.set_enabled(false);
    tick(250);
    pointer(&ui, 40., 76., false);
    assert_eq!(ui.get_delay_clicks(), 2);
    ui.set_enabled(true);
    click(&ui, 40., 126.);
    assert_eq!(ui.get_progress_clicks(), 1);
    ui.set_busy(true);
    click(&ui, 40., 126.);
    assert_eq!(ui.get_progress_clicks(), 1);
    ui.set_busy(false);
    assert!(!ui.get_valid());
    ui.set_input("中文🙂".into());
    assert!(ui.get_valid());
    click(&ui, 100., 175.);
    key(&ui, Key::Return);
    assert_eq!(ui.get_accepted_inputs(), 1);
    ui.set_input("中文🙂ab".into());
    assert!(!ui.get_valid());
    key(&ui, Key::Return);
    assert_eq!(ui.get_accepted_inputs(), 1);
    ui.set_input("abc".into());
    ui.set_remote_error("server".into());
    assert!(!ui.get_valid());
    ui.set_remote_error("".into());
    assert!(ui.get_valid());
    ui.invoke_set_number(100.);
    assert_eq!(ui.get_number(), 10.);
    ui.invoke_set_number(-20.);
    assert_eq!(ui.get_number(), -10.);
    ui.invoke_set_number(1.25);
    click(&ui, 120., 263.);
    key(&ui, Key::UpArrow);
    assert_eq!(ui.get_number(), 1.5);
    ui.set_number_max(1.);
    tick(1);
    assert_eq!(ui.get_number(), 1.);
    ui.set_number_min(2.);
    tick(1);
    assert_eq!(ui.get_number(), 2.);
    ui.set_enabled(false);
    ui.invoke_set_number(9.);
    assert_eq!(ui.get_number(), 2.);
    ui.set_enabled(true);
    ui.set_draft("Rust".into());
    ui.invoke_submit_tag();
    assert_eq!(ui.get_tag_request().as_str(), "Rust");
    assert!(ui.get_draft().is_empty());
    ui.set_tags(ModelRc::new(VecModel::from(vec!["A".into(), "B".into()])));
    ui.set_draft("C".into());
    ui.invoke_submit_tag();
    assert_eq!(ui.get_tag_request().as_str(), "Rust");
    assert_eq!(ui.get_draft().as_str(), "C");
    ui.invoke_record();
    key(&ui, Key::Escape);
    assert!(!ui.get_recording());
    assert_eq!(ui.get_shortcut_value().as_str(), "Ctrl+K");
    ui.invoke_record();
    ui.window().dispatch_event(WindowEvent::KeyPressed {
        text: Key::Control.into(),
    });
    tick(1);
    assert!(ui.get_recording());
    key(&ui, "j");
    ui.window().dispatch_event(WindowEvent::KeyReleased {
        text: Key::Control.into(),
    });
    assert_eq!(ui.get_shortcut_value().as_str(), "Ctrl+K");
    ui.invoke_confirm_shortcut();
    assert_eq!(ui.get_shortcut_value().as_str(), "Ctrl+J");
    ui.invoke_clear_shortcut();
    assert!(ui.get_shortcut_value().is_empty());
    ui.set_suggest_text("sl".into());
    ui.set_suggestions_open(true);
    tick(1);
    click(&ui, 420., 64.);
    assert_eq!(ui.get_suggestion_picked().as_str(), "Slint");
    assert!(!ui.get_suggestions_open());
    let choices = vec![
        ForgeChoiceItem {
            id: "a".into(),
            label: "A".into(),
            enabled: true,
            selected: false,
        },
        ForgeChoiceItem {
            id: "b".into(),
            label: "B".into(),
            enabled: false,
            selected: false,
        },
    ];
    ui.set_choices(ModelRc::new(VecModel::from(choices)));
    click(&ui, 420., 238.);
    assert!(ui.get_multi_open());
    click(&ui, 370., 282.);
    assert!(ui.get_choices().row_data(0).unwrap().selected);
    assert_eq!(ui.get_selection_count(), 1);
    click(&ui, 370., 320.);
    assert_eq!(ui.get_selection_count(), 1);
    click(&ui, 1000., 700.);
    assert!(!ui.get_multi_open());
    click(&ui, 420., 238.);
    assert!(ui.get_choices().row_data(0).unwrap().selected);
    ui.set_enabled(false);
    tick(1);
    assert!(!ui.get_multi_open());
    ui.set_enabled(true);
    key(&ui, Key::Escape);
    click(&ui, 480., 298.);
    assert_eq!(ui.get_segment().as_str(), "a");
    click(&ui, 580., 298.);
    assert_eq!(ui.get_segment().as_str(), "c");
    assert_eq!(ui.get_page_count(), 24);
    ui.invoke_go(999);
    assert_eq!(ui.get_page(), 24);
    ui.invoke_go(-3);
    assert_eq!(ui.get_page(), 1);
    ui.set_total_items(0);
    tick(1);
    assert_eq!((ui.get_page(), ui.get_page_count()), (0, 0));
    ui.set_total_items(20);
    ui.set_page_size(0);
    tick(1);
    assert_eq!(ui.get_page_count(), 0);
    ui.set_page_size(10);
    tick(1);
    assert_eq!(ui.get_page(), 1);
    ui.invoke_rate(99);
    assert_eq!(ui.get_rating(), 5);
    ui.set_rating_readonly(true);
    ui.invoke_rate(1);
    click(&ui, 350., 419.);
    assert_eq!(ui.get_rating(), 5);
    ui.set_rating_readonly(false);
    ui.invoke_rate(2);
    click(&ui, 400., 419.);
    key(&ui, Key::End);
    assert_eq!(ui.get_rating(), 5);
    ui.set_rating_maximum(25);
    tick(1);
    key(&ui, Key::End);
    assert_eq!(ui.get_rating(), 25);
    ui.set_rating_maximum(3);
    tick(1);
    assert_eq!(ui.get_rating(), 3);
    ui.set_rating_maximum(-1);
    tick(1);
    assert_eq!(ui.get_rating(), 0);
    ui.set_rating_maximum(5);
    tick(1);
    pointer(&ui, 420., 570., true);
    pointer(&ui, 420., 500., false);
    assert!(ui.get_dial_value() > 85.);
    key(&ui, Key::Home);
    assert_eq!(ui.get_dial_value(), 0.);
    key(&ui, Key::RightArrow);
    assert_eq!(ui.get_dial_value(), 5.);
    ui.set_enabled(false);
    key(&ui, Key::End);
    assert_eq!(ui.get_dial_value(), 5.);
    ui.set_enabled(true);
    tick(40);
    ui.set_shimmer_running(false);
    tick(1);
    let phase = ui.get_phase();
    tick(200);
    assert_eq!(ui.get_phase(), phase);
    ui.set_shimmer_cycle(0);
    assert_eq!(ui.get_phase(), 0.);
    ui.invoke_set_retained("retained".into());
    ui.set_loading(false);
    tick(1);
    ui.set_loading(true);
    tick(1);
    ui.set_loading(false);
    assert_eq!(ui.get_retained().as_str(), "retained");
    click(&ui, 760., 28.);
    assert!(ui.get_dropdown_open());
    click(&ui, 740., 103.);
    assert!(ui.get_dropdown_picked().is_empty());
    click(&ui, 740., 65.);
    assert_eq!(ui.get_dropdown_picked().as_str(), "one");
    assert!(!ui.get_dropdown_open());
    ui.set_color_red(999);
    ui.set_color_alpha(-20);
    tick(1);
    assert_eq!(ui.get_color_value().red(), 255);
    assert_eq!(ui.get_color_value().alpha(), 0);
    assert!(ui.get_color_events() > 0);
    let tree = vec![
        ForgeTreeItem {
            id: "project".into(),
            label: "Project".into(),
            folder: true,
            visible: true,
            enabled: true,
            ..Default::default()
        },
        ForgeTreeItem {
            id: "file".into(),
            label: "file.txt".into(),
            parent_id: "project".into(),
            depth: 1,
            visible: true,
            enabled: true,
            ..Default::default()
        },
        ForgeTreeItem {
            id: "locked".into(),
            label: "locked.txt".into(),
            depth: 1,
            visible: true,
            enabled: false,
            ..Default::default()
        },
    ];
    ui.set_tree(ModelRc::new(VecModel::from(tree)));
    tick(1);
    click(&ui, 665., 78.);
    assert_eq!(ui.get_tree_expanded_id().as_str(), "project");
    click(&ui, 800., 114.);
    assert_eq!(ui.get_tree_selected_id().as_str(), "file");
    click(&ui, 800., 150.);
    assert_eq!(ui.get_tree_selected_id().as_str(), "file");
    ui.set_tree_multi(true);
    tick(1);
    click(&ui, 760., 78.);
    assert_eq!(ui.get_tree_selected_id().as_str(), "project");
}

#[test]
fn overlays_block_background_and_handle_focus_escape_and_tour() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = WidgetTestWindow::new().unwrap();
    ui.show().unwrap();
    tick(1);
    click(&ui, 990., 780.);
    assert_eq!(ui.get_behind_clicks(), 1);
    ui.set_dialog_open(true);
    tick(1);
    click(&ui, 990., 780.);
    assert_eq!(ui.get_behind_clicks(), 1);
    assert!(ui.get_dialog_open());
    key(&ui, Key::Escape);
    assert!(!ui.get_dialog_open());
    assert_eq!(ui.get_dialog_canceled(), 1);
    ui.set_dialog_open(true);
    tick(1);
    key(&ui, Key::Return);
    assert!(!ui.get_dialog_open());
    assert_eq!(ui.get_dialog_accepted(), 1);
    ui.set_primary_enabled(false);
    ui.set_dialog_open(true);
    tick(1);
    key(&ui, Key::Tab);
    key(&ui, Key::Return);
    assert_eq!(ui.get_dialog_accepted(), 1);
    assert!(!ui.get_dialog_open());
    ui.set_drawer_open(true);
    tick(1);
    click(&ui, 860., 90.);
    key(&ui, Key::Escape);
    assert!(!ui.get_drawer_open());
    ui.set_drawer_open(true);
    tick(1);
    click(&ui, 50., 780.);
    assert!(!ui.get_drawer_open());
    ui.set_tour_active(true);
    tick(1);
    assert_eq!(ui.get_tour_step(), 0);
    key(&ui, Key::RightArrow);
    assert_eq!(ui.get_tour_step(), 1);
    ui.invoke_previous_step();
    assert_eq!(ui.get_tour_step(), 0);
    ui.invoke_next_step();
    ui.invoke_next_step();
    assert!(!ui.get_tour_active());
    assert_eq!(ui.get_tours_finished(), 1);
    ui.invoke_start_tour();
    tick(1);
    assert_eq!(ui.get_tour_step(), 0);
    key(&ui, Key::Escape);
    assert!(!ui.get_tour_active());
    assert_eq!(ui.get_tours_dismissed(), 1);
}
