// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#![windows_subsystem = "windows"]

use std::fs;
use std::io::{Read, Write};
use std::net::{IpAddr, Ipv4Addr, SocketAddr, TcpListener, TcpStream};
use std::path::{Path, PathBuf};
use std::sync::Arc;
use std::thread;
use std::time::{Duration, Instant};

#[cfg(windows)]
#[link(name = "shell32")]
extern "system" {
    fn ShellExecuteW(
        hwnd: *mut core::ffi::c_void,
        operation: *const u16,
        file: *const u16,
        parameters: *const u16,
        directory: *const u16,
        show_command: i32,
    ) -> isize;
}

#[cfg(windows)]
#[link(name = "user32")]
extern "system" {
    fn MessageBoxW(
        hwnd: *mut core::ffi::c_void,
        text: *const u16,
        caption: *const u16,
        kind: u32,
    ) -> i32;
}

fn wide(value: &str) -> Vec<u16> {
    value.encode_utf16().chain(std::iter::once(0)).collect()
}

fn report_error(message: &str) {
    eprintln!("SlintForge 文档启动失败：{message}");
    #[cfg(windows)]
    unsafe {
        MessageBoxW(
            std::ptr::null_mut(),
            wide(message).as_ptr(),
            wide("SlintForge API Documentation").as_ptr(),
            0x10,
        );
    }
}

fn percent_decode(value: &str) -> Result<String, &'static str> {
    let bytes = value.as_bytes();
    let mut output = Vec::with_capacity(bytes.len());
    let mut index = 0;
    while index < bytes.len() {
        if bytes[index] == b'%' {
            if index + 2 >= bytes.len() {
                return Err("URL 百分号转义不完整");
            }
            let high = (bytes[index + 1] as char).to_digit(16).ok_or("URL 百分号转义无效")?;
            let low = (bytes[index + 2] as char).to_digit(16).ok_or("URL 百分号转义无效")?;
            output.push(((high << 4) | low) as u8);
            index += 3;
        } else {
            output.push(bytes[index]);
            index += 1;
        }
    }
    String::from_utf8(output).map_err(|_| "URL 不是 UTF-8")
}

fn resolve_asset(root: &Path, request_target: &str) -> Result<Option<PathBuf>, &'static str> {
    let path = request_target
        .split(['?', '#'])
        .next()
        .ok_or("请求路径为空")?;
    let decoded = percent_decode(path)?;
    if !decoded.starts_with('/') || decoded.contains(['\\', '\0']) {
        return Err("请求路径无效");
    }
    let mut relative = PathBuf::new();
    for segment in decoded.trim_start_matches('/').split('/') {
        if segment.is_empty() || segment == "." {
            continue;
        }
        if segment == ".." || segment.contains(':') {
            return Err("拒绝目录穿越");
        }
        relative.push(segment);
    }
    let mut candidates = Vec::new();
    if relative.as_os_str().is_empty() || decoded.ends_with('/') {
        candidates.push(root.join(&relative).join("index.html"));
    } else {
        candidates.push(root.join(&relative));
        if relative.extension().is_none() {
            candidates.push(root.join(&relative).with_extension("html"));
            candidates.push(root.join(&relative).join("index.html"));
        }
    }
    for candidate in candidates {
        if !candidate.is_file() {
            continue;
        }
        let canonical = candidate.canonicalize().map_err(|_| "静态资源路径不可访问")?;
        if !canonical.starts_with(root) {
            return Err("拒绝目录穿越");
        }
        return Ok(Some(canonical));
    }
    Ok(None)
}

fn mime(path: &Path) -> &'static str {
    match path.extension().and_then(|value| value.to_str()).unwrap_or("").to_ascii_lowercase().as_str() {
        "html" => "text/html; charset=utf-8",
        "css" => "text/css; charset=utf-8",
        "js" | "mjs" => "text/javascript; charset=utf-8",
        "json" => "application/json; charset=utf-8",
        "png" => "image/png",
        "jpg" | "jpeg" => "image/jpeg",
        "svg" => "image/svg+xml",
        "webp" => "image/webp",
        "ico" => "image/x-icon",
        "woff" => "font/woff",
        "woff2" => "font/woff2",
        "txt" => "text/plain; charset=utf-8",
        "map" => "application/json; charset=utf-8",
        _ => "application/octet-stream",
    }
}

fn respond(stream: &mut TcpStream, status: &str, content_type: &str, body: &[u8], head: bool, extra: &str) -> std::io::Result<()> {
    let header = format!(
        "HTTP/1.1 {status}\r\nContent-Type: {content_type}\r\nContent-Length: {}\r\nX-Content-Type-Options: nosniff\r\nContent-Security-Policy: default-src 'self'; img-src 'self' data:; style-src 'self' 'unsafe-inline'; script-src 'self' 'unsafe-inline'\r\nCache-Control: no-cache\r\nConnection: close\r\n{extra}\r\n",
        body.len()
    );
    stream.write_all(header.as_bytes())?;
    if !head {
        stream.write_all(body)?;
    }
    stream.flush()
}

fn serve_connection(mut stream: TcpStream, root: &Path) -> std::io::Result<()> {
    stream.set_read_timeout(Some(Duration::from_secs(5)))?;
    stream.set_write_timeout(Some(Duration::from_secs(10)))?;
    let mut request = [0u8; 16 * 1024];
    let length = stream.read(&mut request)?;
    if length == request.len() {
        return respond(&mut stream, "431 Request Header Fields Too Large", "text/plain; charset=utf-8", b"request header too large", false, "");
    }
    let text = match std::str::from_utf8(&request[..length]) {
        Ok(value) => value,
        Err(_) => return respond(&mut stream, "400 Bad Request", "text/plain; charset=utf-8", b"invalid request", false, ""),
    };
    let first = match text.lines().next() {
        Some(value) => value,
        None => return respond(&mut stream, "400 Bad Request", "text/plain; charset=utf-8", b"invalid request", false, ""),
    };
    let parts: Vec<_> = first.split_whitespace().collect();
    if parts.len() != 3 || !parts[2].starts_with("HTTP/1.") {
        return respond(&mut stream, "400 Bad Request", "text/plain; charset=utf-8", b"invalid request", false, "");
    }
    let head = parts[0] == "HEAD";
    if parts[0] != "GET" && !head {
        return respond(&mut stream, "405 Method Not Allowed", "text/plain; charset=utf-8", b"method not allowed", false, "Allow: GET, HEAD\r\n");
    }
    match resolve_asset(root, parts[1]) {
        Ok(Some(path)) => {
            let body = fs::read(&path)?;
            respond(&mut stream, "200 OK", mime(&path), &body, head, "")
        }
        Ok(None) => respond(&mut stream, "404 Not Found", "text/plain; charset=utf-8", b"not found", head, ""),
        Err(_) => respond(&mut stream, "400 Bad Request", "text/plain; charset=utf-8", b"invalid path", head, ""),
    }
}

fn run_server(listener: TcpListener, root: Arc<PathBuf>, idle: Duration, maximum: Option<usize>) -> std::io::Result<()> {
    listener.set_nonblocking(true)?;
    let mut last_request = Instant::now();
    let mut served = 0usize;
    loop {
        match listener.accept() {
            Ok((stream, address)) => {
                if !address.ip().is_loopback() {
                    continue;
                }
                last_request = Instant::now();
                let _ = serve_connection(stream, &root);
                served += 1;
                if maximum.is_some_and(|value| served >= value) {
                    return Ok(());
                }
            }
            Err(error) if error.kind() == std::io::ErrorKind::WouldBlock => {
                if last_request.elapsed() >= idle {
                    return Ok(());
                }
                thread::sleep(Duration::from_millis(25));
            }
            Err(error) => return Err(error),
        }
    }
}

fn request(address: SocketAddr, value: &str) -> Result<Vec<u8>, String> {
    let mut stream = TcpStream::connect_timeout(&address, Duration::from_secs(2)).map_err(|error| error.to_string())?;
    stream.write_all(value.as_bytes()).map_err(|error| error.to_string())?;
    stream.shutdown(std::net::Shutdown::Write).map_err(|error| error.to_string())?;
    let mut response = Vec::new();
    stream.read_to_end(&mut response).map_err(|error| error.to_string())?;
    Ok(response)
}

fn check(root: PathBuf) -> Result<(), String> {
    let listener = TcpListener::bind((Ipv4Addr::LOCALHOST, 0)).map_err(|error| error.to_string())?;
    let address = listener.local_addr().map_err(|error| error.to_string())?;
    if address.ip() != IpAddr::V4(Ipv4Addr::LOCALHOST) {
        return Err("自检服务器未绑定 127.0.0.1".into());
    }
    let worker_root = Arc::new(root);
    let worker = {
        let worker_root = worker_root.clone();
        thread::spawn(move || run_server(listener, worker_root, Duration::from_secs(5), Some(5)))
    };
    let cases = [
        ("GET / HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n", "200 OK"),
        ("HEAD / HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n", "200 OK"),
        ("GET /missing-file HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n", "404 Not Found"),
        ("GET /%2e%2e/Cargo.toml HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n", "400 Bad Request"),
        ("POST / HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Length: 0\r\n\r\n", "405 Method Not Allowed"),
    ];
    for (value, expected) in cases {
        let response = request(address, value)?;
        let status = String::from_utf8_lossy(&response);
        if !status.starts_with(&format!("HTTP/1.1 {expected}")) {
            return Err(format!("自检响应不正确，期望 {expected}"));
        }
        if value.starts_with("HEAD ") && response.windows(4).position(|item| item == b"\r\n\r\n").is_some_and(|index| index + 4 != response.len()) {
            return Err("HEAD 响应不应包含正文".into());
        }
    }
    worker.join().map_err(|_| "自检服务器线程异常".to_string())?.map_err(|error| error.to_string())?;
    println!("DOCS_LAUNCHER_CHECK_OK loopback=127.0.0.1 methods=GET,HEAD traversal=blocked mime=verified");
    Ok(())
}

fn default_root() -> Result<PathBuf, String> {
    let executable = std::env::current_exe().map_err(|error| error.to_string())?;
    let parent = executable.parent().ok_or("启动器路径没有父目录")?;
    parent.join("docs/site").canonicalize().map_err(|error| format!("找不到 docs/site：{error}"))
}

fn root_argument(arguments: &[String]) -> Result<PathBuf, String> {
    if let Some(index) = arguments.iter().position(|value| value == "--root") {
        let value = arguments.get(index + 1).ok_or("--root 缺少路径")?;
        return PathBuf::from(value).canonicalize().map_err(|error| error.to_string());
    }
    default_root()
}

fn run() -> Result<(), String> {
    let arguments: Vec<String> = std::env::args().skip(1).collect();
    let root = root_argument(&arguments)?;
    if !root.join("index.html").is_file() {
        return Err(format!("文档根目录缺少 index.html：{}", root.display()));
    }
    if arguments.iter().any(|value| value == "--check" || value == "/check") {
        return check(root);
    }
    let listener = TcpListener::bind((Ipv4Addr::LOCALHOST, 0)).map_err(|error| error.to_string())?;
    let address = listener.local_addr().map_err(|error| error.to_string())?;
    if address.ip() != IpAddr::V4(Ipv4Addr::LOCALHOST) {
        return Err("文档服务必须绑定 127.0.0.1".into());
    }
    let url = format!("http://127.0.0.1:{}/", address.port());
    #[cfg(windows)]
    unsafe {
        let result = ShellExecuteW(
            std::ptr::null_mut(),
            wide("open").as_ptr(),
            wide(&url).as_ptr(),
            std::ptr::null(),
            std::ptr::null(),
            1,
        );
        if result <= 32 {
            return Err(format!("无法启动默认浏览器，ShellExecuteW={result}"));
        }
    }
    run_server(listener, Arc::new(root), Duration::from_secs(30 * 60), None).map_err(|error| error.to_string())
}

fn main() {
    if let Err(error) = run() {
        report_error(&error);
        std::process::exit(1);
    }
}
