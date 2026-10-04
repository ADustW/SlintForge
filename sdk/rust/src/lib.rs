//! SlintForge 的 Rust 宿主模型。视觉组件由 ui/slintforge.slint 统一定义。

mod icons;
pub use icons::{IconCatalog, IconGlyph};
mod window_corners;
pub use window_corners::apply_window_corners;
mod staggered_layout;
pub use staggered_layout::{StaggeredInput, StaggeredLayout, StaggeredPlacement};
mod capture;
mod widgets;
pub mod charts;
pub mod plot;
mod plot_bindings;
pub mod grids;
pub use widgets::{highlight_syntax, process_backdrop, qr_code_pixels, SyntaxLine};
pub use capture::{
    capture_region, capture_window, crop_capture, save_capture_png, CaptureError, CaptureRect,
};

use std::collections::{HashMap, HashSet};
use std::path::PathBuf;

#[derive(Clone, Debug, PartialEq, Eq)]
pub enum Error {
    EmptyId,
    DuplicateId(String),
    UnknownRoute(String),
    UnknownParent(String),
    Cycle(String),
    InvalidParent(String),
    NotGroup(String),
    NoHistory,
}

impl std::fmt::Display for Error {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::EmptyId => write!(f, "路由或导航 ID 不能为空"),
            Self::DuplicateId(id) => write!(f, "重复 ID: {id}"),
            Self::UnknownRoute(id) => write!(f, "未知路由: {id}"),
            Self::UnknownParent(id) => write!(f, "未知父节点: {id}"),
            Self::Cycle(id) => write!(f, "导航树存在环: {id}"),
            Self::InvalidParent(id) => write!(f, "父节点不是分组: {id}"),
            Self::NotGroup(id) => write!(f, "节点不是分组: {id}"),
            Self::NoHistory => write!(f, "没有可返回的历史页面"),
        }
    }
}

impl std::error::Error for Error {}

pub type Result<T> = std::result::Result<T, Error>;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct RouteParam {
    pub key: String,
    pub value: String,
}

impl RouteParam {
    pub fn new(key: impl Into<String>, value: impl Into<String>) -> Self {
        Self {
            key: key.into(),
            value: value.into(),
        }
    }
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct RouteState {
    pub id: String,
    pub params: Vec<RouteParam>,
}

impl RouteState {
    pub fn new(id: impl Into<String>, params: Vec<RouteParam>) -> Self {
        Self {
            id: id.into(),
            params,
        }
    }
}

#[derive(Clone, Debug)]
pub struct Router {
    registered: HashSet<String>,
    current: RouteState,
    history: Vec<RouteState>,
}

impl Router {
    pub fn new(ids: impl IntoIterator<Item = impl Into<String>>, initial: &str) -> Result<Self> {
        let mut registered = HashSet::new();
        for id in ids {
            let id = id.into();
            if id.is_empty() {
                return Err(Error::EmptyId);
            }
            if !registered.insert(id.clone()) {
                return Err(Error::DuplicateId(id));
            }
        }
        if initial.is_empty() {
            return Err(Error::EmptyId);
        }
        if !registered.contains(initial) {
            return Err(Error::UnknownRoute(initial.into()));
        }
        Ok(Self {
            registered,
            current: RouteState::new(initial, vec![]),
            history: vec![],
        })
    }

    fn ensure_route(&self, id: &str) -> Result<()> {
        if id.is_empty() {
            return Err(Error::EmptyId);
        }
        if !self.registered.contains(id) {
            return Err(Error::UnknownRoute(id.into()));
        }
        Ok(())
    }

    pub fn go(&mut self, id: &str, params: Vec<RouteParam>) -> Result<bool> {
        self.ensure_route(id)?;
        let next = RouteState::new(id, params);
        if next == self.current {
            return Ok(false);
        }
        self.history
            .push(std::mem::replace(&mut self.current, next));
        Ok(true)
    }

    pub fn replace(&mut self, id: &str, params: Vec<RouteParam>) -> Result<bool> {
        self.ensure_route(id)?;
        let next = RouteState::new(id, params);
        if next == self.current {
            return Ok(false);
        }
        self.current = next;
        Ok(true)
    }

    pub fn back(&mut self) -> Result<&RouteState> {
        self.current = self.history.pop().ok_or(Error::NoHistory)?;
        Ok(&self.current)
    }

    pub fn current(&self) -> &RouteState {
        &self.current
    }
    pub fn can_go_back(&self) -> bool {
        !self.history.is_empty()
    }
    pub fn history_len(&self) -> usize {
        self.history.len()
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum NavItemKind {
    Page,
    Group,
    Header,
    Separator,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct NavItem {
    pub id: String,
    pub parent_id: Option<String>,
    pub title: String,
    pub icon: String,
    pub kind: NavItemKind,
    pub enabled: bool,
}

impl NavItem {
    pub fn new(
        id: &str,
        parent_id: Option<&str>,
        title: &str,
        icon: &str,
        kind: NavItemKind,
    ) -> Self {
        Self {
            id: id.into(),
            parent_id: parent_id.map(str::to_owned),
            title: title.into(),
            icon: icon.into(),
            kind,
            enabled: true,
        }
    }
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct VisibleNavItem {
    pub item: NavItem,
    pub depth: usize,
    pub expanded: bool,
    pub has_children: bool,
}

#[derive(Clone, Debug)]
pub struct NavTree {
    items: Vec<NavItem>,
    by_id: HashMap<String, usize>,
    children: HashMap<String, Vec<usize>>,
    roots: Vec<usize>,
    expanded: HashSet<String>,
}

impl NavTree {
    pub fn new(items: Vec<NavItem>) -> Result<Self> {
        let mut by_id = HashMap::new();
        for (index, item) in items.iter().enumerate() {
            if item.id.is_empty() {
                return Err(Error::EmptyId);
            }
            if by_id.insert(item.id.clone(), index).is_some() {
                return Err(Error::DuplicateId(item.id.clone()));
            }
        }
        let mut children: HashMap<String, Vec<usize>> = HashMap::new();
        let mut roots = vec![];
        let mut parents = vec![None; items.len()];
        for (index, item) in items.iter().enumerate() {
            if let Some(parent) = item
                .parent_id
                .as_deref()
                .filter(|parent| !parent.is_empty())
            {
                let parent_index = *by_id
                    .get(parent)
                    .ok_or_else(|| Error::UnknownParent(parent.into()))?;
                if items[parent_index].kind != NavItemKind::Group {
                    return Err(Error::InvalidParent(parent.into()));
                }
                parents[index] = Some(parent_index);
                children.entry(parent.into()).or_default().push(index);
            } else {
                roots.push(index);
            }
        }
        // 三色标记使每个父链节点只检查一次，深层树的校验保持 O(n)。
        let mut colors = vec![0u8; items.len()];
        let mut path = Vec::new();
        for index in 0..items.len() {
            let mut cursor = Some(index);
            while let Some(current) = cursor {
                if colors[current] == 2 { break; }
                if colors[current] == 1 { return Err(Error::Cycle(items[current].id.clone())); }
                colors[current] = 1;
                path.push(current);
                cursor = parents[current];
            }
            for current in path.drain(..) { colors[current] = 2; }
        }
        Ok(Self {
            items,
            by_id,
            children,
            roots,
            expanded: HashSet::new(),
        })
    }

    pub fn toggle(&mut self, id: &str) -> Result<bool> {
        if id.is_empty() {
            return Err(Error::EmptyId);
        }
        let index = *self
            .by_id
            .get(id)
            .ok_or_else(|| Error::UnknownRoute(id.into()))?;
        if self.items[index].kind != NavItemKind::Group {
            return Err(Error::NotGroup(id.into()));
        }
        if self.expanded.remove(id) {
            Ok(false)
        } else {
            self.expanded.insert(id.into());
            Ok(true)
        }
    }

    pub fn set_expanded(&mut self, id: &str, expanded: bool) -> Result<()> {
        if id.is_empty() {
            return Err(Error::EmptyId);
        }
        let index = *self
            .by_id
            .get(id)
            .ok_or_else(|| Error::UnknownRoute(id.into()))?;
        if self.items[index].kind != NavItemKind::Group {
            return Err(Error::NotGroup(id.into()));
        }
        if expanded {
            self.expanded.insert(id.into());
        } else {
            self.expanded.remove(id);
        }
        Ok(())
    }

    pub fn visible_items(&self) -> Vec<VisibleNavItem> {
        let mut out = vec![];
        // 显式栈保留先序顺序，展开层级不再占用线程调用栈。
        let mut stack: Vec<_> = self.roots.iter().rev().map(|&index| (index, 0)).collect();
        while let Some((index, depth)) = stack.pop() {
            let item = &self.items[index];
            let expanded = self.expanded.contains(&item.id);
            let children = self.children.get(&item.id);
            out.push(VisibleNavItem {
                item: item.clone(), depth, expanded,
                has_children: children.is_some_and(|c| !c.is_empty()),
            });
            if expanded {
                if let Some(children) = children {
                    stack.extend(children.iter().rev().map(|&child| (child, depth + 1)));
                }
            }
        }
        out
    }
}

/// build.rs 中传给 `CompilerConfiguration::with_library_paths` 的入口。
pub fn ui_library_path() -> PathBuf {
    PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("../../ui/slintforge.slint")
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn router_keeps_state_on_error_and_deduplicates() {
        let mut r = Router::new(["home", "detail"], "home").unwrap();
        assert!(!r.go("home", vec![]).unwrap());
        assert_eq!(r.history_len(), 0);
        assert_eq!(
            r.go("missing", vec![]),
            Err(Error::UnknownRoute("missing".into()))
        );
        assert_eq!(r.current().id, "home");
        r.go("detail", vec![RouteParam::new("id", "7")]).unwrap();
        assert!(!r.go("detail", vec![RouteParam::new("id", "7")]).unwrap());
        assert_eq!(r.history_len(), 1);
        assert_eq!(r.back().unwrap().id, "home");
        assert_eq!(r.back(), Err(Error::NoHistory));
        r.go("detail", vec![RouteParam::new("id", "8")]).unwrap();
        r.replace("home", vec![]).unwrap();
        assert_eq!(r.history_len(), 1);
        assert_eq!(r.current().id, "home");
        assert_eq!(r.go("", vec![]), Err(Error::EmptyId));
        assert_eq!(r.current().id, "home");
        assert!(matches!(
            Router::new(["home", "home"], "home"),
            Err(Error::DuplicateId(_))
        ));
    }

    #[test]
    fn tree_expands_multilevel_and_rejects_invalid_shapes() {
        let items = vec![
            NavItem::new("root", None, "Root", "R", NavItemKind::Group),
            NavItem::new("sub", Some("root"), "Sub", "S", NavItemKind::Group),
            NavItem::new("leaf", Some("sub"), "Leaf", "L", NavItemKind::Page),
        ];
        let mut tree = NavTree::new(items).unwrap();
        assert_eq!(tree.visible_items().len(), 1);
        tree.toggle("root").unwrap();
        assert_eq!(tree.visible_items().len(), 2);
        tree.toggle("sub").unwrap();
        assert_eq!(tree.visible_items()[2].depth, 2);
        assert!(matches!(
            NavTree::new(vec![NavItem::new(
                "x",
                Some("x"),
                "X",
                "",
                NavItemKind::Group
            )]),
            Err(Error::Cycle(_))
        ));
        assert!(matches!(
            NavTree::new(vec![
                NavItem::new("x", None, "X", "", NavItemKind::Page),
                NavItem::new("x", None, "X", "", NavItemKind::Page)
            ]),
            Err(Error::DuplicateId(_))
        ));
        assert!(matches!(
            NavTree::new(vec![NavItem::new(
                "child",
                Some("absent"),
                "X",
                "",
                NavItemKind::Page
            )]),
            Err(Error::UnknownParent(_))
        ));
        assert!(matches!(
            NavTree::new(vec![
                NavItem::new("parent", None, "P", "", NavItemKind::Page),
                NavItem::new("child", Some("parent"), "C", "", NavItemKind::Page)
            ]),
            Err(Error::InvalidParent(_))
        ));
        assert_eq!(tree.toggle("leaf"), Err(Error::NotGroup("leaf".into())));
        assert_eq!(tree.visible_items().len(), 3);
    }
}
