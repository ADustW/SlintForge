// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#include "ForgeRendering.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <stdexcept>

ForgeRenderConfiguration forge_configure_rendering(ForgeRenderOptions options) {
    ForgeRenderConfiguration result;
    result.preference = options.preference;
    switch (options.preference) {
    case ForgeRenderPreference::GpuPreferred: result.backend = "winit-skia"; break;
    case ForgeRenderPreference::Software: result.backend = "winit-skia-software"; break;
    default: throw std::invalid_argument("无效的 SlintForge UI 渲染偏好");
    }
    if (options.respect_environment) {
        if (const char* environment = std::getenv("SLINT_BACKEND")) {
            std::string backend(environment);
            const auto first = backend.find_first_not_of(" \t\r\n");
            if (first != std::string::npos) {
                backend = backend.substr(first, backend.find_last_not_of(" \t\r\n") - first + 1);
                std::transform(backend.begin(), backend.end(), backend.begin(), [](unsigned char value) { return static_cast<char>(std::tolower(value)); });
                result.backend = std::move(backend);
                result.from_environment = true;
            }
        }
    }
#if defined(_WIN32)
    const int error = _putenv_s("SLINT_BACKEND", result.backend.c_str());
#else
    const int error = setenv("SLINT_BACKEND", result.backend.c_str(), 1);
#endif
    if (error != 0) throw std::runtime_error("无法配置 SlintForge UI 渲染后端");
    return result;
}
