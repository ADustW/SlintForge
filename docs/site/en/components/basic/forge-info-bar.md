---
title: ForgeInfoBar
description: ForgeInfoBar public API, behavior, example, and host integration.
---

# ForgeInfoBar

`ForgeInfoBar` is a public component in the **Basic** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-info-bar" name="ForgeInfoBar" />

## Purpose and usage

- **Use when:** composing a basic screen that needs the behavior represented by `ForgeInfoBar`.
- **Inheritance:** `ForgeInfoBar` → `Rectangle`.
- **Source:** `ui/basic/info-bar.slint:7`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeInfoBar } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `level` | `in` | `ForgeInfoLevel` | `ForgeInfoLevel.info` | Constrained by its type and default expression | Host-owned input |
| `message` | `in` | `string` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `dismissible` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `dismissed` | `()` | Emitted by the component and handled by the host. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** No direct key-event handler is declared in this component source.
- **Pointer:** No direct pointer handler is declared in this component source.
- **Accessibility:** No explicit accessible metadata was detected; provide surrounding labels and validate the composed screen.
- **Localization:** No internal translatable text was detected; caller-supplied strings remain host-owned.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-info-bar.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/basic/forge-info-bar)
