# Widgets 公共类型

本子系统的公共结构体、枚举和全局对象。

| 符号 | 类型 | 成员 / 枚举值 | 源码 |
| --- | --- | --- | --- |
| `ForgeCaptureRect` | 结构体 | `x: length`, `y: length`, `width: length`, `height: length` | `ui/types/capture.slint:2` |
| `ForgeCaptureResult` | 结构体 | `ok: bool`, `image: image`, `pixel-width: int`, `pixel-height: int`, `error: string` | `ui/types/capture.slint:9` |
| `ForgeCaptureService` | 全局对象 | `connected`, `capture-window`, `capture-region` | `ui/types/capture.slint:18` |
| `ForgeChoiceItem` | 结构体 | `id: string`, `label: string`, `enabled: bool`, `selected: bool` | `ui/types/widgets.slint:2` |
| `ForgeTreeItem` | 结构体 | `id: string`, `parent-id: string`, `label: string`, `depth: int`, `folder: bool`, `expanded: bool`, `selected: bool`, `visible: bool`, `enabled: bool` | `ui/types/widgets.slint:3` |
| `ForgeTimelineItem` | 结构体 | `title: string`, `description: string`, `time: string`, `completed: bool` | `ui/types/widgets.slint:7` |
| `ForgeTourStep` | 结构体 | `title: string`, `description: string`, `x: length`, `y: length`, `width: length`, `height: length` | `ui/types/widgets.slint:8` |
| `ForgeSyntaxLine` | 结构体 | `number: int`, `content: styled-text` | `ui/types/widgets.slint:9` |
| `ForgeQRCodeResult` | 结构体 | `ok: bool`, `image: image`, `modules: int`, `error: string` | `ui/types/widgets.slint:10` |
| `ForgeWidgetServices` | 全局对象 | `connected`, `screen-color-supported`, `color-to-hex`, `color-hex-valid`, `color-from-hex`, `screen-color`, `screen-color-input`, `qr-code`, `syntax-lines`, `backdrop`, `suggestion-index` | `ui/types/widgets.slint:12` |
