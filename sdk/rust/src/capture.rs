// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

//! 截图坐标采用窗口逻辑像素，像素数据采用不预乘的 RGBA8。
use slint::{Rgba8Pixel, SharedPixelBuffer, Window};
use std::path::Path;

#[derive(Clone, Copy, Debug)]
pub struct CaptureRect {
    pub x: f32,
    pub y: f32,
    pub width: f32,
    pub height: f32,
}

#[derive(Debug)]
pub struct CaptureError(pub String);

impl std::fmt::Display for CaptureError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str(&self.0)
    }
}
impl std::error::Error for CaptureError {}

/// 必须在所属 UI 线程、窗口显示并完成布局后调用。
pub fn capture_window(window: &Window) -> Result<SharedPixelBuffer<Rgba8Pixel>, CaptureError> {
    if !window.is_visible() {
        return Err(CaptureError("窗口未显示".into()));
    }
    let pixels = window
        .take_snapshot()
        .map_err(|e| CaptureError(format!("渲染器截图失败: {e}")))?;
    if pixels.width() == 0 || pixels.height() == 0 {
        return Err(CaptureError("窗口截图为空".into()));
    }
    Ok(pixels)
}

/// 获取当前窗口画面中的矩形区域，保留滚动位置及画面覆盖层。
pub fn capture_region(
    window: &Window,
    rect: CaptureRect,
) -> Result<SharedPixelBuffer<Rgba8Pixel>, CaptureError> {
    let pixels = capture_window(window)?;
    crop_capture(&pixels, rect, window.scale_factor())
}

/// 对已有截图裁剪，也用于不依赖显示器 DPI 的坐标换算测试。
pub fn crop_capture(
    pixels: &SharedPixelBuffer<Rgba8Pixel>,
    rect: CaptureRect,
    scale: f32,
) -> Result<SharedPixelBuffer<Rgba8Pixel>, CaptureError> {
    use slintforge_runtime_api::Wire;
    let mut arena = slintforge_runtime_api::Arena::default();
    let args = slintforge_runtime_api::models::ArgsCrop {
        pixels: crate::pixels_view(pixels),
        rect: rect.view(&mut arena),
        scale,
    };
    crate::runtime::invoke::<slintforge_runtime_api::Pixels, _, _>(
        slintforge_runtime_api::models::OP_CROP,
        &args,
        crate::pixels_from_view,
    )
    .map_err(|e| CaptureError(e.message()))
}

/// 调用方决定输出路径；不自动创建父目录，保留明确的保存错误。
pub fn save_capture_png(
    pixels: &SharedPixelBuffer<Rgba8Pixel>,
    path: impl AsRef<Path>,
) -> Result<(), CaptureError> {
    #[cfg(windows)]
    let path: Vec<u16> = {
        use std::os::windows::ffi::OsStrExt;
        path.as_ref().as_os_str().encode_wide().collect()
    };
    #[cfg(not(windows))]
    let path: Vec<u16> = path.as_ref().to_string_lossy().encode_utf16().collect();
    let args = slintforge_runtime_api::models::ArgsSavePng {
        pixels: crate::pixels_view(pixels),
        path: slintforge_runtime_api::Slice::new(path.as_ptr(), path.len()),
    };
    crate::runtime::invoke::<u8, _, _>(slintforge_runtime_api::models::OP_SAVEPNG, &args, |_| ())
        .map_err(|e| CaptureError(e.message()))
}

/// 将消费工程生成的 Slint 服务与结果类型接到当前窗口，不形成引用环。
#[macro_export]
macro_rules! bind_capture_service {
    ($ui:expr, $service:ty, $result:ty) => {{
        use slint::ComponentHandle as _;
        $crate::runtime::initialize().expect("截图运行库初始化失败");
        let weak = $ui.as_weak();
        $ui.global::<$service>().on_capture_window(move || {
            let mut result = <$result>::default();
            match weak
                .upgrade()
                .ok_or_else(|| $crate::CaptureError("窗口已销毁".into()))
                .and_then(|ui| $crate::capture_window(ui.window()))
            {
                Ok(pixels) => {
                    result.ok = true;
                    result.pixel_width = pixels.width() as i32;
                    result.pixel_height = pixels.height() as i32;
                    result.image = slint::Image::from_rgba8(pixels);
                }
                Err(error) => result.error = error.to_string().into(),
            }
            result
        });
        $crate::runtime::initialize().expect("截图运行库初始化失败");
        let weak = $ui.as_weak();
        $ui.global::<$service>().on_capture_region(move |rect| {
            let mut result = <$result>::default();
            match weak
                .upgrade()
                .ok_or_else(|| $crate::CaptureError("窗口已销毁".into()))
                .and_then(|ui| {
                    $crate::capture_region(
                        ui.window(),
                        $crate::CaptureRect {
                            x: rect.x,
                            y: rect.y,
                            width: rect.width,
                            height: rect.height,
                        },
                    )
                }) {
                Ok(pixels) => {
                    result.ok = true;
                    result.pixel_width = pixels.width() as i32;
                    result.pixel_height = pixels.height() as i32;
                    result.image = slint::Image::from_rgba8(pixels);
                }
                Err(error) => result.error = error.to_string().into(),
            }
            result
        });
        $ui.global::<$service>().set_connected(true);
    }};
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn crop_matches_logical_bounds_at_fractional_scales() {
        let mut source = SharedPixelBuffer::<Rgba8Pixel>::new(40, 30);
        for (index, pixel) in source.make_mut_slice().iter_mut().enumerate() {
            *pixel = Rgba8Pixel {
                r: (index % 40) as u8,
                g: (index / 40) as u8,
                b: 77,
                a: 128,
            };
        }
        for scale in [1.0f32, 1.25, 1.5, 2.0] {
            let rect = CaptureRect {
                x: 1.2,
                y: 2.4,
                width: 7.3,
                height: 4.2,
            };
            let crop = crop_capture(&source, rect, scale).unwrap();
            let left = (f64::from(rect.x) * f64::from(scale)).floor() as u8;
            let top = (f64::from(rect.y) * f64::from(scale)).floor() as u8;
            assert_eq!(
                crop.as_slice()[0],
                Rgba8Pixel {
                    r: left,
                    g: top,
                    b: 77,
                    a: 128
                }
            );
            assert_eq!(
                crop.width(),
                ((f64::from(rect.x) + f64::from(rect.width)) * f64::from(scale)).ceil() as u32
                    - u32::from(left)
            );
            assert_eq!(
                crop.height(),
                ((f64::from(rect.y) + f64::from(rect.height)) * f64::from(scale)).ceil() as u32
                    - u32::from(top)
            );
        }
        let edge = crop_capture(
            &source,
            CaptureRect {
                x: -5.0,
                y: -4.0,
                width: 9.0,
                height: 10.0,
            },
            1.0,
        )
        .unwrap();
        assert_eq!((edge.width(), edge.height()), (4, 6));
        for scale in [0.0, -1.0, f32::NAN, f32::INFINITY] {
            assert!(crop_capture(
                &source,
                CaptureRect {
                    x: 1.0,
                    y: 1.0,
                    width: 2.0,
                    height: 2.0
                },
                scale
            )
            .is_err());
        }
        for rect in [
            CaptureRect {
                x: 90.0,
                y: 0.0,
                width: 2.0,
                height: 2.0,
            },
            CaptureRect {
                x: 0.0,
                y: 0.0,
                width: 0.0,
                height: 2.0,
            },
            CaptureRect {
                x: f32::NAN,
                y: 0.0,
                width: 2.0,
                height: 2.0,
            },
        ] {
            assert!(crop_capture(&source, rect, 1.0).is_err());
        }
    }

    #[test]
    fn png_round_trip_preserves_rgba_and_reports_write_failures() {
        let directory =
            std::env::temp_dir().join(format!("slintforge-截图-{}", std::process::id()));
        std::fs::create_dir_all(&directory).unwrap();
        let file = directory.join("透明像素.png");
        let mut source = SharedPixelBuffer::<Rgba8Pixel>::new(2, 1);
        source.make_mut_slice().copy_from_slice(&[
            Rgba8Pixel {
                r: 255,
                g: 10,
                b: 0,
                a: 128,
            },
            Rgba8Pixel {
                r: 0,
                g: 20,
                b: 255,
                a: 0,
            },
        ]);
        save_capture_png(&source, &file).unwrap();
        let mut reader =
            png::Decoder::new(std::io::BufReader::new(std::fs::File::open(&file).unwrap()))
                .read_info()
                .unwrap();
        let mut bytes = vec![0; reader.output_buffer_size().unwrap()];
        let info = reader.next_frame(&mut bytes).unwrap();
        assert_eq!((info.width, info.height), (2, 1));
        assert_eq!(&bytes[..info.buffer_size()], source.as_bytes());
        assert!(save_capture_png(&source, &directory).is_err());
        std::fs::remove_file(file).unwrap();
        std::fs::remove_dir(directory).unwrap();
    }
}
