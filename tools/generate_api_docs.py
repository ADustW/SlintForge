#!/usr/bin/env python3
# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

"""从公共 Slint 入口生成双语 API 文档、清单和最小示例。"""

from __future__ import annotations

import json
from pathlib import Path
import re

from api_docs_model import ROOT, Symbol, capabilities, category, kebab, kind_counts, load_contract, service_dependencies


SITE = ROOT / "docs" / "site"
MANIFEST = ROOT / "docs" / "api" / "components.json"
EXAMPLES = ROOT / "examples" / "api-components"

CATEGORY_LABELS = {
    "basic": ("基础组件", "Basic"),
    "inputs": ("输入组件", "Inputs"),
    "form": ("表单组件", "Forms"),
    "navigation": ("导航组件", "Navigation"),
    "layouts": ("布局组件", "Layouts"),
    "surfaces": ("容器组件", "Surfaces"),
    "data": ("数据展示", "Data Display"),
    "feedback": ("反馈组件", "Feedback"),
    "popups": ("弹层组件", "Popups"),
    "theme": ("主题与材质", "Theme and Materials"),
    "icons": ("图标", "Icons"),
    "window": ("窗口", "Window"),
    "other": ("其他组件", "Other"),
    "plot": ("ForgePlot", "ForgePlot"),
    "flow": ("ForgeFlow", "ForgeFlow"),
}

ROUTES = {
    "ForgeIconButton": "iconbutton", "ForgeCard": "card", "ForgeInfoBar": "infobar",
    "ForgeSplitLayout": "layouts-split", "ForgeStaggeredGridView": "layouts-staggered",
    "ForgeFlowLayout": "layouts-flow", "ForgeAdaptiveGrid": "layouts-adaptive-grid",
    "ForgePageStack": "layouts-page-stack", "ForgeStackPage": "layouts-page-stack",
    "ForgeFormRow": "layouts-form-row", "ForgeDockLayout": "layouts-dock",
    "ForgePage": "surfaces-page", "ForgeRoundImageView": "surfaces-round-image",
    "ForgeIconLabel": "surfaces-icon-label", "ForgeExpander": "surfaces-expander",
    "ForgeWatermark": "surfaces-watermark", "ForgeMarquee": "surfaces-marquee",
    "ForgeWindow": "forge-window", "ForgeNavigationView": "navigation",
    "ForgeBreadcrumb": "breadcrumb", "ForgeTabBar": "tabbar", "ForgePageHeader": "pageheader",
    "ForgeSegmentedControl": "navigation-segmented", "ForgePagination": "navigation-pagination",
    "ForgePillButton": "inputs-pill-button", "ForgeDelayButton": "inputs-delay-button",
    "ForgeDropDownButton": "inputs-dropdown-button", "ForgeProgressButton": "inputs-progress-button",
    "ForgeInputValidation": "inputs-validation", "ForgeChip": "inputs-chip",
    "ForgeTagInputBox": "inputs-tag-input", "ForgeNumberBox": "form-number-box",
    "ForgeAutoSuggestBox": "form-auto-suggest", "ForgeColorPicker": "form-color-picker",
    "ForgeDatePicker": "form-date-picker", "ForgeCalendarPicker": "form-calendar-picker",
    "ForgeDatePickerPopup": "form-date-picker", "ForgeTimePicker": "form-time-picker",
    "ForgeTimePickerPopup": "form-time-picker", "ForgeShortcutPicker": "form-shortcut-picker",
    "ForgeMultiSelectComboBox": "form-multi-select-combo",
    "ForgeMultiTreeSelectComboBox": "form-multi-tree-combo", "ForgeFileTree": "data-file-tree",
    "ForgeDataGrid": "data-grid", "ForgeTreeDataGrid": "data-tree-grid",
    "ForgeTimeline": "data-timeline", "ForgeStatistic": "data-statistic", "ForgeQRCode": "data-qrcode",
    "ForgeInfoBadge": "feedback-info-badge", "ForgeResult": "feedback-result",
    "ForgeRatingControl": "feedback-rating", "ForgeShimmer": "feedback-shimmer",
    "ForgeSkeletonView": "feedback-skeleton", "ForgeContentDialog": "popups-content-dialog",
    "ForgeDrawer": "popups-drawer", "ForgeAcrylic": "theme-acrylic",
    "ForgeLiquidGlass": "theme-liquid-glass", "ForgeIcon": "theme-icons",
    "ForgeSyntaxView": "other-syntax", "ForgeTour": "other-tour", "ForgeDial": "other-dial",
    "ForgePlot": "chart-professional", "ForgeFlow": "flow-basic",
}

FLOW_ROUTES = ["flow-basic", "flow-custom", "flow-flowchart", "flow-topology", "flow-mindmap"]
OPEN_COMPONENTS = {"ForgeDatePickerPopup", "ForgeTimePickerPopup", "ForgeContentDialog", "ForgeDrawer", "ForgeTour"}


def route_for(symbol: Symbol, flow_index: int) -> str:
    if symbol.name in ROUTES:
        return ROUTES[symbol.name]
    if category(symbol) == "flow":
        return FLOW_ROUTES[flow_index % len(FLOW_ROUTES)]
    raise ValueError(f"缺少 Gallery 截图路由: {symbol.name}")


def type_group(symbol: Symbol) -> str:
    source = symbol.source.replace("\\", "/")
    if "/types/flow.slint" in source:
        return "flow"
    if "/types/plot.slint" in source:
        return "plot"
    if "/types/grids.slint" in source:
        return "grids"
    if "/types/navigation.slint" in source or "/types/content.slint" in source:
        return "navigation"
    if "/types/widgets.slint" in source or "/types/capture.slint" in source:
        return "widgets"
    return "theme"


def manifest_entries(symbols: list[Symbol]) -> list[dict]:
    flow_index = 0
    entries = []
    for symbol in symbols:
        if symbol.kind != "component":
            continue
        route = route_for(symbol, flow_index)
        if category(symbol) == "flow" and symbol.name not in ROUTES:
            flow_index += 1
        slug = kebab(symbol.name)
        entries.append({
            "name": symbol.name,
            "category": category(symbol),
            "slug": slug,
            "source": symbol.source,
            "line": symbol.line,
            "page": f"/components/{category(symbol)}/{slug}",
            "example": f"examples/api-components/{slug}.slint",
            "services": service_dependencies(symbol),
            "screenshot": {"route": route, "open": symbol.name in OPEN_COMPONENTS, "focus": symbol.name},
        })
    return entries


def write(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content.rstrip() + "\n", encoding="utf-8", newline="\n")


def escape_table(value: str) -> str:
    return value.replace("|", "\\|").replace("\n", " ")


def property_rows(symbol: Symbol, english: bool) -> str:
    rows = []
    ownership = {
        "in": ("宿主输入", "Host-owned input"),
        "in-out": ("宿主与组件共享", "Shared by host and component"),
        "out": ("组件只读输出", "Read-only component output"),
    }
    for member in symbol.members:
        if member.kind != "property":
            continue
        default = member.default or ("类型默认值" if not english else "Type default")
        constraint = "由类型与默认表达式约束" if not english else "Constrained by its type and default expression"
        rows.append(
            f"| `{member.name}` | `{member.direction}` | `{escape_table(member.value_type)}` | "
            f"`{escape_table(default)}` | {constraint} | {ownership[member.direction][1 if english else 0]} |"
        )
    if not rows:
        return "| — | — | — | — | " + ("无直接公开属性；请参阅继承类型。" if not english else "No direct public properties; see the inherited type.") + " | — |"
    return "\n".join(rows)


def callable_rows(symbol: Symbol, english: bool) -> str:
    labels = {
        "callback": ("回调", "Callback"),
        "function": ("公开函数", "Public function"),
        "slot": ("默认插槽", "Default slot"),
    }
    rows = []
    for member in symbol.members:
        if member.kind == "property":
            continue
        description = {
            "callback": ("由组件触发，宿主负责处理。", "Emitted by the component and handled by the host."),
            "function": ("由宿主调用，组件同步处理。", "Called by the host and handled synchronously by the component."),
            "slot": ("承载调用方提供的 Slint 子元素。", "Hosts Slint children supplied by the caller."),
        }[member.kind][1 if english else 0]
        rows.append(f"| {labels[member.kind][1 if english else 0]} | `{member.name}` | `{escape_table(member.signature)}` | {description} |")
    if not rows:
        return "| — | — | — | " + ("无直接公开回调、函数或插槽。" if not english else "No direct public callbacks, functions, or slots.") + " |"
    return "\n".join(rows)


def behavior(symbol: Symbol, english: bool) -> str:
    caps = capabilities(symbol)
    if english:
        return "\n".join([
            f"- **Keyboard:** {'The implementation contains focus or key-event handling.' if caps['keyboard'] else 'No direct key-event handler is declared in this component source.'}",
            f"- **Pointer:** {'The implementation contains pointer/touch handling.' if caps['pointer'] else 'No direct pointer handler is declared in this component source.'}",
            f"- **Accessibility:** {'Explicit accessible metadata is present; the host must still provide meaningful labels.' if caps['accessible'] else 'No explicit accessible metadata was detected; provide surrounding labels and validate the composed screen.'}",
            f"- **Localization:** {'The component contains translated text or a language input.' if caps['localized'] else 'No internal translatable text was detected; caller-supplied strings remain host-owned.'}",
            f"- **Theme:** {'Uses SlintForge theme/typography tokens.' if caps['themed'] else 'Uses its declared colors and inherited Slint styling.'}",
            "- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.",
        ])
    return "\n".join([
        f"- **键盘：** {'源码包含焦点或按键事件处理。' if caps['keyboard'] else '该组件源码未声明直接按键处理器。'}",
        f"- **鼠标/触控：** {'源码包含指针或触控处理。' if caps['pointer'] else '该组件源码未声明直接指针处理器。'}",
        f"- **可访问性：** {'已声明可访问性元数据；宿主仍需提供有意义的标签。' if caps['accessible'] else '未检测到显式可访问性元数据；应在组合页面提供标签并做整页验证。'}",
        f"- **本地化：** {'组件包含翻译文本或语言输入。' if caps['localized'] else '未检测到内部可翻译文本；调用方传入的字符串由宿主管理。'}",
        f"- **主题：** {'使用 SlintForge 主题或排版令牌。' if caps['themed'] else '使用组件声明颜色及继承的 Slint 样式。'}",
        "- **平台限制：** Slint 声明中未检测到平台条件；原生窗口、剪贴板、文件和截图行为仍取决于 Slint 后端与宿主接入。",
    ])


def binding_snippets(services: list[str], english: bool) -> str:
    if not services:
        return "无需额外宿主服务。" if not english else "No additional host service is required."
    service_list = ", ".join(f"`{item}`" for item in services)
    intro = (f"本组件引用 {service_list}。先初始化运行库，再绑定由 Slint 生成的全局服务类型：" if not english
             else f"This component references {service_list}. Initialize the runtime and bind the generated Slint global service type:")
    macro = "bind_widget_services"
    header = "ForgeWidgets.hpp"
    cpp = "forge_bind_widget_services(ui);"
    joined = " ".join(services)
    if "Capture" in joined:
        macro, header, cpp = "bind_capture_service", "ForgeCapture.hpp", "forge_bind_capture_service(ui);"
    elif "Grid" in joined:
        macro, header, cpp = "bind_grid_services", "ForgeGrids.hpp", "forge_bind_grid_services(ui);"
    elif "Plot" in joined:
        macro, header, cpp = "bind_plot_services", "ForgePlotBindings.hpp", "forge_bind_plot_services(ui);"
    elif "Flow" in joined:
        macro, header, cpp = "bind_flow_services", "ForgeFlowBindings.hpp", "forge_bind_flow_services(ui);"
    rust_note = "宏的类型参数应替换为消费工程 `slint!`/`include_modules!` 生成的类型。" if not english else "Replace macro type arguments with the types generated by the consumer's `slint!`/`include_modules!` call."
    return f"""{intro}

```rust
slintforge::runtime::initialize()?;
slintforge::{macro}!(ui, /* generated Slint service/result types */);
```

{rust_note}

```cpp
#include <{header}>

{cpp}
```
"""


def component_page(symbol: Symbol, entry: dict, english: bool) -> str:
    cat = CATEGORY_LABELS[entry["category"]][1 if english else 0]
    services = entry["services"]
    example_depth = "../../../../../" if english else "../../../../"
    source_ref = f"`{entry['source']}:{entry['line']}`"
    inheritance = f"`{symbol.name}` → `{symbol.inherits}`"
    if english:
        return f"""---
title: {symbol.name}
description: {symbol.name} public API, behavior, example, and host integration.
---

# {symbol.name}

`{symbol.name}` is a public component in the **{cat}** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="{entry['slug']}" name="{symbol.name}" />

## Purpose and usage

- **Use when:** composing a {cat.lower()} screen that needs the behavior represented by `{symbol.name}`.
- **Inheritance:** {inheritance}.
- **Source:** {source_ref}.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import {{ {symbol.name} }} from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
{property_rows(symbol, True)}

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
{callable_rows(symbol, True)}

## Interaction, accessibility, localization, and platform notes

{behavior(symbol, True)}

## Minimal compilable example

<<< {example_depth}{entry['example']}{{qml}}

## Host integration

{binding_snippets(services, True)}

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page]({entry['page']})
"""
    return f"""---
title: {symbol.name}
description: {symbol.name} 的公共 API、行为、示例与宿主接入说明。
---

# {symbol.name}

`{symbol.name}` 是 **{cat}** 子系统的公共组件。本页由当前 `ui/slintforge.slint` 契约生成，只记录直接声明的公共接口，不改变运行时行为。

<ApiScreenshot slug="{entry['slug']}" name="{symbol.name}" />

## 用途与适用场景

- **适用场景：** 在 {cat} 页面中组合 `{symbol.name}` 所代表的交互或展示能力。
- **继承关系：**{inheritance}。
- **源码位置：**{source_ref}。
- **状态所有权：**`in` 由宿主持有，`in-out` 由宿主与组件共享，`out` 是组件只读输出。

## 导入方式

```qml
import {{ {symbol.name} }} from "slintforge.slint";
```

## 属性

| 属性 | 方向 | 类型 | 默认值 | 约束 | 状态所有权 |
| --- | --- | --- | --- | --- | --- |
{property_rows(symbol, False)}

## 回调、公开函数与插槽

| 类型 | 名称 | 签名 | 契约 |
| --- | --- | --- | --- |
{callable_rows(symbol, False)}

## 键盘、鼠标、可访问性、本地化、主题与平台限制

{behavior(symbol, False)}

## 可编译最小示例

<<< {example_depth}{entry['example']}{{qml}}

## 宿主接入

{binding_snippets(services, False)}

## 相关 API

- [公共类型索引](/types/)
- [Rust SDK](/sdk/rust)
- [C++ SDK](/sdk/cpp)
- [English page](/en{entry['page']})
"""


def example(symbol: Symbol) -> str:
    if symbol.name == "ForgeWindow":
        return f"""import {{ {symbol.name} }} from "../../ui/slintforge.slint";

export component ApiExample inherits {symbol.name} {{
    title: "{symbol.name} API Example";
    width: 800px;
    height: 600px;
}}
"""
    return f"""import {{ {symbol.name} }} from "../../ui/slintforge.slint";

export component ApiExample inherits Window {{
    title: "{symbol.name} API Example";
    width: 800px;
    height: 600px;

    {symbol.name} {{
        x: 32px;
        y: 32px;
        width: parent.width - 64px;
        height: parent.height - 64px;
    }}
}}
"""


def symbol_details(symbol: Symbol, english: bool) -> str:
    source = f"`{symbol.source}:{symbol.line}`"
    if symbol.kind == "struct":
        fields = []
        for item in re.split(r",\s*", re.sub(r"\s+", " ", symbol.body.strip())):
            if ":" in item:
                name, value_type = item.split(":", 1)
                fields.append(f"`{name.strip()}: {value_type.strip()}`")
        detail = ", ".join(fields) or ("No fields" if english else "无字段")
    elif symbol.kind == "enum":
        values = [value.strip() for value in symbol.body.replace("\n", " ").split(",") if value.strip()]
        detail = ", ".join(f"`{value}`" for value in values)
    else:
        names = [f"`{member.name}`" for member in symbol.members]
        detail = ", ".join(names) or ("No public members" if english else "无公开成员")
    kind = {"struct": ("结构体", "Struct"), "enum": ("枚举", "Enum"), "global": ("全局对象", "Global")}[symbol.kind][1 if english else 0]
    return f"| `{symbol.name}` | {kind} | {detail} | {source} |"


def type_pages(symbols: list[Symbol], english: bool) -> None:
    locale = "en/" if english else ""
    types = [symbol for symbol in symbols if symbol.kind != "component"]
    title = "Shared Types" if english else "公共类型"
    intro = ("The current public entry exports 45 structs, 21 enums, and 12 globals. Every symbol below is sourced from `ui/slintforge.slint`."
             if english else "当前公共入口导出 45 个结构体、21 个枚举和 12 个全局对象。以下符号均以 `ui/slintforge.slint` 为准。")
    links = []
    for group in ["navigation", "widgets", "grids", "plot", "flow", "theme"]:
        links.append(f"- [{group.title()}](./{group})")
    write(SITE / locale / "types" / "index.md", f"# {title}\n\n{intro}\n\n" + "\n".join(links))
    for group in ["navigation", "widgets", "grids", "plot", "flow", "theme"]:
        subset = [symbol for symbol in types if type_group(symbol) == group]
        rows = "\n".join(symbol_details(symbol, english) for symbol in subset)
        description = ("Public structs, enums, and globals for this subsystem."
                       if english else "本子系统的公共结构体、枚举和全局对象。")
        write(
            SITE / locale / "types" / f"{group}.md",
            f"# {group.title()} {title}\n\n{description}\n\n"
            f"| {'Symbol' if english else '符号'} | {'Kind' if english else '类型'} | {'Members / values' if english else '成员 / 枚举值'} | {'Source' if english else '源码'} |\n"
            "| --- | --- | --- | --- |\n" + (rows or "| — | — | — | — |"),
        )


def shared_pages(entries: list[dict], symbols: list[Symbol], english: bool) -> None:
    locale = "en/" if english else ""
    prefix = "/en" if english else ""
    if english:
        home = """---
layout: home
title: SlintForge API Documentation
hero:
  name: SlintForge API
  text: Components and host SDK reference
  tagline: 79 public components, shared Slint types, and Rust/C++ integration for v0.1.2.
  actions:
    - theme: brand
      text: Browse components
      link: /en/components/basic/forge-card
    - theme: alt
      text: Integration guide
      link: /en/guide/
features:
  - title: Symmetric bilingual routes
    details: Switch languages while remaining on the same component API.
  - title: Source-derived contracts
    details: Properties, callbacks, functions, examples, and screenshots share one manifest.
  - title: Offline delivery
    details: The installer includes a loopback-only documentation launcher.
---
"""
        guide = """# Documentation overview

This site documents the public contract exported by `ui/slintforge.slint` for SlintForge v0.1.2. Markdown is the canonical content source; generated site assets are not committed.

Each component page records direct public properties, callbacks, functions, slots, a compilable Slint example, its Gallery screenshot, and host-service requirements.
"""
        integration = """# Integration

Import UI symbols from the single public entry `ui/slintforge.slint`. Rust consumers use `slintforge` and C++ consumers include headers under `sdk/cpp/include`. Initialize the runtime before binding capture, widgets, grid, plot, or flow services.

Use `npm run docs:check` before building the site and `npm run docs:build` to produce `build/api-docs-site`.
"""
        migration = """# Migration and compatibility

This first documentation release covers the current v0.1.2 API only. It does not add a version selector and does not alter component runtime contracts. Legacy Markdown reports remain available in `docs/` and remain Chinese-only when they record historical verification work.
"""
    else:
        home = """---
layout: home
title: SlintForge API 文档
hero:
  name: SlintForge API
  text: 组件与宿主 SDK 参考
  tagline: v0.1.2 的 79 个公共组件、Slint 公共类型及 Rust/C++ 接入说明。
  actions:
    - theme: brand
      text: 浏览组件
      link: /components/basic/forge-card
    - theme: alt
      text: 接入指南
      link: /guide/
features:
  - title: 中英文路径对称
    details: 切换语言后仍停留在同一组件 API。
  - title: 契约从源码提取
    details: 属性、回调、函数、示例与截图共用一份清单。
  - title: 离线交付
    details: 安装包内置仅监听回环地址的文档启动器。
---
"""
        guide = """# 文档说明

本站记录 SlintForge v0.1.2 由 `ui/slintforge.slint` 导出的公共契约。Markdown 是唯一内容源，生成的站点产物不提交。

每个组件页包含直接公开属性、回调、函数、插槽、可编译 Slint 示例、Gallery 截图和宿主服务依赖。
"""
        integration = """# 接入指南

UI 符号统一从公共入口 `ui/slintforge.slint` 导入。Rust 消费工程使用 `slintforge` crate，C++ 消费工程包含 `sdk/cpp/include` 下的头文件。绑定截图、Widget、Grid、Plot 或 Flow 服务前先初始化运行库。

构建站点前运行 `npm run docs:check`，使用 `npm run docs:build` 生成 `build/api-docs-site`。
"""
        migration = """# 迁移与兼容性

首版文档仅覆盖当前 v0.1.2 API，不增加版本选择器，也不改变组件运行时契约。`docs/` 下的旧版 Markdown 继续保留；记录历史实施和验收工作的文档维持中文。
"""
    write(SITE / locale / "index.md", home)
    write(SITE / locale / "guide" / "index.md", guide)
    write(SITE / locale / "guide" / "integration.md", integration)
    write(SITE / locale / "guide" / "migration.md", migration)
    if english:
        licenses = """# Third-party licenses

The generated documentation site includes the following MIT-licensed runtime code:

- **VitePress 1.6.4** — Copyright (c) 2019-present Evan You
- **Vue 3** — Copyright (c) 2018-present Yuxi (Evan) You and Vue contributors
- **MiniSearch 7.2.0** — Copyright (c) 2018-present Luca Ongaro

The portable package includes the complete corresponding license texts under `licenses/`. Other SlintForge and Slint licenses remain documented by the existing package notices.
"""
    else:
        licenses = """# 第三方许可证

生成后的文档站包含以下采用 MIT 许可证的运行时代码：

- **VitePress 1.6.4** — Copyright (c) 2019-present Evan You
- **Vue 3** — Copyright (c) 2018-present Yuxi (Evan) You and Vue contributors
- **MiniSearch 7.2.0** — Copyright (c) 2018-present Luca Ongaro

便携包在 `licenses/` 下附带对应的完整许可证文本；SlintForge 与 Slint 的其他许可证继续由现有发布说明覆盖。
"""
    write(SITE / locale / "licenses.md", licenses)

    rust_modules = [path.relative_to(ROOT).as_posix() for path in sorted((ROOT / "sdk/rust/src").glob("*.rs"))]
    cpp_headers = [path.relative_to(ROOT).as_posix() for path in sorted((ROOT / "sdk/cpp/include").glob("*.hpp"))]
    if english:
        rust_page = "# Rust SDK\n\nThe Rust SDK covers routing, navigation trees, screenshots, icons, staggered layout, Widget services, Grid, Plot, Flow, runtime initialization, window corners, and build integration.\n\n## Modules\n\n" + "\n".join(f"- `{item}`" for item in rust_modules) + "\n\n## Initialization\n\n```rust\nslintforge::runtime::initialize()?;\n```"
        cpp_page = "# C++ SDK\n\nThe header SDK covers the same host-service families and links to the shared runtime. Bind generated Slint globals only after the UI component is created.\n\n## Headers\n\n" + "\n".join(f"- `{item}`" for item in cpp_headers) + "\n\n## Build integration\n\nUse the repository CMake targets so the generated Slint headers, runtime library, and SDK include directory stay version-aligned."
    else:
        rust_page = "# Rust SDK\n\nRust SDK 覆盖路由、导航树、截图、图标、瀑布流、Widget 服务、Grid、Plot、Flow、运行库初始化、窗口边角和构建集成。\n\n## 模块\n\n" + "\n".join(f"- `{item}`" for item in rust_modules) + "\n\n## 初始化\n\n```rust\nslintforge::runtime::initialize()?;\n```"
        cpp_page = "# C++ SDK\n\n头文件 SDK 覆盖相同的宿主服务族，并链接共享运行库。创建 UI 组件后再绑定 Slint 生成的全局对象。\n\n## 头文件\n\n" + "\n".join(f"- `{item}`" for item in cpp_headers) + "\n\n## 构建集成\n\n使用仓库 CMake 目标，使 Slint 生成头、运行库和 SDK 包含目录保持同一版本。"
    write(SITE / locale / "sdk" / "rust.md", rust_page)
    write(SITE / locale / "sdk" / "cpp.md", cpp_page)


def main() -> None:
    symbols = load_contract()
    counts = kind_counts(symbols)
    if counts != {"component": 79, "struct": 45, "enum": 21, "global": 12}:
        raise SystemExit(f"公共契约数量变化，拒绝生成: {counts}")
    entries = manifest_entries(symbols)
    MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(entries, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    by_name = {symbol.name: symbol for symbol in symbols}
    for entry in entries:
        symbol = by_name[entry["name"]]
        write(EXAMPLES / f"{entry['slug']}.slint", example(symbol))
        write(SITE / "components" / entry["category"] / f"{entry['slug']}.md", component_page(symbol, entry, False))
        write(SITE / "en" / "components" / entry["category"] / f"{entry['slug']}.md", component_page(symbol, entry, True))
    type_pages(symbols, False)
    type_pages(symbols, True)
    shared_pages(entries, symbols, False)
    shared_pages(entries, symbols, True)
    print(f"API_DOCS_GENERATED components={len(entries)} structs={counts['struct']} enums={counts['enum']} globals={counts['global']}")


if __name__ == "__main__":
    main()
