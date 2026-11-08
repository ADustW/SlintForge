// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

//! 计算核心使用的纯数值颜色和像素存储，不创建 GUI 对象。
#[derive(Clone, Copy, Default, Debug, PartialEq, Eq)]
pub struct Color(u32);
impl Color {
    pub fn from_argb_u8(a: u8, r: u8, g: u8, b: u8) -> Self {
        Self(u32::from_be_bytes([a, r, g, b]))
    }
    pub fn from_rgb_u8(r: u8, g: u8, b: u8) -> Self {
        Self::from_argb_u8(255, r, g, b)
    }
    pub fn alpha(self) -> u8 {
        (self.0 >> 24) as u8
    }
    pub fn red(self) -> u8 {
        (self.0 >> 16) as u8
    }
    pub fn green(self) -> u8 {
        (self.0 >> 8) as u8
    }
    pub fn blue(self) -> u8 {
        self.0 as u8
    }
}
#[repr(C)]
#[derive(Clone, Copy, Default, Debug, PartialEq, Eq)]
pub struct Rgba8Pixel {
    pub r: u8,
    pub g: u8,
    pub b: u8,
    pub a: u8,
}
#[derive(Clone, Debug)]
pub struct SharedPixelBuffer<P> {
    width: u32,
    height: u32,
    pixels: Vec<P>,
}
impl<P: Default + Clone> SharedPixelBuffer<P> {
    pub fn new(width: u32, height: u32) -> Self {
        let size = (width as usize)
            .checked_mul(height as usize)
            .expect("像素尺寸溢出");
        Self {
            width,
            height,
            pixels: vec![P::default(); size],
        }
    }
}
impl<P> SharedPixelBuffer<P> {
    pub fn width(&self) -> u32 {
        self.width
    }
    pub fn height(&self) -> u32 {
        self.height
    }
    pub fn as_slice(&self) -> &[P] {
        &self.pixels
    }
    pub fn make_mut_slice(&mut self) -> &mut [P] {
        &mut self.pixels
    }
}
impl SharedPixelBuffer<Rgba8Pixel> {
    pub fn as_bytes(&self) -> &[u8] {
        unsafe { std::slice::from_raw_parts(self.pixels.as_ptr().cast(), self.pixels.len() * 4) }
    }
}
impl slintforge_runtime_api::Wire for SharedPixelBuffer<Rgba8Pixel> {
    type View = slintforge_runtime_api::Pixels;
    fn view(&self, _: &mut slintforge_runtime_api::Arena) -> Self::View {
        Self::View {
            width: self.width,
            height: self.height,
            bytes: slintforge_runtime_api::Slice::new(
                self.as_bytes().as_ptr(),
                self.as_bytes().len(),
            ),
        }
    }
    unsafe fn read(value: Self::View) -> Self {
        let length = (value.width as usize)
            .checked_mul(value.height as usize)
            .and_then(|n| n.checked_mul(4))
            .expect("像素尺寸溢出");
        assert_eq!(length, value.bytes.len, "像素字节长度不匹配");
        let bytes = value.bytes.read();
        let pixels = bytes
            .chunks_exact(4)
            .map(|p| Rgba8Pixel {
                r: p[0],
                g: p[1],
                b: p[2],
                a: p[3],
            })
            .collect();
        Self {
            width: value.width,
            height: value.height,
            pixels,
        }
    }
}
