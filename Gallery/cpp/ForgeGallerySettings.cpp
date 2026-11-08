// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#include "ForgeGallerySettings.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <iostream>
#include <utility>

ForgeGallerySettingsWriter::ForgeGallerySettingsWriter(std::function<void(const ForgeGallerySettings&)> save)
    : save_(std::move(save)), worker_([this] {
        for (;;) {
            std::unique_lock lock(mutex_);
            ready_.wait(lock, [this] { return stopping_ || pending_.has_value(); });
            if (!pending_) return;
            auto settings = std::move(*pending_);
            pending_.reset();
            lock.unlock();
            try { save_(settings); }
            catch (const std::exception& error) { std::cerr << "设置保存失败: " << error.what() << '\n'; }
        }
    }) {}

ForgeGallerySettingsWriter::~ForgeGallerySettingsWriter() {
    { std::lock_guard lock(mutex_); stopping_ = true; }
    ready_.notify_one();
    worker_.join();
}

void ForgeGallerySettingsWriter::request(const ForgeGallerySettings& settings) {
    { std::lock_guard lock(mutex_); pending_ = settings; }
    ready_.notify_one();
}

static std::filesystem::path forge_gallery_settings_path() {
#ifdef _WIN32
    const wchar_t* appdata = _wgetenv(L"APPDATA");
#else
    const char* appdata = std::getenv("XDG_CONFIG_HOME");
#endif
    if (!appdata || !*appdata) return {};
    return std::filesystem::path(appdata) / "SlintForge" / "Gallery" / "settings.ini";
}

ForgeGallerySettings forge_gallery_load_settings() {
    ForgeGallerySettings settings;
    const auto path = forge_gallery_settings_path();
    if (path.empty()) return settings;
    std::ifstream input(path);
    if (!input) return settings;
    std::string line;
    while (std::getline(input, line)) {
        const auto separator = line.find('=');
        if (separator == std::string::npos) continue;
        const auto key = line.substr(0, separator);
        const auto value = line.substr(separator + 1);
        if (key == "language" && (value == "zh" || value == "en")) settings.language = value;
        else if (key == "font_family" && (value.empty() || value == "Microsoft YaHei UI" || value == "Segoe UI" || value == "Consolas")) settings.font_family = value;
        else if (key == "font_scale") {
            if (value == "90") settings.font_scale = 0.9f;
            else if (value == "100") settings.font_scale = 1.0f;
            else if (value == "110") settings.font_scale = 1.1f;
            else if (value == "125") settings.font_scale = 1.25f;
        }
        else if (key == "pinned" && (value == "0" || value == "1")) settings.pinned = value == "1";
        else if (key == "rounded_corners" && (value == "0" || value == "1")) settings.rounded_corners = value == "1";
        else if (key == "dark_mode" && (value == "0" || value == "1")) settings.dark_mode = value == "1";
        else if (key == "placement" && (value == "left" || value == "right" || value == "top")) settings.placement = value;
        else if (key == "pane_state" && (value == "expanded" || value == "compact" || value == "hidden")) settings.pane_state = value;
    }
    return settings;
}

void forge_gallery_save_settings(const ForgeGallerySettings& settings) {
    const auto path = forge_gallery_settings_path();
    if (path.empty()) return;
    std::error_code error;
    std::filesystem::create_directories(path.parent_path(), error);
    if (error) return;
    std::ofstream output(path, std::ios::trunc);
    if (!output) return;
    output << "language=" << settings.language << '\n'
           << "font_family=" << settings.font_family << '\n'
           << "font_scale=" << static_cast<int>(settings.font_scale * 100.0f + 0.5f) << '\n'
           << "pinned=" << (settings.pinned ? '1' : '0') << '\n'
           << "rounded_corners=" << (settings.rounded_corners ? '1' : '0') << '\n'
           << "dark_mode=" << (settings.dark_mode ? '1' : '0') << '\n'
           << "placement=" << settings.placement << '\n'
           << "pane_state=" << settings.pane_state << '\n';
}
