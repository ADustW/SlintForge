# Gallery 结构与当前范围

Gallery 使用与参考工程相近的窗口层、内部控件层和分类页面层。SlintForge 的实现与视觉素材均为独立编写。

```text
Gallery/
├─ data/                        # navigation.txt 分类树、component-status.txt 实现状态
├─ ui/gallery.slint             # 单一导出入口
├─ ui/screen/main-screen.slint  # ForgeWindow、导航与固定页面区域
├─ ui/component/                # 页面框架、示例卡片、设置项与图标展示
├─ ui/page/                     # 各组件独立演示页面
├─ ui/assets/brand/             # 原创 Gallery 图标、PNG 与 Windows ICO
├─ i18n/                        # Gallery 中英译文与生成脚本
├─ rust/                        # Rust 路由、导航与采样宿主
└─ cpp/                         # C++ 同语义宿主
```

左侧分类、搜索、顶栏、页脚、首页分区和示例卡片采用参考 Gallery 的布局方式。顶部模式只占一行，点击分类打开分层下拉菜单；右侧“更多”菜单可访问顶栏宽度之外的分类。导航条目来自同一份 `navigation.txt`，Rust 与 C++ 分别将其注册到本地 `Router/NavTree`。按页面 ID 导航时，宿主展开祖先分组并清除搜索，使当前条目可见。两端只控制状态，页面结构在 `ui/` 中保持唯一。

当前 96 个导航页面全部提供演示：27 页使用原生能力，41 页使用共享组件，28 页为已有演示及辅助页。原有基础、Layouts、Surfaces、字体、图标和设置演示保留。组件页以交互预览区为主，显示 API 提示，新增页面还提供可复制且独立编译验证过的 Slint 用法。预览内容裁剪到卡片边界。布局页面可验证换列、滚动、停靠拖动与隐藏页状态；窗口页面操作 Gallery 主窗口，验证置顶、边角、最大化和截图。`lifecycle` 仍是不显示在导航中的内部验收页。具体范围见 `../docs/COMPONENTS.md`。

专业图表入口为“图表 → ForgePlot（该菜单第一项）”，英文界面为“Chart → ForgePlot”。首页“最近添加的示例”有 ForgePlot、DataGrid、TreeDataGrid 入口；也可在导航搜索框输入 `ForgePlot`。直接打开专业图表示例：

```powershell
cargo run --locked -p slintforge-gallery-rust -- --preview-route=chart-professional
# C++ 运行前将 Slint SDK 的 lib 目录加入 PATH。
& build/cpp/Gallery/cpp/Debug/slintforge_gallery_cpp.exe --preview-route=chart-professional
```

常用构建程序分别位于 `target/debug/slintforge-gallery-rust.exe` 和 `build/cpp/Gallery/cpp/Debug/slintforge_gallery_cpp.exe`。修改共享 UI 后重新构建这些目标，使日常运行与验收源码一致。图表图例对齐和入口修复见 [本次 Gallery 图表复核](../docs/GALLERY_CHARTS_REVIEW.md)。

默认语言是中文，设置页可立即切换英文。`Gallery/i18n/en.tsv` 是英文译文源；新增 `@tr` 字符串后运行 `py Gallery/i18n/sync.py` 生成 Rust/C++ 使用的 PO 文件，再用 `py Gallery/i18n/sync.py --check` 检查一致性。导航目录可在第七列给出中文标题；未提供时沿用英文标题。文字字体使用系统默认、Microsoft YaHei UI、Segoe UI 或 Consolas，字号比例可选 90%、100%、110%、125%；图标页按名称搜索、分页浏览官方字体的 9,767 个变体并显示码位。

Rust/C++ 两端共用 `%APPDATA%\SlintForge\Gallery\settings.ini`，保存语言、主题、置顶、圆角／直角、文字字体和字号比例、导航位置及面板状态。`--smoke` 忽略并且不修改此文件，以固定中文默认基线。图标字体内嵌在 Slint 组件库，安装 C++ 包时与许可文件及完整目录一起安装。边角切换通过 Windows 11 DWM 接口请求，实际圆角效果受系统窗口策略限制。

新页面在 `ui/page/` 中单独定义，按类别由 `ui/component/*-pages.slint` 创建固定实例，再接入主窗口。分类分组限制 MSVC 生成代码的嵌套深度，同时保留页面状态。`tools/component_catalog.py` 维护逐项状态，`tools/generate_component_pages.py` 用于重新生成本次新增页面；重新生成会覆盖这些页面的手工修改，运行前应保留需要的变更。构建两端后页面进入目录，不支持运行时加载外部 `.slint`。

## 截图示例

容器分类的 `ForgePage` 页面可修改输入并获取页面区域截图，预览放在目标之外；`ForgeWindow` 页面提供整个客户区的截图。两页均显示物理像素尺寸并支持导出 PNG。两端宿主共用同一套页面和 API，服务按窗口绑定。`--capture-smoke` 会生成真实窗口、页面、调整尺寸后和最大化后的 PNG，并自动退出；该模式使用默认设置且不保存用户设置。

`--components-smoke` 输出全部 96 页的深色中文、浅色英文/125% 字号、窄窗口中文和 20 个操作状态，每端共 308 张截图。设置 `SLINTFORGE_COMPONENT_QA_FILTER=chart` 可只检查八个图表页面，每端 32 张截图。共享组件的键盘、指针、禁用、边界值和状态保留测试位于 `rust/tests/`；两端 SDK 另有模型、数值和导出测试。

本轮最终结果、实际截图与安装后消费者验证见 [组件验证记录](../docs/COMPONENT_VERIFICATION.md)。
