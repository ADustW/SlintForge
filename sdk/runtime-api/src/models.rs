// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

//! 自动生成的 C 数据视图；使用 tools/generate_runtime_ffi.py 更新。
use crate::{Optional, Pixels, Slice, Triple};

#[repr(C)]
#[derive(Clone, Copy)]
pub struct FfiRouteParam {
    pub key: Slice<u8>,
    pub value: Slice<u8>,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct FfiRouteState {
    pub id: Slice<u8>,
    pub params: Slice<FfiRouteParam>,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct FfiNavItem {
    pub id: Slice<u8>,
    pub parent_id: Optional<Slice<u8>>,
    pub title: Slice<u8>,
    pub icon: Slice<u8>,
    pub kind: u32,
    pub enabled: u8,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct FfiVisibleNavItem {
    pub item: FfiNavItem,
    pub depth: usize,
    pub expanded: u8,
    pub has_children: u8,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct FfiIconGlyph {
    pub name: Slice<u8>,
    pub codepoint: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct FfiStaggeredInput {
    pub id: Slice<u8>,
    pub height: f32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct FfiStaggeredPlacement {
    pub id: Slice<u8>,
    pub column: usize,
    pub top: f32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct FfiStaggeredLayout {
    pub placements: Slice<FfiStaggeredPlacement>,
    pub content_height: f32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct FfiGridCellData {
    pub text: Slice<u8>,
    pub value: f64,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct FfiGridColumnData {
    pub id: Slice<u8>,
    pub title: Slice<u8>,
    pub width: f64,
    pub min_width: f64,
    pub max_width: f64,
    pub numeric: u8,
    pub sortable: u8,
    pub editable: u8,
    pub hidden: u8,
    pub frozen: u8,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct FfiGridRowData {
    pub id: Slice<u8>,
    pub parent_id: Slice<u8>,
    pub cells: Slice<FfiGridCellData>,
    pub expanded: u8,
    pub selected: u8,
    pub disabled: u8,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct FfiGridViewColumnData {
    pub column: FfiGridColumnData,
    pub source_index: i32,
    pub x: f64,
    pub width: f64,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct FfiGridViewRowData {
    pub row: FfiGridRowData,
    pub source_index: i32,
    pub depth: i32,
    pub has_children: u8,
    pub expanded: u8,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct FfiGridSceneData {
    pub status: i32,
    pub rows: Slice<FfiGridViewRowData>,
    pub columns: Slice<FfiGridViewColumnData>,
    pub total_width: f64,
    pub frozen_width: f64,
    pub selected_count: i32,
    pub all_visible_selected: u8,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct FfiSyntaxLine {
    pub number: i32,
    pub plain: Slice<u8>,
    pub markdown: Slice<u8>,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct FfiCaptureRect {
    pub x: f32,
    pub y: f32,
    pub width: f32,
    pub height: f32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct FfiRouterSnapshot {
    pub handle: u64,
    pub current: FfiRouteState,
    pub history_len: usize,
    pub changed: u8,
}

pub const OP_ROUTERNEW: u32 = 1;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsRouterNew {
    pub ids: Slice<Slice<u8>>,
    pub initial: Slice<u8>,
}

pub const OP_ROUTERCLONE: u32 = 2;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsRouterClone {
    pub handle: u64,
}

pub const OP_ROUTERGO: u32 = 3;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsRouterGo {
    pub handle: u64,
    pub id: Slice<u8>,
    pub params: Slice<FfiRouteParam>,
}

pub const OP_ROUTERREPLACE: u32 = 4;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsRouterReplace {
    pub handle: u64,
    pub id: Slice<u8>,
    pub params: Slice<FfiRouteParam>,
}

pub const OP_ROUTERBACK: u32 = 5;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsRouterBack {
    pub handle: u64,
}

pub const OP_ROUTERFREE: u32 = 6;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsRouterFree {
    pub handle: u64,
}

pub const OP_NAVNEW: u32 = 7;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsNavNew {
    pub items: Slice<FfiNavItem>,
}

pub const OP_NAVCLONE: u32 = 8;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsNavClone {
    pub handle: u64,
}

pub const OP_NAVTOGGLE: u32 = 9;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsNavToggle {
    pub handle: u64,
    pub id: Slice<u8>,
}

pub const OP_NAVEXPAND: u32 = 10;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsNavExpand {
    pub handle: u64,
    pub id: Slice<u8>,
    pub expanded: u8,
}

pub const OP_NAVVISIBLE: u32 = 11;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsNavVisible {
    pub handle: u64,
}

pub const OP_NAVFREE: u32 = 12;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsNavFree {
    pub handle: u64,
}

pub const OP_ICONSALL: u32 = 13;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsIconsAll {
    pub reserved: u8,
}

pub const OP_ICONSFIND: u32 = 14;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsIconsFind {
    pub name: Slice<u8>,
}

pub const OP_ICONSSEARCH: u32 = 15;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsIconsSearch {
    pub query: Slice<u8>,
    pub offset: usize,
    pub limit: usize,
}

pub const OP_ARRANGE: u32 = 16;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsArrange {
    pub items: Slice<FfiStaggeredInput>,
    pub columns: usize,
    pub gap: f32,
    pub padding: f32,
}

pub const OP_COLORPARSE: u32 = 17;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsColorParse {
    pub text: Slice<u8>,
}

pub const OP_COLORTEXT: u32 = 18;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsColorText {
    pub color: u32,
}

pub const OP_SCREENCOLOR: u32 = 19;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsScreenColor {
    pub reserved: u8,
}

pub const OP_SCREENINPUT: u32 = 20;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsScreenInput {
    pub reserved: u8,
}

pub const OP_RESERVED21: u32 = 21;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsReserved21 {
    pub reserved: u8,
}

pub const OP_RESERVED22: u32 = 22;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsReserved22 {
    pub reserved: u8,
}

pub const OP_RESERVED23: u32 = 23;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsReserved23 {
    pub reserved: u8,
}

pub const OP_GRIDPREPARE: u32 = 24;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsGridPrepare {
    pub rows: Slice<FfiGridRowData>,
    pub columns: Slice<FfiGridColumnData>,
    pub tree: u8,
    pub sort_column: i32,
    pub descending: u8,
    pub filter: Slice<u8>,
}

pub const OP_GRIDUPDATE: u32 = 25;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsGridUpdate {
    pub rows: Slice<FfiGridRowData>,
    pub visible: Slice<i32>,
    pub operation: i32,
    pub target: i32,
    pub anchor: i32,
}

pub const OP_GRIDCOMMIT: u32 = 26;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsGridCommit {
    pub rows: Slice<FfiGridRowData>,
    pub columns: Slice<FfiGridColumnData>,
    pub row: i32,
    pub column: i32,
    pub text: Slice<u8>,
}

pub const OP_GRIDSTEP: u32 = 27;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsGridStep {
    pub disabled: Slice<u8>,
    pub current: i32,
    pub delta: i32,
}

pub const OP_QR: u32 = 28;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsQr {
    pub text: Slice<u8>,
}

pub const OP_HIGHLIGHT: u32 = 29;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsHighlight {
    pub code: Slice<u8>,
    pub language: Slice<u8>,
}

pub const OP_BACKDROP: u32 = 30;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsBackdrop {
    pub pixels: Pixels,
    pub radius: i32,
    pub glass: u8,
}

pub const OP_CROP: u32 = 31;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsCrop {
    pub pixels: Pixels,
    pub rect: FfiCaptureRect,
    pub scale: f32,
}

pub const OP_SAVEPNG: u32 = 32;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsSavePng {
    pub pixels: Pixels,
    pub path: Slice<u16>,
}

pub const OP_DECODEIMAGE: u32 = 33;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsDecodeImage {
    pub bytes: Slice<u8>,
}

pub const OP_SAVEIMAGE: u32 = 34;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsSaveImage {
    pub pixels: Pixels,
    pub path: Slice<u16>,
    pub format: i32,
}

pub const OP_MERGESELECTION: u32 = 35;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsMergeSelection {
    pub current: Slice<Triple<i32, i32, i32>>,
    pub incoming: Slice<Triple<i32, i32, i32>>,
    pub mode: i32,
}

pub const OP_FLOWNEW: u32 = 36;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsFlowNew {
    pub mode: Slice<u8>,
}

pub const OP_FLOWCALL: u32 = 37;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsFlowCall {
    pub handle: u64,
    pub command: Slice<u8>,
}

pub const OP_FLOWSNAPSHOT: u32 = 38;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsFlowSnapshot {
    pub handle: u64,
}

pub const OP_FLOWEXPORT: u32 = 39;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsFlowExport {
    pub handle: u64,
}

pub const OP_FLOWFREE: u32 = 40;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct ArgsFlowFree {
    pub handle: u64,
}
