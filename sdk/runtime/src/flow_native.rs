// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

use std::ffi::{c_char, c_void, CStr, CString};
extern "C" {
    fn forge_flow_create(mode: *const c_char) -> *mut c_void;
    fn forge_flow_destroy(flow: *mut c_void);
    fn forge_flow_command(flow: *mut c_void, command: *const c_char) -> *const c_char;
    fn forge_flow_snapshot(flow: *mut c_void) -> *const c_char;
    fn forge_flow_export(flow: *mut c_void) -> *const c_char;
}
pub struct FlowNative(*mut c_void);
// 文档无 GUI 对象，所有访问由运行库对象表的互斥锁串行保护。
unsafe impl Send for FlowNative {}
impl FlowNative {
    pub fn new(mode: &str) -> Result<Self, String> {
        let mode = CString::new(mode).map_err(|e| e.to_string())?;
        let pointer = unsafe { forge_flow_create(mode.as_ptr()) };
        if pointer.is_null() {
            Err("无法创建图文档".into())
        } else {
            Ok(Self(pointer))
        }
    }
    pub fn execute(&mut self, command: &str) -> Result<String, String> {
        let command = CString::new(command).map_err(|e| e.to_string())?;
        Ok(
            unsafe { CStr::from_ptr(forge_flow_command(self.0, command.as_ptr())) }
                .to_string_lossy()
                .into_owned(),
        )
    }
    pub fn snapshot(&mut self) -> String {
        unsafe { CStr::from_ptr(forge_flow_snapshot(self.0)) }
            .to_string_lossy()
            .into_owned()
    }
    pub fn export(&mut self) -> String {
        unsafe { CStr::from_ptr(forge_flow_export(self.0)) }
            .to_string_lossy()
            .into_owned()
    }
}
impl Drop for FlowNative {
    fn drop(&mut self) {
        unsafe { forge_flow_destroy(self.0) }
    }
}
