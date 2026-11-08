---
title: ForgePagination
description: ForgePagination public API, behavior, example, and host integration.
---

# ForgePagination

`ForgePagination` is a public component in the **Navigation** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-pagination" name="ForgePagination" />

## Purpose and usage

- **Use when:** composing a navigation screen that needs the behavior represented by `ForgePagination`.
- **Inheritance:** `ForgePagination` → `Rectangle`.
- **Source:** `ui/navigation/pagination.slint:3`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgePagination } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `total-items` | `in` | `int` | `0` | Constrained by its type and default expression | Host-owned input |
| `page-size` | `in` | `int` | `10` | Constrained by its type and default expression | Host-owned input |
| `enabled` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `current-page` | `in-out` | `int` | `1` | Constrained by its type and default expression | Shared by host and component |
| `page-count` | `out` | `int` | `root.total-items <= 0 \|\| root.page-size <= 0 ? 0 : ceil(root.total-items / root.page-size)` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `page-changed` | `(int)` | Emitted by the component and handled by the host. |
| Public function | `go-to` | `(page: int)` | Called by the host and handled synchronously by the component. |
| Public function | `normalize` | `()` | Called by the host and handled synchronously by the component. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** No direct key-event handler is declared in this component source.
- **Pointer:** No direct pointer handler is declared in this component source.
- **Accessibility:** Explicit accessible metadata is present; the host must still provide meaningful labels.
- **Localization:** The component contains translated text or a language input.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-pagination.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/navigation/forge-pagination)
