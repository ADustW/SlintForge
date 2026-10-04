use std::fs;
use std::io;
use std::path::PathBuf;

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
