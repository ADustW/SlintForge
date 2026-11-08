// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#include "ForgeRendering.hpp"
#include "rendering.h"
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

static void forge_render_test_environment(const char* value) {
#if defined(_WIN32)
    _putenv_s("SLINT_BACKEND", value);
#else
    if (*value) setenv("SLINT_BACKEND", value, 1); else unsetenv("SLINT_BACKEND");
#endif
}

int main(int argc, char** argv) {
    const std::string scenario = argc > 1 ? argv[1] : "gpu";
    if (scenario == "policy") {
        forge_render_test_environment("");
        auto config = forge_configure_rendering();
        assert(config.preference == ForgeRenderPreference::GpuPreferred);
        assert(config.backend == "winit-skia" && !config.from_environment);
        forge_render_test_environment(" Winit-Skia-OpenGL ");
        config = forge_configure_rendering();
        assert(config.backend == "winit-skia-opengl" && config.from_environment);
        config = forge_configure_rendering({ForgeRenderPreference::Software, false});
        assert(config.backend == "winit-skia-software" && !config.from_environment);
        forge_render_test_environment("   ");
        assert(forge_configure_rendering().backend == "winit-skia");
        std::cout << "RENDER_POLICY_OK\n";
        return 0;
    }
    forge_render_test_environment("winit-software");
    const auto configuration = forge_configure_rendering({scenario == "software" ? ForgeRenderPreference::Software : ForgeRenderPreference::GpuPreferred, false});
    auto window = RenderingProbe::create();
    bool rendered = false;
    slint::Timer timer;
    timer.start(slint::TimerMode::SingleShot, std::chrono::milliseconds(700), [&] {
        const auto snapshot = window->window().take_snapshot();
        if (!snapshot || snapshot->width() == 0 || snapshot->height() == 0) throw std::runtime_error("渲染探针没有产生图像");
        const auto pixel = snapshot->begin()[snapshot->height() / 2 * snapshot->width() + snapshot->width() / 2];
        assert(pixel.r == 0x60 && pixel.g == 0xcd && pixel.b == 0xff);
        rendered = true;
        slint::quit_event_loop();
    });
    window->run();
    assert(rendered);
    std::cout << "RENDER_WINDOW_OK backend=" << configuration.backend << '\n';
}
