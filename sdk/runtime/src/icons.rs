// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

use std::collections::HashMap;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct IconGlyph {
    pub name: String,
    pub codepoint: u32,
}

impl IconGlyph {
    pub fn glyph(&self) -> String {
        char::from_u32(self.codepoint)
            .expect("固定图标目录的码位有效")
            .to_string()
    }

    pub fn codepoint_hex(&self) -> String {
        format!("{:X}", self.codepoint)
    }
}

#[derive(Clone, Debug)]
pub struct IconCatalog {
    items: Vec<IconGlyph>,
    by_name: HashMap<String, usize>,
}

impl Default for IconCatalog {
    fn default() -> Self {
        Self::new()
    }
}

impl IconCatalog {
    pub fn new() -> Self {
        let mut items = Vec::new();
        let mut by_name = HashMap::new();
        for line in include_str!("../../../ui/icons/catalog.tsv").lines() {
            let (name, hex) = line.split_once('\t').expect("固定图标目录格式有效");
            let codepoint = u32::from_str_radix(hex, 16).expect("固定图标码位有效");
            by_name.insert(name.to_owned(), items.len());
            items.push(IconGlyph {
                name: name.to_owned(),
                codepoint,
            });
        }
        Self { items, by_name }
    }

    pub fn len(&self) -> usize {
        self.items.len()
    }

    pub fn is_empty(&self) -> bool {
        self.items.is_empty()
    }

    pub fn find(&self, name: &str) -> Option<&IconGlyph> {
        self.by_name.get(name).map(|index| &self.items[*index])
    }

    pub fn search(&self, query: &str, offset: usize, limit: usize) -> (Vec<IconGlyph>, usize) {
        let query = query.trim().to_ascii_lowercase();
        let mut count = 0;
        let mut page = Vec::with_capacity(limit.min(100));
        for item in &self.items {
            if !item.name.contains(&query) {
                continue;
            }
            if count >= offset && page.len() < limit {
                page.push(item.clone());
            }
            count += 1;
        }
        (page, count)
    }
}

impl IconCatalog {
    pub(crate) fn find_index(&self, name: &str) -> Option<usize> {
        self.by_name.get(name).copied()
    }
}
