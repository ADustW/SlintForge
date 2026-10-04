# 图表组件

7 个公共组件共用 Slint 绘图层：`ForgeBarChart`、`ForgeLineChart`、`ForgePieChart`、`ForgePolarAreaChart`、`ForgeBubbleChart`、`ForgeScatterChart`、`ForgeRadarChart`。基线为 Slint 1.18.0，图形使用原生 Rectangle、Path 和 Text；两端 SDK 负责数据校验、归一化几何与命中测试。共享入口为 `ui/slintforge.slint`。

## 数据与行为

| 项目 | 约定 |
|---|---|
| `series: [ForgeChartSeries]` | `name`、`color`、`hidden`、`points`；点包含 `x`、`y`、`size`、`label`、`color`。透明色采用默认系列颜色。 |
| `slices: [ForgeChartSlice]` | 饼图和极区图使用 `label`、`value`、`color`、`hidden`。忽略非正值和非有限值，所有项隐藏时显示空数据提示。 |
| `categories: [string]` | 分类标签；雷达图至少需要 3 个维度，点顺序对应维度顺序。 |
| 自动坐标 | 默认 `auto-range=true`；提供刻度与单值范围扩展。分类图表只用类别域内的有效点计算范围。`include-zero` 控制是否包含零，柱图与雷达图默认包含零。 |
| 手动坐标 | 设置 `auto-range=false` 及 `x-min/x-max/y-min/y-max`。端点及跨度须有限且最小值小于最大值；跨度溢出返回无效范围状态。 |
| 柱图 | 分类使用从 0 开始的 x 索引；`stacked` 切换分组/堆叠，正值和负值分别累计。 |
| 折线 | `smooth` 切换三次曲线；`fill-area` 填充到零基线。非有限点断开折线；曲线与填充采用相同边界。 |
| 饼图 | `doughnut` 切换环形；单项 360° 使用两段圆弧，环形中心不参与选择。 |
| 极区图 | 扇区等角，半径按值的平方根变化，使面积与数值成比例。 |
| 气泡/散点 | 连续 `x/y`；气泡 `size` 为正的逻辑像素半径，非正或非有限 size 不显示；散点使用 `point-size`。 |
| 雷达 | 按维度顺序闭合，默认填充；两个或更少维度返回无效数据提示。 |
| 图例 | `show-legend` 控制显示；点击复选框隐藏/显示系列或扇区。 |
| 选择 | 悬停显示标签，点击触发 `point-selected(series-index, point-index, value)`；方向键循环、Home/End 首末项、Escape 清除。索引从 0 开始。 |
| 禁用 | `enabled=false` 阻止鼠标、键盘和图例操作。 |

`scene.status` 为 0（正常）、1（无有效数据）、2（范围/维度无效）、3（未绑定服务）。`selected-index`、`selected-series`、`selected-point`、`hovered-index` 可供应用读取。模型变化时重置悬停和选择，Slint 的 changed 回调在下一轮更新执行。

可调用 `select-point(index)`、`clear-selection()`、`hide-series(index, hidden)`、`hide-slice(index, hidden)`。替换模型会重新计算；宿主就地修改嵌套模型后调用 `refresh()`。密集或高频数据可由应用预先降采样。

## 两端接入

在编译入口中导出服务和返回类型：

```slint
export { ForgeChartServices, ForgeChartScene, ForgeChartMark,
    ForgeChartLine, ForgeChartTick, ForgeChartLabel } from "@slintforge";
```

创建窗口后绑定：

```rust
slintforge::bind_chart_services!(ui, ForgeChartServices, ForgeChartScene,
    ForgeChartMark, ForgeChartLine, ForgeChartTick, ForgeChartLabel);
```

```cpp
#include "ForgeCharts.hpp"
forge_bind_chart_services<ForgeChartServices, ForgeChartScene, ForgeChartMark,
    ForgeChartLine, ForgeChartTick, ForgeChartLabel>(ui);
```

公共入口消费者为 `examples/ui/charts.slint`，由 Rust/C++ 共享。`--charts-smoke` 检查 7 个组件实例的数据数量、几何状态和窗口截图。安装后的 C++ SDK 提供 ForgeCharts.hpp、静态实现及全部 Slint 文件。

底层接口为 Rust `charts::prepare_chart` / `chart_hit_test` 和 C++ `forge_prepare_chart` / `forge_chart_hit_test`。几何结果由跨宿主 CSV 比对验证。圆弧语义见 [Slint Path 官方文档](https://docs.slint.dev/latest/docs/slint/reference/elements/path/)。
