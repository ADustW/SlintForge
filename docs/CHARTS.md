# ForgePlot 统一图表与迁移

SlintForge 使用 `ForgePlot` 作为唯一图表组件。共享的 C++ 绘图内核负责数据、路径、命中、选择、图例和矢量导出，Rust/C++ SDK 只负责模型适配和位图输出。无需 ECharts。

## 图形配置

| 展示 | 配置 |
|---|---|
| 分组／正负堆叠柱图 | `kind: bars`；`stacked`、`stack-group`、`bar-width` |
| 折线／面积 | `kind: graph`；`line-style: line / smooth`；`fill-color` |
| 散点 | `kind: graph`、`line-style: none`、`scatter-style: disc` |
| 气泡 | `kind: bubble`；`point-size` 为默认直径，`point-styles` 覆盖逐点尺寸和颜色 |
| 饼／环 | `kind: pie`；`inner-radius: 0 / 0.6` |
| 极区面积 | `kind: polar-area`；等角扇区，半径正比于数值平方根 |
| 雷达 | `kind: radar`；`panels.categories` 至少 3 项，多系列维度一致 |

旧枚举 `graph` 至 `polar` 的数值 0–6 保持不变；新增 `bubble=7`、`pie=8`、`polar-area=9`、`radar=10`。`smooth` 追加在已有线型之后。曲线采用 Catmull–Rom 插值并细分到共享绘制路径，面积和命中沿同一曲线边界。

```slint
import { ForgePlot } from "@slintforge";
ForgePlot {
    panels: [{id: 0, width: 1, height: 1, categories: ["研发", "设计", "测试"]}];
    legend-placement: outside;
    series: [{
        id: 10, kind: pie, name: "资源分布", inner-radius: 0.6,
        points: [{y: 35}, {y: 20}, {y: 45}],
        point-styles: [{index: 0, color: #0078d4}, {index: 1, color: #00b7c3}, {index: 2, color: #ffb900}],
    }];
}
```

`ForgePlotPointStyle` 是可选旁路数据，不增加实时波形点的体积。`index` 始终是原始点索引；`hidden` 隐藏对应点；透明 `color` 使用默认颜色；`size=0` 继承系列默认尺寸，负值、NaN 或超过 16384 的气泡尺寸不显示。尺寸单位为逻辑像素直径。

饼图与极区图忽略非正、非有限和隐藏值，图例按原始数据项排列并允许重新显示隐藏项。命中返回系列 `id` 和原始 `index`，环形中心不会命中。`inner-radius` 必须在 `[0,1)`。每个饼图／极区系列使用独立绘图区。笛卡尔、原有 polar、pie、polar-area、radar 坐标体系不得静默混绘；无效配置通过 `scene.status=2` 与 `scene.error` 返回。

径向图表保留悬停、选择和图例，禁用轴平移、缩放、波形测量及历史窗口。笛卡尔图表保留追加、降采样、游标、历史和缓存。主题切换重新渲染视图，保留数据、范围、选择和游标。

## 两端绑定

入口导出：

```slint
export { ForgePlotServices, ForgePlotScene, ForgePlotAxis, ForgePlotRange,
    ForgePlotRect, ForgePlotHit, ForgePlotSelection, ForgePlotExportResult } from "@slintforge";
```

Rust：

```rust
slintforge::bind_plot_services!(ui, ForgePlotServices, ForgePlotScene, ForgePlotAxis,
    ForgePlotRange, ForgePlotRect, ForgePlotHit, ForgePlotSelection, ForgePlotExportResult);
```

C++：

```cpp
#include "ForgePlot.hpp"
forge_bind_plot_services<ForgePlotServices, ForgePlotScene, ForgePlotAxis,
    ForgePlotRange, ForgePlotRect, ForgePlotHit, ForgePlotSelection, ForgePlotExportResult>(ui);
```

`export-file(path, format)` 支持 PNG、JPEG、BMP、SVG、PDF。位图导出使用 Slint 字体上下文；无可见窗口的 C++ 消费者也应先创建 Slint 组件再执行位图导出。示例见 `examples/ui/charts.slint` 和 `tests/cpp/ForgePlotChartTests.cpp`。

## ABI v2 迁移

本次直接删除旧 `ForgeChart` 系列、旧类型及 `bind_chart_services!` / `forge_bind_chart_services`，没有兼容壳。原 `series/slices` 统一为 `ForgePlotSeries.points`；切片 `value` 改为 `y`，标签放在 `panels.categories` 或点 `label`；旧气泡半径迁移时乘以 2 转为直径；`doughnut` 改为 `inner-radius`；`point-selected` 改为单个 `ForgePlotHit` 参数。

DLL 入口为 `slintforge_runtime_get_api_v2`，版本、表大小及契约指纹均验证。旧操作编号 21–23 退役，不再接受请求，也不重新分配；其他运行库操作编号保持不变。消费者必须同步更新 UI、Rust/C++ 绑定和 DLL。版本不匹配明确返回 ABI 不兼容错误。

Gallery 继续使用 `chart-bar`、`chart-line`、`chart-pie`、`chart-polar`、`chart-bubble`、`chart-scatter`、`chart-radar` 路由。
