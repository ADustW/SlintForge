// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

//! 桌面 UI 渲染偏好；在主线程创建任何 Slint 窗口前调用。

/// 渲染偏好不是当前 GPU/驱动状态；实际初始化发生在窗口创建和显示阶段。
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub enum RenderPreference {
    /// 优先使用 Skia 的 GPU 后端，初始化失败时由 Slint/Skia 尝试可用的降级后端。
    #[default]
    GpuPreferred,
    /// 使用 Skia 的 CPU 光栅化，保留 Path 等绘图能力。
    Software,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct RenderOptions {
    pub preference: RenderPreference,
    /// 保留应用部署或测试脚本通过 SLINT_BACKEND 指定的后端；设为 false 可强制应用偏好。
    pub respect_environment: bool,
}

impl Default for RenderOptions {
    fn default() -> Self {
        Self {
            preference: RenderPreference::GpuPreferred,
            respect_environment: true,
        }
    }
}

/// 已提交给 Slint 的配置，不将其误报为 GPU 已成功启用。
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct RenderConfiguration {
    pub preference: RenderPreference,
    pub backend: String,
    pub from_environment: bool,
}

fn resolve(options: RenderOptions, environment: Option<&str>) -> RenderConfiguration {
    let environment = environment.map(str::trim).filter(|value| !value.is_empty());
    let from_environment = options.respect_environment && environment.is_some();
    let backend = if from_environment {
        environment.unwrap().to_lowercase()
    } else {
        match options.preference {
            RenderPreference::GpuPreferred => "winit-skia",
            RenderPreference::Software => "winit-skia-software",
        }
        .into()
    };
    RenderConfiguration {
        preference: options.preference,
        backend,
        from_environment,
    }
}

/// 在主线程、首个 Slint 窗口创建之前选择渲染器。
///
/// 默认 GPU 优先，保留显式 SLINT_BACKEND；不会修改 WGPU_BACKEND 等驱动选择。
/// Slint 后端已初始化时返回错误，不支持运行中切换。未调用本函数的应用仍遵循 Slint 自身配置。
pub fn configure_rendering(
    options: RenderOptions,
) -> Result<RenderConfiguration, slint::PlatformError> {
    let environment = std::env::var("SLINT_BACKEND").ok();
    let configuration = resolve(options, environment.as_deref());
    slint::BackendSelector::new()
        .backend_name(configuration.backend.clone())
        .select()?;
    Ok(configuration)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn defaults_to_gpu_and_preserves_explicit_deployment_overrides() {
        let default = RenderOptions::default();
        assert_eq!(default.preference, RenderPreference::GpuPreferred);
        for unset in [None, Some(""), Some("   ")] {
            let resolved = resolve(default, unset);
            assert_eq!(resolved.backend, "winit-skia");
            assert!(!resolved.from_environment);
        }
        let override_config = resolve(default, Some(" Winit-Skia-OpenGL "));
        assert_eq!(override_config.backend, "winit-skia-opengl");
        assert!(override_config.from_environment);
    }

    #[test]
    fn explicit_software_can_override_environment_without_changing_driver_settings() {
        let options = RenderOptions {
            preference: RenderPreference::Software,
            respect_environment: false,
        };
        let resolved = resolve(options, Some("winit-skia"));
        assert_eq!(resolved.backend, "winit-skia-software");
        assert!(!resolved.from_environment);
        assert_eq!(
            resolve(
                RenderOptions {
                    respect_environment: true,
                    ..options
                },
                None
            )
            .backend,
            "winit-skia-software"
        );
    }
}
