// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

//! SlintForge 的 Rust 宿主模型。视觉组件由 ui/slintforge.slint 统一定义。

pub mod flow;
mod flow_bindings;
mod flow_export;
pub use flow_export::save_flow_image;
mod icons;
pub mod runtime;
mod wire;
pub use icons::{IconCatalog, IconGlyph};
mod window_corners;
pub use window_corners::apply_window_corners;
mod rendering;
pub use rendering::{configure_rendering, RenderConfiguration, RenderOptions, RenderPreference};
mod staggered_layout;
pub use staggered_layout::{StaggeredInput, StaggeredLayout, StaggeredPlacement};
mod capture;
mod color;
mod widgets;
pub use color::{color_from_hex, color_to_hex, screen_color, screen_color_input};
pub mod grids;
pub mod plot;
mod plot_bindings;
pub use capture::{
    capture_region, capture_window, crop_capture, save_capture_png, CaptureError, CaptureRect,
};
pub use widgets::{highlight_syntax, process_backdrop, qr_code_pixels, SyntaxLine};

use std::path::PathBuf;

#[derive(Clone, Debug, PartialEq, Eq)]
pub enum Error {
    EmptyId,
    DuplicateId(String),
    UnknownRoute(String),
    UnknownParent(String),
    Cycle(String),
    InvalidParent(String),
    NotGroup(String),
    NoHistory,
    /// 运行库加载、ABI 校验或调用本身失败（不属于路由/导航规则错误）。
    Runtime { code: i32, message: String },
}

impl std::fmt::Display for Error {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::EmptyId => write!(f, "路由或导航 ID 不能为空"),
            Self::DuplicateId(id) => write!(f, "重复 ID: {id}"),
            Self::UnknownRoute(id) => write!(f, "未知路由: {id}"),
            Self::UnknownParent(id) => write!(f, "未知父节点: {id}"),
            Self::Cycle(id) => write!(f, "导航树存在环: {id}"),
            Self::InvalidParent(id) => write!(f, "父节点不是分组: {id}"),
            Self::NotGroup(id) => write!(f, "节点不是分组: {id}"),
            Self::NoHistory => write!(f, "没有可返回的历史页面"),
            Self::Runtime { code, message } => write!(f, "组件运行库调用失败（{code}）：{message}"),
        }
    }
}

impl std::error::Error for Error {}

pub type Result<T> = std::result::Result<T, Error>;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct RouteParam {
    pub key: String,
    pub value: String,
}

impl RouteParam {
    pub fn new(key: impl Into<String>, value: impl Into<String>) -> Self {
        Self {
            key: key.into(),
            value: value.into(),
        }
    }
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct RouteState {
    pub id: String,
    pub params: Vec<RouteParam>,
}

impl RouteState {
    pub fn new(id: impl Into<String>, params: Vec<RouteParam>) -> Self {
        Self {
            id: id.into(),
            params,
        }
    }
}

#[derive(Debug)]
pub struct Router {
    handle: u64,
    current: RouteState,
    history_len: usize,
}
impl Router {
    pub fn new(ids: impl IntoIterator<Item = impl Into<String>>, initial: &str) -> Result<Self> {
        use slintforge_runtime_api::Wire;
        let ids = ids.into_iter().map(Into::into).collect::<Vec<String>>();
        let initial = initial.to_owned();
        let mut arena = slintforge_runtime_api::Arena::default();
        let args = slintforge_runtime_api::models::ArgsRouterNew {
            ids: ids.view(&mut arena),
            initial: initial.view(&mut arena),
        };
        let state = runtime::try_value::<RouterSnapshot, _>(
            slintforge_runtime_api::models::OP_ROUTERNEW,
            &args,
        )
        .map_err(route_runtime_error)?;
        Ok(Self {
            handle: state.handle,
            current: state.current,
            history_len: state.history_len,
        })
    }
    fn update(&mut self, state: RouterSnapshot) -> bool {
        self.current = state.current;
        self.history_len = state.history_len;
        state.changed
    }
    pub fn go(&mut self, id: &str, params: Vec<RouteParam>) -> Result<bool> {
        self.navigate(id, params, false)
    }
    pub fn replace(&mut self, id: &str, params: Vec<RouteParam>) -> Result<bool> {
        self.navigate(id, params, true)
    }
    fn navigate(&mut self, id: &str, params: Vec<RouteParam>, replace: bool) -> Result<bool> {
        use slintforge_runtime_api::{models::*, Wire};
        let id = id.to_owned();
        let mut arena = slintforge_runtime_api::Arena::default();
        let args = ArgsRouterGo {
            handle: self.handle,
            id: id.view(&mut arena),
            params: params.view(&mut arena),
        };
        let state = runtime::try_value::<RouterSnapshot, _>(
            if replace {
                OP_ROUTERREPLACE
            } else {
                OP_ROUTERGO
            },
            &args,
        )
        .map_err(route_runtime_error)?;
        Ok(self.update(state))
    }
    pub fn back(&mut self) -> Result<&RouteState> {
        use slintforge_runtime_api::models::*;
        let state = runtime::try_value::<RouterSnapshot, _>(
            OP_ROUTERBACK,
            &ArgsRouterBack {
                handle: self.handle,
            },
        )
        .map_err(route_runtime_error)?;
        self.update(state);
        Ok(&self.current)
    }
    pub fn current(&self) -> &RouteState {
        &self.current
    }
    pub fn can_go_back(&self) -> bool {
        self.history_len != 0
    }
    pub fn history_len(&self) -> usize {
        self.history_len
    }
}
impl Clone for Router {
    fn clone(&self) -> Self {
        use slintforge_runtime_api::models::*;
        let state = runtime::value::<RouterSnapshot, _>(
            OP_ROUTERCLONE,
            &ArgsRouterClone {
                handle: self.handle,
            },
        );
        Self {
            handle: state.handle,
            current: state.current,
            history_len: state.history_len,
        }
    }
}
impl Drop for Router {
    fn drop(&mut self) {
        runtime::release_object(slintforge_runtime_api::models::OP_ROUTERFREE, self.handle);
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum NavItemKind {
    Page,
    Group,
    Header,
    Separator,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct NavItem {
    pub id: String,
    pub parent_id: Option<String>,
    pub title: String,
    pub icon: String,
    pub kind: NavItemKind,
    pub enabled: bool,
}

impl NavItem {
    pub fn new(
        id: &str,
        parent_id: Option<&str>,
        title: &str,
        icon: &str,
        kind: NavItemKind,
    ) -> Self {
        Self {
            id: id.into(),
            parent_id: parent_id.map(str::to_owned),
            title: title.into(),
            icon: icon.into(),
            kind,
            enabled: true,
        }
    }
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct VisibleNavItem {
    pub item: NavItem,
    pub depth: usize,
    pub expanded: bool,
    pub has_children: bool,
}

#[derive(Debug)]
pub struct NavTree {
    handle: u64,
}
impl NavTree {
    pub fn new(items: Vec<NavItem>) -> Result<Self> {
        use slintforge_runtime_api::{models::*, Wire};
        let mut arena = slintforge_runtime_api::Arena::default();
        let args = ArgsNavNew {
            items: items.view(&mut arena),
        };
        runtime::try_value::<u64, _>(OP_NAVNEW, &args)
            .map(|handle| Self { handle })
            .map_err(route_runtime_error)
    }
    pub fn toggle(&mut self, id: &str) -> Result<bool> {
        use slintforge_runtime_api::{models::*, Wire};
        let id = id.to_owned();
        let mut arena = slintforge_runtime_api::Arena::default();
        runtime::try_value::<bool, _>(
            OP_NAVTOGGLE,
            &ArgsNavToggle {
                handle: self.handle,
                id: id.view(&mut arena),
            },
        )
        .map_err(route_runtime_error)
    }
    pub fn set_expanded(&mut self, id: &str, expanded: bool) -> Result<()> {
        use slintforge_runtime_api::{models::*, Wire};
        let id = id.to_owned();
        let mut arena = slintforge_runtime_api::Arena::default();
        runtime::try_value::<u8, _>(
            OP_NAVEXPAND,
            &ArgsNavExpand {
                handle: self.handle,
                id: id.view(&mut arena),
                expanded: u8::from(expanded),
            },
        )
        .map(|_| ())
        .map_err(route_runtime_error)
    }
    pub fn visible_items(&self) -> Vec<VisibleNavItem> {
        use slintforge_runtime_api::models::*;
        runtime::value_or(
            OP_NAVVISIBLE,
            &ArgsNavVisible {
                handle: self.handle,
            },
            Vec::new,
        )
    }
}
impl Clone for NavTree {
    fn clone(&self) -> Self {
        use slintforge_runtime_api::models::*;
        Self {
            handle: runtime::value(
                OP_NAVCLONE,
                &ArgsNavClone {
                    handle: self.handle,
                },
            ),
        }
    }
}
impl Drop for NavTree {
    fn drop(&mut self) {
        runtime::release_object(slintforge_runtime_api::models::OP_NAVFREE, self.handle);
    }
}

fn route_runtime_error(error: runtime::RuntimeError) -> Error {
    let (code, text) = error.operation_error();
    match code {
        1 => Error::EmptyId,
        2 => Error::DuplicateId(text),
        3 => Error::UnknownRoute(text),
        4 => Error::UnknownParent(text),
        5 => Error::Cycle(text),
        6 => Error::InvalidParent(text),
        7 => Error::NotGroup(text),
        8 => Error::NoHistory,
        _ => Error::Runtime {
            code,
            message: text,
        },
    }
}
fn wire_color(color: slint::Color) -> u32 {
    u32::from_be_bytes([color.alpha(), color.red(), color.green(), color.blue()])
}
fn wire_color_from(color: u32) -> slint::Color {
    let [a, r, g, b] = color.to_be_bytes();
    slint::Color::from_argb_u8(a, r, g, b)
}
fn wire_nav_kind(value: u32) -> NavItemKind {
    match value {
        0 => NavItemKind::Page,
        1 => NavItemKind::Group,
        2 => NavItemKind::Header,
        3 => NavItemKind::Separator,
        _ => panic!("导航类型 ABI 无效"),
    }
}
fn pixels_view(
    pixels: &slint::SharedPixelBuffer<slint::Rgba8Pixel>,
) -> slintforge_runtime_api::Pixels {
    slintforge_runtime_api::Pixels {
        width: pixels.width(),
        height: pixels.height(),
        bytes: slintforge_runtime_api::Slice::new(
            pixels.as_bytes().as_ptr(),
            pixels.as_bytes().len(),
        ),
    }
}
fn pixels_from_view(
    value: slintforge_runtime_api::Pixels,
) -> slint::SharedPixelBuffer<slint::Rgba8Pixel> {
    let count = (value.width as usize)
        .checked_mul(value.height as usize)
        .and_then(|n| n.checked_mul(4))
        .expect("像素尺寸溢出");
    assert_eq!(count, value.bytes.len, "像素返回长度错误");
    let bytes = unsafe { value.bytes.read() };
    let mut out = slint::SharedPixelBuffer::new(value.width, value.height);
    for (p, b) in out.make_mut_slice().iter_mut().zip(bytes.chunks_exact(4)) {
        *p = slint::Rgba8Pixel {
            r: b[0],
            g: b[1],
            b: b[2],
            a: b[3],
        };
    }
    out
}
fn decode_image_pixels(
    bytes: &[u8],
) -> std::result::Result<slint::SharedPixelBuffer<slint::Rgba8Pixel>, String> {
    use slintforge_runtime_api::{models::*, Pixels, Slice};
    let args = ArgsDecodeImage {
        bytes: Slice::new(bytes.as_ptr(), bytes.len()),
    };
    runtime::invoke::<Pixels, _, _>(OP_DECODEIMAGE, &args, pixels_from_view)
        .map_err(|e| e.message())
}
fn save_image_pixels(
    pixels: &slint::SharedPixelBuffer<slint::Rgba8Pixel>,
    path: &std::path::Path,
    format: i32,
) -> std::result::Result<(), String> {
    #[cfg(windows)]
    let wide: Vec<u16> = {
        use std::os::windows::ffi::OsStrExt;
        path.as_os_str().encode_wide().collect()
    };
    #[cfg(not(windows))]
    let wide: Vec<u16> = path.to_string_lossy().encode_utf16().collect();
    use slintforge_runtime_api::{models::*, Slice};
    let args = ArgsSaveImage {
        pixels: pixels_view(pixels),
        path: Slice::new(wide.as_ptr(), wide.len()),
        format,
    };
    runtime::invoke::<u8, _, _>(OP_SAVEIMAGE, &args, |_| ()).map_err(|e| e.message())
}
#[derive(Clone, Debug)]
pub(crate) struct RouterSnapshot {
    pub handle: u64,
    pub current: RouteState,
    pub history_len: usize,
    pub changed: bool,
}
/// build.rs 中传给 `CompilerConfiguration::with_library_paths` 的入口。
pub fn ui_library_path() -> PathBuf {
    slintforge_build::ui_library_path()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn runtime_failures_become_errors_instead_of_panics() {
        let missing = route_runtime_error(runtime::RuntimeError::Missing { paths: vec![] });
        assert!(matches!(missing, Error::Runtime { code: 1000, .. }), "{missing:?}");
        let invalid = route_runtime_error(runtime::RuntimeError::Call {
            code: 1003,
            message: "参数无效".into(),
        });
        assert_eq!(
            invalid,
            Error::Runtime {
                code: 1003,
                message: "参数无效".into()
            }
        );
        assert_eq!(
            route_runtime_error(runtime::RuntimeError::Call {
                code: 8,
                message: String::new()
            }),
            Error::NoHistory
        );
    }

    #[test]
    fn router_keeps_state_on_error_and_deduplicates() {
        let mut r = Router::new(["home", "detail"], "home").unwrap();
        assert!(!r.go("home", vec![]).unwrap());
        assert_eq!(r.history_len(), 0);
        assert_eq!(
            r.go("missing", vec![]),
            Err(Error::UnknownRoute("missing".into()))
        );
        assert_eq!(r.current().id, "home");
        r.go("detail", vec![RouteParam::new("id", "7")]).unwrap();
        assert!(!r.go("detail", vec![RouteParam::new("id", "7")]).unwrap());
        assert_eq!(r.history_len(), 1);
        assert_eq!(r.back().unwrap().id, "home");
        assert_eq!(r.back(), Err(Error::NoHistory));
        r.go("detail", vec![RouteParam::new("id", "8")]).unwrap();
        r.replace("home", vec![]).unwrap();
        assert_eq!(r.history_len(), 1);
        assert_eq!(r.current().id, "home");
        assert_eq!(r.go("", vec![]), Err(Error::EmptyId));
        assert_eq!(r.current().id, "home");
        assert!(matches!(
            Router::new(["home", "home"], "home"),
            Err(Error::DuplicateId(_))
        ));
    }

    #[test]
    fn tree_expands_multilevel_and_rejects_invalid_shapes() {
        let items = vec![
            NavItem::new("root", None, "Root", "R", NavItemKind::Group),
            NavItem::new("sub", Some("root"), "Sub", "S", NavItemKind::Group),
            NavItem::new("leaf", Some("sub"), "Leaf", "L", NavItemKind::Page),
        ];
        let mut tree = NavTree::new(items).unwrap();
        assert_eq!(tree.visible_items().len(), 1);
        tree.toggle("root").unwrap();
        assert_eq!(tree.visible_items().len(), 2);
        tree.toggle("sub").unwrap();
        assert_eq!(tree.visible_items()[2].depth, 2);
        assert!(matches!(
            NavTree::new(vec![NavItem::new(
                "x",
                Some("x"),
                "X",
                "",
                NavItemKind::Group
            )]),
            Err(Error::Cycle(_))
        ));
        assert!(matches!(
            NavTree::new(vec![
                NavItem::new("x", None, "X", "", NavItemKind::Page),
                NavItem::new("x", None, "X", "", NavItemKind::Page)
            ]),
            Err(Error::DuplicateId(_))
        ));
        assert!(matches!(
            NavTree::new(vec![NavItem::new(
                "child",
                Some("absent"),
                "X",
                "",
                NavItemKind::Page
            )]),
            Err(Error::UnknownParent(_))
        ));
        assert!(matches!(
            NavTree::new(vec![
                NavItem::new("parent", None, "P", "", NavItemKind::Page),
                NavItem::new("child", Some("parent"), "C", "", NavItemKind::Page)
            ]),
            Err(Error::InvalidParent(_))
        ));
        assert_eq!(tree.toggle("leaf"), Err(Error::NotGroup("leaf".into())));
        assert_eq!(tree.visible_items().len(), 3);
    }
}
