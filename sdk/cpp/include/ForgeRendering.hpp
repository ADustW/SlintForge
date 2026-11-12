// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <string>

enum class ForgeRenderPreference {
    GpuPreferred = 0,
    Software = 1,
};

struct ForgeRenderOptions {
    ForgeRenderPreference preference = ForgeRenderPreference::GpuPreferred;
    // 保留部署与测试脚本指定的 SLINT_BACKEND；false 表示强制使用 preference。
    bool respect_environment = true;
};

struct ForgeRenderConfiguration {
    ForgeRenderPreference preference = ForgeRenderPreference::GpuPreferred;
    std::string backend;
    bool from_environment = false;
};

// 在主线程、创建任何 Slint 窗口及工作线程之前调用，不支持运行中切换。
// C++ Slint 1.18 通过进程环境接收选择，不修改系统/用户环境或 WGPU_BACKEND。
// 返回请求的配置，实际 GPU 初始化及降级由 Slint/Skia 在显示窗口时处理。
// 参数无效或进程环境更新失败时抛出异常；未调用时沿用 Slint 自身默认配置。
[[nodiscard]] ForgeRenderConfiguration forge_configure_rendering(ForgeRenderOptions options = {});
