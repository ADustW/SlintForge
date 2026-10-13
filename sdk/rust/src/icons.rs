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
}

impl Default for IconCatalog {
    fn default() -> Self {
        Self::new()
    }
}

impl IconCatalog {
    pub fn new() -> Self {
        let items = crate::runtime::value::<Vec<IconGlyph>, _>(
            slintforge_runtime_api::models::OP_ICONSALL,
            &slintforge_runtime_api::models::ArgsIconsAll { reserved: 0 },
        );
        Self { items }
    }
    pub fn len(&self) -> usize {
        self.items.len()
    }
    pub fn is_empty(&self) -> bool {
        self.items.is_empty()
    }
    pub fn find(&self, name: &str) -> Option<&IconGlyph> {
        use slintforge_runtime_api::{models::*, Arena, Wire};
        let name = name.to_owned();
        let mut arena = Arena::default();
        let index = crate::runtime::value::<Option<usize>, _>(
            OP_ICONSFIND,
            &ArgsIconsFind {
                name: name.view(&mut arena),
            },
        );
        index.and_then(|i| self.items.get(i))
    }
    pub fn search(&self, query: &str, offset: usize, limit: usize) -> (Vec<IconGlyph>, usize) {
        use slintforge_runtime_api::{models::*, Arena, Wire};
        let query = query.to_owned();
        let mut arena = Arena::default();
        crate::runtime::value(
            OP_ICONSSEARCH,
            &ArgsIconsSearch {
                query: query.view(&mut arena),
                offset,
                limit,
            },
        )
    }
}
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn complete_catalog_can_find_and_page_glyphs() {
        let catalog = IconCatalog::new();
        assert_eq!(catalog.len(), 9767);
        let home = catalog.find("home_20_regular").unwrap();
        assert_eq!(home.codepoint, 0xF480);
        assert_eq!(home.glyph(), "\u{f480}");
        let (first, total) = catalog.search("home", 0, 8);
        let (second, second_total) = catalog.search("home", 8, 8);
        assert!(total > 8);
        assert_eq!(total, second_total);
        assert_eq!(first.len(), 8);
        assert_ne!(first[0], second[0]);
        assert!(catalog.find("missing_icon").is_none());
    }
}
