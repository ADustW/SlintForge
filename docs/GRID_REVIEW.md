# DataGrid / TreeDataGrid 接手审查与优化

> 阶段记录：本文保留该组件实施时的范围、测试数量和验证结果；v0.1.1 当前能力见接入与交付说明，本次验收以时间戳发布目录中的 ACCEPTANCE.md 和 verification 为准。

日期：2026-10-03。工程：`D:\ai_workspace\SlintForge\SlintForge`。平台：Windows x64、Slint 1.18.0；C++ 使用 `D:\.clion-env\qt-6.8.3-msvc.bat` 加载的 CLion MSVC 环境。

已等待开发任务“实现 Gallery 待补组件”（`01a0fd3a-9486-7cb2-813f-706ce3899134`）完成，再接手当前工程。开发方的固定源码、程序、608 帧截图及 [原验证记录](GRIDS_VERIFICATION.md) 保留。本次检查使用独立的 `build/plot-qa` 和 `build/plot-cpp`，记录当前新增专业图表后的 96 页工程。

## 审查结论与修复

| 问题 | 修改 | 验证 |
|---|---|---|
| 文本排序每次比较创建并折叠字符串，长文本的大列表开销明显 | 大兄弟集合预计算每行 ASCII 大小写折叠键；小集合逐字节比较，遇到差异即返回，不分配临时字符串 | 1 万/10 万行、平面/四分支树、排序与筛选；原始 ID、等值稳定顺序及无效数值置后保持 |
| SDK 把纯空白筛选当作无筛选，UI 却按原字符串长度禁用展开 | 共享 `filter-active` 服务以相同 ASCII 空白规则判断有效查询；UI 的展开、折叠和空结果提示采用服务结果 | 窗口事件测试在 `" \t\n"` 查询下实际折叠父节点 |
| 深层树和很窄的树列可能算出负缩进 | 缩进下限设为 0，并按可用列宽限制 | 契约编译、列宽和树交互回归、窄窗口截图检查 |

缓存阈值为同级集合至少 32 行。数值排序不创建文本键；较小兄弟集合使用无分配比较。文本大小写折叠与既有行为一致，针对 ASCII 字母；没有引入区域语言排序规则。

## 功能检查

- DataGrid：文本/数值升降序、稳定行标识、Ctrl/Shift 多选、可见行全选、禁用行、连续列宽拖动、隐藏列、横向滚动和冻结列命中、替换模型后刷新。
- 编辑：实际双击和键盘输入、Enter 提交、Escape 取消、非法数值拒绝；禁用状态阻止编辑。
- TreeDataGrid：无序输入重建父子层级、同级排序、展开/折叠、左右键父子定位、祖先路径筛选、删除子树；重复 ID、缺失父节点和循环关系拒绝。
- 边界：万层树的迭代遍历，万行窗口的 End/Home 与滚动，缺失单元格、NaN/无穷值、等值大小写文本；新增 1 万/10 万行平面及四分支树的模型准备检查。

这些事件测试使用 Slint 的窗口事件分发与测试后端。实际 Gallery 另以 `winit-software` 打开窗口并捕获画面。ListView 负责行虚拟化，SDK 仍持有完整模型；当前性能测量没有把模型准备耗时当作滚动帧率。

## 性能

数据为混合大小写、约 80 字节长文本和数值列；树每个父节点最多 4 个孩子，全部展开。操作 0 为无排序准备，1 为文本排序，2 为数值降序，3 为文本筛选。计时包含校验、树重建、排序/筛选和结果行复制，排除测试数据生成、Slint 模型转换和窗口绘制。

C++ 使用 `/O2`，原实现从开发方固定源码独立编译，与当前实现交替运行 3 轮，取中位数；测试期间未同时进行构建或截图。

| 行数 / 形状 | 原文本排序 ms | 优化后 ms | 耗时下降 |
|---|---:|---:|---:|
| 1 万 / 平面 | 26.616 | 5.761 | 78.4% |
| 1 万 / 四分支树 | 4.118 | 3.369 | 18.2% |
| 10 万 / 平面 | 363.432 | 86.829 | 76.1% |
| 10 万 / 四分支树 | 64.672 | 53.889 | 16.7% |

原始样本和中位数保存在 `build/plot-qa/grid-review/cpp-paired-{before,after}-{1..3}.csv`、`paired-summary.json`。早期仅使用缓存的版本在小兄弟树上有约 5% 开销增加，已改为无分配比较并重新测试；中间结果保存在 `cache-only-paired-summary.json`。

Rust 使用 Release；原实现和优化后完整的 16 种场景分别记录在 `rust-before.csv` 和 `rust-after.csv`。两端 16 种场景的可见行数完全一致。Rust 初始记录与 C++ 三轮对比的统计方法不同，不据此比较语言运行速度。

本次 Rust 的十万行文本排序单次记录：平面 67.327 → 60.257 ms；四分支树 74.960 → 61.124 ms。这些单次记录用于定位开销，未按三轮中位数统计。

## 当前工程验证与证据

| 检查 | 结果 | 证据 |
|---|---|---|
| Rust 工作区 | 33 项通过，包括网格模型和窗口事件回归 | `build/plot-qa/workspace-tests.log` |
| C++ CTest | 13/13 通过，包括原网格测试、新增审查测试和消费者 | `build/plot-qa/workspace-cpp-tests.log` |
| 规模与双端结果 | 16 种场景通过，1 万/10 万行、平面/树、4 种操作，可见行数一致 | `grid-review/{rust,cpp}-after.csv`、`export-verification.log` |
| 公共契约与代码规则 | 66 段独立示例编译，第一方 C++ namespace 检查通过 | `build/plot-qa/public-contracts.log` |
| 当前 Gallery | 每端 308 帧，共 616 帧；96 页 × 3 状态及 20 操作帧，集合、尺寸、更新时间检查通过 | `build/plot-qa/gallery/{rust,cpp}`、`gallery-verification.log` |

网格深色、英文大字号、窄窗口和编辑状态的画面在接手阶段复核。上述截图总数包括其他组件，不表示逐帧人工检查了所有组件。

## 复现

在项目根目录执行；以下构建目录与开发方的历史验收目录隔离：

```powershell
$env:CARGO_TARGET_DIR = "$PWD\build\plot-qa\rust-target"
cargo test --workspace --offline --locked
cargo run --release --manifest-path examples/plot-rust/Cargo.toml --bin grid-review --offline --locked -- rust-after.csv
cmd /c build\grid-review-tests.cmd cpp-after.csv
cmd /c build\plot-qa\grid-paired.cmd
```

`grid-review` 参数是输出 CSV 文件名，写入 `build/plot-qa/grid-review`。固定源码保持不变；当前源码和验证程序的 SHA-256 见 `build/plot-qa/verified-source-sha256.json`。图表和完整工程的证据见 [专业图表验证记录](PLOT_VERIFICATION.md)，组件接入见 [网格 API](GRIDS.md)。
