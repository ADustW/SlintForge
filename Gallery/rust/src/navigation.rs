// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

use slintforge::{IconCatalog, NavItem, NavItemKind};

pub fn is_deferred(id: &str) -> bool {
    include_str!("../../data/component-status.txt")
        .lines()
        .any(|line| {
            let mut fields = line.split('|');
            fields.next() == Some(id) && fields.next() == Some("deferred")
        })
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum GalleryArea {
    Main,
    Footer,
    Hidden,
}

#[derive(Clone, Debug)]
pub struct GalleryEntry {
    pub item: NavItem,
    pub area: GalleryArea,
    pub zh_title: String,
}

// 两种宿主都读取 Gallery/data 中同一份导航目录。
pub fn entries() -> Vec<GalleryEntry> {
    let icons = IconCatalog::new();
    include_str!("../../data/navigation.txt")
        .lines()
        .enumerate()
        .filter(|(_, line)| !line.trim().is_empty() && !line.starts_with('#'))
        .map(|(index, line)| {
            let fields: Vec<_> = line.split('|').map(str::trim).collect();
            assert!(
                fields.len() == 6 || fields.len() == 7,
                "导航目录第 {} 行字段数错误",
                index + 1
            );
            let kind = match fields[0] {
                "P" => NavItemKind::Page,
                "G" => NavItemKind::Group,
                "H" => NavItemKind::Header,
                "S" => NavItemKind::Separator,
                other => panic!("导航目录第 {} 行条目类型错误: {other}", index + 1),
            };
            let area = match fields[5] {
                "main" => GalleryArea::Main,
                "footer" => GalleryArea::Footer,
                "hidden" => GalleryArea::Hidden,
                other => panic!("导航目录第 {} 行区域错误: {other}", index + 1),
            };
            let glyph = if kind == NavItemKind::Page || kind == NavItemKind::Group {
                icons
                    .find(fields[4])
                    .unwrap_or_else(|| {
                        panic!("导航目录第 {} 行图标不存在: {}", index + 1, fields[4])
                    })
                    .glyph()
            } else {
                String::new()
            };
            GalleryEntry {
                item: NavItem::new(
                    fields[1],
                    (!fields[2].is_empty()).then_some(fields[2]),
                    fields[3],
                    &glyph,
                    kind,
                ),
                area,
                zh_title: fields.get(6).copied().unwrap_or("").to_owned(),
            }
        })
        .collect()
}

pub fn title<'a>(entry: &'a GalleryEntry, language: &str) -> &'a str {
    if language == "zh" && !entry.zh_title.is_empty() {
        &entry.zh_title
    } else {
        &entry.item.title
    }
}

pub fn category(entries: &[GalleryEntry], id: &str, language: &str) -> String {
    let mut parts = Vec::new();
    let mut current = entries
        .iter()
        .find(|entry| entry.item.id == id)
        .and_then(|entry| entry.item.parent_id.as_deref());
    while let Some(parent_id) = current {
        let parent = entries
            .iter()
            .find(|entry| entry.item.id == parent_id)
            .expect("导航树已校验父节点");
        parts.push(title(parent, language).to_owned());
        current = parent.item.parent_id.as_deref();
    }
    parts.reverse();
    if parts.is_empty() {
        "Gallery".into()
    } else {
        parts.join(" / ")
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use slintforge::{NavTree, Router};

    #[test]
    fn manifest_registers_every_page_and_has_valid_tree() {
        let entries = entries();
        assert!(entries.len() > 100);
        NavTree::new(entries.iter().map(|entry| entry.item.clone()).collect()).unwrap();
        let routes = entries
            .iter()
            .filter(|entry| entry.item.kind == NavItemKind::Page)
            .map(|entry| entry.item.id.clone());
        Router::new(routes, "home").unwrap();
        for id in [
            "forge-window",
            "iconbutton",
            "card",
            "infobar",
            "navigation",
            "lifecycle",
            "chart-professional",
        ] {
            assert!(entries.iter().any(|entry| entry.item.id == id));
        }
        let plot = entries
            .iter()
            .find(|entry| entry.item.id == "chart-professional")
            .unwrap();
        assert_eq!(title(plot, "zh"), "专业图表");
        assert_eq!(title(plot, "en"), "ForgePlot");
        assert_eq!(plot.item.parent_id.as_deref(), Some("chart"));
        assert_eq!(
            entries
                .iter()
                .find(|entry| entry.item.parent_id.as_deref() == Some("chart"))
                .unwrap()
                .item
                .id,
            "chart-professional"
        );
    }

    #[test]
    fn every_navigation_level_has_a_chinese_label_and_english_restores() {
        let entries = entries();
        for entry in &entries {
            if entry.item.kind == NavItemKind::Separator {
                continue;
            }
            assert!(
                title(entry, "zh")
                    .chars()
                    .any(|c| ('\u{4e00}'..='\u{9fff}').contains(&c)),
                "导航项缺少中文名称: {}",
                entry.item.id
            );
            assert_eq!(title(entry, "en"), entry.item.title);
        }
        assert_eq!(
            category(&entries, "form-date-picker", "zh"),
            "表单 / 选择器"
        );
        assert_eq!(
            category(&entries, "form-date-picker", "en"),
            "Form / Picker"
        );
        assert_eq!(
            category(&entries, "inputs-filled-button", "zh"),
            "输入 / 按钮"
        );
    }
}
