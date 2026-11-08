// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <string>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <optional>
#include <thread>

struct ForgeGallerySettings {
    std::string language = "zh";
    std::string font_family;
    float font_scale = 1.0f;
    bool pinned = false;
    bool rounded_corners = true;
    bool dark_mode = true;
    std::string placement = "left";
    std::string pane_state = "expanded";
};

ForgeGallerySettings forge_gallery_load_settings();
void forge_gallery_save_settings(const ForgeGallerySettings& settings);

// 合并连续设置请求，后台串行写入；析构等待最后一次保存完成。
class ForgeGallerySettingsWriter {
public:
    explicit ForgeGallerySettingsWriter(std::function<void(const ForgeGallerySettings&)> save = forge_gallery_save_settings);
    ~ForgeGallerySettingsWriter();
    void request(const ForgeGallerySettings& settings);
private:
    std::mutex mutex_;
    std::condition_variable ready_;
    std::optional<ForgeGallerySettings> pending_;
    bool stopping_ = false;
    std::function<void(const ForgeGallerySettings&)> save_;
    std::thread worker_;
};
