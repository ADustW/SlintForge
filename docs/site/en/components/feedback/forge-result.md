---
title: ForgeResult
description: ForgeResult public API, behavior, example, and host integration.
---

# ForgeResult

`ForgeResult` is a public component in the **Feedback** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-result" name="ForgeResult" />

## Purpose and usage

- **Use when:** composing a feedback screen that needs the behavior represented by `ForgeResult`.
- **Inheritance:** `ForgeResult` → `Rectangle`.
- **Source:** `ui/feedback/result.slint:6`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeResult } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `level` | `in` | `ForgeInfoLevel` | `success` | Constrained by its type and default expression | Host-owned input |
| `title` | `in` | `string` | `@tr("操作成功")` | Constrained by its type and default expression | Host-owned input |
| `description` | `in` | `string` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `action-text` | `in` | `string` | `@tr("继续")` | Constrained by its type and default expression | Host-owned input |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `action-requested` | `()` | Emitted by the component and handled by the host. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** No direct key-event handler is declared in this component source.
- **Pointer:** No direct pointer handler is declared in this component source.
- **Accessibility:** No explicit accessible metadata was detected; provide surrounding labels and validate the composed screen.
- **Localization:** The component contains translated text or a language input.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-result.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/feedback/forge-result)
