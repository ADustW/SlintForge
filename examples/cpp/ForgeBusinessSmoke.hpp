#pragma once
#include <cstdlib>

// 真实窗口的验证失败在 Release 中也必须返回非零，不依赖 assert。
static void forge_business_require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
static void forge_business_key(slint::Window& window, std::u8string_view key) {
    window.dispatch_key_press_event(key); window.dispatch_key_release_event(key);
}
static void forge_business_save(const slint::Window& window, const char* name) {
    const auto setting = std::getenv("SLINTFORGE_INTERACTION_QA_DIR");
    const auto directory = std::filesystem::path(setting ? setting : "build/business-qa/cpp");
    std::filesystem::create_directories(directory);
    const auto image = forge_capture_window(window);
    if (!image.ok()) throw std::runtime_error(image.error);
    const auto error = forge_save_capture_png(*image.pixels, directory / name);
    if (!error.empty()) throw std::runtime_error(error);
}
static int forge_business_suggestion(bool smoke) {
    auto ui = SuggestionWindow::create();
    forge_bind_widget_services<ForgeWidgetServices, ForgeQRCodeResult, ForgeSyntaxLine>(ui);
    slint::Timer timer; const auto phase = std::make_shared<int>(0); const auto failure = std::make_shared<std::string>();
    const slint::ComponentWeakHandle<SuggestionWindow> weak(ui);
    if (smoke) timer.start(slint::TimerMode::Repeated, std::chrono::milliseconds(180), [weak, phase, failure]() {
        try {
            auto window = *weak.lock();
            switch ((*phase)++) {
            case 0: window->set_query("sl"); window->invoke_focus_input(); break;
            case 1: forge_business_key(window->window(), slint::platform::key_codes::DownArrow); forge_business_key(window->window(), slint::platform::key_codes::DownArrow); break;
            case 2: forge_business_require(window->get_candidate_index() == 1, "候选索引不正确"); forge_business_save(window->window(), "suggestion-open.png"); forge_business_key(window->window(), slint::platform::key_codes::Return); break;
            case 3: forge_business_require(window->get_picked() == "Slider" && window->get_selections() == 1 && !window->get_suggestions_open(), "候选提交失败"); window->set_input_enabled(false); window->invoke_move_candidate(1); break;
            default: forge_business_require(!window->get_suggestions_open(), "禁用时仍展开建议"); std::cout << "BUSINESS_SUGGESTION_OK host=cpp\n"; slint::quit_event_loop();
            }
        } catch (const std::exception& error) { *failure = error.what(); slint::quit_event_loop(); }
    });
    ui->run(); if (!failure->empty()) { std::cerr << "BUSINESS_SUGGESTION_FAILED " << *failure << '\n'; return 1; } return 0;
}
static int forge_business_dialog(bool smoke) {
    auto ui = BusinessDialogWindow::create();
    slint::Timer timer; const auto phase = std::make_shared<int>(0); const auto failure = std::make_shared<std::string>();
    const slint::ComponentWeakHandle<BusinessDialogWindow> weak(ui);
    if (smoke) timer.start(slint::TimerMode::Repeated, std::chrono::milliseconds(180), [weak, phase, failure]() {
        try {
            auto window = *weak.lock();
            switch ((*phase)++) {
            case 0: window->invoke_open_dialog(); break;
            case 1: forge_business_require(window->get_focus_position() == 0, "初始焦点不正确"); forge_business_key(window->window(), slint::platform::key_codes::Tab); break;
            case 2: forge_business_require(window->get_focus_position() == 2, "未跳过禁用输入"); forge_business_key(window->window(), slint::platform::key_codes::Tab); break;
            case 3: forge_business_require(window->get_focus_position() == 3, "操作按钮焦点不正确"); window->window().dispatch_key_press_event(slint::platform::key_codes::Shift); forge_business_key(window->window(), slint::platform::key_codes::Tab); window->window().dispatch_key_release_event(slint::platform::key_codes::Shift); break;
            case 4: forge_business_require(window->get_focus_position() == 2, "反向遍历失败"); window->set_device_name("设备 A"); window->set_directory("D:/记录 数据"); forge_business_save(window->window(), "dialog-open.png"); forge_business_key(window->window(), slint::platform::key_codes::Escape); break;
            case 5: forge_business_require(window->get_launch_focused() && window->get_canceled_count() == 1, "取消后未恢复焦点"); window->invoke_open_dialog(); break;
            case 6: forge_business_require(window->get_device_name() == "设备 A" && window->get_directory() == "D:/记录 数据", "关闭后内容丢失"); window->invoke_focus_target(3); forge_business_key(window->window(), slint::platform::key_codes::Return); break;
            default: forge_business_require(window->get_accepted_count() == 1 && window->get_launch_focused(), "确认后未恢复焦点"); std::cout << "BUSINESS_DIALOG_OK host=cpp\n"; slint::quit_event_loop();
            }
        } catch (const std::exception& error) { *failure = error.what(); slint::quit_event_loop(); }
    });
    ui->run(); if (!failure->empty()) { std::cerr << "BUSINESS_DIALOG_FAILED " << *failure << '\n'; return 1; } return 0;
}
