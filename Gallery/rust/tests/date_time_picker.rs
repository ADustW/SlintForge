// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

include!(concat!(env!("OUT_DIR"), "/slintforge_style.rs"));
use slint::platform::{PointerEventButton, WindowEvent};
use slint::ComponentHandle;

slintforge_ui! {

    export { ForgeDateTimeTestWindow } from "../../../tests/ui/date-time-picker.slint";
}

#[test]
fn calendar_adjacent_month_cells_follow_locale_range_and_draft_commit() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = ForgeDateTimeTestWindow::new().unwrap();
    ui.show().unwrap();
    // 月历左上格在中文周一制与英文周日制下分别是 1 月 29 日、28 日。
    for (language, day) in [("zh", 29), ("en", 28)] {
        ui.set_language(language.into());
        ui.invoke_open_date();
        let position = slint::LogicalPosition::new(40., 114.);
        for event in [
            WindowEvent::PointerMoved { position },
            WindowEvent::PointerPressed {
                position,
                button: PointerEventButton::Left,
            },
            WindowEvent::PointerReleased {
                position,
                button: PointerEventButton::Left,
            },
        ] {
            ui.window().dispatch_event(event);
        }
        assert_eq!(
            ui.get_draft_date(),
            Date {
                year: 2024,
                month: 1,
                day
            }
        );
        assert_eq!(
            ui.get_selected_date(),
            Date {
                year: 2024,
                month: 2,
                day: 28
            }
        );
        ui.invoke_cancel_date();
        assert_eq!(ui.get_selected_date().month, 2);
    }
    ui.set_language("zh".into());
    ui.set_minimum_date(Date {
        year: 2024,
        month: 2,
        day: 1,
    });
    ui.invoke_open_date();
    let position = slint::LogicalPosition::new(40., 114.);
    ui.window().dispatch_event(WindowEvent::PointerPressed {
        position,
        button: PointerEventButton::Left,
    });
    ui.window().dispatch_event(WindowEvent::PointerReleased {
        position,
        button: PointerEventButton::Left,
    });
    assert_eq!(ui.get_draft_date().month, 2);
    assert_eq!(ui.get_draft_date().day, 28);
    ui.invoke_confirm_date();
    assert_eq!(ui.get_date_accepted(), 1);
}

#[test]
fn calendar_localizes_live_and_commits_only_on_confirmation() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = ForgeDateTimeTestWindow::new().unwrap();
    ui.show().unwrap();
    ui.invoke_open_date();
    ui.invoke_choose_day(29);
    assert_eq!(ui.get_selected_date().day, 28);
    assert_eq!(ui.get_date_text(), "2024年2月29日 星期四");
    ui.set_language("en".into());
    assert_eq!(ui.get_date_text(), "Thu, Feb 29, 2024");
    assert_eq!(ui.get_month_text(), "February 2024");
    assert_eq!(ui.get_draft_date().day, 29);
    ui.invoke_choose_day(30);
    assert_eq!(ui.get_draft_date().day, 29);
    ui.invoke_cancel_date();
    assert_eq!(ui.get_selected_date().day, 28);
    assert_eq!(ui.get_date_canceled(), 1);
    assert_eq!(ui.get_date_accepted(), 0);
    ui.invoke_open_date();
    assert_eq!(ui.get_draft_date().day, 28);
    ui.invoke_choose_day(29);
    ui.invoke_confirm_date();
    assert_eq!(ui.get_selected_date().day, 29);
    assert_eq!(ui.get_date_accepted(), 1);
    ui.set_language("zh".into());
    assert_eq!(ui.get_date_text(), "2024年2月29日 星期四");
}

#[test]
fn calendar_handles_centuries_month_navigation_and_range_edges() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = ForgeDateTimeTestWindow::new().unwrap();
    ui.show().unwrap();
    for (year, expected) in [(1900, 28), (2000, 29), (2024, 29), (2100, 28)] {
        assert_eq!(ui.invoke_days_in_month(year, 2), expected);
    }
    for (year, month, day, weekday) in [
        (1, 1, 1, 1),
        (1970, 1, 1, 4),
        (2000, 1, 1, 6),
        (2026, 10, 8, 4),
        (9999, 12, 31, 5),
    ] {
        assert_eq!(ui.invoke_weekday(Date { year, month, day }), weekday);
    }
    ui.set_selected_date(Date {
        year: 2025,
        month: 12,
        day: 31,
    });
    ui.invoke_open_date();
    ui.invoke_browse_month(1);
    assert_eq!(ui.get_month_text(), "2026年1月");
    ui.invoke_choose_day(1);
    ui.invoke_confirm_date();
    assert_eq!(
        ui.get_selected_date(),
        Date {
            year: 2026,
            month: 1,
            day: 1
        }
    );
    ui.invoke_open_date();
    ui.invoke_browse_month(-1);
    assert_eq!(ui.get_month_text(), "2025年12月");
    ui.invoke_browse_month(-200_000);
    assert_eq!(ui.get_month_text(), "1年1月");
    ui.invoke_browse_month(200_000);
    assert_eq!(ui.get_month_text(), "9999年12月");
    ui.invoke_cancel_date();
}

#[test]
fn time_localizes_midnight_noon_seconds_and_cancelled_drafts() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = ForgeDateTimeTestWindow::new().unwrap();
    ui.show().unwrap();
    ui.invoke_open_time();
    assert_eq!(ui.get_time_text(), "上午 12:07:09");
    ui.set_language("en".into());
    assert_eq!(ui.get_time_text(), "12:07:09 AM");
    ui.invoke_set_period(true);
    assert_eq!(ui.get_draft_time().hour, 12);
    assert_eq!(ui.get_time_text(), "12:07:09 PM");
    ui.invoke_set_hour(1);
    assert_eq!(ui.get_draft_time().hour, 13);
    ui.invoke_set_minute(59);
    ui.invoke_set_second(58);
    ui.set_language("zh".into());
    assert_eq!(ui.get_time_text(), "下午 01:59:58");
    ui.invoke_cancel_time();
    assert_eq!(ui.get_selected_time().hour, 0);
    assert_eq!(ui.get_time_canceled(), 1);
    assert_eq!(ui.get_time_accepted(), 0);
    ui.invoke_open_time();
    assert_eq!(ui.get_time_text(), "上午 12:07:09");
    ui.set_use_24_hour_format(true);
    ui.invoke_set_hour(23);
    ui.invoke_set_minute(59);
    ui.invoke_set_second(58);
    assert_eq!(ui.get_time_text(), "23:59:58");
    ui.invoke_confirm_time();
    assert_eq!(
        ui.get_selected_time(),
        Time {
            hour: 23,
            minute: 59,
            second: 58
        }
    );
    assert_eq!(ui.get_time_accepted(), 1);
}
