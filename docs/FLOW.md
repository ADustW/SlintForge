# ForgeFlow 节点与连线

> **兼容提示：**本页继续保留 ForgeFlow 专题背景与实施记录。逐组件中英文 API 见 [中文 API 站点源码](site/index.md) 与 [English API source](site/en/index.md)。

ForgeFlow 提供卡片节点编辑、流程图、拓扑图及思维导图，不包含流程执行、多人协作或最终用户模板设计器。两端共享 Slint 1.18.0 界面和 C++ 编辑核心；Rust 1.98.0 通过运行库 DLL 调用同一核心。

## 公共组件

| 组件 | 职责 |
| --- | --- |
| `ForgeFlow` | 默认卡片、新增删除、标题编辑、画布及编辑控制 |
| `ForgeFlowCanvas` | 视口、网格、连线、框选；`@children` 是世界坐标下的节点层 |
| `ForgeFlowNode` | 标题拖动、选中状态、折叠、端口和正文 `@children` |
| `ForgeFlowPort` | 方向、位置、连接状态及连接请求 |
| `ForgeFlowBackground` | 随视口移动的点阵、网格或纯色背景 |
| `ForgeFlowEdge` | 复用核心几何绘制连线、箭头和标签，支持自定义标签内容 |
| `ForgeFlowEdgeEditor` | 线型、线宽、箭头、颜色、虚线和标签编辑面板 |
| `ForgeFlowNodeToolbar` | 节点上方的复制／子主题、删除和属性操作 |
| `ForgeFlowControls` | 缩放、适应、撤销和重做 |
| `ForgeFlowMiniMap` | 视口指示及点击、拖动导航、滚轮缩放 |
| `ForgeFlowLayoutControls` | 分层／树布局、四方向及间距设置，发送单次布局命令 |
| `ForgeFlowAlignmentControls` | 六种对齐及水平／垂直等距排列，可独立嵌入业务面板 |
| `ForgeFlowNodePalette` | 可配置类型／标签的节点库，上报点击及拖放手势 |
| `ForgeFlowContextMenu` | 自定义命令／标签／可用状态，支持键盘及外部点击关闭 |
| `ForgeFlowNodeProperties` | 选区批量权限和尺寸编辑，一次提交一步撤销 |
| `ForgeFlowPanel` | 屏幕坐标中的业务面板容器，可组合工具栏或小地图 |
| `ForgeFlowTheme` | 独立的深浅主题令牌，可覆盖画布、卡片、边框、强调色等 |

本轮与 Vue Flow 的功能和样式对比见 [对比审查与优化记录](FLOW_VUEFLOW_REVIEW.md)。

2026-10-14 最新完成增量撤销历史与实例级内存预算。相同 500 节点／1000 连线、900 次编辑场景中，C++／Rust 私有内存采样峰值从 901.37／901.91 MiB 降到 471.84／458.48 MiB，降低 47.7%／49.2%。双端各三次有效性能复测全部通过，最差帧 P95 为 26.8031ms、缩放阶段最差 28.2044ms、反馈 P95 最差 26.6954ms。见 [增量历史与内存验证](FLOW_HISTORY_MEMORY.md) 和 `build/flow-history-memory-20261014/result.json`。

密集连线专项修复见 [连线审查记录](FLOW_ROUTING_REVIEW.md)。正文强制缓存保持回退。性能中 Rust 使用 Skia DX12，C++ 使用显式 Skia OpenGL，不能代表 C++ 默认 DX12；首次有 3 例拖动未触发的无效测量，保持原样重测后三轮全部有效，无效记录保留。既往缓存、负载和性能记录见 `FLOW_VUEFLOW_REVIEW.md`。当前字节预算仅约束历史计费存储，多小时耐久和剩余内存来源的堆分析尚未完成。

每个画布独立创建文档，模型与历史不会跨实例共享。`ForgeFlowServices` 管理句柄，不保存全局当前文档。创建窗口后绑定服务，窗口入口从 `@slintforge` 导出下列全部类型：

```cpp
#include "ForgeFlowBindings.hpp"
forge_bind_flow_services<ForgeFlowPointData, ForgeFlowPortData, ForgeFlowNodeData,
    ForgeFlowEdgeData, ForgeFlowViewport, ForgeFlowRect,
    ForgeFlowAction, ForgeFlowState, ForgeFlowServices>(ui);
```

```rust
slintforge::bind_flow_services!(ui, ForgeFlowServices,
    ForgeFlowPointData, ForgeFlowPortData, ForgeFlowNodeData, ForgeFlowEdgeData,
    ForgeFlowViewport, ForgeFlowRect, ForgeFlowAction, ForgeFlowState);
```

完整接入项目位于 `examples/flow-cpp` 和 `examples/flow-rust`，共享窗口在 `examples/ui/flow.slint`。`tools/generate_flow_bindings.py --check` 检查共享数据契约与双端绑定是否一致。

## 渲染器

Gallery 和 Flow 示例启动时默认使用 `winit-skia`，CPU 路径为 `SLINT_BACKEND=winit-skia-software`。渲染器属于应用，不能随页面切换。Windows 使用既有 CLion 环境 `D:/.clion-env/qt-6.8.3-msvc.bat`。C++ 需要实际包含 Skia 的 Slint DLL。

Rust 显式开启 `slint = { version = "=1.18.0", features = ["renderer-skia"] }` 和 `slintforge` 的 `flow` 特性。二进制 SDK 编译入口使用 `Compile-Rust.ps1 -Flow`。独立 Rust 示例另启用 `unstable-wgpu-30` 获取性能回调，业务组件不要求该特性。Windows 默认使用 `WGPU_BACKEND=dx12`，避免选择当前 Skia 构建未支持的 Vulkan；用户已有的环境选择优先。

`winit-software` 与 `winit-skia-software` 不同，前者保留用于旧组件回归，不支持此组件需要的整体缩放。参考 [Slint 容器机制](https://docs.slint.dev/latest/docs/slint/reference/language/container-components/) 和 [winit 渲染器](https://docs.slint.dev/latest/docs/slint/guide/backends-and-renderers/backend_winit/)。

## 自定义卡片

调用方持有节点循环，在编译期按 `kind` 组合 Slint 控件；同一图中可混用不同卡片。正文控件处理自身输入、按钮、滚动和焦点，拖动仅从标题栏发起。示例：

```slint
import { ForgeFlowCanvas, ForgeFlowNode, ForgeFlowServices } from "@slintforge";
import { LineEdit } from "std-widgets.slint";
export component DeviceGraph inherits ForgeFlowCanvas {
    for n in root.state.nodes : ForgeFlowNode {
        node: n; visible: root.node-visible(n);
        read-only: root.read-only; pan-mode: root.pan-mode; snap: root.snap;
        zoom: root.viewport.zoom;
        candidate-port: root.connection-target == n.id ? root.connection-port : "";
        action(a) => { root.act(a); }
        LineEdit {
            text: ForgeFlowServices.field(n.data-json, "name");
            enabled: !root.read-only;
            accepted => {
                root.act({op: "field", id: n.id, target: "name", text: self.text});
            }
        }
    }
}
```

按稳定 ID 刷新模型行，使用 `visible` 裁剪，避免重建业务组件及丢失未提交输入。在回车、提交或明确操作边界更新业务数据，避免把每个按键记录成历史。`connection-allowed(ForgeFlowAction) -> bool` 在连接预览期间及松开提交前调用，含准确的 `id/port/target/target-port`；应实现为无副作用的快速校验函数，返回 `false` 拒绝连接或重连。有效目标端口变绿并吸附；重复、自连接和方向错误在预览阶段即被排除。未知卡片类型显示占位内容，业务数据保留。

节点通过 `accent/card-background/corner-radius` 定制外观，`resizable: false` 可关闭四角缩放。`ForgeFlowTheme` 随 Slint 的深浅主题切换，并允许业务统一覆盖；其作用域是窗口内的 Flow，不修改其他组件的全局色板。`ForgeFlowCanvas.background-variant` 为 0 点阵、1 网格、2 纯色；`grid: false` 兼容旧接口，强制纯色。

自定义连线时设置 `ForgeFlowCanvas.show-edges: false`，在节点循环之前自行循环 `state.edges`，实例化 `ForgeFlowEdge`。设置其 `show-label: false`，在 `@children` 中放置按钮、状态等 Slint 内容；内容坐标相对于 `edge.min-x/min-y`，例如标签位置为 `(edge.label-x - edge.min-x, edge.label-y - edge.min-y)`。调用方负责隐藏 `edge.hidden` 项及视口裁剪。核心仍处理连线命中、端点重连和选择。

组合示例见 [`flow-custom-edge.slint`](../examples/component-usage/flow-custom-edge.slint)，演示在选中连线的标签位置放置可点击的删除按钮。

## 编辑操作

滚轮围绕鼠标位置缩放，中键或空格加左键平移。工具栏可切换选择／平移模式，画布获得焦点时 V／H 快捷切换。标题栏拖动节点，Ctrl／Shift 点击追加选择，空白处拖动框选。选中节点后可从四角调整尺寸，连续调整只占一步撤销；默认卡片最小 160×112，核心允许最小 64×48。节点上方提供复制、删除和标题属性入口。Ctrl+A 全选、Ctrl+D 复制、Delete 删除、Ctrl+Z 撤销、Ctrl+Y 或 Ctrl+Shift+Z 重做，Escape 取消当前拖动或连接。正文中的输入、按钮和滚动组件优先处理自己的事件。

示例的“添加节点”面板提供文本、表单、设备、图片四类卡片，点击放入画布中心，也可拖入指定位置。设置面板提供背景、小地图、网格吸附和文档路径；顶栏保存按钮保存当前文档。流程图展示通过／未通过分支，拓扑图展示可操作的设备网络，五个 Gallery 页面各自保留文档状态。

从输出或双向端口拖到兼容端口创建连线。点击连线后可删除、拖动端点圆环重新连接，也会显示连线样式面板，可修改直线／贝塞尔曲线／折线／圆角折线、线宽（1～8）、箭头、虚线、颜色和标签。颜色提供预设及 `#RRGGBB` 自定义输入，重置恢复主题默认颜色。Ctrl／Shift 追加选择多条连线时，面板对选中连线批量应用，记为一步撤销。示例顶部线型下拉框在选中连线时修改现有连线，未选中时设置后续新建连线的线型。`show-edge-editor: false` 可隐藏内置面板，由业务自行提供编辑入口。

自动路径按端口朝向引出、从目标端口外侧进入。路径受卡片遮挡时尝试卡片外侧的水平／垂直通道，复杂障碍使用网格寻路；曲线和圆角折线会再次检查圆角实际轮廓。避障为非端点卡片预留描边半径与 2 个逻辑像素的额外间距，窄通道依次缩减额外间距和描边净空。直线模式优先两端直连，受阻时改用折线绕行。若卡片重叠堵住端口，或通道窄于线宽，需移动卡片或减小线宽；算法不移动用户的卡片。展开分组的正文允许连线通过，整个标题区域参与避障。

同一端口可连接多个节点，默认仍拒绝重复的同端口连接；允许并行连接时，同一对节点按稳定 ID 分配通道。一对多、多对一在端口附近错开转弯距离，分支可以分别选中；隐藏连线不占用自动通道。共用端口附近仍可能有短暂重合，密集图中的不同连线也可能相交，不保证所有线条互不相交。显式 `parallel_offset` 和手工 `polyline/cubic` 路径保留业务控制，手工路径不会自动避障。需要严格的两点直线时，可使用 `path_kind: "polyline", control_points: []`。

绘制、箭头、标签和命中使用同一份世界坐标几何，描边不再额外缩小路径。移动或调整节点尺寸会重算相邻连线和受到遮挡影响的其他连线；未改变的模型行保持不动。小地图可点击或拖动导航；只读模式拒绝样式及其他编辑请求。

## 数据和命令

节点包含 `id/kind/title/x/y/width/height/ports/data`，坐标是世界坐标逻辑像素，最小尺寸 64×48。端口 ID 在节点内唯一；`direction` 为 0 输入、1 输出、2 双向；`side` 为 0 左、1 右、2 上、3 下；`offset` 为 0～1 的比例。节点尺寸变化会更新相关端口及连线。

连线包含 `id/source_node/source_port/target_node/target_port/style/arrow/label`，另支持可选 `stroke_width`（1～8，旧文档默认 2）、`stroke_color`（空字符串跟随主题，或 `#RRGGBB`）和 `dashed`（布尔值，默认 false）。样式 0 直线、1 贝塞尔曲线、2 折线、3 圆角折线。虚线不影响连续路径的命中及标签定位。Slint 字段用连字符，核心 JSON 字段用下划线；绘制模型额外提供 RGB 分量，业务保存以 `stroke_color` 为准。`ForgeFlowState.selected-edge` 提供当前选中连线的显示属性，`selected-edge-count` 为可见选中连线数；没有选中时 `selected-edge.id` 为空。

| 请求 | 参数与行为 |
| --- | --- |
| `insert` / `node` | 完整 `node` 对象；更新另传 `id`，ID 不可改 |
| `add` | `text/x/y`，`target` 可指定卡片类型，自动分配 ID |
| `select` / `select-all` / `select-rect` | `id/additive`；框选 `x/y`、`width/height` 表示两个角的世界坐标 |
| `drag-begin` / `drag-move` / `drag-end` | 起始节点和世界坐标；`flag=true` 吸附 20 像素网格；`cancel` 取消 |
| `resize-begin` / `resize-move` / `resize-end` | 起始 `id/x/y`，`number=0/1/2/3` 为左上／右上／左下／右下；起始 `width/height` 指定最小尺寸；移动传世界坐标和吸附 `flag`；`cancel` 恢复 |
| `duplicate` / `delete` | 复制或删除选择；复制包含选中节点之间的连线 |
| `connect` | `id/port/target/target_port/number/flag/text`，后三项为样式、箭头、标签 |
| `edge` / `remove-edge` | 完整 `edge` 更新或按 `id` 删除 |
| `edge-appearance` | `id` 指定连线，省略或为空则修改选中连线；`appearance` 对象允许基础外观以及下方列出的高级样式字段 |
| `title` / `data` / `field` | 标题、完整业务 JSON 字符串、单字段；核心 `field.value` 可提交任意 JSON 值 |
| `undo` / `redo` | 默认最多 100 步、32 MiB；连续拖动一步；视口及选择不进入历史 |
| `history-configure` | `options` 或 `text` 中设置 `max_steps/max_bytes`，仅作用于当前控制器 |
| `view` | `x/y/zoom`，默认限制 10%～400%，可由文档导航选项配置 |
| `add-child` / `add-sibling` | 当前节点 `id` 和标题 `text` |
| `reparent` / `collapse` / `layout` | `id/target` 修改父级；折叠保留数据；重新布局可撤销 |
| `load` | `text` 为完整文档 JSON，验证失败保留当前文档和历史 |

C++ `ForgeFlowDocument` 和 Rust `FlowDocument` 提供节点、数据、连接、选择、撤销、布局、视口及导入导出方法，`execute` 可使用全部命令。返回 JSON 的 `ok/error` 为业务校验结果；Rust 外层 `Result` 为 DLL/ABI 错误，两层都要检查。运行库操作号 36～40 追加在已有编号之后，接口指纹不匹配由加载器明确拒绝。

### 增量撤销与内存预算

历史按稳定节点／连线 ID 保存双向字段差量，新增和删除只携带受影响元素及位置，真正重排才额外保存 ID 顺序。历史编码为 CBOR，撤销与重做移动同一条记录，不复制整图快照。提交批量事务时只记录前后差量；连续移动期间不追加历史，结束时记一步。保存点、业务数据、分组、树布局及连线属性继续参与内容恢复，视口和选择不进入历史。

```cpp
document.configure_history(100, 32 * 1024 * 1024);
auto history = document.history_status();
```

```rust
document.configure_history(100, 32 * 1024 * 1024)?;
let history = document.history_status()?;
```

共享 Slint 界面可调用 `canvas.act({op: "history-configure", text: "{\"max_steps\":100,\"max_bytes\":33554432}"})`。查询使用 `ForgeFlowServices.query(token, "{\"target\":\"history\"}")`；`document_status()` 也包含下列统计。此扩展使用既有 JSON 命令通道，不改变 DLL 操作编号或共享 Slint 结构；Flow 契约更新为 14 并刷新接口指纹，宿主应重新编译并部署配套 DLL，旧版 DLL 会被加载器明确拒绝。

| 字段 | 含义 |
| --- | --- |
| `undo_count / redo_count` | 两端可用步数，共用 `history_limit` 上限 |
| `undo_bytes / redo_bytes / history_bytes` | 两端与合计计费字节；撤销重做不增加合计 |
| `history_limit / history_budget_bytes` | 当前实例的步数与字节上限，默认 100 和 33554432 |
| `history_evicted` | 因预算裁剪或超大编辑边界释放的旧记录数 |
| `history_barriers` | 无法记录的编辑边界次数，包括关闭历史后的实际编辑 |
| `history_accounting` | `cbor_payload_and_entry`：有效载荷、条目对象及链表链接 |

`max_steps` 允许 0～10000，`max_bytes` 允许 0～1073741824，必须为整数，任一为 0 则关闭历史。缩小预算立即裁剪最早的撤销记录，再裁剪最远的重做记录，保留从当前状态可达的连续历史。单次编辑超过预算仍生效，但会清空撤销和重做，建立历史边界；后续小编辑可以继续撤销。无变化操作和失败操作不消费历史，也不丢弃重做分支。手势进行中和批量事务内部不允许修改预算。

预算属于控制器运行配置，不写入文档 JSON、不进入撤销栈、不改变未保存状态。成功加载／切换示例清空历史及统计，保留实例预算；失败加载保留原文档和历史。预算不是进程内存上限：活动文档、保存点、一次手势／事务的临时数据、差量编码临时内存、渲染器／GPU 和分配器开销另计。超过预算的巨型编辑仍可能产生临时峰值，应通过业务数据引用管理大型图片或附件。

更新正在显示的画布时，通过其 `act()` 或 `ForgeFlowServices.execute(token, action)` 提交请求。Slint 的 `ForgeFlowAction` 不包含任意 JSON 对象，因此 `insert`、`node`、`edge` 同时接受 `text` 中的完整 JSON 字符串，保持同一文档、行模型和撤销历史。例如 `canvas.act({op: "insert", text: "{\"id\":\"device-1\",\"kind\":\"device\",\"title\":\"设备\",\"data\":{\"status\":\"running\"}}"})`。不要直接替换只用于绘制的模型行来绕过文档控制器。

双端 `update_edge_appearance(id, appearance)` 可只更新外观字段，保留端点、ID 和业务扩展数据。例如 C++ `document.update_edge_appearance("edge-3", {{"style", 3}, {"stroke_color", "#7c5ce7"}, {"dashed", true}})`；Rust `document.update_edge_appearance("edge-3", &json!({"style": 3, "stroke_color": "#7c5ce7", "dashed": true}))`。Slint 使用单字段请求：`canvas.act({op: "edge-appearance", id: "edge-3", target: "style", number: 3})`。`target` 为 `arrow/dashed` 时使用 `flag`，`label/stroke_color` 使用 `text`，`stroke_width` 使用 `width`。每次请求进入历史，校验失败原子回滚。

通用图允许环，拒绝悬空引用；默认拒绝自连接和同端口重复连接，并按严格端口方向校验，可通过连接选项调整。思维导图为单根树，另含 `parent/order/side/collapsed`：根父级为空，`side=-1/1` 为左右分支，0 为首次自动分配。布局按实际卡片尺寸和稳定顺序确定子树高度；修改父级拒绝循环或第二个根。标题栏加减按钮折叠展开，父主题 ID 输入框回车提交修改。

## 第二批扩展（功能门禁已通过）

普通图节点的 `is_group: true` 表示容器，也可使用 `kind: "group"`。`parent` 指向容器，保存的 `x/y` 是父级局部坐标；没有父级时是画布坐标。`state.nodes` 始终提供画布坐标，并附带 `local_x/local_y/depth`。`node-world` 查询返回画布坐标。思维导图的既有树坐标语义保持不变。

- `group`：将同一父级下的选择创建为分组。`ungroup`：移除容器并保留后代和内部连线。`reparent`：保持画布位置修改父级。`collapse`：折叠仅改变显示，保留全部数据和展开尺寸。`constrain_parent` 限制子节点位于父级正文内；非法拖入恢复整个手势。
- `copy/cut/paste`：应用内剪贴板可跨文档使用。片段保存选中根及全部后代、内部连线、端口 ID、业务数据和手动控制点；粘贴重建节点／边 ID 及父级引用。`selection-fragment` 查询提供可序列化片段，宿主可自行接入系统剪贴板或跨进程传递。默认快捷键仅在画布焦点下处理，卡片输入框保留自己的剪贴板行为。
- `paste` 可提供 `fragment` 对象或 `text` JSON，`target` 为目标父级；`flag: true` 时 `x/y` 指定片段根包围框左上角的画布位置，否则偏移 32。图和思维导图片段不能混合；非空思维导图要求目标父主题。复制不进入历史，剪切／粘贴各占一步。原子事务拒绝共享剪贴板副作用，允许显式片段粘贴。
- `layout` 的 `options` 或 `text` JSON 可设置 `algorithm: layered/tree`、`direction: LR/RL/TB/BT`、`rank_gap`、`node_gap`、`parent` 和 `recursive`。分层布局允许环；树布局要求森林，不接受环或多父节点。布局使用实际尺寸，`layout_fixed` 或不可拖动节点保持原位；展开的分组递归布局并按内容调整尺寸。一次布局只产生一步历史。若手动边两端整体平移，控制点随之平移；两端相对位置变化时保留原控制点，由业务选择是否恢复自动路径。
- `align` 的 `target` 支持 `left/right/top/bottom/center-x/center-y/distribute-x/distribute-y`。分组和其后代同时选中时只移动最上层选择；移动权限、父级边界与撤销使用同一控制器校验。空间不足时拒绝等距排列。
- 文档 `options.grid_size` 默认 20，范围 2～256；显示网格、移动和调整尺寸吸附使用同一参数。`guides_enabled`、`guide_snap` 默认开启，`guide_tolerance` 默认 6 屏幕像素。拖动时可显示参考线，画布 `snap` 开启后才调整位置。参考线属于临时显示状态，结束或取消手势后清除，不进入保存内容。

双端 SDK 提供 `group_selection/ungroup/reparent/toggle_collapse/node_world`、`copy_selection/cut_selection/selection_fragment/paste_fragment/paste_clipboard`、`layout_graph` 和 `align_selected`。布局、对齐控制组件可独立嵌入业务 UI。Ctrl+G／Ctrl+Shift+G 创建／解除分组，Ctrl+C／X／V 操作图剪贴板。

画布新增 `fit-selected()`、`focus-node(id)`，F 适应全部、Shift+F 适应选区、Ctrl+F 打开搜索。`search(text)` 查询返回匹配 ID、标题或类型的节点 ID；ASCII 不区分大小写，中文使用原文匹配。折叠后代可被搜索，业务隐藏节点及其后代不会出现。`reveal(id)` 展开祖先并选择目标，展开操作可一次撤销；只读画布只能定位已经显示的节点。`bounds(selected,id)` 返回世界坐标边界，选区包含选中的连线路径。

导航参数放在文档 `options` 中，加载时统一校验：

| 选项 | 默认值和范围 |
| --- | --- |
| `min_zoom/max_zoom` | 0.1／4；允许 0.01～16，最小值不得大于最大值 |
| `wheel_mode` | `zoom`；可选 `pan/none`；平移模式下 Ctrl+滚轮缩放 |
| `wheel_zoom_step` | 1.12，必须大于 1 且不超过 2 |
| `wheel_zoom_requires_control` | false；开启后普通滚轮不缩放 |
| `auto_pan` | true；拖动节点或连接到边缘时平移 |
| `auto_pan_margin/auto_pan_speed` | 32 屏幕像素／每次 16ms 定时更新 8 逻辑像素，范围 8～160／大于 0 且不超过 40 |
| `touch_enabled` | true；Slint 的双指缩放事件围绕手势中心改变视口 |

配置缩放范围时会限制当前视口；撤销文档修改不会回退平移和缩放。双指操作使用 `ScaleRotateGestureHandler`；验收程序通过固定 Slint 1.18.0 的后端事件入口测试分发与锚点，不代表实体触摸屏或 Windows 原生触控板手势验收。Rust 示例对 `i-slint-core` 的固定版本依赖仅用于这项验收，业务 SDK 不要求使用内部接口。

`node-properties`／双端 `patch_selected_nodes(patch)` 对当前节点选区应用公共属性，支持移动、选择、连接、删除、尺寸调整权限，以及 `layout_fixed/header_visible` 和尺寸约束。不能在批量公共属性中改 ID、父级、端口或业务数据；这些操作可通过原有原子 `batch` 组合提交。批量尺寸修改会检查全部节点的调整权限与父级边界，失败保留整个选区的原数据。

默认画布提供右键菜单，可用 `show-context-menu: false` 关闭后自行接入 `ForgeFlowContextMenu`；命令、标签和禁用状态由调用方指定。`ForgeFlowNodePalette` 只上报手势，调用方负责节点模板和坐标转换。自定义工具栏或小地图放在 Canvas 同级的 `ForgeFlowPanel` 中，绑定 `canvas-width/canvas-height` 与锚点，不随画布缩放。小地图可设置视口描边／填充、节点圆角，并通过 `node-color(node)` 按业务状态着色。组合用法见 `examples/component-usage/flow-editor.slint`；布局与对齐分别见 `flow-layout.slint`、`flow-alignment.slint`。自行循环节点时应绑定 `visible: canvas.node-visible(node)`，同时处理业务隐藏、折叠及视口裁剪。

阶段验证入口为 `tools/windows/Test-FlowBatch2.ps1`；`-Quick` 检查默认 Skia 100% 与 Skia 软件 200%，默认检查完整渲染器／应用缩放组合。它不替代 Gallery、外部 SDK 消费或第三批性能与长期运行验收；整批完成状态以能力清单为准。

## 保存和验证

### 第一批扩展接口（功能场景已验证，性能待处理）

默认仍禁止自连接并检查输入输出方向。`configure(options)` 或 Slint 的 `act({op:"options", text:"…JSON…"})` 设置 `connection_mode`（`strict`／`loose`）、`connection_radius`（1～200 个屏幕逻辑像素，默认 16）、`allow_self_connections`、`click_to_connect`。宽松模式允许任意方向组合，但不放宽引用、容量、重复连接和同一端口约束。点击起点、再点击终点建立连接，Escape 取消。端口的 `max_connections: 0` 表示不限量，容量统计包括输入、输出和自连接。配置须与当前文档相容，否则原子拒绝。

节点增加 `draggable/selectable/connectable/deletable/resizable`（默认 true）、`hidden`（默认 false）、`header_visible/header_height`（默认 true／36）、`min_width/min_height/max_width/max_height/aspect_ratio`。比例 0 表示自由缩放，正数表示宽高比。连线增加 `selectable/deletable/reconnectable/hidden`，端口增加 `connectable`。禁止交互不影响业务主动更新数据；隐藏节点时隐藏相关连线，保存时保留数据。

`ForgeFlowNode.drag-area` 使用节点局部坐标，默认标题栏，示例图片卡片演示整卡拖动。`minimum-resize-width/height` 默认 160／112，受文档尺寸范围约束。正文控件优先处理事件。业务绑定 `content-width/content-height` 并启用 `auto-size` 后，内容变化通过 `auto-size` 命令提交实际尺寸。这是业务内容的派生布局，不额外增加撤销步骤；`auto-form` 示例演示内容展开及尺寸变化。

连线显示模型补充了 `source_node/source_port/target_node/target_port/kind/data_json`；文档业务数据仍为独立的 `data` 对象。`path_kind: auto` 使用默认路由；`polyline` 使用 `control_points: [{x,y},…]`；`cubic` 每段提供两个控制点，中间段另含该段终点，最终终点始终跟随目标端口。一段三次曲线需要两个控制点，两段需要五个。控制点为世界坐标，最多 512 个。绘制、箭头、标签、命中和边界共用几何计算，重连保留业务数据及控制点。

连线属性面板可添加折点，拖动方形手柄改变路径，右键删除折点；“自动路径”恢复自动寻路。三次曲线控制点通过 `patch_edge` 成组设置，手柄可逐点拖动。一次连续拖动占一步历史。临时连接预览复用相同几何计算。

`ForgeFlowCanvas` 增加 `edit-allowed(action) -> bool`、`edited(action,state)`、`edit-rejected(action,error)` 和 `interaction(name,action)`。事件包括节点／连线的点击、双击、右键、进入和离开，以及 `drag/resize/bend-begin/move/end`。鼠标事件命名如 `node-click`、`edge-context-menu`；`connection-allowed` 继续处理连接业务校验。连续操作结束被拒绝时恢复操作前的状态，取消操作始终允许。

双端文档对象新增 `batch/configure/patch_node/patch_edge/update_edge_data` 和 `query/upstream/downstream/connected_edges/ports/intersections`。查询不修改历史或修订号，邻居查询可选择直接相邻或递归遍历。`batch` 接受命令数组，全部成功后提交并合为一步撤销，失败恢复文档、选择和历史；不接受文档替换、撤销重做、嵌套事务或连续手势。

C++ `apply_checked` 以异常报告拒绝；Rust 对应返回 `Result<Value, FlowError>`，区分运行库错误、业务拒绝和响应错误。原 `execute` 及 `ok/error` 接口保留。`set_before_edit/set_after_edit` 提供 SDK 层审核及通知，回调内不要递归编辑同一文档。批量命令整体审核、整体通知。文档对象须在所属 UI 线程串行使用。

绑定类型列表增加 `ForgeFlowPointData`。操作号仍为 36～40，ABI 指纹同时覆盖 C 核心契约和 Slint Flow 数据契约；旧 DLL 被明确拒绝。独立示例的 `--sdk-contract` 检查公开事务、查询、审核和保存加载接口，新增窗口场景为 `examples/ui/flow-batch1-qa.json`。

共用核心统一导出整数 `version: 2` 的 JSON，并兼容加载 `version: 1`。加载时在完整校验后迁移版本，保留未知卡片类型、独立业务 `data` 和扩展字段；不支持的版本或非法引用不会替换当前文档、历史或未保存状态。保存包含节点、端口、连线、视口和树状态，选中项保留在编辑状态。第三方 JSON 固定为 nlohmann/json 3.12.0，许可证和校验值见 `third_party/nlohmann/SOURCE.md`。

在 Rust DLL 中编译共用 C++ 核心时，MSVC 必须启用 `/EHsc`。否则 JSON 库会禁用异常，语法错误可能终止进程，无法进入原子加载的错误处理。当前运行库构建入口已显式启用；双端测试覆盖损坏 JSON、未知版本和非法引用。

构建入口为 `tools/windows/Flow-Dev.ps1 -Task cpp/rust -Config Release`。两端 `--smoke` 读取 `examples/ui/flow-qa.json`，向真实窗口分发指针和键盘事件并验证模型与截图。`--benchmark` 和 `--benchmark --complex` 分别测量 500 节点、1000 连线的标准与混合卡片。帧样本不足会标记 `measurement_complete=false`，不会以空样本判定达标。

`SLINTFORGE_FLOW_QA_DIR` 设置证据目录，`SLINTFORGE_FLOW_QA_SCENARIO` 可覆盖默认验收场景；`SLINT_SCALE_FACTOR=1/1.5/2` 检查应用缩放。连线增量验收入口为 `tools/windows/Test-FlowConnections.ps1`，仅覆盖 Flow 核心、相关 DLL 接口、双端窗口及五个 Gallery 页面，不逐页回归其他组件。其中 `flow-edge-qa.json` 验证四种线型、颜色、虚线与保存；`flow-refinement-qa.json` 验证缩放手柄、节点库拖放、平移模式、有效端口提示及深浅窄窗口。对比和本轮实测记录见 `FLOW_VUEFLOW_REVIEW.md`；之前的连线与首版验收见 `FLOW_CONNECTIONS_VERIFICATION.md`、`FLOW_VERIFICATION.md`。

## 第三批：高级连线

选中连线后，在属性面板点击“更多”修改起点／终点标记、流动动画、速度、曲率、通道偏移及层级。多选时修改选中的全部连线，一次提交占一步历史。只读模式保留样式查看入口并禁止修改。

| JSON 字段 | 约定 |
| --- | --- |
| `marker_start` / `marker_end` | 名称 `none/arrow/open-arrow/circle/diamond`，或包含 `shape/size/filled/points` 的对象。缺省起点无标记，终点沿用 `arrow`。标记尺寸 4～64，默认 11。 |
| 自定义标记 | `shape: "custom"`，坐标以尖端为原点，负 X 向连线内部；X 为 -2～0，Y 为 -1～1，再乘 `size` 并按端点方向旋转。轮廓 2～64 点，填充至少 3 点。自定义轮廓由开发者通过 SDK 提供。 |
| `animated` / `animation_speed` | 是否流动、速度（5～600 世界像素／秒，默认 60）。沿完整几何路径运动，不改变文档或历史。 |
| `curvature` | 自动贝塞尔曲线的控制距离系数，0～2，默认 0.4。存在手动控制点时沿用手动路径。 |
| `layer` | -900～1500，默认 0；普通节点从 1 开始。重合连线的命中遵守层级；卡片正文仍优先处理自身输入。 |
| `parallel_offset` | 单条自动连线的通道偏移覆盖，-512～512。未设置时根据同一对节点间的稳定边 ID 自动分配。 |

`options.parallel_spacing` 配置自动通道间距（0～256，默认 24）；`allow_parallel_connections: true` 才允许通用图的同端口重复边。默认仍拒绝重复连接，思维导图仍保持树约束。曲线使用偏移控制点，折线采用独立通道；通道被障碍物阻塞时优先避障，因此不保证所有拥挤区域都保持指定间距。自定义手动路径不自动偏移。

双端 SDK 的 `update_edge_appearance` 接受以上字段。Slint 单字段请求中，`animated` 使用 `flag`，`marker_start/marker_end` 使用 `text`，`curvature/animation_speed/parallel_offset` 使用 `width`，`layer` 使用 `number`。高级对象可用 `edge-patch` 的 `text` JSON 提交。显式修改旧 `arrow` 开关会清除高级终点标记覆盖，起点标记保持原样。

`ForgeFlowEdge` 和 `ForgeFlowCanvas` 提供 `animations-enabled`，供宿主响应减少动态效果的偏好或暂停预览。绘制只读取核心缓存的等距路径采样，视口外边不启动动画。Gallery 示例还在页面不活动时关闭动画。显示模型增加 `start-marker/end-marker/marker-stroke-path/motion-points/path-length/curvature/layer/parallel-offset`，接入时应使用配套 DLL 的接口指纹。

最终功能证据：`build/flow-batch3-acceptance-20261014-052944/result.json`，234 步第三批场景覆盖双端、两种 Skia 渲染器和三档应用缩放，共 12 个组合，并回归五组已有场景。Gallery、外部 SDK 接入及动画／导出文件检查通过；持续编辑和释放检查也有独立记录。汇总中的 `functional_passed` 为 true、`passed` 为 false，后者保留 Rust 帧耗时未达标结论。


## 第三批：文档状态与图片导出

`ForgeFlowState.dirty` / Canvas `dirty` 表示内容与保存点不同；视口和选择不影响它。新建／加载／示例文档建立未修改基线。`document-status` 查询还返回 `schema_version/loaded_version/save_serial/editing/undo_count/redo_count/history_limit`。业务自己写入 JSON 后，应在写入成功后调用 C++ `mark_saved()` 或 Rust `mark_saved()?`；`export_json` 和图片导出本身不会清除未保存状态。连续拖动未结束时拒绝保存。

Canvas `save(path)` 在写入成功后刷新状态，发出 `save-finished(path,error)`；空错误串表示成功。`dirty-changed(bool)` 可用于标题栏标记或离开页面的提示。SDK 不擅自关闭窗口，也不在页面切换时丢弃业务文档。

Canvas `export-image(path,selected,options-json)` 支持 `.svg` / `.png`，结束后发出 `image-exported(path,error)`。示例的“画布与文件”面板提供整图／选区导出。整图使用所有可见节点与连线的世界坐标范围，不依赖当前视口；选中容器包含其后代及内部连线，显式选中的边也可独立导出。业务隐藏、折叠后代、编辑控件、选择框不会进入图片；动画导出为静态路径。

共用核心负责 SVG 序列化。C++ `export_svg(selected,options,adapter)` 与 Rust `export_svg_with(selected,options,adapter)` 可提供卡片适配器，输入为含业务数据的世界坐标节点，返回图元数组。Slint 宿主注册 `ForgeFlowServices.export-card(node-json)`，返回图元数组 JSON；空字符串使用默认正文。适配器只读，不应修改或释放正在导出的文档。C++ 使用 `ForgeFlowImageExport.hpp` 中的 `forge_flow_save_image`，Rust 使用 `slintforge::save_flow_image` 保存 SVG 或通过 Slint 解码生成 PNG。

支持 `text/rect/ellipse/path/image` 图元，坐标相对卡片正文左上角；正文会被裁剪到卡片内。文本统一转义，`path.points` 为点数组，图片接受内嵌 PNG/JPEG Base64。配色使用 `#RRGGBB` 或 `none`。任意 Slint 内容不能自动转成 SVG；业务应提供自己的静态表示，示例同时演示表单值、设备状态和图片卡片的矢量表示。未知卡片默认显示类型占位，原始数据仍保留在 JSON 中。

导出选项包括 `theme: light/dark`、`background`、`padding`（0～512，默认 24）、`scale`（0.01～8，默认 1），以及按节点 ID 提供的 `node_content`。每边最大 32768 像素，总像素不超过 6400 万；大图可缩小倍率或导出选区。SVG 与 PNG 的背景和几何一致，中文字体取决于接入机器安装的字体。导出适配器不是卡片设计器，也不保留输入框和按钮的交互行为。


## 第三批：键盘、可访问性与生命周期

画布获得焦点后，方向键将选中节点移动 1 个世界像素，Shift 加方向键移动 10 个；一次按键占一步历史。多选和分组遵守现有局部坐标、锁定、父级约束及手动路径规则。Ctrl+PageDown / PageUp 顺序选择下一个／上一个可见可选节点；Alt+PageDown / PageUp 选择连线。定位到视口外的元素时保持当前缩放并移入可见区域。

Tab 可依次进入节点标题、正文控件和端口；标题焦点有独立轮廓，方向键移动卡片。正文输入框保留自己的方向键、删除和剪贴板语义。端口获得焦点后，Enter 或空格发起连接，Tab 移到目标端口，再按 Enter 或空格提交；Escape 取消。连接仍经过方向、容量、重复引用和业务校验，与鼠标连接共用核心。

节点标题、端口、连线和画布提供可访问名称、选中和只读语义，节点提供展开动作，端口提供默认连接动作，错误提示使用低优先级状态播报。`ForgeFlowNode.chinese` 用于组件提示语言。`ForgeFlowEdge.activated` 供组合调用方连接到选中行为；默认 Canvas 已绑定。当前验证范围为 Slint 真实窗口的键盘分发和可访问属性，不代表已经通过所有屏幕阅读器的人工验收。

Canvas `dispose()` 释放其控制器，清空模型并结束手势、定时平移和弹层；重复调用安全，释放后的 `act` 会拒绝修改。`reopen()` 显式创建新的空白或配置的示例文档，不恢复已释放内容。动态移除组件的宿主应先保存需要保留的 JSON，再调用 `dispose()`；仅切换 Gallery 页面则继续保留文档，关闭页面动画即可。整个窗口销毁时绑定服务持有的所有控制器一起释放。`ForgeFlowServices.active-documents()` 可供宿主诊断实例数量。

`tools/windows/Test-FlowLifecycle.ps1` 在两端实际窗口中分别创建 500 节点／1000 连线、混合卡片与分组，持续提交 900 次编辑，验证 100 步历史、边界撤销重做、60 次画布重建及 64 次临时控制器释放，记录进程私有内存和工作集曲线。它不将短期内存趋势当作多小时运行或堆泄漏分析结论。

早期记录为 `build/flow-lifecycle-20261014-052022/result.json`，双端通过，分别持续约 152／149 秒。C++／Rust 进程私有内存峰值约为 751／744MiB；当时历史使用完整文档内容快照。旧 `winit-software` 的输入验证和标准按钮增量回归见 `build/flow-batch3-legacy-software-20261014-053531/result.json`，双端各 14 个状态通过。

上述为历史记录。缓存改造后曾测得 C++／Rust 峰值约 909／867MiB，见 `build/flow-performance-resumed-20261014/loop3-lifecycle/result.json`。历史优化前的基线为 `build/flow-performance-resumed2-20261014/lifecycle/result.json`：两端各 1379 步通过，峰值 901.37／901.91MiB，不能据此证明缓存回退降低了总体内存。增量历史后的同场景复测位于 `build/flow-history-memory-20261014/lifecycle/result.json`：峰值 471.84／458.48MiB，分别持续 140.37／140.61 秒，全部步骤通过。多小时编辑和堆分配验证仍为独立待办。
