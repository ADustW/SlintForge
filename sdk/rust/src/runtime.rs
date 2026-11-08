// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

//! 运行库加载、ABI 校验和 DLL 所有权封装。
use slintforge_runtime_api::{CallResult, RuntimeApiV2, Wire, ABI_SCHEMA, ABI_VERSION};
use std::path::{Path, PathBuf};
use std::sync::{Mutex, OnceLock};

#[derive(Debug, Clone)]
pub enum RuntimeError {
    Missing { paths: Vec<PathBuf> },
    Load { path: PathBuf, detail: String },
    Abi { path: PathBuf, detail: String },
    AlreadyInitialized { loaded: PathBuf, requested: PathBuf },
    Call { code: i32, message: String },
}
impl RuntimeError {
    pub fn message(&self) -> String {
        match self {
            Self::Call { message, .. } => message.clone(),
            _ => self.to_string(),
        }
    }
    pub(crate) fn operation_error(&self) -> (i32, String) {
        match self {
            Self::Call { code, message } => (*code, message.clone()),
            _ => (1000, self.to_string()),
        }
    }
}
impl std::fmt::Display for RuntimeError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::Missing { paths } => write!(
                f,
                "找不到 slintforge_runtime.dll；检查位置：{}",
                paths
                    .iter()
                    .map(|p| p.display().to_string())
                    .collect::<Vec<_>>()
                    .join("；")
            ),
            Self::Load { path, detail } => write!(
                f,
                "无法加载 Windows x64 运行库 {}：{detail}",
                path.display()
            ),
            Self::Abi { path, detail } => {
                write!(f, "运行库 ABI 不兼容 {}：{detail}", path.display())
            }
            Self::AlreadyInitialized { loaded, requested } => write!(
                f,
                "运行库已经从 {} 加载，不能切换为 {}",
                loaded.display(),
                requested.display()
            ),
            Self::Call { code, message } => write!(f, "{message}（运行库错误 {code}）"),
        }
    }
}
impl std::error::Error for RuntimeError {}

struct Runtime {
    _library: libloading::Library,
    api: RuntimeApiV2,
    path: PathBuf,
}
static RUNTIME: OnceLock<Runtime> = OnceLock::new();
static LOAD_LOCK: Mutex<()> = Mutex::new(());

/// 优先使用 EXE 同目录的 DLL；开发时可使用接口包内的运行库。
pub fn initialize() -> Result<(), RuntimeError> {
    if RUNTIME.get().is_some() {
        return Ok(());
    }
    let mut candidates = Vec::new();
    if let Ok(exe) = std::env::current_exe() {
        if let Some(parent) = exe.parent() {
            candidates.push(parent.join("slintforge_runtime.dll"));
            // Cargo 的测试和示例位于 profile/deps、profile/examples。
            if matches!(
                parent.file_name().and_then(|n| n.to_str()),
                Some("deps" | "examples")
            ) {
                if let Some(profile) = parent.parent() {
                    candidates.push(profile.join("slintforge_runtime.dll"));
                }
            }
        }
    }
    candidates
        .push(PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("runtime/slintforge_runtime.dll"));
    for path in &candidates {
        if path.is_file() {
            return initialize_from(path);
        }
    }
    Err(RuntimeError::Missing { paths: candidates })
}

/// 必须在首次 SDK 调用前设置；加载成功后运行库保活至进程结束。
pub fn initialize_from(path: impl AsRef<Path>) -> Result<(), RuntimeError> {
    let requested = path
        .as_ref()
        .canonicalize()
        .map_err(|e| RuntimeError::Load {
            path: path.as_ref().to_path_buf(),
            detail: e.to_string(),
        })?;
    let _lock = LOAD_LOCK.lock().unwrap_or_else(|e| e.into_inner());
    if let Some(runtime) = RUNTIME.get() {
        return if runtime.path == requested {
            Ok(())
        } else {
            Err(RuntimeError::AlreadyInitialized {
                loaded: runtime.path.clone(),
                requested,
            })
        };
    }
    #[cfg(windows)]
    {
        use std::io::{Read, Seek, SeekFrom};
        let inspect = (|| -> std::io::Result<u16> {
            let mut file = std::fs::File::open(&requested)?;
            let mut dos = [0u8; 64];
            file.read_exact(&mut dos)?;
            if &dos[..2] != b"MZ" {
                return Err(std::io::Error::new(
                    std::io::ErrorKind::InvalidData,
                    "缺少 PE 文件头",
                ));
            }
            let offset = u32::from_le_bytes(dos[60..64].try_into().unwrap());
            file.seek(SeekFrom::Start(offset as u64))?;
            let mut pe = [0u8; 6];
            file.read_exact(&mut pe)?;
            if &pe[..4] != b"PE\0\0" {
                return Err(std::io::Error::new(
                    std::io::ErrorKind::InvalidData,
                    "PE 文件签名无效",
                ));
            }
            Ok(u16::from_le_bytes([pe[4], pe[5]]))
        })();
        let machine = inspect.map_err(|e| RuntimeError::Load {
            path: requested.clone(),
            detail: e.to_string(),
        })?;
        if machine != 0x8664 {
            return Err(RuntimeError::Load {
                path: requested,
                detail: format!("运行库不是 Windows x64 DLL，PE machine=0x{machine:04x}"),
            });
        }
    }
    unsafe {
        #[cfg(windows)]
        let library: libloading::Library =
            libloading::os::windows::Library::load_with_flags(&requested, 0x00000100 | 0x00000800)
                .map(Into::into)
                .map_err(|e| RuntimeError::Load {
                    path: requested.clone(),
                    detail: e.to_string(),
                })?;
        #[cfg(not(windows))]
        let library = libloading::Library::new(&requested).map_err(|e| RuntimeError::Load {
            path: requested.clone(),
            detail: e.to_string(),
        })?;
        let query = library
            .get::<unsafe extern "C" fn() -> *const RuntimeApiV2>(
                b"slintforge_runtime_get_api_v2\0",
            )
            .map_err(|e| RuntimeError::Abi {
                path: requested.clone(),
                detail: format!("缺少 ABI v2 入口：{e}"),
            })?;
        let pointer = query();
        if pointer.is_null() {
            return Err(RuntimeError::Abi {
                path: requested,
                detail: "函数表为空".into(),
            });
        }
        // 先读取固定头，拒绝不兼容版本或短表后才读取完整函数表。
        #[repr(C)]
        struct Header {
            version: u32,
            size: u32,
        }
        let header = &*pointer.cast::<Header>();
        if header.version != ABI_VERSION
            || (header.size as usize) < std::mem::size_of::<RuntimeApiV2>()
        {
            return Err(RuntimeError::Abi {
                path: requested,
                detail: format!(
                    "要求 ABI {ABI_VERSION}/{} 字节，收到 ABI {}/{} 字节",
                    std::mem::size_of::<RuntimeApiV2>(),
                    header.version,
                    header.size
                ),
            });
        }
        #[repr(C)]
        struct SchemaHeader {
            version: u32,
            size: u32,
            schema: [u8; 32],
        }
        if (*pointer.cast::<SchemaHeader>()).schema != ABI_SCHEMA {
            return Err(RuntimeError::Abi {
                path: requested,
                detail: "数据契约指纹不一致".into(),
            });
        }
        let api = *pointer;
        let _ = RUNTIME.set(Runtime {
            _library: library,
            api,
            path: requested,
        });
    }
    Ok(())
}

pub(crate) fn api() -> &'static RuntimeApiV2 {
    initialize().unwrap_or_else(|error| panic!("SlintForge 初始化失败：{error}"));
    &RUNTIME.get().expect("运行库已经初始化").api
}
struct OutputGuard {
    result: CallResult,
    api: &'static RuntimeApiV2,
}
impl Drop for OutputGuard {
    fn drop(&mut self) {
        if self.result.owner != 0 {
            unsafe {
                (self.api.release)(self.result.owner);
            }
        }
    }
}
pub(crate) fn invoke<V: Copy, A, R>(
    operation: u32,
    args: &A,
    decode: impl FnOnce(V) -> R,
) -> Result<R, RuntimeError> {
    initialize()?;
    let api = &RUNTIME.get().expect("运行库已经初始化").api;
    let mut output = OutputGuard {
        result: CallResult::default(),
        api,
    };
    let status = unsafe { (api.call)(operation, (args as *const A).cast(), &mut output.result) };
    if status != 0 || output.result.status != 0 {
        let message = unsafe {
            std::str::from_utf8(output.result.message.read())
                .unwrap_or("运行库返回无效 UTF-8 错误文本")
                .to_owned()
        };
        return Err(RuntimeError::Call {
            code: output.result.code,
            message,
        });
    }
    if output.result.value.is_null() {
        return Err(RuntimeError::Call {
            code: 1000,
            message: "运行库返回空结果".into(),
        });
    }
    assert_eq!(
        (output.result.value as usize) % std::mem::align_of::<V>(),
        0,
        "运行库返回的结构未对齐"
    );
    Ok(decode(unsafe { *output.result.value.cast::<V>() }))
}
pub(crate) fn try_value<T: Wire, A>(operation: u32, args: &A) -> Result<T, RuntimeError> {
    invoke::<T::View, _, _>(operation, args, |value| unsafe { T::read(value) })
}
// 仅用于已持有有效句柄、无法返回错误的路径（如 Clone）；运行库此时必然已加载。
pub(crate) fn value<T: Wire, A>(operation: u32, args: &A) -> T {
    try_value(operation, args).unwrap_or_else(|error| panic!("SlintForge 调用失败：{error}"))
}
// 服务调用失败时记录错误并返回兜底值，避免在 Slint 回调中 panic 终止 UI 线程。
pub(crate) fn value_or<T: Wire, A>(operation: u32, args: &A, fallback: impl FnOnce() -> T) -> T {
    try_value(operation, args).unwrap_or_else(|error| {
        eprintln!("SlintForge 调用失败（操作 {operation}）：{error}");
        fallback()
    })
}
pub(crate) fn release_object(operation: u32, handle: u64) {
    if let Err(error) = try_value::<u8, _>(operation, &handle) {
        eprintln!("SlintForge 对象释放失败：{error}");
    }
}
