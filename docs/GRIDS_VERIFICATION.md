# DataGrid / TreeDataGrid 验证记录

> 阶段记录：本文保留该组件实施时的范围、测试数量和验证结果；v0.1.1 当前能力见接入与交付说明，本次验收以时间戳发布目录中的 ACCEPTANCE.md 和 verification 为准。

日期：2026-10-03。工程：`D:\ai_workspace\SlintForge\SlintForge`。基线：Windows x64、Slint 1.18.0、fluent；C++ 使用 `D:\.clion-env\qt-6.8.3-msvc.bat` 加载的 CLion MSVC 环境。

两个组件已实现，共用 Slint UI 与模型契约，提供 Rust/C++ 服务绑定及独立消费者。Gallery 当前 95 个导航页面均有演示：27 页使用原生能力、40 页使用新增共享组件、28 页为已有演示及辅助页，无暂缓页面。API 见 [表格文档](GRIDS.md)，此前图表阶段的记录保留在 [组件验证记录](COMPONENT_VERIFICATION.md)。

为避免并行图表修改覆盖共享构建结果，最终验收使用固定源码 `build/grids-qa/verified-source`，Rust 验证程序位于 `build/grids-qa/bin/rust`，C++ 构建位于 `build/grids-qa/verified-cpp`。固定源码包含本次全部表格修改，388 个原始文件的 SHA-256 保存在 `build/grids-qa/verified-source-sha256.json`；后续图表修改由其任务另行验收。

## 验证范围

- 平面表格：文本/数值稳定排序、行 ID 保持、Ctrl/Shift 多选、可见行全选、禁用行、连续列宽拖拽、横向滚动下的冻结列命中、隐藏列及模型刷新。
- 编辑：实际双击进入编辑、原生 LineEdit 的键盘输入、非法数字拒绝、Enter 提交、Escape 取消、禁用后阻止操作；同一轮事件中替换列模型再进入编辑仍保持有效编辑。
- 树形表格：无序输入重建层级、同级排序、展开/折叠、左右键父子定位、祖先路径筛选、删除所选子树；重复标识、缺失父节点与循环关系被拒绝。
- 数据规模：SDK 检查万层树的迭代遍历；窗口事件检查万行列表的 End/Home 定位与真实滚动；Gallery 两端的新增、更新及万行生成也经过 smoke 检查。

原生 ListView 负责行虚拟化；SDK 仍持有并处理完整数据模型。以上验证表明测试数据可用，不是任意数据量的性能指标。

## 检查结果与证据

| 检查 | 结果 | 证据 |
|---|---|---|
| 固定源码的 Rust 工作区 | 33 项通过，0 失败；包括表格模型和真实窗口事件测试 | `build/grids-workspace-tests.log` |
| Rust / C++ 构建 | 均成功；Rust 使用 offline、locked | `build/grids-rust-build.log`、`build/grids-cpp-build.log` |
| C++ CTest | 11/11 通过；包含模型、Gallery、截图和消费者 | `build/grids-cpp-ctest.log` |
| 共享契约一致性 | 10 个场景、59 条记录完全一致 | `build/grids-qa/{rust,cpp}-contract.csv` |
| 分类、译文、命名规则与示例 | 95 页、40 共享组件、0 暂缓；65 段独立示例编译通过 | `build/grids-static-checks.log` |
| 两端独立表格消费者 | 每端 7 种状态，均生成窗口 PNG | `build/grids-qa/consumer-{rust,cpp}-{0..6}.png` |
| Gallery 实际窗口 | 每端 304 帧，共 608 帧；截图集合、尺寸与更新时间核对通过 | `build/grids-qa/gallery/{rust,cpp}`、`build/grids-presentation-final.log` |
| Gallery 编辑状态 | 两个表格操作帧包含编辑器，宿主在截图前断言 editing=true | 两端 `*-data-*-dark-zh-open.png` |
| 安装后 C++ 消费者 | 独立配置、构建、七种状态运行通过 | `build/grids-installed-consumer-build.log`、`build/grids-qa/installed-run/installed-grids.log` |
| 安装内容 | 63 个共享 Slint 文件与源码一致，表格头文件一致，静态库存在 | `build/grids-final-evidence.log` |

截图状态为深色中文 95 帧、浅色英文/125% 字号 95 帧、960×720 窄窗口中文 95 帧，以及 19 个操作帧。两个新表格的深色、浅色、窄窗口与编辑状态均进行视觉复核；窄窗口和较大字号通过外层页面滚动及表格内滚动容纳内容。

修复的交互问题包括：固定列表头默认居中、计算模型替换中断鼠标抓取、无变化选择销毁双击目标、隐藏冻结层抢占编辑焦点，以及延后的列变更回调取消刚开始的编辑。对应行为已纳入事件测试或实际窗口状态断言。

## 复现

```powershell
cargo test --manifest-path build/grids-qa/verified-source/Cargo.toml --workspace --offline --locked
cargo build --manifest-path build/grids-qa/verified-source/Cargo.toml --workspace --offline --locked
$env:SLINT_BACKEND = 'winit-software'
& .\build\grids-qa\bin\rust\slintforge-example-rust.exe --grids-smoke
$env:SLINTFORGE_COMPONENT_QA_DIR = "$PWD\build\grids-qa\gallery\rust"
& .\build\grids-qa\bin\rust\slintforge-gallery-rust.exe --components-smoke
```

截图检查显式使用 winit-software。仅设置 `SLINT_RENDERER=software` 不能保证 Rust 选择软件渲染器。

C++ 先加载指定环境，再用 CLion 内置 CMake 配置到 `build/grids-qa/verified-cpp`、Debug 构建，并运行 CTest；将 SDK 的 `lib` 目录加入 PATH，Gallery 的 QA_DIR 指向 `build/grids-qa/gallery/cpp`。本次安装前缀为 `build/grids-qa/install`，独立消费者构建目录为 `build/grids-qa/verified-external-cpp`。

```powershell
$python = 'C:\Users\wzp\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
& $python tools/verify_components.py --compiler D:/ai_workspace/SlintForge/slint-sdk-1.18.0/bin/slint-compiler.exe
& $python tools/verify_grids.py --host rust --host cpp --install-prefix build/grids-qa/install `
  --source-root build/grids-qa/verified-source --cpp-build-dir build/grids-qa/verified-cpp `
  --rust-build-dir build/grids-qa/bin/rust
& $python tools/verify_gallery_presentation.py --geometry --host rust --host cpp `
  --qa-root build/grids-qa/gallery --cpp-build-dir build/grids-qa/verified-cpp `
  --rust-build-dir build/grids-qa/bin/rust
```

最终证据摘要位于 `build/grids-final-evidence.log`，当前源码 SHA-256 清单位于 `build/grids-qa/final-source-sha256.json`。修改前备份保留于 `build/backups/grids_20261003_095419`。
