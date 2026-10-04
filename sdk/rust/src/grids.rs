//! 表格的排序、树展开、筛选和稳定行标识操作；界面仍由共享 Slint 定义。
use std::cmp::Ordering;
use std::collections::{HashMap, HashSet};

#[derive(Clone, Debug, Default, PartialEq)]
pub struct GridCellData {
    pub text: String,
    pub value: f64,
}
#[derive(Clone, Debug, Default)]
pub struct GridColumnData {
    pub id: String,
    pub title: String,
    pub width: f64,
    pub min_width: f64,
    pub max_width: f64,
    pub numeric: bool,
    pub sortable: bool,
    pub editable: bool,
    pub hidden: bool,
    pub frozen: bool,
}
#[derive(Clone, Debug, Default)]
pub struct GridRowData {
    pub id: String,
    pub parent_id: String,
    pub cells: Vec<GridCellData>,
    pub expanded: bool,
    pub selected: bool,
    pub disabled: bool,
}
#[derive(Clone, Debug, Default)]
pub struct GridViewColumnData {
    pub column: GridColumnData,
    pub source_index: i32,
    pub x: f64,
    pub width: f64,
}
#[derive(Clone, Debug, Default)]
pub struct GridViewRowData {
    pub row: GridRowData,
    pub source_index: i32,
    pub depth: i32,
    pub has_children: bool,
    pub expanded: bool,
}
#[derive(Clone, Debug, Default)]
pub struct GridSceneData {
    pub status: i32,
    pub rows: Vec<GridViewRowData>,
    pub columns: Vec<GridViewColumnData>,
    pub total_width: f64,
    pub frozen_width: f64,
    pub selected_count: i32,
    pub all_visible_selected: bool,
}

fn bounded(value: f64, fallback: f64, minimum: f64, maximum: f64) -> f64 {
    if value.is_finite() && value > 0. {
        value.clamp(minimum, maximum)
    } else {
        fallback
    }
}
fn compare(
    a: &GridRowData,
    b: &GridRowData,
    column: usize,
    numeric: bool,
    descending: bool,
    left_key: &str,
    right_key: &str,
    use_keys: bool,
) -> Ordering {
    let left = a.cells.get(column);
    let right = b.cells.get(column);
    let valid = |cell: Option<&GridCellData>| cell.is_some_and(|c| !numeric || c.value.is_finite());
    if valid(left) != valid(right) {
        return if valid(left) {
            Ordering::Less
        } else {
            Ordering::Greater
        };
    }
    let order = match (left, right) {
        (Some(x), Some(y)) if numeric => x.value.partial_cmp(&y.value).unwrap_or(Ordering::Equal),
        (Some(_), Some(_)) if use_keys => left_key.cmp(right_key),
        // 小兄弟集合逐字节折叠比较，避免为每次比较创建两个字符串。
        (Some(x), Some(y)) => x.text.bytes().map(|c| c.to_ascii_lowercase())
            .cmp(y.text.bytes().map(|c| c.to_ascii_lowercase())),
        _ => Ordering::Equal,
    };
    if descending {
        order.reverse()
    } else {
        order
    }
}

pub fn prepare_grid(
    rows: &[GridRowData],
    columns: &[GridColumnData],
    tree: bool,
    sort_column: i32,
    descending: bool,
    filter: &str,
) -> GridSceneData {
    let mut scene = GridSceneData {
        selected_count: rows.iter().filter(|r| r.selected).count() as i32,
        ..Default::default()
    };
    let mut column_ids = HashSet::new();
    if columns
        .iter()
        .any(|c| c.id.is_empty() || !column_ids.insert(c.id.as_str()))
    {
        scene.status = 2;
        return scene;
    }
    for frozen in [true, false] {
        for (index, column) in columns
            .iter()
            .enumerate()
            .filter(|(_, c)| !c.hidden && c.frozen == frozen)
        {
            let minimum = bounded(column.min_width, 40., 32., 2048.);
            let maximum = bounded(column.max_width, 4096., minimum, 4096.).max(minimum);
            let width = bounded(column.width, 140., minimum, maximum).clamp(minimum, maximum);
            scene.columns.push(GridViewColumnData {
                column: column.clone(),
                source_index: index as i32,
                x: scene.total_width,
                width,
            });
            scene.total_width += width;
        }
        if frozen {
            scene.frozen_width = scene.total_width;
        }
    }
    let mut ids = HashMap::new();
    for (index, row) in rows.iter().enumerate() {
        if row.id.is_empty() || ids.insert(row.id.as_str(), index).is_some() {
            scene.status = 2;
            return scene;
        }
    }
    let mut children = vec![Vec::new(); rows.len()];
    let mut roots = Vec::new();
    for (index, row) in rows.iter().enumerate() {
        if tree && !row.parent_id.is_empty() {
            if let Some(&parent) = ids.get(row.parent_id.as_str()) {
                children[parent].push(index);
            } else {
                scene.status = 2;
                return scene;
            }
        } else {
            roots.push(index);
        }
    }
    // 迭代拓扑遍历检查循环，并避免深层树递归造成栈溢出。
    let mut order = roots.clone();
    let mut cursor = 0;
    while cursor < order.len() {
        let index = order[cursor];
        order.extend(&children[index]);
        cursor += 1;
    }
    if order.len() != rows.len() {
        scene.status = 2;
        return scene;
    }
    let query = filter
        .trim_matches(|c: char| c.is_ascii_whitespace())
        .to_ascii_lowercase();
    let mut included: Vec<bool> = rows
        .iter()
        .map(|r| {
            query.is_empty()
                || r.cells
                    .iter()
                    .any(|c| c.text.to_ascii_lowercase().contains(&query))
        })
        .collect();
    if tree && !query.is_empty() {
        for &index in order.iter().rev() {
            if included[index] && !rows[index].parent_id.is_empty() {
                included[ids[rows[index].parent_id.as_str()]] = true;
            }
        }
    }
    if let Some(column) = usize::try_from(sort_column)
        .ok()
        .and_then(|index| columns.get(index))
    {
        if column.sortable {
            // 每行只折叠一次，避免 O(n log n) 次比较重复分配字符串。
            let use_keys=!column.numeric&&(roots.len()>=32||children.iter().any(|group|group.len()>=32));
            let text_keys = if !use_keys { Vec::new() } else {
                rows.iter().map(|row|row.cells.get(sort_column as usize).map_or_else(String::new,|cell|cell.text.to_ascii_lowercase())).collect::<Vec<_>>()
            };
            let compare_indices = |a: &usize, b: &usize| {
                compare(
                    &rows[*a],
                    &rows[*b],
                    sort_column as usize,
                    column.numeric,
                    descending,
                    if use_keys {&text_keys[*a]} else {""},
                    if use_keys {&text_keys[*b]} else {""},
                    use_keys,
                )
                .then(a.cmp(b))
            };
            roots.sort_by(compare_indices);
            for siblings in &mut children {
                siblings.sort_by(compare_indices);
            }
        }
    }
    let mut stack: Vec<(usize, i32)> = roots.into_iter().rev().map(|i| (i, 0)).collect();
    while let Some((index, depth)) = stack.pop() {
        if !included[index] {
            continue;
        }
        let expanded = rows[index].expanded || !query.is_empty();
        let mut row = rows[index].clone();
        row.cells
            .resize(columns.len().max(row.cells.len()), GridCellData::default());
        scene.rows.push(GridViewRowData {
            row,
            source_index: index as i32,
            depth,
            has_children: !children[index].is_empty(),
            expanded,
        });
        if expanded {
            for &child in children[index].iter().rev() {
                stack.push((child, depth + 1));
            }
        }
    }
    let selectable: Vec<_> = scene.rows.iter().filter(|v| !v.row.disabled).collect();
    scene.all_visible_selected =
        !selectable.is_empty() && selectable.iter().all(|v| v.row.selected);
    if scene.rows.is_empty() || scene.columns.is_empty() {
        scene.status = 1;
    }
    scene
}

// 0 单选、1 切换、2 范围、3 追加范围、4 可视全选、5 清空、6 切换展开、7 展开、8 折叠、9 全部展开、10 全部折叠、11 删除所选、12 选择可见行。
pub fn update_grid(
    rows: &[GridRowData],
    visible: &[i32],
    operation: i32,
    target: i32,
    anchor: i32,
) -> Vec<GridRowData> {
    let mut result = rows.to_vec();
    let valid = |index: i32| usize::try_from(index).ok().filter(|&i| i < rows.len());
    if operation == 11 {
        let mut removed: HashSet<String> = rows
            .iter()
            .filter(|r| r.selected)
            .map(|r| r.id.clone())
            .collect();
        if target == 1 {
            let mut children: HashMap<&str, Vec<&str>> = HashMap::new();
            for row in rows {
                children.entry(&row.parent_id).or_default().push(&row.id);
            }
            let mut queue: Vec<String> = removed.iter().cloned().collect();
            let mut cursor = 0;
            while cursor < queue.len() {
                if let Some(children) = children.get(queue[cursor].as_str()) {
                    for id in children {
                        if removed.insert((*id).into()) {
                            queue.push((*id).into());
                        }
                    }
                }
                cursor += 1;
            }
        }
        result.retain(|row| !removed.contains(&row.id));
        return result;
    }
    if operation == 5 {
        for row in &mut result {
            row.selected = false;
        }
        return result;
    }
    if operation == 9 || operation == 10 {
        for row in &mut result {
            if !row.disabled {
                row.expanded = operation == 9;
            }
        }
        return result;
    }
    if operation == 4 || operation == 12 {
        let all = visible
            .iter()
            .filter_map(|&i| valid(i))
            .filter(|&i| !rows[i].disabled)
            .all(|i| rows[i].selected);
        for i in visible.iter().filter_map(|&i| valid(i)) {
            if !result[i].disabled {
                result[i].selected = operation == 12 || !all;
            }
        }
        return result;
    }
    let Some(target) = valid(target) else {
        return result;
    };
    if rows[target].disabled {
        return result;
    }
    match operation {
        0 => {
            for row in &mut result {
                row.selected = false;
            }
            result[target].selected = true;
        }
        1 => result[target].selected = !result[target].selected,
        2 | 3 => {
            let Some(end) = visible.iter().position(|&i| i == target as i32) else {
                return result;
            };
            let start = visible.iter().position(|&i| i == anchor).unwrap_or(end);
            if operation == 2 {
                for row in &mut result {
                    row.selected = false;
                }
            }
            for &index in &visible[start.min(end)..=start.max(end)] {
                if let Some(i) = valid(index) {
                    if !result[i].disabled {
                        result[i].selected = true;
                    }
                }
            }
        }
        6 => result[target].expanded = !result[target].expanded,
        7 => result[target].expanded = true,
        8 => result[target].expanded = false,
        _ => {}
    }
    result
}

pub fn commit_grid(
    rows: &[GridRowData],
    columns: &[GridColumnData],
    row: i32,
    column: i32,
    text: &str,
) -> (i32, Vec<GridRowData>) {
    let mut result = rows.to_vec();
    let (Some(r), Some(c)) = (
        usize::try_from(row).ok().filter(|&i| i < rows.len()),
        usize::try_from(column).ok().filter(|&i| i < columns.len()),
    ) else {
        return (1, result);
    };
    if rows[r].disabled || !columns[c].editable || columns[c].hidden {
        return (1, result);
    }
    let value = if columns[c].numeric {
        let input = text.trim_matches(|c: char| c.is_ascii_whitespace());
        let nonzero = input
            .split(['e', 'E'])
            .next()
            .unwrap_or("")
            .bytes()
            .any(|b| (b'1'..=b'9').contains(&b));
        match input.parse::<f32>() {
            Ok(value) if value.is_finite() && (value != 0. || !nonzero) => value as f64,
            _ => return (2, result),
        }
    } else {
        rows[r].cells.get(c).map_or(0., |cell| cell.value)
    };
    let length = columns.len().max(result[r].cells.len());
    result[r].cells.resize(length, GridCellData::default());
    result[r].cells[c] = GridCellData {
        text: text.into(),
        value,
    };
    (0, result)
}

pub fn step_grid(disabled: &[bool], current: i32, delta: i32) -> i32 {
    if disabled.is_empty() {
        return -1;
    }
    let direction = if delta < 0 { -1 } else { 1 };
    let mut index = if current < 0 {
        if direction > 0 {
            0
        } else {
            disabled.len() as i32 - 1
        }
    } else {
        (current.saturating_add(delta)).clamp(0, disabled.len() as i32 - 1)
    };
    while index >= 0 && index < disabled.len() as i32 {
        if !disabled[index as usize] {
            return index;
        }
        index += direction;
    }
    if current >= 0 && (current as usize) < disabled.len() && !disabled[current as usize] {
        current
    } else {
        -1
    }
}

// 生成类型保持在应用中，SDK 通过宏连接同一份表格契约。
#[macro_export]
macro_rules! bind_grid_services {
    ($ui:expr, $service:ty, $scene:ident, $view_row:ident, $view_column:ident, $row:ident, $cell:ident, $column:ident, $edit:ident) => {{
        use slint::{ComponentHandle, Model};
        fn read_rows(model: slint::ModelRc<$row>) -> Vec<$crate::grids::GridRowData> {
            model.iter().map(|r| $crate::grids::GridRowData {
                id: r.id.to_string(), parent_id: r.parent_id.to_string(), expanded: r.expanded,
                selected: r.selected, disabled: r.disabled,
                cells: r.cells.iter().map(|c| $crate::grids::GridCellData { text: c.text.to_string(), value: c.value as f64 }).collect(),
            }).collect()
        }
        fn read_columns(model: slint::ModelRc<$column>) -> Vec<$crate::grids::GridColumnData> {
            model.iter().map(|c| $crate::grids::GridColumnData {
                id: c.id.to_string(), title: c.title.to_string(), width: c.width as f64,
                min_width: c.min_width as f64, max_width: c.max_width as f64,
                numeric: c.numeric, sortable: c.sortable, editable: c.editable, hidden: c.hidden, frozen: c.frozen,
            }).collect()
        }
        fn cells(values: Vec<$crate::grids::GridCellData>) -> slint::ModelRc<$cell> {
            slint::ModelRc::new(slint::VecModel::from(values.into_iter().map(|c| $cell { text: c.text.into(), value: c.value as f32 }).collect::<Vec<_>>()))
        }
        fn write_column(c: $crate::grids::GridColumnData) -> $column {
            $column { id: c.id.into(), title: c.title.into(), width: c.width as f32,
                min_width: c.min_width as f32, max_width: c.max_width as f32,
                numeric: c.numeric, sortable: c.sortable, editable: c.editable, hidden: c.hidden, frozen: c.frozen }
        }
        fn write_rows(values: Vec<$crate::grids::GridRowData>) -> slint::ModelRc<$row> {
            slint::ModelRc::new(slint::VecModel::from(values.into_iter().map(|r| $row {
                id: r.id.into(), parent_id: r.parent_id.into(), cells: cells(r.cells), expanded: r.expanded,
                selected: r.selected, disabled: r.disabled,
            }).collect::<Vec<_>>()))
        }
        let service = $ui.global::<$service>();
        service.on_prepare(|rows, columns, tree, sort, descending, filter, _revision| {
            let source = $crate::grids::prepare_grid(&read_rows(rows), &read_columns(columns), tree, sort, descending, filter.as_str());
            $scene { status: source.status, total_width: source.total_width as f32,
                frozen_width: source.frozen_width as f32, selected_count: source.selected_count,
                all_visible_selected: source.all_visible_selected,
                rows: slint::ModelRc::new(slint::VecModel::from(source.rows.into_iter().map(|v| $view_row {
                    id: v.row.id.into(), parent_id: v.row.parent_id.into(), cells: cells(v.row.cells),
                    source_index: v.source_index, depth: v.depth, has_children: v.has_children, expanded: v.expanded,
                    selected: v.row.selected, disabled: v.row.disabled,
                }).collect::<Vec<_>>())),
                columns: slint::ModelRc::new(slint::VecModel::from(source.columns.into_iter().map(|v| $view_column {
                    column: write_column(v.column), source_index: v.source_index, x: v.x as f32, width: v.width as f32,
                }).collect::<Vec<_>>())),
            }
        });
        service.on_update(|rows, view, operation, target, anchor| {
            let source = read_rows(rows.clone());
            let updated = $crate::grids::update_grid(&source, &view.iter().map(|v| v.source_index).collect::<Vec<_>>(), operation, target, anchor);
            // 无变化的选择操作保留模型，防止第二次点击销毁原生鼠标抓取目标。
            if source.len() == updated.len() && source.iter().zip(&updated).all(|(a, b)| a.selected == b.selected && a.expanded == b.expanded) { rows }
            else { write_rows(updated) }
        });
        service.on_commit(|rows, columns, row, column, text| {
            let (status, rows) = $crate::grids::commit_grid(&read_rows(rows), &read_columns(columns), row, column, text.as_str());
            $edit { status, rows: write_rows(rows) }
        });
        service.on_find_row(|rows, id| rows.iter().position(|r| r.id == id).map_or(-1, |i| i as i32));
        service.on_find_column(|columns, source| columns.iter().position(|c| c.source_index == source).map_or(-1, |i| i as i32));
        service.on_filter_active(|filter| !filter.trim_matches(|c:char|c.is_ascii_whitespace()).is_empty());
        service.on_step(|rows, current, delta| $crate::grids::step_grid(&rows.iter().map(|r| r.disabled).collect::<Vec<_>>(), current, delta));
        service.set_connected(true);
    }};
}

#[cfg(test)]
mod tests {
    use super::*;
    fn columns() -> Vec<GridColumnData> {
        vec![
            GridColumnData {
                id: "name".into(),
                title: "名称".into(),
                width: 160.,
                sortable: true,
                editable: true,
                frozen: true,
                ..Default::default()
            },
            GridColumnData {
                id: "count".into(),
                title: "数量".into(),
                width: 80.,
                numeric: true,
                sortable: true,
                editable: true,
                ..Default::default()
            },
        ]
    }
    fn row(id: &str, parent: &str, text: &str, value: f64) -> GridRowData {
        GridRowData {
            id: id.into(),
            parent_id: parent.into(),
            expanded: true,
            cells: vec![
                GridCellData {
                    text: text.into(),
                    value: 0.,
                },
                GridCellData {
                    text: value.to_string(),
                    value,
                },
            ],
            ..Default::default()
        }
    }
    fn rows() -> Vec<GridRowData> {
        let mut rows = vec![
            row("b", "root", "Beta", 10.),
            row("a", "root", "alpha", 2.),
            row("root", "", "Root", 12.),
            row("z", "", "Archive", 24.),
        ];
        rows[3].disabled = true;
        rows
    }
    #[test]
    fn grid_tree_sort_filter_validation_and_deep_models() {
        let mut rows = rows();
        let columns = columns();
        let ids = |s: GridSceneData| {
            s.rows
                .into_iter()
                .map(|v| (v.row.id, v.depth))
                .collect::<Vec<_>>()
        };
        assert_eq!(
            ids(prepare_grid(&rows, &columns, true, -1, false, "")),
            [
                ("root".into(), 0),
                ("b".into(), 1),
                ("a".into(), 1),
                ("z".into(), 0)
            ]
        );
        assert_eq!(
            ids(prepare_grid(&rows, &columns, true, 1, false, "")),
            [
                ("root".into(), 0),
                ("a".into(), 1),
                ("b".into(), 1),
                ("z".into(), 0)
            ]
        );
        rows[2].expanded = false;
        assert_eq!(
            prepare_grid(&rows, &columns, true, -1, false, "")
                .rows
                .len(),
            2
        );
        assert_eq!(
            ids(prepare_grid(&rows, &columns, true, -1, false, " ALPHA ")),
            [("root".into(), 0), ("a".into(), 1)]
        );
        assert_eq!(
            prepare_grid(&rows, &columns, true, -1, false, "absent").status,
            1
        );
        assert_eq!(
            prepare_grid(&rows, &columns, false, 1, false, "").rows[0]
                .row
                .id,
            "a"
        );
        let mut invalid = rows.clone();
        invalid[0].id = "root".into();
        assert_eq!(
            prepare_grid(&invalid, &columns, true, -1, false, "").status,
            2
        );
        invalid = rows.clone();
        invalid[0].parent_id = "missing".into();
        assert_eq!(
            prepare_grid(&invalid, &columns, true, -1, false, "").status,
            2
        );
        invalid = rows.clone();
        invalid[2].parent_id = "b".into();
        assert_eq!(
            prepare_grid(&invalid, &columns, true, -1, false, "").status,
            2
        );
        let deep = (0..10000)
            .map(|i| {
                row(
                    &format!("n{i}"),
                    &if i == 0 {
                        "".into()
                    } else {
                        format!("n{}", i - 1)
                    },
                    "深层节点",
                    i as f64,
                )
            })
            .collect::<Vec<_>>();
        let scene = prepare_grid(&deep, &columns, true, -1, false, "");
        assert_eq!(scene.rows.len(), 10000);
        assert_eq!(scene.rows.last().unwrap().depth, 9999);
    }
    #[test]
    fn grid_selection_uses_visible_order_and_deletes_subtrees() {
        let rows = rows();
        let visible = [2, 1, 0, 3];
        let selected = update_grid(&rows, &visible, 2, 0, 2);
        assert!(
            selected[0].selected
                && selected[1].selected
                && selected[2].selected
                && !selected[3].selected
        );
        let selected = update_grid(&selected, &visible, 1, 1, -1);
        assert!(!selected[1].selected && selected[0].selected);
        let selected = update_grid(&selected, &[1], 12, -1, -1);
        assert!(selected[1].selected && selected[0].selected);
        let selected = update_grid(&selected, &[1], 4, -1, -1);
        assert!(!selected[1].selected && selected[0].selected);
        let selected = update_grid(&rows, &visible, 0, 2, -1);
        assert_eq!(
            update_grid(&selected, &visible, 11, 1, -1)
                .iter()
                .map(|r| r.id.as_str())
                .collect::<Vec<_>>(),
            ["z"]
        );
        assert_eq!(update_grid(&selected, &visible, 11, 0, -1).len(), 3);
        assert_eq!(step_grid(&[true, false, true, false], -1, 1), 1);
        assert_eq!(step_grid(&[true, false, true, false], 1, 1), 3);
        assert_eq!(step_grid(&[true, true], -1, 1), -1);
        assert_eq!(step_grid(&[false, false], 0, i32::MAX), 1);
    }
    #[test]
    fn grid_numeric_edits_missing_cells_and_column_geometry() {
        let rows = rows();
        let mut columns = columns();
        let (status, edited) = commit_grid(&rows, &columns, 0, 1, " +3.5 ");
        assert_eq!(status, 0);
        assert_eq!(edited[0].cells[1].value, 3.5);
        for text in ["NaN", "inf", "1e100", "1e-100", "", "+-3", "2x"] {
            assert_eq!(commit_grid(&rows, &columns, 0, 1, text).0, 2, "{text}");
        }
        assert_eq!(commit_grid(&rows, &columns, 3, 1, "30").0, 1);
        columns[1].editable = false;
        assert_eq!(commit_grid(&rows, &columns, 0, 1, "30").0, 1);
        columns[0].min_width = 100.;
        columns[0].max_width = 200.;
        columns[0].width = f64::INFINITY;
        columns[1].width = 9000.;
        columns[1].max_width = 240.;
        let scene = prepare_grid(
            &[GridRowData {
                id: "empty".into(),
                ..Default::default()
            }],
            &columns,
            false,
            -1,
            false,
            "",
        );
        assert_eq!(scene.rows[0].row.cells.len(), 2);
        assert_eq!(scene.total_width, 380.);
        assert_eq!(scene.frozen_width, 140.);
        columns[0].hidden = true;
        assert_eq!(
            prepare_grid(&rows, &columns, false, -1, false, "").columns[0].source_index,
            1
        );
    }
    #[test]
    fn grid_contract_for_both_hosts() {
        use std::fmt::Write;
        let mut output = String::new();
        let rows = rows();
        let mut columns = columns();
        for case in 0..10 {
            let mut data = rows.clone();
            if case == 4 {
                data[2].expanded = false;
            }
            if case == 5 {
                data[0].selected = true;
            }
            if case == 6 {
                data[0].parent_id = "invalid".into();
            }
            if case == 7 {
                columns[1].hidden = true;
            }
            if case == 8 {
                data[0].cells.clear();
            }
            if case == 9 {
                data[1].cells[1].value = f64::NAN;
            }
            let scene = prepare_grid(
                &data,
                &columns,
                case != 0,
                if case == 1 || case == 2 { 1 } else { -1 },
                case == 2,
                if case == 3 { "beta" } else { "" },
            );
            writeln!(
                output,
                "scene,{case},{},{},{},{},{}",
                scene.status,
                scene.total_width,
                scene.frozen_width,
                scene.selected_count,
                i32::from(scene.all_visible_selected)
            )
            .unwrap();
            for c in scene.columns {
                writeln!(
                    output,
                    "column,{case},{},{},{},{},{}",
                    c.column.id,
                    c.source_index,
                    c.x,
                    c.width,
                    i32::from(c.column.frozen)
                )
                .unwrap();
            }
            for r in scene.rows {
                writeln!(
                    output,
                    "row,{case},{},{},{},{},{},{}",
                    r.row.id,
                    r.source_index,
                    r.depth,
                    i32::from(r.has_children),
                    i32::from(r.expanded),
                    i32::from(r.row.disabled)
                )
                .unwrap();
            }
        }
        let path = std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join("../../build/grids-qa");
        std::fs::create_dir_all(&path).unwrap();
        std::fs::write(path.join("rust-contract.csv"), output).unwrap();
    }
}
