// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

//! DLL 与安全封装共用的纯 C 数据契约；此包不包含组件计算实现。
use std::any::Any;
use std::ffi::c_void;

pub mod models;
pub mod plot;

pub const ABI_VERSION: u32 = 2;

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct Slice<T> {
    pub data: *const T,
    pub len: usize,
}
impl<T> Slice<T> {
    pub fn new(data: *const T, len: usize) -> Self {
        Self { data, len }
    }
    /// # Safety
    /// 非空视图必须指向调用期间有效、对齐的完整数组。
    pub unsafe fn read<'a>(&self) -> &'a [T] {
        if self.len == 0 {
            return &[];
        }
        assert!(!self.data.is_null(), "ABI 数组指针为空");
        assert!(
            self.len <= isize::MAX as usize / std::mem::size_of::<T>().max(1),
            "ABI 数组长度溢出"
        );
        assert_eq!(
            (self.data as usize) % std::mem::align_of::<T>(),
            0,
            "ABI 数组未对齐"
        );
        std::slice::from_raw_parts(self.data, self.len)
    }
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct Optional<T: Copy> {
    pub present: u8,
    pub value: std::mem::MaybeUninit<T>,
}
#[repr(C)]
#[derive(Clone, Copy)]
pub struct Pair<A, B> {
    pub first: A,
    pub second: B,
}
#[repr(C)]
#[derive(Clone, Copy)]
pub struct Triple<A, B, C> {
    pub first: A,
    pub second: B,
    pub third: C,
}
#[repr(C)]
#[derive(Clone, Copy)]
pub struct Pixels {
    pub width: u32,
    pub height: u32,
    pub bytes: Slice<u8>,
}
#[repr(C)]
#[derive(Clone, Copy)]
pub struct CallResult {
    pub status: i32,
    pub code: i32,
    pub owner: u64,
    pub value: *const c_void,
    pub message: Slice<u8>,
}
impl Default for CallResult {
    fn default() -> Self {
        Self {
            status: 0,
            code: 0,
            owner: 0,
            value: std::ptr::null(),
            message: Slice::new(std::ptr::null(), 0),
        }
    }
}

/// 转换期间的临时分配；DLL 输出分配池只由 DLL 持有和销毁。
#[derive(Default)]
pub struct Arena {
    items: Vec<Box<dyn Any>>,
}
impl Arena {
    pub fn hold<T: 'static>(&mut self, value: T) -> *const T {
        let item = Box::new(value);
        let pointer = &*item as *const T;
        self.items.push(item);
        pointer
    }
    pub fn slice<T: 'static>(&mut self, values: Vec<T>) -> Slice<T> {
        let view = Slice::new(values.as_ptr(), values.len());
        self.items.push(Box::new(values));
        view
    }
}

/// 安全封装仅在同步调用期间借用输入，输出必须在释放 owner 前复制。
pub trait Wire: Sized {
    type View: Copy + 'static;
    fn view(&self, arena: &mut Arena) -> Self::View;
    /// # Safety
    /// 视图及其递归引用必须有效，且符合当前 ABI 的类型与枚举约束。
    unsafe fn read(view: Self::View) -> Self;
}
macro_rules! scalar {
    ($($t:ty),*) => { $(impl Wire for $t {
        type View = Self;
        fn view(&self, _: &mut Arena) -> Self { *self }
        unsafe fn read(value: Self) -> Self { value }
    })* };
}
scalar!(u8, u16, u32, u64, i32, usize, f32, f64);
impl Wire for bool {
    type View = u8;
    fn view(&self, _: &mut Arena) -> u8 {
        u8::from(*self)
    }
    unsafe fn read(value: u8) -> Self {
        assert!(value <= 1, "ABI 布尔值无效");
        value != 0
    }
}
impl Wire for String {
    type View = Slice<u8>;
    fn view(&self, _: &mut Arena) -> Self::View {
        Slice::new(self.as_ptr(), self.len())
    }
    unsafe fn read(value: Self::View) -> Self {
        std::str::from_utf8(value.read())
            .expect("ABI 文本必须是 UTF-8")
            .to_owned()
    }
}
impl<T: Wire> Wire for Vec<T> {
    type View = Slice<T::View>;
    fn view(&self, arena: &mut Arena) -> Self::View {
        slice_view(self, arena)
    }
    unsafe fn read(value: Self::View) -> Self {
        value.read().iter().map(|item| T::read(*item)).collect()
    }
}
impl<T: Wire> Wire for Option<T> {
    type View = Optional<T::View>;
    fn view(&self, arena: &mut Arena) -> Self::View {
        match self {
            Some(value) => Optional {
                present: 1,
                value: std::mem::MaybeUninit::new(value.view(arena)),
            },
            None => Optional {
                present: 0,
                value: std::mem::MaybeUninit::uninit(),
            },
        }
    }
    unsafe fn read(value: Self::View) -> Self {
        match value.present {
            0 => None,
            1 => Some(T::read(value.value.assume_init())),
            _ => panic!("ABI 可选值标记无效"),
        }
    }
}
pub fn slice_view<T: Wire>(values: &[T], arena: &mut Arena) -> Slice<T::View> {
    let items = values.iter().map(|value| value.view(arena)).collect();
    arena.slice(items)
}
impl<A: Wire, B: Wire> Wire for (A, B) {
    type View = Pair<A::View, B::View>;
    fn view(&self, arena: &mut Arena) -> Self::View {
        Pair {
            first: self.0.view(arena),
            second: self.1.view(arena),
        }
    }
    unsafe fn read(value: Self::View) -> Self {
        (A::read(value.first), B::read(value.second))
    }
}
impl<A: Wire, B: Wire, C: Wire> Wire for (A, B, C) {
    type View = Triple<A::View, B::View, C::View>;
    fn view(&self, arena: &mut Arena) -> Self::View {
        Triple {
            first: self.0.view(arena),
            second: self.1.view(arena),
            third: self.2.view(arena),
        }
    }
    unsafe fn read(value: Self::View) -> Self {
        (
            A::read(value.first),
            B::read(value.second),
            C::read(value.third),
        )
    }
}

// 函数表由生成器从同一份绘图契约和模型生成，字段顺序属于 ABI v2。
include!("table.rs");
