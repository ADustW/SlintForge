# Navigation Shared Types

Public structs, enums, and globals for this subsystem.

| Symbol | Kind | Members / values | Source |
| --- | --- | --- | --- |
| `ForgeNavPlacement` | Enum | `left`, `right`, `top` | `ui/types/navigation.slint:3` |
| `ForgePaneState` | Enum | `expanded`, `compact`, `hidden` | `ui/types/navigation.slint:5` |
| `ForgeNavItemKind` | Enum | `page`, `group`, `header`, `separator` | `ui/types/navigation.slint:7` |
| `ForgeRouteParam` | Struct | `key: string`, `value: string` | `ui/types/navigation.slint:9` |
| `ForgeNavigationItem` | Struct | `id: string`, `parent-id: string`, `title: string`, `icon: string`, `kind: ForgeNavItemKind`, `depth: int`, `expanded: bool`, `has-children: bool`, `child-count: int`, `enabled: bool` | `ui/types/navigation.slint:11` |
| `ForgeBreadcrumbItem` | Struct | `id: string`, `title: string` | `ui/types/content.slint:3` |
| `ForgeTabItem` | Struct | `id: string`, `title: string` | `ui/types/content.slint:5` |
