# UI 渲染偏好

SlintForge 提供 Rust/C++ 宿主接口，在主线程创建第一个 Slint 窗口之前设置 UI 渲染偏好。它不属于计算运行库 DLL 的 ABI，也不在 `.slint` 组件初始化中选择后端。

| 偏好 | 后端配置 | 行为 |
|---|---|---|
| `GpuPreferred`（默认） | `winit-skia` | 优先尝试 Skia 的 GPU 渲染路径，初始化失败时由 Slint/Skia 尝试可用的降级路径。 |
| `Software` | `winit-skia-software` | 使用 Skia CPU 光栅化，保留 Path 和图像等绘图能力。 |

默认 `respect_environment = true`，非空的 `SLINT_BACKEND` 优先于上述偏好，以兼容部署和测试脚本；空白值视为未设置。设置为 `false` 时强制采用应用指定的偏好。接口不修改 `WGPU_BACKEND`、GPU 适配器、系统 DPI 或系统/用户环境配置。

返回的 `RenderConfiguration` / `ForgeRenderConfiguration` 包含请求的 `preference`、提交的 `backend` 和 `from_environment`。这不是“GPU 已初始化”的状态；窗口显示时才会初始化实际渲染设备。GPU 不可用、驱动失败和当前 Slint 构建缺少后端时，以窗口初始化结果和 Slint 渲染日志为准。

## Rust

```rust
fn main() -> Result<(), Box<dyn std::error::Error>> {
    // 默认 GPU 优先；必须早于 MyWindow::new()。
    let rendering = slintforge::configure_rendering(Default::default())?;
    let ui = MyWindow::new()?;
    ui.run()?;
    Ok(())
}
```

强制使用软件渲染：

```rust
slintforge::configure_rendering(slintforge::RenderOptions {
    preference: slintforge::RenderPreference::Software,
    respect_environment: false,
})?;
```

Rust SDK 默认包含 `renderer-skia` 功能；配置通过 Slint 的 `BackendSelector` 提交，不改写进程环境。已经初始化 Slint 后端时调用会返回错误。

## C++

```cpp
#include "ForgeRendering.hpp"

int main() {
    // 默认 GPU 优先；必须早于 MyWindow::create() 和工作线程启动。
    const auto rendering = forge_configure_rendering();
    auto ui = MyWindow::create();
    ui->run();
}
```

强制使用软件渲染：

```cpp
const auto rendering = forge_configure_rendering({ForgeRenderPreference::Software, false});
```

Slint 1.18 的 C++ 公共 API 没有 `BackendSelector`；该封装通过当前进程的 `SLINT_BACKEND` 提交配置。首次窗口创建后再调用不受支持，不能用修改环境变量热切换已有窗口。参数无效或进程环境设置失败时抛出异常。使用的 Slint C++ SDK 必须包含 Skia 渲染器。

偏好改变需要重新启动应用。未调用初始化接口的消费者继续遵循 Slint 原有配置。Gallery 和普通 Rust/C++ 示例已接入默认初始化；自行使用 Slint 后端、嵌入式平台或专门的图形 API 初始化时，不要重复调用本接口。

## 定向验证

- Rust：`cargo test -p slintforge --lib rendering::tests` 验证默认值和环境覆盖规则；`cargo test -p slintforge --test rendering` 在独立进程创建 GPU 优先、Skia 软件和环境覆盖窗口，检查实际截图像素及重复初始化错误。
- C++：构建 `slintforge_rendering_tests`，运行 `ctest -R '^slintforge_rendering_(policy|gpu|software)$' --output-on-failure`。
- 可在隔离测试进程设置 `SLINT_DEBUG_PERFORMANCE=refresh_lazy,console` 查看 Slint 报告的实际渲染后端。正常启动不设置性能采集环境变量。

后端名称与平台支持参考 [Slint Winit 文档](https://docs.slint.dev/latest/docs/slint/guide/backends-and-renderers/backend_winit/)。
