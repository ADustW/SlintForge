// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#include "date-time-picker.h"
#include <iostream>
#include <stdexcept>
#include <string_view>

static void forge_picker_require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}

int main() {
    auto ui = ForgeDateTimeTestWindow::create();
    ui->show();
    ui->invoke_open_date();
    ui->invoke_choose_day(29);
    forge_picker_require(ui->get_selected_date().day == 28, "未确认日期发生变化");
    forge_picker_require(std::string_view(ui->get_date_text()) == "2024年2月29日 星期四", "中文日期");
    ui->set_language("en");
    forge_picker_require(std::string_view(ui->get_date_text()) == "Thu, Feb 29, 2024", "英文日期");
    forge_picker_require(std::string_view(ui->get_month_text()) == "February 2024", "英文月份");
    ui->invoke_choose_day(30);
    forge_picker_require(ui->get_draft_date().day == 29, "非法日期改变选择");
    ui->invoke_cancel_date();
    forge_picker_require(ui->get_selected_date().day == 28 && ui->get_date_canceled() == 1 && ui->get_date_accepted() == 0, "取消日期");
    ui->invoke_open_date();
    forge_picker_require(ui->get_draft_date().day == 28, "重新打开日期");
    ui->invoke_choose_day(29);
    ui->invoke_confirm_date();
    forge_picker_require(ui->get_selected_date().day == 29 && ui->get_date_accepted() == 1, "确认日期");
    forge_picker_require(ui->invoke_days_in_month(1900, 2) == 28 && ui->invoke_days_in_month(2000, 2) == 29 && ui->invoke_days_in_month(2100, 2) == 28, "世纪闰年");
    forge_picker_require(ui->invoke_weekday(Date{.year=1, .month=1, .day=1}) == 1 && ui->invoke_weekday(Date{.year=2026, .month=10, .day=8}) == 4 && ui->invoke_weekday(Date{.year=9999, .month=12, .day=31}) == 5, "公历星期");
    ui->set_selected_date(Date{.year=2025, .month=12, .day=31});
    ui->invoke_open_date();
    ui->invoke_browse_month(1);
    forge_picker_require(std::string_view(ui->get_month_text()) == "January 2026", "跨年下个月");
    ui->invoke_browse_month(-1);
    forge_picker_require(std::string_view(ui->get_month_text()) == "December 2025", "跨年上个月");
    ui->invoke_browse_month(-200000);
    forge_picker_require(std::string_view(ui->get_month_text()) == "January 1", "日期下界");
    ui->invoke_browse_month(200000);
    forge_picker_require(std::string_view(ui->get_month_text()) == "December 9999", "日期上界");
    ui->invoke_cancel_date();
    ui->set_language("zh");
    ui->invoke_open_time();
    forge_picker_require(std::string_view(ui->get_time_text()) == "上午 12:07:09", "中文午夜");
    ui->set_language("en");
    forge_picker_require(std::string_view(ui->get_time_text()) == "12:07:09 AM", "英文午夜");
    ui->invoke_set_period(true);
    forge_picker_require(ui->get_draft_time().hour == 12 && std::string_view(ui->get_time_text()) == "12:07:09 PM", "正午");
    ui->invoke_set_hour(1);
    ui->invoke_set_minute(59);
    ui->invoke_set_second(58);
    ui->set_language("zh");
    forge_picker_require(std::string_view(ui->get_time_text()) == "下午 01:59:58", "中文下午保留分秒");
    ui->invoke_cancel_time();
    forge_picker_require(ui->get_selected_time().hour == 0 && ui->get_time_canceled() == 1 && ui->get_time_accepted() == 0, "取消时间");
    ui->invoke_open_time();
    forge_picker_require(std::string_view(ui->get_time_text()) == "上午 12:07:09", "重新打开时间");
    ui->set_use_24_hour_format(true);
    ui->invoke_set_hour(23);
    ui->invoke_set_minute(59);
    ui->invoke_set_second(58);
    ui->invoke_confirm_time();
    forge_picker_require(ui->get_selected_time().hour == 23 && ui->get_selected_time().minute == 59 && ui->get_selected_time().second == 58 && ui->get_time_accepted() == 1, "确认时间");
    ui->hide();
    std::cout << "DATE_TIME_PICKER_OK languages=2 calendar=leap_weekday_navigation_commit_cancel time=midnight_noon_seconds_commit_cancel\n";
}
