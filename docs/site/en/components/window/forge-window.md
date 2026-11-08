---
title: ForgeWindow
description: ForgeWindow public API, behavior, example, and host integration.
---

# ForgeWindow

`ForgeWindow` is a public component in the **Window** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-window" name="ForgeWindow" />

## Purpose and usage

- **Use when:** composing a window screen that needs the behavior represented by `ForgeWindow`.
- **Inheritance:** `ForgeWindow` → `Window`.
- **Source:** `ui/window/forge-window.slint:9`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeWindow } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `brand-symbol` | `in` | `string` | `"S"` | Constrained by its type and default expression | Host-owned input |
| `brand-icon` | `in` | `image` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `show-brand-icon` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `version-text` | `in` | `string` | `""` | Constrained by its type and default expression | Host-owned input |
| `show-menu-button` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `leading-menu-button` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `show-back-button` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `back-enabled` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `show-search-box` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `search-text` | `in` | `string` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `search-placeholder` | `in` | `string` | `@tr("搜索组件")` | Constrained by its type and default expression | Host-owned input |
| `search-has-focus` | `out` | `bool` | `root.show-search-box && search-input.has-focus` | Constrained by its type and default expression | Read-only component output |
| `navigation-glyph` | `in` | `string` | `ForgeIcons.panel-left` | Constrained by its type and default expression | Host-owned input |
| `show-theme-button` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `show-pin-button` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `show-settings-button` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `show-maximize-button` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `dark-mode` | `in-out` | `bool` | `false` | Constrained by its type and default expression | Shared by host and component |
| `pinned` | `in-out` | `bool` | `false` | Constrained by its type and default expression | Shared by host and component |
| `rounded-corners` | `in-out` | `bool` | `true` | Constrained by its type and default expression | Shared by host and component |
| `title-bar-height` | `out` | `length` | `ForgeMetrics.title-bar-height` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `menu-requested` | `()` | Emitted by the component and handled by the host. |
| Callback | `back-clicked` | `()` | Emitted by the component and handled by the host. |
| Callback | `search-edited` | `(string)` | Emitted by the component and handled by the host. |
| Callback | `search-accepted` | `(string)` | Emitted by the component and handled by the host. |
| Callback | `theme-toggle-requested` | `()` | Emitted by the component and handled by the host. |
| Callback | `pin-state-changed` | `(bool)` | Emitted by the component and handled by the host. |
| Callback | `settings-requested` | `()` | Emitted by the component and handled by the host. |
| Public function | `focus-search` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `capture` | `() -> ForgeCaptureResult` | Called by the host and handled synchronously by the component. |
| Default slot | `default` | `@children` | Hosts Slint children supplied by the caller. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** The implementation contains focus or key-event handling.
- **Pointer:** The implementation contains pointer/touch handling.
- **Accessibility:** No explicit accessible metadata was detected; provide surrounding labels and validate the composed screen.
- **Localization:** The component contains translated text or a language input.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-window.slint{qml}

## Host integration

This component references `ForgeCaptureService`. Initialize the runtime and bind the generated Slint global service type:

```rust
slintforge::runtime::initialize()?;
slintforge::bind_capture_service!(ui, /* generated Slint service/result types */);
```

Replace macro type arguments with the types generated by the consumer's `slint!`/`include_modules!` call.

```cpp
#include <ForgeCapture.hpp>

forge_bind_capture_service(ui);
```


## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/window/forge-window)
