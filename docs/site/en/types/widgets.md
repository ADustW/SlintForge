# Widgets Shared Types

Public structs, enums, and globals for this subsystem.

| Symbol | Kind | Members / values | Source |
| --- | --- | --- | --- |
| `ForgeCaptureRect` | Struct | `x: length`, `y: length`, `width: length`, `height: length` | `ui/types/capture.slint:2` |
| `ForgeCaptureResult` | Struct | `ok: bool`, `image: image`, `pixel-width: int`, `pixel-height: int`, `error: string` | `ui/types/capture.slint:9` |
| `ForgeCaptureService` | Global | `connected`, `capture-window`, `capture-region` | `ui/types/capture.slint:18` |
| `ForgeChoiceItem` | Struct | `id: string`, `label: string`, `enabled: bool`, `selected: bool` | `ui/types/widgets.slint:2` |
| `ForgeTreeItem` | Struct | `id: string`, `parent-id: string`, `label: string`, `depth: int`, `folder: bool`, `expanded: bool`, `selected: bool`, `visible: bool`, `enabled: bool` | `ui/types/widgets.slint:3` |
| `ForgeTimelineItem` | Struct | `title: string`, `description: string`, `time: string`, `completed: bool` | `ui/types/widgets.slint:7` |
| `ForgeTourStep` | Struct | `title: string`, `description: string`, `x: length`, `y: length`, `width: length`, `height: length` | `ui/types/widgets.slint:8` |
| `ForgeSyntaxLine` | Struct | `number: int`, `content: styled-text` | `ui/types/widgets.slint:9` |
| `ForgeQRCodeResult` | Struct | `ok: bool`, `image: image`, `modules: int`, `error: string` | `ui/types/widgets.slint:10` |
| `ForgeWidgetServices` | Global | `connected`, `screen-color-supported`, `color-to-hex`, `color-hex-valid`, `color-from-hex`, `screen-color`, `screen-color-input`, `qr-code`, `syntax-lines`, `backdrop`, `suggestion-index` | `ui/types/widgets.slint:12` |
