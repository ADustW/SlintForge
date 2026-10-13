# ForgeFlow 节点与连线

ForgeFlow 提供卡片节点编辑、流程图、拓扑图及思维导图，不包含流程执行、多人协作或最终用户模板设计器。两端共享 Slint 1.18.0 界面和 C++ 编辑核心；Rust 1.98.0 通过运行库 DLL 调用同一核心。

## 公共组件

| 组件 | 职责 |
| --- | --- |
| `ForgeFlow` | 默认卡片、新增删除、标题编辑、画布及编辑控制 |
| `ForgeFlowCanvas` | 视口、网格、连线、框选；`@children` 是世界坐标下的节点层 |
| `ForgeFlowNode` | 标题拖动、选中状态、折叠、端口和正文 `@children` |
| `ForgeFlowPort` | 方向、位置、连接状态及连接请求 |
| `ForgeFlowControls` | 缩放、适应、撤销和重做 |
| `ForgeFlowMiniMap` | 视口指示及点击、拖动导航 |

每个画布独立创建文档，模型与历史不会跨实例共享。`ForgeFlowServices` 管理句柄，不保存全局当前文档。创建窗口后绑定服务，窗口入口从 `@slintforge` 导出下列全部类型：

```cpp
#include "ForgeFlowBindings.hpp"
forge_bind_flow_services<ForgeFlowPortData, ForgeFlowNodeData,
    ForgeFlowEdgeData, ForgeFlowViewport, ForgeFlowRect,
    ForgeFlowAction, ForgeFlowState, ForgeFlowServices>(ui);
```

```rust
slintforge::bind_flow_services!(ui, ForgeFlowServices,
    ForgeFlowPortData, ForgeFlowNodeData, ForgeFlowEdgeData,
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

按稳定 ID 刷新模型行，使用 `visible` 裁剪，避免重建业务组件及丢失未提交输入。在回车、提交或明确操作边界更新业务数据，避免把每个按键记录成历史。`connection-allowed(ForgeFlowAction) -> bool` 在端口解析后、写入前调用，含准确的 `id/port/target/target-port`；返回 `false` 拒绝连接或重连。未知卡片类型显示占位内容，业务数据保留。

## 编辑操作

滚轮围绕鼠标位置缩放，中键或空格加左键平移。标题栏拖动节点，Ctrl／Shift 点击追加选择，空白处拖动框选。画布获得焦点时，Ctrl+A 全选、Ctrl+D 复制、Delete 删除、Ctrl+Z 撤销、Ctrl+Y 或 Ctrl+Shift+Z 重做，Escape 取消当前拖动或连接。正文中的输入、按钮和滚动组件优先处理自己的事件。

从输出或双向端口拖到兼容端口创建连线。点击连线后可删除、拖动端点圆环重新连接，也会显示连线样式面板，可修改直线／曲线／折线、线宽（1～8）、箭头和标签。Ctrl／Shift 追加选择多条连线时，面板对选中连线批量应用，记为一步撤销。示例顶部线型下拉框在选中连线时修改现有连线，未选中时设置后续新建连线的线型。`show-edge-editor: false` 可隐藏内置面板，由业务自行提供编辑入口。

曲线和折线按端口朝向引出、从目标端口外侧进入。路径受卡片遮挡时尝试卡片外侧的水平／垂直通道，曲线使用圆滑转角，标签及命中检测跟随实际路径。直线模式保持两端直连。若卡片重叠并堵住端口，需先移动卡片留出通道。移动或调整节点尺寸会重算相邻连线和受到遮挡影响的其他连线；未改变的模型行保持不动。小地图可点击或拖动导航；只读模式拒绝样式及其他编辑请求。

## 数据和命令

节点包含 `id/kind/title/x/y/width/height/ports/data`，坐标是世界坐标逻辑像素，最小尺寸 64×48。端口 ID 在节点内唯一；`direction` 为 0 输入、1 输出、2 双向；`side` 为 0 左、1 右、2 上、3 下；`offset` 为 0～1 的比例。节点尺寸变化会更新相关端口及连线。

连线包含 `id/source_node/source_port/target_node/target_port/style/arrow/label`，另支持可选 `stroke_width`（1～8，旧文档默认 2）。样式 0 直线、1 曲线、2 折线。Slint 字段用连字符，核心 JSON 字段用下划线。`ForgeFlowState.selected-edge` 提供当前选中连线的显示属性，`selected-edge-count` 为可见选中连线数；没有选中时 `selected-edge.id` 为空。

| 请求 | 参数与行为 |
| --- | --- |
| `insert` / `node` | 完整 `node` 对象；更新另传 `id`，ID 不可改 |
| `add` | `text/x/y`，自动分配 ID |
| `select` / `select-all` / `select-rect` | `id/additive`；框选 `x/y`、`width/height` 表示两个角的世界坐标 |
| `drag-begin` / `drag-move` / `drag-end` | 起始节点和世界坐标；`flag=true` 吸附 20 像素网格；`cancel` 取消 |
| `duplicate` / `delete` | 复制或删除选择；复制包含选中节点之间的连线 |
| `connect` | `id/port/target/target_port/number/flag/text`，后三项为样式、箭头、标签 |
| `edge` / `remove-edge` | 完整 `edge` 更新或按 `id` 删除 |
| `edge-appearance` | `id` 指定连线，省略或为空则修改选中连线；`appearance` 对象仅允许 `style/arrow/label/stroke_width` |
| `title` / `data` / `field` | 标题、完整业务 JSON 字符串、单字段；核心 `field.value` 可提交任意 JSON 值 |
| `undo` / `redo` | 最近 100 步；连续拖动一步；视口及选择不进入历史 |
| `view` | `x/y/zoom`，限制 10%～400% |
| `add-child` / `add-sibling` | 当前节点 `id` 和标题 `text` |
| `reparent` / `collapse` / `layout` | `id/target` 修改父级；折叠保留数据；重新布局可撤销 |
| `load` | `text` 为完整文档 JSON，验证失败保留当前文档和历史 |

C++ `ForgeFlowDocument` 和 Rust `FlowDocument` 提供节点、数据、连接、选择、撤销、布局、视口及导入导出方法，`execute` 可使用全部命令。返回 JSON 的 `ok/error` 为业务校验结果；Rust 外层 `Result` 为 DLL/ABI 错误，两层都要检查。运行库操作号 36～40 追加在已有编号之后，接口指纹不匹配由加载器明确拒绝。

更新正在显示的画布时，通过其 `act()` 或 `ForgeFlowServices.execute(token, action)` 提交请求。Slint 的 `ForgeFlowAction` 不包含任意 JSON 对象，因此 `insert`、`node`、`edge` 同时接受 `text` 中的完整 JSON 字符串，保持同一文档、行模型和撤销历史。例如 `canvas.act({op: "insert", text: "{\"id\":\"device-1\",\"kind\":\"device\",\"title\":\"设备\",\"data\":{\"status\":\"running\"}}"})`。不要直接替换只用于绘制的模型行来绕过文档控制器。

双端 `update_edge_appearance(id, appearance)` 可只更新外观字段，保留端点、ID 和业务扩展数据。例如 C++ `document.update_edge_appearance("edge-3", {{"style", 2}, {"stroke_width", 4}, {"arrow", true}})`；Rust `document.update_edge_appearance("edge-3", &json!({"style": 2, "stroke_width": 4, "arrow": true}))`。Slint 使用单字段请求：`canvas.act({op: "edge-appearance", id: "edge-3", target: "style", number: 2})`。`target` 为 `arrow/label/stroke_width` 时分别使用 `flag/text/width`。每次请求进入历史，校验失败原子回滚。

通用图允许环，但拒绝悬空引用、自连接、错误端口方向和同端口重复连接。思维导图为单根树，另含 `parent/order/side/collapsed`：根父级为空，`side=-1/1` 为左右分支，0 为首次自动分配。布局按实际卡片尺寸和稳定顺序确定子树高度；修改父级拒绝循环或第二个根。标题栏加减按钮折叠展开，父主题 ID 输入框回车提交修改。

## 保存和验证

共用核心统一导出整数 `version: 1` 的 JSON，保存节点、端口、连线、独立 `data` 对象、视口和树状态。业务内容不拼入显示标题。选中项保留在编辑状态。第三方 JSON 固定为 nlohmann/json 3.12.0，许可证和校验值见 `third_party/nlohmann/SOURCE.md`。

在 Rust DLL 中编译共用 C++ 核心时，MSVC 必须启用 `/EHsc`。否则 JSON 库会禁用异常，语法错误可能终止进程，无法进入原子加载的错误处理。当前运行库构建入口已显式启用；双端测试覆盖损坏 JSON、未知版本和非法引用。

构建入口为 `tools/windows/Flow-Dev.ps1 -Task cpp/rust -Config Release`。两端 `--smoke` 读取 `examples/ui/flow-qa.json`，向真实窗口分发指针和键盘事件并验证模型与截图。`--benchmark` 和 `--benchmark --complex` 分别测量 500 节点、1000 连线的标准与混合卡片。帧样本不足会标记 `measurement_complete=false`，不会以空样本判定达标。

`SLINTFORGE_FLOW_QA_DIR` 设置证据目录，`SLINTFORGE_FLOW_QA_SCENARIO` 可覆盖默认验收场景；`SLINT_SCALE_FACTOR=1/1.5/2` 检查应用缩放。连线增量验收入口为 `tools/windows/Test-FlowConnections.ps1`，仅覆盖 Flow 核心、相关 DLL 接口、双端窗口及五个 Gallery 页面，不逐页回归其他组件。具体记录见 `FLOW_CONNECTIONS_VERIFICATION.md`；首版完整验收见 `FLOW_VERIFICATION.md`。
