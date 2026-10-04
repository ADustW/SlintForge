# 容器组件与截图接口实施记录

本次完成 Gallery 容器分类的 8 个组件，移除两种仅包装原生圆角矩形／裁剪的重复容器。实现、公共导出、导航、测试清单及译文中均已清理旧入口，不保留兼容别名。原生 `Rectangle` 替代写法见 [公共 API](API.md)。

## 改动清单

| 组件 | 实施结果 |
|---|---|
| `ForgeRoundImageView` | `image-fit` 默认 `cover`，支持 `contain`／`fill`；空图片保留背景，尺寸变化时重排，边框不被图片覆盖。 |
| `ForgeIconLabel` | 使用内容网格布局，支持横竖排列、标题／说明换行与字号缩放。 |
| `ForgeExpander` | 默认自然内容高度，正值 `content-height` 显式指定；鼠标、Enter／Space 切换，折叠保留输入；标题栏固定在顶部。 |
| `ForgeWatermark` | 按实际宽高平铺，默认 180×120px，透明度限制为 0–1；水印不拦截内容点击。 |
| `ForgeMarquee` | 仅溢出时滚动；暂停后原位继续，文本／宽度／周期改变后重算；非正周期静止。 |
| `ForgeCard` | 内容自然尺寸、可配置内边距与间距，空标题省略，长标题换行。 |
| `ForgeWindow` | 保留原有窗口操作，新增整个客户区截图，包含自定义标题栏。 |
| `ForgePage` | 新增常驻页面容器，提供 `@children`、内边距、裁剪与 `active`；隐藏保留实例，可获取当前页面区域截图。 |

公共契约新增 `ui/types/capture.slint`，由 `ui/slintforge.slint` 导出截图矩形、结果和服务。Rust 的 `sdk/rust/src/capture.rs` 与 C++ 的 `ForgeCapture.hpp/.cpp` 提供整窗、区域、已有像素裁剪及 PNG 保存；返回不预乘的 RGBA8。Rust 使用锁定的 `png 0.18.1`，Windows C++ 使用 WIC，CMake 安装目标传递 `windowscodecs` 与 `ole32`。

Gallery 两端绑定所属窗口的截图服务，新增页面示例与整窗截图按钮、预览、像素尺寸及时间戳 PNG 导出。共用场景 `tests/ui/capture-scene.slint` 验证嵌套位置、滚动、页面外覆盖层、图片适配和空图片。两端最小消费者均接入截图与 PNG 接口。中英译文、[中文 API](API.md)、[英文接入说明](CAPTURE.en.md)、README 和架构说明已同步。

## 验证结果

基线为 Windows x64、Slint 1.18.0、`fluent`，Rust MSVC 工具链和 Visual Studio 2026 x64。C++ 使用官方 SDK `D:/ai_workspace/SlintForge/slint-sdk-1.18.0`；本次独立构建目录为 `build/cpp-containers`。

| 验证 | 结果与证据 |
|---|---|
| Rust workspace | `cargo test --workspace --locked`：14 项通过，0 失败；包含 SDK 7 项、Gallery 2 项、容器交互 1 项和已有布局交互 4 项。 |
| 容器交互 | 折叠前后鼠标及 Enter／Space、禁用状态、显式／自然高度、输入保留、水印平铺／调整宽度／点击穿透、跑马灯暂停继续／异常周期／宽度及文本重算、长标题／空标题、字号和横竖排列均通过。 |
| C++ 构建／CTest | Debug 构建通过；4/4 通过：SDK、裁剪／PNG、Gallery smoke、真实窗口截图。 |
| 两端 Gallery smoke | 均输出 `SMOKE_OK`；路由、导航、设置、原有布局及后台采样检查通过。 |
| 两端真实截图 | 均输出 `CAPTURE_OK`；整窗尺寸等于 `Window.size()`，页面像素与整窗按相同坐标裁剪逐像素一致，调整窗口及最大化后尺寸匹配。 |
| 滚动／覆盖层／图片 | 共用原生场景通过：保留 64px 滚动位置、包含页面子树之外的黄色覆盖层；图片圆角、边框、contain 留白、cover 铺满、fill 调整尺寸和空图片背景符合预期。 |
| PNG／错误路径 | Rust PNG 解码器与 C++ 独立 WIC 解码器核对尺寸及原始 RGBA，覆盖 alpha=128／0、中文路径和保存失败。未接入服务、未显示窗口、未激活页面、无效区域／无交集及错误缩放均验证。 |
| 实际 PNG 独立复核 | Pillow 对两端四种缩放的 PNG 重新解码，核对实际尺寸、绿色滚动内容、黄色覆盖层与 alpha；结果写入 `build/captures/verification.json`。 |
| 消费者 | Rust、工程内 C++、安装后外部 C++ 均输出 `CONSUMER_CAPTURE_OK`，实际导出 PNG。 |
| 安装包 | `build/install-containers` 含截图头文件及页面／截图 Slint 契约，无旧容器文件；外部消费者从此安装目录通过 `find_package` 构建、链接和运行。 |
| 静态检查 | Rust 格式检查、Gallery 中英译文同步检查通过；源码、译文、文档及新安装包无旧入口，第一方 C++ 未声明 namespace。 |
| 界面检查 | 已逐页查看 8 个容器的真实截图；修正标题／预览区域过度伸展、折叠标题栏默认居中及示例内容越出预览区的问题，复测通过。 |

真实缩放测试使用 `SLINT_SCALE_FACTOR`，两端分别运行 100%／125%／150%／200%。未修改 Windows 全局 DPI 设置。测试采用 Windows 默认渲染器；其他渲染器的支持边界见 API 说明。

## 缩放与实际像素尺寸

两端结果相同，以下尺寸来自实际 PNG 的独立解码：

| 缩放 | Gallery 整窗 | Gallery 页面 | 共用场景窗口 | 滚动页面 | 最大化整窗 |
|---|---|---|---|---|---|
| 100% | 1380×860 | 1020×180 | 800×600 | 280×160 | 2560×1392 |
| 125% | 1725×1075 | 1275×226 | 1000×750 | 351×200 | 2560×1392 |
| 150% | 2070×1290 | 1530×271 | 1200×900 | 420×240 | 2560×1392 |
| 200% | 2760×1720 | 2040×360 | 1600×1200 | 560×320 | 2560×1392 |

125% 下滚动页面的 x=70px，物理起点为 `floor(87.5)=87`，终点为 `ceil(437.5)=438`，因此宽度是 351px。页面高度的额外像素同样来自起点向下／终点向上取整，符合接口约定。

## 截图样本

| 样本 | 文件 |
|---|---|
| Rust 页面（1020×180） | [page.png](../build/captures/rust-1790957170728/page.png) |
| Rust 最大化／英文／字号 150% | [maximized.png](../build/captures/rust-1790957170728/maximized.png) |
| Rust 滚动页面与外部覆盖层 | [scrolled-page-1.png](../build/captures/rust-1790957170728/scrolled-page-1.png) |
| C++ 页面 | [page.png](../build/captures/cpp-1790957170240/page.png) |
| C++ 原生图片／滚动测试场景 | [scene-1.png](../build/captures/cpp-1790957170240/scene-1.png) |
| C++ 125% 页面 | [scrolled-page-1.png](../build/captures/cpp-1790956638607/scrolled-page-1.png) |
| 修正后的折叠面板 | [preview.png](../build/captures/cpp-1790957174211/preview.png) |
| 安装后 C++ 消费者 | [consumer PNG](../build/captures/consumer-cpp-17909572069551879.png) |
| Rust 消费者 | [consumer PNG](../build/captures/consumer-rust-1790957201118232900.png) |
| 两端四种缩放的完整尺寸记录 | [verification.json](../build/captures/verification.json) |

## 复现命令

```powershell
cargo test --workspace --locked
cargo run --locked -p slintforge-gallery-rust -- --smoke
cargo run --locked -p slintforge-gallery-rust -- --capture-smoke
cargo run --locked -p slintforge-example-rust -- --capture-smoke

cmake -S . -B build/cpp-containers -G "Visual Studio 18 2026" -A x64 `
  -DCMAKE_PREFIX_PATH="D:/ai_workspace/SlintForge/slint-sdk-1.18.0" -DSLINT_STYLE=fluent
cmake --build build/cpp-containers --config Debug
$env:PATH = "D:\ai_workspace\SlintForge\slint-sdk-1.18.0\lib;$env:PATH"
ctest --test-dir build/cpp-containers -C Debug --output-on-failure

# 可分别设置为 1、1.25、1.5、2，运行两端真实截图检查。
$env:SLINT_SCALE_FACTOR = "1.25"
& target/debug/slintforge-gallery-rust.exe --capture-smoke
& build/cpp-containers/Gallery/cpp/Debug/slintforge_gallery_cpp.exe --capture-smoke
Remove-Item Env:SLINT_SCALE_FACTOR

cmake --install build/cpp-containers --config Debug --prefix build/install-containers
cmake -S examples/cpp -B build/external-cpp-containers -G "Visual Studio 18 2026" -A x64 `
  -DCMAKE_PREFIX_PATH="D:/ai_workspace/SlintForge/slint-sdk-1.18.0;D:/ai_workspace/SlintForge/SlintForge/build/install-containers"
cmake --build build/external-cpp-containers --config Debug
& build/external-cpp-containers/Debug/slintforge_example_cpp.exe --capture-smoke
```

截图须在 UI 线程、窗口显示且完成布局后调用。页面接口返回当前窗口画面与页面矩形的交集，保留背景和覆盖层；不独立渲染隐藏页或生成长截图。使用页面作为可视容器，将 `ScrollView` 放在页面内部，并将 `active` 绑定到路由状态。
