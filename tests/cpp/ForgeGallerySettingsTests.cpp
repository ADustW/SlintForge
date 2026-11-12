// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#include "../../Gallery/cpp/ForgeGallerySettings.hpp"
#include <chrono>
#include <future>
#include <iostream>
#include <stdexcept>
#include <vector>

int main() {
    std::promise<void> started;
    std::promise<void> release;
    auto release_future = release.get_future();
    auto started_future = started.get_future();
    std::vector<std::string> saved;
    {
        ForgeGallerySettingsWriter writer([&](const ForgeGallerySettings& settings) {
            if (saved.empty()) {
                started.set_value();
                if (release_future.wait_for(std::chrono::seconds(5)) != std::future_status::ready) throw std::runtime_error("保存队列阻塞了请求线程");
            }
            saved.push_back(settings.pane_state);
        });
        writer.request({});
        if (started_future.wait_for(std::chrono::seconds(2)) != std::future_status::ready) throw std::runtime_error("保存线程未启动");
        ForgeGallerySettings settings;
        settings.pane_state = "hidden";
        for (int i = 0; i < 100; ++i) writer.request(settings);
        settings.pane_state = "compact";
        writer.request(settings);
        release.set_value();
    }
    if (saved != std::vector<std::string>{"expanded", "compact"}) throw std::runtime_error("未合并中间状态或退出时丢失最后设置");
    std::cout << "GALLERY_SETTINGS_QUEUE_OK latest=compact flushed=true\n";
}
