// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

use slint::ComponentHandle;
use slintforge::{configure_rendering, RenderOptions, RenderPreference};

slint::slint! {
    export component RenderingProbe inherits Window {
        width: 160px; height: 120px; background: #005fb8;
        Rectangle { x: 40px; y: 30px; width: 80px; height: 60px; background: #60cdff; }
    }
}

#[test]
fn rendering_preferences_create_real_windows_in_isolated_processes() {
    let Ok(scenario) = std::env::var("SLINTFORGE_RENDER_TEST") else {
        for scenario in ["gpu", "software", "environment"] {
            let status = std::process::Command::new(std::env::current_exe().unwrap())
                .args([
                    "--exact",
                    "rendering_preferences_create_real_windows_in_isolated_processes",
                    "--nocapture",
                ])
                .env("SLINTFORGE_RENDER_TEST", scenario)
                .env("SLINT_BACKEND", "winit-software")
                .status()
                .unwrap();
            assert!(status.success(), "渲染模式失败: {scenario}");
        }
        return;
    };
    // 子测试在独立进程中运行，避免进程内 Slint 后端只能初始化一次的限制。
    let configuration = configure_rendering(RenderOptions {
        preference: if scenario == "software" {
            RenderPreference::Software
        } else {
            RenderPreference::GpuPreferred
        },
        respect_environment: scenario == "environment",
    })
    .unwrap();
    assert_eq!(
        configuration.backend,
        match scenario.as_str() {
            "software" => "winit-skia-software",
            "environment" => "winit-software",
            _ => "winit-skia",
        }
    );
    let window = RenderingProbe::new().unwrap();
    let rendered = std::rc::Rc::new(std::cell::Cell::new(false));
    let complete = rendered.clone();
    let weak = window.as_weak();
    let timer = slint::Timer::default();
    timer.start(
        slint::TimerMode::SingleShot,
        std::time::Duration::from_millis(700),
        move || {
            let window = weak.upgrade().unwrap();
            let snapshot = window.window().take_snapshot().unwrap();
            let pixel = snapshot.as_slice()
                [((snapshot.height() / 2) * snapshot.width() + snapshot.width() / 2) as usize];
            assert_eq!((pixel.r, pixel.g, pixel.b), (0x60, 0xcd, 0xff));
            complete.set(true);
            slint::quit_event_loop().unwrap();
        },
    );
    window.run().unwrap();
    assert!(rendered.get());
    assert!(
        configure_rendering(Default::default()).is_err(),
        "初始化后不能静默切换后端"
    );
    println!("RENDER_WINDOW_OK backend={}", configuration.backend);
}
