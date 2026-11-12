// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

//! SlintForge 纯计算运行库。GUI 对象只存在于客户应用。
mod models;
pub use models::*;
pub mod capture;
pub mod color;
mod flow_native;
pub mod grids;
pub mod icons;
mod pixels;
mod plot_helpers;
mod plot_native;
pub mod staggered_layout;
pub mod widgets;
mod wire;
use slintforge_runtime_api::models::*;
use slintforge_runtime_api::{Arena, CallResult, RuntimeApiV2, Wire, ABI_SCHEMA, ABI_VERSION};
use std::collections::HashMap;
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::{Mutex, OnceLock};

fn wire_color(color: pixels::Color) -> u32 {
    u32::from_be_bytes([color.alpha(), color.red(), color.green(), color.blue()])
}
fn wire_color_from(value: u32) -> pixels::Color {
    let [a, r, g, b] = value.to_be_bytes();
    pixels::Color::from_argb_u8(a, r, g, b)
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

enum Object {
    Flow(flow_native::FlowNative),
    Router(Router),
    Nav(NavTree),
}
static OBJECTS: OnceLock<Mutex<HashMap<u64, Object>>> = OnceLock::new();
static NEXT_ID: AtomicU64 = AtomicU64::new(1);
struct OutputArena {
    _storage: Arena,
}
// 输出池只装入纯计算数据和指向池内分配的只读 C 视图；无 GUI/线程局部对象。
unsafe impl Send for OutputArena {}
static OUTPUTS: OnceLock<Mutex<HashMap<u64, Box<OutputArena>>>> = OnceLock::new();
fn objects() -> &'static Mutex<HashMap<u64, Object>> {
    OBJECTS.get_or_init(|| Mutex::new(HashMap::new()))
}
fn outputs() -> &'static Mutex<HashMap<u64, Box<OutputArena>>> {
    OUTPUTS.get_or_init(|| Mutex::new(HashMap::new()))
}
fn store(object: Object) -> u64 {
    let id = NEXT_ID.fetch_add(1, Ordering::Relaxed);
    objects()
        .lock()
        .unwrap_or_else(|e| e.into_inner())
        .insert(id, object);
    id
}
fn own<T: Wire + 'static>(value: T) -> CallResult {
    let mut arena = Arena::default();
    let source = arena.hold(value);
    let view = unsafe { (&*source).view(&mut arena) };
    let pointer = arena.hold(view).cast();
    let id = NEXT_ID.fetch_add(1, Ordering::Relaxed);
    outputs()
        .lock()
        .unwrap_or_else(|e| e.into_inner())
        .insert(id, Box::new(OutputArena { _storage: arena }));
    CallResult {
        owner: id,
        value: pointer,
        ..CallResult::default()
    }
}
struct Failure {
    code: i32,
    message: String,
}
impl From<Error> for Failure {
    fn from(error: Error) -> Self {
        let (code, message) = match error {
            Error::EmptyId => (1, String::new()),
            Error::DuplicateId(s) => (2, s),
            Error::UnknownRoute(s) => (3, s),
            Error::UnknownParent(s) => (4, s),
            Error::Cycle(s) => (5, s),
            Error::InvalidParent(s) => (6, s),
            Error::NotGroup(s) => (7, s),
            Error::NoHistory => (8, String::new()),
        };
        Self { code, message }
    }
}
fn failure(code: i32, message: impl Into<String>) -> Failure {
    Failure {
        code,
        message: message.into(),
    }
}
fn router_snapshot(handle: u64, router: &Router, changed: bool) -> RouterSnapshot {
    RouterSnapshot {
        handle,
        current: router.current().clone(),
        history_len: router.history_len(),
        changed,
    }
}
unsafe fn arg<'a, T>(pointer: *const std::ffi::c_void) -> std::result::Result<&'a T, Failure> {
    if pointer.is_null() || (pointer as usize) % std::mem::align_of::<T>() != 0 {
        return Err(failure(1003, "ABI 参数为空或未对齐"));
    }
    Ok(&*pointer.cast::<T>())
}
unsafe fn text(view: slintforge_runtime_api::Slice<u8>) -> String {
    String::read(view)
}
fn icons_catalog() -> &'static icons::IconCatalog {
    static ICONS: OnceLock<icons::IconCatalog> = OnceLock::new();
    ICONS.get_or_init(icons::IconCatalog::new)
}
unsafe fn native_path(path: slintforge_runtime_api::Slice<u16>) -> std::path::PathBuf {
    #[cfg(windows)]
    {
        use std::os::windows::ffi::OsStringExt;
        std::path::PathBuf::from(std::ffi::OsString::from_wide(path.read()))
    }
    #[cfg(not(windows))]
    {
        std::path::PathBuf::from(String::from_utf16_lossy(path.read()))
    }
}

unsafe fn dispatch(
    operation: u32,
    input: *const std::ffi::c_void,
) -> std::result::Result<CallResult, Failure> {
    Ok(match operation {
        OP_FLOWNEW => {
            let a = arg::<ArgsFlowNew>(input)?;
            let document =
                flow_native::FlowNative::new(&text(a.mode)).map_err(|e| failure(1100, e))?;
            own(store(Object::Flow(document)))
        }
        OP_FLOWCALL | OP_FLOWSNAPSHOT | OP_FLOWEXPORT | OP_FLOWFREE => {
            let handle = arg::<ArgsFlowSnapshot>(input)?.handle;
            let mut all = objects().lock().unwrap_or_else(|e| e.into_inner());
            let Some(Object::Flow(document)) = all.get_mut(&handle) else {
                return Err(failure(1101, "图文档句柄无效"));
            };
            if operation == OP_FLOWFREE {
                all.remove(&handle);
                own(true)
            } else if operation == OP_FLOWCALL {
                let a = arg::<ArgsFlowCall>(input)?;
                own(document
                    .execute(&text(a.command))
                    .map_err(|e| failure(1102, e))?)
            } else if operation == OP_FLOWSNAPSHOT {
                own(document.snapshot())
            } else {
                own(document.export())
            }
        }
        OP_ROUTERNEW => {
            let a = arg::<ArgsRouterNew>(input)?;
            let ids = Vec::<String>::read(a.ids);
            let initial = text(a.initial);
            let router = Router::new(ids, &initial)?;
            let id = store(Object::Router(router));
            let all = objects().lock().unwrap_or_else(|e| e.into_inner());
            if let Object::Router(r) = all.get(&id).unwrap() {
                own(router_snapshot(id, r, false))
            } else {
                unreachable!()
            }
        }
        OP_ROUTERCLONE => {
            let a = arg::<ArgsRouterClone>(input)?;
            let cloned = {
                let all = objects().lock().unwrap_or_else(|e| e.into_inner());
                match all.get(&a.handle) {
                    Some(Object::Router(r)) => r.clone(),
                    _ => return Err(failure(1001, "路由句柄无效")),
                }
            };
            let state = router_snapshot(0, &cloned, false);
            let handle = store(Object::Router(cloned));
            own(RouterSnapshot { handle, ..state })
        }
        OP_ROUTERGO | OP_ROUTERREPLACE => {
            let a = arg::<ArgsRouterGo>(input)?;
            let id = text(a.id);
            let params = Vec::<RouteParam>::read(a.params);
            let mut all = objects().lock().unwrap_or_else(|e| e.into_inner());
            let Some(Object::Router(r)) = all.get_mut(&a.handle) else {
                return Err(failure(1001, "路由句柄无效"));
            };
            let changed = if operation == OP_ROUTERGO {
                r.go(&id, params)?
            } else {
                r.replace(&id, params)?
            };
            own(router_snapshot(a.handle, r, changed))
        }
        OP_ROUTERBACK => {
            let a = arg::<ArgsRouterBack>(input)?;
            let mut all = objects().lock().unwrap_or_else(|e| e.into_inner());
            let Some(Object::Router(r)) = all.get_mut(&a.handle) else {
                return Err(failure(1001, "路由句柄无效"));
            };
            r.back()?;
            own(router_snapshot(a.handle, r, true))
        }
        OP_ROUTERFREE | OP_NAVFREE => {
            let handle = *arg::<u64>(input)?;
            let mut all = objects().lock().unwrap_or_else(|e| e.into_inner());
            let correct = matches!(
                (operation, all.get(&handle)),
                (OP_ROUTERFREE, Some(Object::Router(_))) | (OP_NAVFREE, Some(Object::Nav(_)))
            );
            if !correct {
                return Err(failure(1001, "对象句柄无效或类型不匹配"));
            }
            all.remove(&handle);
            own(0u8)
        }
        OP_NAVNEW => {
            let a = arg::<ArgsNavNew>(input)?;
            let nav = NavTree::new(Vec::<NavItem>::read(a.items))?;
            own(store(Object::Nav(nav)))
        }
        OP_NAVCLONE => {
            let a = arg::<ArgsNavClone>(input)?;
            let cloned = {
                let all = objects().lock().unwrap_or_else(|e| e.into_inner());
                match all.get(&a.handle) {
                    Some(Object::Nav(n)) => n.clone(),
                    _ => return Err(failure(1001, "导航句柄无效")),
                }
            };
            own(store(Object::Nav(cloned)))
        }
        OP_NAVTOGGLE => {
            let a = arg::<ArgsNavToggle>(input)?;
            let id = text(a.id);
            let mut all = objects().lock().unwrap_or_else(|e| e.into_inner());
            let Some(Object::Nav(n)) = all.get_mut(&a.handle) else {
                return Err(failure(1001, "导航句柄无效"));
            };
            own(n.toggle(&id)?)
        }
        OP_NAVEXPAND => {
            let a = arg::<ArgsNavExpand>(input)?;
            let id = text(a.id);
            let mut all = objects().lock().unwrap_or_else(|e| e.into_inner());
            let Some(Object::Nav(n)) = all.get_mut(&a.handle) else {
                return Err(failure(1001, "导航句柄无效"));
            };
            n.set_expanded(&id, bool::read(a.expanded))?;
            own(0u8)
        }
        OP_NAVVISIBLE => {
            let a = arg::<ArgsNavVisible>(input)?;
            let all = objects().lock().unwrap_or_else(|e| e.into_inner());
            let Some(Object::Nav(n)) = all.get(&a.handle) else {
                return Err(failure(1001, "导航句柄无效"));
            };
            own(n.visible_items())
        }
        OP_ICONSALL => own(icons_catalog().search("", 0, icons_catalog().len()).0),
        OP_ICONSFIND => {
            let a = arg::<ArgsIconsFind>(input)?;
            own(icons_catalog().find_index(&text(a.name)))
        }
        OP_ICONSSEARCH => {
            let a = arg::<ArgsIconsSearch>(input)?;
            own(icons_catalog().search(&text(a.query), a.offset, a.limit))
        }
        OP_ARRANGE => {
            let a = arg::<ArgsArrange>(input)?;
            own(staggered_layout::StaggeredLayout::arrange(
                &Vec::<staggered_layout::StaggeredInput>::read(a.items),
                a.columns,
                a.gap,
                a.padding,
            ))
        }
        OP_COLORPARSE => {
            let a = arg::<ArgsColorParse>(input)?;
            own(color::color_from_hex(&text(a.text)).map(wire_color))
        }
        OP_COLORTEXT => {
            let a = arg::<ArgsColorText>(input)?;
            own(color::color_to_hex(wire_color_from(a.color)))
        }
        OP_SCREENCOLOR => own(wire_color(color::screen_color())),
        OP_SCREENINPUT => own(color::screen_color_input()),
        OP_GRIDPREPARE => {
            let a = arg::<ArgsGridPrepare>(input)?;
            own(grids::prepare_grid(
                &Vec::<grids::GridRowData>::read(a.rows),
                &Vec::<grids::GridColumnData>::read(a.columns),
                bool::read(a.tree),
                a.sort_column,
                bool::read(a.descending),
                &text(a.filter),
            ))
        }
        OP_GRIDUPDATE => {
            let a = arg::<ArgsGridUpdate>(input)?;
            own(grids::update_grid(
                &Vec::<grids::GridRowData>::read(a.rows),
                &Vec::<i32>::read(a.visible),
                a.operation,
                a.target,
                a.anchor,
            ))
        }
        OP_GRIDCOMMIT => {
            let a = arg::<ArgsGridCommit>(input)?;
            own(grids::commit_grid(
                &Vec::<grids::GridRowData>::read(a.rows),
                &Vec::<grids::GridColumnData>::read(a.columns),
                a.row,
                a.column,
                &text(a.text),
            ))
        }
        OP_GRIDSTEP => {
            let a = arg::<ArgsGridStep>(input)?;
            own(grids::step_grid(
                &Vec::<bool>::read(a.disabled),
                a.current,
                a.delta,
            ))
        }
        OP_QR => {
            let a = arg::<ArgsQr>(input)?;
            own(widgets::qr_code_pixels(&text(a.text)).map_err(|e| failure(2001, e))?)
        }
        OP_HIGHLIGHT => {
            let a = arg::<ArgsHighlight>(input)?;
            own(widgets::highlight_syntax(&text(a.code), &text(a.language)))
        }
        OP_BACKDROP => {
            let a = arg::<ArgsBackdrop>(input)?;
            own(widgets::process_backdrop(
                &pixels::SharedPixelBuffer::read(a.pixels),
                a.radius,
                bool::read(a.glass),
            ))
        }
        OP_CROP => {
            let a = arg::<ArgsCrop>(input)?;
            own(capture::crop_capture(
                &pixels::SharedPixelBuffer::read(a.pixels),
                capture::CaptureRect::read(a.rect),
                a.scale,
            )
            .map_err(|e| failure(2002, e.0))?)
        }
        OP_SAVEPNG => {
            let a = arg::<ArgsSavePng>(input)?;
            #[cfg(windows)]
            let path = {
                use std::os::windows::ffi::OsStringExt;
                std::path::PathBuf::from(std::ffi::OsString::from_wide(a.path.read()))
            };
            #[cfg(not(windows))]
            let path = std::path::PathBuf::from(
                String::from_utf16(a.path.read()).map_err(|e| failure(1003, e.to_string()))?,
            );
            capture::save_capture_png(&pixels::SharedPixelBuffer::read(a.pixels), path)
                .map_err(|e| failure(2002, e.0))?;
            own(0u8)
        }
        OP_DECODEIMAGE => {
            let a = arg::<ArgsDecodeImage>(input)?;
            let decoded = image::load_from_memory(a.bytes.read())
                .map_err(|e| failure(2003, e.to_string()))?
                .to_rgba8();
            let (width, height) = decoded.dimensions();
            let bytes = decoded.into_raw();
            let view = slintforge_runtime_api::Pixels {
                width,
                height,
                bytes: slintforge_runtime_api::Slice::new(bytes.as_ptr(), bytes.len()),
            };
            own(pixels::SharedPixelBuffer::read(view))
        }
        OP_SAVEIMAGE => {
            let a = arg::<ArgsSaveImage>(input)?;
            let pixels = pixels::SharedPixelBuffer::read(a.pixels);
            let rgb = pixels
                .as_slice()
                .iter()
                .flat_map(|p| {
                    let blend = |c: u8| {
                        ((c as u32 * p.a as u32 + 255 * (255 - p.a as u32) + 127) / 255) as u8
                    };
                    [blend(p.r), blend(p.g), blend(p.b)]
                })
                .collect::<Vec<_>>();
            let format = match a.format {
                1 => image::ImageFormat::Jpeg,
                2 => image::ImageFormat::Bmp,
                _ => return Err(failure(1003, "图像编码格式无效")),
            };
            image::save_buffer_with_format(
                native_path(a.path),
                &rgb,
                pixels.width(),
                pixels.height(),
                image::ColorType::Rgb8,
                format,
            )
            .map_err(|e| failure(2003, e.to_string()))?;
            own(0u8)
        }
        OP_MERGESELECTION => {
            let a = arg::<ArgsMergeSelection>(input)?;
            own(plot_helpers::merge_plot_selection(
                &Vec::<(i32, i32, i32)>::read(a.current),
                &Vec::<(i32, i32, i32)>::read(a.incoming),
                a.mode,
            ))
        }
        _ => return Err(failure(1004, "未知的 ABI 操作")),
    })
}
fn error_output(error: Failure) -> CallResult {
    let mut result = own(error.message);
    let view = unsafe { *result.value.cast::<slintforge_runtime_api::Slice<u8>>() };
    result.status = -1;
    result.code = error.code;
    result.message = view;
    result
}
unsafe extern "C" fn runtime_call(
    operation: u32,
    input: *const std::ffi::c_void,
    output: *mut CallResult,
) -> i32 {
    if output.is_null() || (output as usize) % std::mem::align_of::<CallResult>() != 0 {
        return -1;
    }
    let result =
        std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| dispatch(operation, input)));
    *output = match result {
        Ok(Ok(value)) => value,
        Ok(Err(error)) => error_output(error),
        Err(_) => error_output(failure(1000, "运行库拒绝无效参数或计算发生异常")),
    };
    (*output).status
}
unsafe extern "C" fn runtime_release(owner: u64) -> i32 {
    if owner == 0 {
        return 0;
    }
    if outputs()
        .lock()
        .unwrap_or_else(|e| e.into_inner())
        .remove(&owner)
        .is_some()
    {
        0
    } else {
        -1
    }
}

include!("api_table.rs");
/// 返回进程生命周期内有效的 ABI v2 函数表。
#[no_mangle]
pub extern "C" fn slintforge_runtime_get_api_v2() -> *const RuntimeApiV2 {
    &API
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn invalid_handles_and_double_release_are_reported() {
        unsafe {
            let mut out = CallResult::default();
            let args = ArgsRouterBack { handle: u64::MAX };
            assert_ne!(
                runtime_call(
                    OP_ROUTERBACK,
                    (&args as *const ArgsRouterBack).cast(),
                    &mut out
                ),
                0
            );
            assert_eq!(out.code, 1001);
            assert_eq!(runtime_release(out.owner), 0);
            assert_eq!(runtime_release(out.owner), -1);
        }
    }
    #[test]
    fn invalid_utf8_is_contained_and_output_is_released() {
        unsafe {
            let bytes = [255u8];
            let args = ArgsColorParse {
                text: slintforge_runtime_api::Slice::new(bytes.as_ptr(), 1),
            };
            let mut out = CallResult::default();
            assert_ne!(
                runtime_call(
                    OP_COLORPARSE,
                    (&args as *const ArgsColorParse).cast(),
                    &mut out
                ),
                0
            );
            assert_eq!(out.code, 1000);
            assert_eq!(runtime_release(out.owner), 0);
        }
    }
    #[test]
    fn output_views_survive_input_drop() {
        unsafe {
            let mut out = CallResult::default();
            {
                let text = "中文二维码".to_owned();
                let mut arena = Arena::default();
                let args = ArgsQr {
                    text: text.view(&mut arena),
                };
                assert_eq!(
                    runtime_call(OP_QR, (&args as *const ArgsQr).cast(), &mut out),
                    0
                );
            }
            let image = *out
                .value
                .cast::<slintforge_runtime_api::Pair<slintforge_runtime_api::Pixels, i32>>();
            assert_eq!(
                image.first.bytes.len,
                image.first.width as usize * image.first.height as usize * 4
            );
            assert!(!image.first.bytes.read().is_empty());
            assert_eq!(runtime_release(out.owner), 0);
        }
    }
}
