# Theme 公共类型

本子系统的公共结构体、枚举和全局对象。

| 符号 | 类型 | 成员 / 枚举值 | 源码 |
| --- | --- | --- | --- |
| `ForgeTheme` | 全局对象 | `color-scheme`, `dark`, `accent-light`, `accent-dark`, `background`, `content-background`, `popup-background`, `foreground`, `alternate-background`, `alternate-foreground`, `control-background`, `control-foreground`, `accent-background`, `accent-foreground`, `selection-background`, `selection-foreground`, `border`, `secondary-accent-background`, `tertiary-accent-background`, `accent-disabled`, `accent-control-border`, `control-border`, `text-accent-foreground-secondary`, `text-accent-foreground-disabled`, `text-secondary`, `text-tertiary`, `text-disabled`, `text-control-border`, `control-secondary`, `control-tertiary`, `control-disabled`, `control-alt-secondary`, `control-alt-tertiary`, `control-alt-quaternary`, `control-alt-disabled`, `control-strong-stroke`, `control-strong-stroke-disabled`, `control-solid`, `circle-border`, `control-input-active`, `focus-stroke-inner`, `focus-stroke-outer`, `control-background-stroke-flyout`, `sub-title-secondary`, `sub-title-tertiary`, `shadow`, `subtle`, `subtle-secondary`, `subtle-tertiary`, `divider`, `layer-on-mica-base-alt`, `layer-on-mica-base-alt-secondary`, `card-background`, `card-stroke`, `state`, `state-secondary`, `smoke`, `success`, `warning`, `error`, `success-background`, `warning-background`, `error-background` | `ui/theme/theme.slint:3` |
| `ForgeMetrics` | 全局对象 | `control-height`, `navigation-height`, `title-bar-height`, `pane-width`, `compact-pane-width`, `control-radius`, `surface-radius`, `stroke-width`, `page-padding`, `space-xs`, `space-s`, `space-m`, `space-l`, `space-xl`, `space-xxl` | `ui/theme/theme.slint:86` |
| `ForgeFlowTheme` | 全局对象 | `dark`, `canvas`, `surface`, `raised`, `border`, `text`, `muted`, `accent`, `edge`, `grid`, `success`, `radius` | `ui/flow/theme.slint:4` |
| `ForgeFlowIcons` | 全局对象 | `add`, `minus`, `select`, `hand`, `fit`, `undo`, `redo`, `copy`, `trash`, `lock`, `grid`, `save`, `settings` | `ui/flow/theme.slint:54` |
| `ForgeInfoLevel` | 枚举 | `info`, `success`, `warning`, `error` | `ui/types/feedback.slint:3` |
| `ForgeTypography` | 全局对象 | `family`, `resolved-family`, `scale`, `caption`, `body`, `subtitle`, `title` | `ui/typography/typography.slint:2` |
| `ForgeIcons` | 全局对象 | `add`, `apps`, `arrow-left`, `brightness`, `sun`, `moon`, `checkmark`, `dismiss`, `globe`, `home`, `navigation`, `info`, `warning`, `pin`, `pin-off`, `search`, `settings`, `text-font`, `window`, `panel-left`, `panel-left-expand`, `panel-right`, `panel-right-expand`, `panel-top-contract`, `panel-top-expand`, `arrow-clockwise`, `chevron-down`, `chevron-right`, `calendar`, `eyedropper` | `ui/icons/icon.slint:8` |
| `ForgeIconEntry` | 结构体 | `name: string`, `codepoint: string`, `glyph: string` | `ui/icons/icon.slint:5` |
| `ForgeSplitOrientation` | 枚举 | `horizontal`, `vertical` | `ui/layouts/split-layout.slint:3` |
| `ForgeStaggeredItem` | 结构体 | `id: string`, `title: string`, `description: string`, `tile-height: length`, `accent: color`, `column: int`, `top: length` | `ui/layouts/staggered-grid-view.slint:4` |
| `ForgeDockRect` | 结构体 | `x: length`, `y: length`, `width: length`, `height: length` | `ui/layouts/dock-layout.slint:3` |
| `ForgeDockEdge` | 枚举 | `top`, `bottom`, `left`, `right` | `ui/layouts/dock-layout.slint:4` |
| `ForgeDelayButtonMode` | 枚举 | `hold`, `hold-release`, `click` | `ui/inputs/action-buttons.slint:52` |
| `ForgeDateTime` | 全局对象 | 无公开成员 | `ui/form/date-time.slint:4` |
| `ForgeDrawerEdge` | 枚举 | `left`, `right` | `ui/popups/drawer.slint:5` |
