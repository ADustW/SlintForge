# Navigation 公共类型

本子系统的公共结构体、枚举和全局对象。

| 符号 | 类型 | 成员 / 枚举值 | 源码 |
| --- | --- | --- | --- |
| `ForgeNavPlacement` | 枚举 | `left`, `right`, `top` | `ui/types/navigation.slint:3` |
| `ForgePaneState` | 枚举 | `expanded`, `compact`, `hidden` | `ui/types/navigation.slint:5` |
| `ForgeNavItemKind` | 枚举 | `page`, `group`, `header`, `separator` | `ui/types/navigation.slint:7` |
| `ForgeRouteParam` | 结构体 | `key: string`, `value: string` | `ui/types/navigation.slint:9` |
| `ForgeNavigationItem` | 结构体 | `id: string`, `parent-id: string`, `title: string`, `icon: string`, `kind: ForgeNavItemKind`, `depth: int`, `expanded: bool`, `has-children: bool`, `child-count: int`, `enabled: bool` | `ui/types/navigation.slint:11` |
| `ForgeBreadcrumbItem` | 结构体 | `id: string`, `title: string` | `ui/types/content.slint:3` |
| `ForgeTabItem` | 结构体 | `id: string`, `title: string` | `ui/types/content.slint:5` |
