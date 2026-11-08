// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

include!(concat!(env!("OUT_DIR"), "/slintforge_style.rs"));
use slint::{platform::WindowEvent, ComponentHandle, LogicalPosition, LogicalSize};

slintforge_ui! {
    import { LineEdit, Palette } from "std-widgets.slint";
    import { ForgeWindow } from "../../../ui/window/forge-window.slint";
    import { ForgeTitleBarButton } from "../../../ui/window/internal/title-bar-button.slint";
    import { ForgeNavigationView } from "../../../ui/navigation/navigation-view.slint";
    import { ForgeNavPlacement, ForgePaneState } from "../../../ui/types/navigation.slint";
    import { ForgeTypography } from "../../../ui/typography/typography.slint";
    import { ForgeTheme, ForgeMetrics } from "../../../ui/theme/theme.slint";
    import { ForgeFlowTheme } from "../../../ui/flow/theme.slint";
    import { GalleryRetainedViewport } from "../../ui/component/retained-viewport.slint";
    export { ForgeNavPlacement, ForgePaneState } from "../../../ui/types/navigation.slint";
    export component NavigationShellWindow inherits ForgeWindow {
        preferred-width: 960px; preferred-height: 600px;
        show-back-button: true; show-menu-button: true; leading-menu-button: true;
        show-search-box: true; show-theme-button: true; show-pin-button: true; show-settings-button: true;
        in-out property<bool> allow-back: false;
        back-enabled: root.allow-back;
        in-out property<int> backs;
        in-out property<int> toggles;
        in-out property<string> query;
        in-out property<string> submitted;
        in-out property<bool> theme: false;
        in-out property<float> scale: 1;
        in-out property<ForgeNavPlacement> placement: left;
        in-out property<ForgePaneState> pane: expanded;
        changed theme => { Palette.color-scheme = root.theme ? ColorScheme.dark : ColorScheme.light; }
        changed scale => { ForgeTypography.scale = root.scale; }
        search-text: root.query;
        search-edited(value) => { root.query = value; }
        search-accepted(value) => { root.submitted = value; }
        back-clicked => { root.backs += 1; }
        menu-requested => {
            root.toggles += 1;
            navigation.toggle-pane();
            root.query = "";
        }
        callback focus-search-input();
        focus-search-input => { root.focus-search(); }
        out property<bool> focused: root.search-has-focus;
        out property<float> caption-height: root.title-bar-height / 1px;
        out property<float> content-width: navigation.content-width / 1px;
        out property<float> content-height: navigation.content-height / 1px;
        out property<float> content-x: navigation.content-x / 1px;
        out property<float> content-y: navigation.content-y / 1px;
        out property<float> page-x: page.x / 1px;
        out property<float> page-width: page.width / 1px;
        in-out property<string> draft <=> editor.text;
        navigation := ForgeNavigationView {
            width: root.width; height: root.height - root.title-bar-height;
            placement: root.placement;
            pane-state: root.query != "" ? ForgePaneState.expanded : root.pane;
            pane-state-requested(value) => { root.pane = value; }
            show-app-bar: false; show-pane-header: false; show-pane-search: false;
            page := Rectangle {
                width: navigation.content-width; height: navigation.content-height;
                editor := LineEdit { x: 12px; y: 12px; width: 240px; text: "保留草稿"; }
            }
        }
    }

    export component ThemeContrastWindow inherits Window {
        in-out property<bool> dark;
        changed dark => { Palette.color-scheme = root.dark ? ColorScheme.dark : ColorScheme.light; }
        out property<color> page: Palette.content-background;
        out property<color> surface: Palette.card-background;
        out property<color> text: Palette.foreground;
        out property<color> secondary: Palette.text-secondary;
        out property<color> disabled: Palette.text-disabled;
        out property<color> accent: Palette.accent-background;
        out property<color> on-accent: Palette.accent-foreground;
        out property<color> selected: Palette.navigation-selected;
        out property<color> normal: Palette.control-background;
        out property<color> hover: Palette.control-secondary;
        out property<color> pressed: Palette.control-tertiary;
        out property<color> disabled-fill: Palette.control-disabled;
        out property<color> divider: Palette.divider;
        out property<color> table: Palette.table-background;
        out property<color> table-header: Palette.table-header;
        out property<color> plot: Palette.plot-background;
        out property<color> local-text: ForgeTheme.text-for(root.dark);
        out property<color> local-normal: ForgeTheme.control-for(root.dark, 0);
        in-out property<color> flow-surface <=> ForgeFlowTheme.surface;
        out property<float> search-height: ForgeMetrics.title-bar-search-height / 1px;
    }

    export component RetainedViewportWindow inherits Window {
        width: 960px; height: 600px;
        in-out property<bool> active;
        in-out property<length> available-width: 680px;
        in-out property<length> available-height: 552px;
        in-out property<string> draft <=> retained-editor.text;
        out property<float> retained-width: viewport.content-width / 1px;
        out property<float> retained-height: viewport.content-height / 1px;
        viewport := GalleryRetainedViewport {
            active: root.active;
            viewport-width: root.available-width;
            viewport-height: root.available-height;
            retained-editor := LineEdit { width: viewport.content-width; text: "保留页面草稿"; }
        }
    }

    export component TitlebarFocusWindow inherits Window {
        width: 320px; height: 160px;
        in-out property<bool> dark: false;
        changed dark => { Palette.color-scheme = root.dark ? ColorScheme.dark : ColorScheme.light; }
        in-out property<int> role: 2;
        in-out property<int> clicks;
        in-out property<bool> button-enabled: true;
        out property<bool> button-focused: action.has-focus;
        out property<bool> focus-ring: action.focus-visible;
        out property<bool> editor-focused: editor.has-focus;
        out property<string> text: editor.text;
        callback focus-editor();
        focus-editor => { editor.focus(); }
        action := ForgeTitleBarButton {
            x: 0px; y: 0px; role: root.role; enabled: root.button-enabled;
            clicked => { root.clicks += 1; }
        }
        ForgeTitleBarButton { x: 48px; y: 0px; role: 7; }
        editor := LineEdit { x: 8px; y: 64px; width: 250px; text: "保留输入焦点"; }
    }
}

fn settle() {
    slint::platform::update_timers_and_animations();
}

// 验证可读性和跨组件语义关系，避免主题仅在首页颜色正确。
#[test]
fn light_theme_surfaces_have_readable_text_and_distinct_control_states() {
    fn luminance(c: slint::Color) -> f64 {
        let linear = |v: u8| {
            let s = f64::from(v) / 255.;
            if s <= 0.04045 { s / 12.92 } else { ((s + 0.055) / 1.055).powf(2.4) }
        };
        linear(c.red()) * 0.2126 + linear(c.green()) * 0.7152 + linear(c.blue()) * 0.0722
    }
    fn contrast(a: slint::Color, b: slint::Color) -> f64 {
        let (a, b) = (luminance(a), luminance(b));
        (a.max(b) + 0.05) / (a.min(b) + 0.05)
    }
    i_slint_backend_testing::init_no_event_loop();
    let ui = ThemeContrastWindow::new().unwrap();
    for _ in 0..2 {
        ui.set_dark(false);
        settle();
        for background in [ui.get_page(), ui.get_surface(), ui.get_table(), ui.get_plot(), ui.get_selected()] {
            assert_eq!(background.alpha(), 255, "浅色内容与选择表面须为实色");
            assert!(contrast(ui.get_text(), background) >= 7., "正文对比度不足");
            assert!(contrast(ui.get_secondary(), background) >= 4.5, "说明文字对比度不足");
        }
        assert!(contrast(ui.get_on_accent(), ui.get_accent()) >= 4.5);
        assert_eq!(ui.get_normal(), ui.get_local_normal());
        assert_eq!(ui.get_text(), ui.get_local_text());
        assert_eq!(ui.get_table(), ui.get_plot());
        assert_ne!(ui.get_table(), ui.get_table_header());
        assert_ne!(ui.get_normal(), ui.get_hover());
        assert_ne!(ui.get_hover(), ui.get_pressed());
        assert_ne!(ui.get_text(), ui.get_disabled());
        assert_ne!(ui.get_normal(), ui.get_disabled_fill());
        assert!(contrast(ui.get_divider(), ui.get_page()) >= 1.3);
        assert_eq!(ui.get_search_height(), 28.);
        ui.set_dark(true);
        settle();
        assert_eq!(ui.get_page(), slint::Color::from_rgb_u8(0x28, 0x28, 0x28));
        assert!(contrast(ui.get_text(), ui.get_page()) >= 7.);
        assert_eq!(ui.get_text(), ui.get_local_text());
    }
    let custom = slint::Color::from_rgb_u8(0x43, 0x54, 0x65);
    ui.set_flow_surface(custom);
    ui.set_dark(false);
    settle();
    assert_eq!(ui.get_flow_surface(), custom, "切换主题不能覆盖应用指定的 Flow 颜色");
}
fn click(ui: &NavigationShellWindow, x: f32) {
    use i_slint_core::items::PointerEventButton;
    let position = LogicalPosition::new(x, 18.);
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
    settle();
}
fn key(ui: &NavigationShellWindow, text: &str) {
    ui.window()
        .dispatch_event(WindowEvent::KeyPressed { text: text.into() });
    ui.window()
        .dispatch_event(WindowEvent::KeyReleased { text: text.into() });
    settle();
}

#[test]
fn content_tracks_navigation_extent_without_gaps_and_retains_drafts() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = NavigationShellWindow::new().unwrap();
    ui.show().unwrap();
    assert_eq!(ui.get_caption_height(), 36.);
    for (width, height) in [(960., 600.), (1920., 1080.), (2560., 1440.)] {
        // 使用可调整窗口的首选尺寸，避免把测试窗口锁定成固定宽高。
        ui.window().set_size(LogicalSize::new(width, height));
        for placement in [
            ForgeNavPlacement::Left,
            ForgeNavPlacement::Right,
            ForgeNavPlacement::Top,
        ] {
            ui.set_placement(placement);
            for pane in [
                ForgePaneState::Expanded,
                ForgePaneState::Compact,
                ForgePaneState::Hidden,
            ] {
                ui.set_pane(pane);
                settle();
                let side = if placement == ForgeNavPlacement::Top || pane == ForgePaneState::Hidden
                {
                    0.
                } else if pane == ForgePaneState::Compact {
                    48.
                } else {
                    208.
                };
                let top = if placement == ForgeNavPlacement::Top && pane != ForgePaneState::Hidden {
                    48.
                } else {
                    0.
                };
                assert_eq!(ui.get_content_width(), width - side);
                assert_eq!(ui.get_content_height(), height - 36. - top);
                assert_eq!(
                    ui.get_content_x(),
                    if placement == ForgeNavPlacement::Left {
                        side
                    } else {
                        0.
                    }
                );
                assert_eq!(ui.get_content_y(), top);
                assert_eq!(ui.get_page_x(), 0., "页面不能因重复扣除侧栏宽度而居中留缝");
                assert_eq!(ui.get_page_width(), width - side);
                assert_eq!(ui.get_draft().as_str(), "保留草稿");
            }
        }
    }
}

#[test]
fn leading_actions_and_search_support_keyboard_and_theme_changes() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = NavigationShellWindow::new().unwrap();
    ui.show().unwrap();
    settle();
    click(&ui, 20.);
    assert_eq!(ui.get_backs(), 0);
    ui.invoke_focus_search_input();
    ui.set_allow_back(true);
    click(&ui, 20.);
    assert_eq!(ui.get_backs(), 1);
    assert!(ui.get_focused(), "标题栏鼠标点击不能抢走输入焦点");
    click(&ui, 60.);
    assert_eq!(ui.get_toggles(), 1);
    ui.invoke_focus_search_input();
    key(&ui, "DataGrid");
    assert_eq!(ui.get_query().as_str(), "DataGrid");
    ui.set_theme(true);
    ui.set_scale(1.25);
    settle();
    assert!(ui.get_focused());
    assert_eq!(ui.get_query().as_str(), "DataGrid");
    key(&ui, "\n");
    assert_eq!(ui.get_submitted().as_str(), "DataGrid");
    key(&ui, "\u{1b}");
    assert!(ui.get_query().is_empty());
    assert_eq!(ui.get_draft().as_str(), "保留草稿");
}

#[test]
fn menu_switches_expanded_and_icons_without_hiding_or_losing_state() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = NavigationShellWindow::new().unwrap();
    ui.show().unwrap();
    settle();
    for placement in [
        ForgeNavPlacement::Left,
        ForgeNavPlacement::Right,
        ForgeNavPlacement::Top,
    ] {
        ui.set_placement(placement);
        for _ in 0..4 {
            click(&ui, 60.);
            assert_eq!(ui.get_pane(), ForgePaneState::Compact);
            assert_eq!(
                ui.get_content_width(),
                if placement == ForgeNavPlacement::Top {
                    960.
                } else {
                    912.
                }
            );
            click(&ui, 60.);
            assert_eq!(ui.get_pane(), ForgePaneState::Expanded);
            assert_eq!(ui.get_draft().as_str(), "保留草稿");
        }
        ui.set_pane(ForgePaneState::Hidden);
        click(&ui, 60.);
        assert_eq!(ui.get_pane(), ForgePaneState::Expanded);
        ui.set_pane(ForgePaneState::Compact);
        ui.set_query("按钮".into());
        settle();
        click(&ui, 60.);
        assert_eq!(ui.get_pane(), ForgePaneState::Compact);
        assert!(ui.get_query().is_empty());
        click(&ui, 60.);
        assert_eq!(ui.get_pane(), ForgePaneState::Expanded);
    }
}

#[test]
fn caption_buttons_keep_pointer_focus_outlines_off_and_keyboard_focus_visible() {
    use i_slint_core::items::PointerEventButton;
    i_slint_backend_testing::init_no_event_loop();
    let ui = TitlebarFocusWindow::new().unwrap();
    ui.show().unwrap();
    settle();
    let click_at = |x| {
        let position = LogicalPosition::new(x, 18.);
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
        settle();
    };
    let send_key = |text: &str| {
        ui.window()
            .dispatch_event(WindowEvent::KeyPressed { text: text.into() });
        ui.window()
            .dispatch_event(WindowEvent::KeyReleased { text: text.into() });
        settle();
    };
    for dark in [false, true] {
        ui.set_dark(dark);
        for role in 1..=8 {
            ui.set_role(role);
            ui.invoke_focus_editor();
            settle();
            let before = ui.get_clicks();
            click_at(24.);
            assert_eq!(ui.get_clicks(), before + 1);
            assert!(ui.get_editor_focused());
            assert!(!ui.get_button_focused());
            assert!(!ui.get_focus_ring());
        }
        // 从最后的输入框 Tab 回到第一个按钮，验证真实键盘焦点路径。
        send_key("\t");
        assert!(ui.get_button_focused());
        assert!(ui.get_focus_ring());
        let before = ui.get_clicks();
        send_key(" ");
        send_key("\n");
        assert_eq!(ui.get_clicks(), before + 2);
        click_at(72.);
        assert!(!ui.get_focus_ring(), "改用鼠标后所有标题栏按钮都隐藏焦点框");
        assert_eq!(ui.get_text().as_str(), "保留输入焦点");
    }
    ui.set_button_enabled(false);
    let before = ui.get_clicks();
    click_at(24.);
    assert_eq!(ui.get_clicks(), before);
    assert!(!ui.get_focus_ring());
}

#[test]
fn hidden_heavy_pages_keep_extent_and_resume_with_current_size() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = RetainedViewportWindow::new().unwrap();
    ui.show().unwrap();
    ui.set_active(true);
    settle();
    assert_eq!(
        (ui.get_retained_width(), ui.get_retained_height()),
        (680., 552.)
    );
    ui.set_active(false);
    settle();
    for (width, height) in [(912., 552.), (1640., 1032.), (2280., 1392.)] {
        ui.set_available_width(width);
        ui.set_available_height(height);
        settle();
        assert_eq!(
            (ui.get_retained_width(), ui.get_retained_height()),
            (680., 552.)
        );
        assert_eq!(ui.get_draft().as_str(), "保留页面草稿");
    }
    ui.set_active(true);
    // 显示时立即取得当前尺寸，不等待 changed 回调，避免首次 fit 使用旧尺寸。
    assert_eq!(
        (ui.get_retained_width(), ui.get_retained_height()),
        (2280., 1392.)
    );
    settle();
    ui.set_active(false);
    settle();
    assert_eq!(
        (ui.get_retained_width(), ui.get_retained_height()),
        (2280., 1392.)
    );
    assert_eq!(ui.get_draft().as_str(), "保留页面草稿");
}
