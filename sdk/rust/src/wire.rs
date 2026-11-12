// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

// 自动生成的接口转换；不包含组件计算实现。
use slintforge_runtime_api::models::*;
use slintforge_runtime_api::{Arena, Wire};

impl Wire for crate::RouteParam {
    type View = FfiRouteParam;
    fn view(&self, arena: &mut Arena) -> Self::View {
        FfiRouteParam {
            key: self.key.view(arena),
            value: self.value.view(arena),
        }
    }
    unsafe fn read(value: Self::View) -> Self {
        Self {
            key: <String as Wire>::read(value.key),
            value: <String as Wire>::read(value.value),
        }
    }
}

impl Wire for crate::RouteState {
    type View = FfiRouteState;
    fn view(&self, arena: &mut Arena) -> Self::View {
        FfiRouteState {
            id: self.id.view(arena),
            params: self.params.view(arena),
        }
    }
    unsafe fn read(value: Self::View) -> Self {
        Self {
            id: <String as Wire>::read(value.id),
            params: <Vec<crate::RouteParam> as Wire>::read(value.params),
        }
    }
}

impl Wire for crate::NavItem {
    type View = FfiNavItem;
    fn view(&self, arena: &mut Arena) -> Self::View {
        FfiNavItem {
            id: self.id.view(arena),
            parent_id: self.parent_id.view(arena),
            title: self.title.view(arena),
            icon: self.icon.view(arena),
            kind: (self.kind as u32),
            enabled: self.enabled.view(arena),
        }
    }
    unsafe fn read(value: Self::View) -> Self {
        Self {
            id: <String as Wire>::read(value.id),
            parent_id: <Option<String> as Wire>::read(value.parent_id),
            title: <String as Wire>::read(value.title),
            icon: <String as Wire>::read(value.icon),
            kind: crate::wire_nav_kind(value.kind),
            enabled: <bool as Wire>::read(value.enabled),
        }
    }
}

impl Wire for crate::VisibleNavItem {
    type View = FfiVisibleNavItem;
    fn view(&self, arena: &mut Arena) -> Self::View {
        FfiVisibleNavItem {
            item: self.item.view(arena),
            depth: self.depth.view(arena),
            expanded: self.expanded.view(arena),
            has_children: self.has_children.view(arena),
        }
    }
    unsafe fn read(value: Self::View) -> Self {
        Self {
            item: <crate::NavItem as Wire>::read(value.item),
            depth: <usize as Wire>::read(value.depth),
            expanded: <bool as Wire>::read(value.expanded),
            has_children: <bool as Wire>::read(value.has_children),
        }
    }
}

impl Wire for crate::icons::IconGlyph {
    type View = FfiIconGlyph;
    fn view(&self, arena: &mut Arena) -> Self::View {
        FfiIconGlyph {
            name: self.name.view(arena),
            codepoint: self.codepoint.view(arena),
        }
    }
    unsafe fn read(value: Self::View) -> Self {
        Self {
            name: <String as Wire>::read(value.name),
            codepoint: <u32 as Wire>::read(value.codepoint),
        }
    }
}

impl Wire for crate::staggered_layout::StaggeredInput {
    type View = FfiStaggeredInput;
    fn view(&self, arena: &mut Arena) -> Self::View {
        FfiStaggeredInput {
            id: self.id.view(arena),
            height: self.height.view(arena),
        }
    }
    unsafe fn read(value: Self::View) -> Self {
        Self {
            id: <String as Wire>::read(value.id),
            height: <f32 as Wire>::read(value.height),
        }
    }
}

impl Wire for crate::staggered_layout::StaggeredPlacement {
    type View = FfiStaggeredPlacement;
    fn view(&self, arena: &mut Arena) -> Self::View {
        FfiStaggeredPlacement {
            id: self.id.view(arena),
            column: self.column.view(arena),
            top: self.top.view(arena),
        }
    }
    unsafe fn read(value: Self::View) -> Self {
        Self {
            id: <String as Wire>::read(value.id),
            column: <usize as Wire>::read(value.column),
            top: <f32 as Wire>::read(value.top),
        }
    }
}

impl Wire for crate::staggered_layout::StaggeredLayout {
    type View = FfiStaggeredLayout;
    fn view(&self, arena: &mut Arena) -> Self::View {
        FfiStaggeredLayout {
            placements: self.placements.view(arena),
            content_height: self.content_height.view(arena),
        }
    }
    unsafe fn read(value: Self::View) -> Self {
        Self {
            placements: <Vec<crate::staggered_layout::StaggeredPlacement> as Wire>::read(
                value.placements,
            ),
            content_height: <f32 as Wire>::read(value.content_height),
        }
    }
}

impl Wire for crate::grids::GridCellData {
    type View = FfiGridCellData;
    fn view(&self, arena: &mut Arena) -> Self::View {
        FfiGridCellData {
            text: self.text.view(arena),
            value: self.value.view(arena),
        }
    }
    unsafe fn read(value: Self::View) -> Self {
        Self {
            text: <String as Wire>::read(value.text),
            value: <f64 as Wire>::read(value.value),
        }
    }
}

impl Wire for crate::grids::GridColumnData {
    type View = FfiGridColumnData;
    fn view(&self, arena: &mut Arena) -> Self::View {
        FfiGridColumnData {
            id: self.id.view(arena),
            title: self.title.view(arena),
            width: self.width.view(arena),
            min_width: self.min_width.view(arena),
            max_width: self.max_width.view(arena),
            numeric: self.numeric.view(arena),
            sortable: self.sortable.view(arena),
            editable: self.editable.view(arena),
            hidden: self.hidden.view(arena),
            frozen: self.frozen.view(arena),
        }
    }
    unsafe fn read(value: Self::View) -> Self {
        Self {
            id: <String as Wire>::read(value.id),
            title: <String as Wire>::read(value.title),
            width: <f64 as Wire>::read(value.width),
            min_width: <f64 as Wire>::read(value.min_width),
            max_width: <f64 as Wire>::read(value.max_width),
            numeric: <bool as Wire>::read(value.numeric),
            sortable: <bool as Wire>::read(value.sortable),
            editable: <bool as Wire>::read(value.editable),
            hidden: <bool as Wire>::read(value.hidden),
            frozen: <bool as Wire>::read(value.frozen),
        }
    }
}

impl Wire for crate::grids::GridRowData {
    type View = FfiGridRowData;
    fn view(&self, arena: &mut Arena) -> Self::View {
        FfiGridRowData {
            id: self.id.view(arena),
            parent_id: self.parent_id.view(arena),
            cells: self.cells.view(arena),
            expanded: self.expanded.view(arena),
            selected: self.selected.view(arena),
            disabled: self.disabled.view(arena),
        }
    }
    unsafe fn read(value: Self::View) -> Self {
        Self {
            id: <String as Wire>::read(value.id),
            parent_id: <String as Wire>::read(value.parent_id),
            cells: <Vec<crate::grids::GridCellData> as Wire>::read(value.cells),
            expanded: <bool as Wire>::read(value.expanded),
            selected: <bool as Wire>::read(value.selected),
            disabled: <bool as Wire>::read(value.disabled),
        }
    }
}

impl Wire for crate::grids::GridViewColumnData {
    type View = FfiGridViewColumnData;
    fn view(&self, arena: &mut Arena) -> Self::View {
        FfiGridViewColumnData {
            column: self.column.view(arena),
            source_index: self.source_index.view(arena),
            x: self.x.view(arena),
            width: self.width.view(arena),
        }
    }
    unsafe fn read(value: Self::View) -> Self {
        Self {
            column: <crate::grids::GridColumnData as Wire>::read(value.column),
            source_index: <i32 as Wire>::read(value.source_index),
            x: <f64 as Wire>::read(value.x),
            width: <f64 as Wire>::read(value.width),
        }
    }
}

impl Wire for crate::grids::GridViewRowData {
    type View = FfiGridViewRowData;
    fn view(&self, arena: &mut Arena) -> Self::View {
        FfiGridViewRowData {
            row: self.row.view(arena),
            source_index: self.source_index.view(arena),
            depth: self.depth.view(arena),
            has_children: self.has_children.view(arena),
            expanded: self.expanded.view(arena),
        }
    }
    unsafe fn read(value: Self::View) -> Self {
        Self {
            row: <crate::grids::GridRowData as Wire>::read(value.row),
            source_index: <i32 as Wire>::read(value.source_index),
            depth: <i32 as Wire>::read(value.depth),
            has_children: <bool as Wire>::read(value.has_children),
            expanded: <bool as Wire>::read(value.expanded),
        }
    }
}

impl Wire for crate::grids::GridSceneData {
    type View = FfiGridSceneData;
    fn view(&self, arena: &mut Arena) -> Self::View {
        FfiGridSceneData {
            status: self.status.view(arena),
            rows: self.rows.view(arena),
            columns: self.columns.view(arena),
            total_width: self.total_width.view(arena),
            frozen_width: self.frozen_width.view(arena),
            selected_count: self.selected_count.view(arena),
            all_visible_selected: self.all_visible_selected.view(arena),
        }
    }
    unsafe fn read(value: Self::View) -> Self {
        Self {
            status: <i32 as Wire>::read(value.status),
            rows: <Vec<crate::grids::GridViewRowData> as Wire>::read(value.rows),
            columns: <Vec<crate::grids::GridViewColumnData> as Wire>::read(value.columns),
            total_width: <f64 as Wire>::read(value.total_width),
            frozen_width: <f64 as Wire>::read(value.frozen_width),
            selected_count: <i32 as Wire>::read(value.selected_count),
            all_visible_selected: <bool as Wire>::read(value.all_visible_selected),
        }
    }
}

impl Wire for crate::widgets::SyntaxLine {
    type View = FfiSyntaxLine;
    fn view(&self, arena: &mut Arena) -> Self::View {
        FfiSyntaxLine {
            number: self.number.view(arena),
            plain: self.plain.view(arena),
            markdown: self.markdown.view(arena),
        }
    }
    unsafe fn read(value: Self::View) -> Self {
        Self {
            number: <i32 as Wire>::read(value.number),
            plain: <String as Wire>::read(value.plain),
            markdown: <String as Wire>::read(value.markdown),
        }
    }
}

impl Wire for crate::capture::CaptureRect {
    type View = FfiCaptureRect;
    fn view(&self, arena: &mut Arena) -> Self::View {
        FfiCaptureRect {
            x: self.x.view(arena),
            y: self.y.view(arena),
            width: self.width.view(arena),
            height: self.height.view(arena),
        }
    }
    unsafe fn read(value: Self::View) -> Self {
        Self {
            x: <f32 as Wire>::read(value.x),
            y: <f32 as Wire>::read(value.y),
            width: <f32 as Wire>::read(value.width),
            height: <f32 as Wire>::read(value.height),
        }
    }
}

impl Wire for crate::RouterSnapshot {
    type View = FfiRouterSnapshot;
    fn view(&self, arena: &mut Arena) -> Self::View {
        FfiRouterSnapshot {
            handle: self.handle.view(arena),
            current: self.current.view(arena),
            history_len: self.history_len.view(arena),
            changed: self.changed.view(arena),
        }
    }
    unsafe fn read(value: Self::View) -> Self {
        Self {
            handle: <u64 as Wire>::read(value.handle),
            current: <crate::RouteState as Wire>::read(value.current),
            history_len: <usize as Wire>::read(value.history_len),
            changed: <bool as Wire>::read(value.changed),
        }
    }
}
