// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

use std::fs;
use std::io;
use std::path::PathBuf;
use std::sync::mpsc;
use std::thread::JoinHandle;

// 设置写入串行后台队列；合并连续切换，并在退出时保存最后一个状态。
pub struct SettingsWriter {
    sender: Option<mpsc::Sender<GallerySettings>>,
    worker: Option<JoinHandle<()>>,
}

impl SettingsWriter {
    pub fn new() -> Self {
        Self::with_save(save)
    }

    fn with_save(
        mut save: impl FnMut(&GallerySettings) -> io::Result<()> + Send + 'static,
    ) -> Self {
        let (sender, receiver) = mpsc::channel::<GallerySettings>();
        let worker = std::thread::spawn(move || {
            while let Ok(mut settings) = receiver.recv() {
                while let Ok(latest) = receiver.try_recv() {
                    settings = latest;
                }
                if let Err(error) = save(&settings) {
                    eprintln!("设置保存失败: {error}");
                }
            }
        });
        Self {
            sender: Some(sender),
            worker: Some(worker),
        }
    }

    pub fn request(&self, settings: GallerySettings) {
        if let Some(sender) = &self.sender {
            let _ = sender.send(settings);
        }
    }
}

impl Drop for SettingsWriter {
    fn drop(&mut self) {
        self.sender.take();
        if let Some(worker) = self.worker.take() {
            let _ = worker.join();
        }
    }
}

#[derive(Clone, Debug)]
pub struct GallerySettings {
    pub language: String,
    pub font_family: String,
    pub font_scale: f32,
    pub pinned: bool,
    pub rounded_corners: bool,
    pub dark_mode: bool,
    pub placement: String,
    pub pane_state: String,
}

impl Default for GallerySettings {
    fn default() -> Self {
        Self {
            language: "zh".into(),
            font_family: String::new(),
            font_scale: 1.0,
            pinned: false,
            rounded_corners: true,
            dark_mode: true,
            placement: "left".into(),
            pane_state: "expanded".into(),
        }
    }
}

fn settings_path() -> Option<PathBuf> {
    let base = std::env::var_os("APPDATA")?;
    Some(
        PathBuf::from(base)
            .join("SlintForge")
            .join("Gallery")
            .join("settings.ini"),
    )
}

fn parse(text: &str) -> GallerySettings {
    let mut settings = GallerySettings::default();
    for line in text.lines() {
        let Some((key, value)) = line.split_once('=') else {
            continue;
        };
        match (key, value) {
            ("language", "zh" | "en") => settings.language = value.into(),
            ("font_family", "" | "Microsoft YaHei UI" | "Segoe UI" | "Consolas") => {
                settings.font_family = value.into()
            }
            ("font_scale", "90") => settings.font_scale = 0.9,
            ("font_scale", "100") => settings.font_scale = 1.0,
            ("font_scale", "110") => settings.font_scale = 1.1,
            ("font_scale", "125") => settings.font_scale = 1.25,
            ("pinned", "0" | "1") => settings.pinned = value == "1",
            ("rounded_corners", "0" | "1") => settings.rounded_corners = value == "1",
            ("dark_mode", "0" | "1") => settings.dark_mode = value == "1",
            ("placement", "left" | "right" | "top") => settings.placement = value.into(),
            ("pane_state", "expanded" | "compact" | "hidden") => settings.pane_state = value.into(),
            _ => {}
        }
    }
    settings
}

pub fn load() -> GallerySettings {
    settings_path()
        .and_then(|path| fs::read_to_string(path).ok())
        .map(|text| parse(&text))
        .unwrap_or_default()
}

pub fn save(settings: &GallerySettings) -> io::Result<()> {
    let Some(path) = settings_path() else {
        return Ok(());
    };
    if let Some(parent) = path.parent() {
        fs::create_dir_all(parent)?;
    }
    fs::write(path, format!(
        "language={}\nfont_family={}\nfont_scale={}\npinned={}\nrounded_corners={}\ndark_mode={}\nplacement={}\npane_state={}\n",
        settings.language,
        settings.font_family,
        (settings.font_scale * 100.0).round() as i32,
        i32::from(settings.pinned),
        i32::from(settings.rounded_corners),
        i32::from(settings.dark_mode),
        settings.placement,
        settings.pane_state,
    ))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn queued_saves_do_not_block_requests_and_flush_latest_on_drop() {
        use std::sync::{Arc, Mutex};
        use std::time::Duration;
        let (started_tx, started_rx) = mpsc::channel();
        let (release_tx, release_rx) = mpsc::channel();
        let saved = Arc::new(Mutex::new(Vec::new()));
        let observed = saved.clone();
        let mut first = true;
        let writer = SettingsWriter::with_save(move |settings| {
            if first {
                first = false;
                started_tx.send(()).unwrap();
                release_rx.recv_timeout(Duration::from_secs(5)).unwrap();
            }
            observed.lock().unwrap().push(settings.pane_state.clone());
            Ok(())
        });
        writer.request(GallerySettings::default());
        started_rx.recv_timeout(Duration::from_secs(2)).unwrap();
        // 首次磁盘写入被阻塞时，UI 请求仍能完成；中间状态不必重复写盘。
        for _ in 0..100 {
            writer.request(GallerySettings {
                pane_state: "hidden".into(),
                ..Default::default()
            });
        }
        writer.request(GallerySettings {
            pane_state: "compact".into(),
            ..Default::default()
        });
        release_tx.send(()).unwrap();
        drop(writer);
        assert_eq!(*saved.lock().unwrap(), vec!["expanded", "compact"]);
    }

    #[test]
    fn invalid_values_keep_defaults() {
        let settings = parse(
            "language=de\nfont_family=Unknown\nfont_scale=800\npinned=yes\nplacement=bottom\n",
        );
        assert_eq!(settings.language, "zh");
        assert_eq!(settings.font_scale, 1.0);
        assert!(!settings.pinned);
        assert_eq!(settings.placement, "left");
    }
}
