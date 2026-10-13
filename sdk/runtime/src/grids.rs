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
        (Some(x), Some(y)) => x
            .text
            .bytes()
            .map(|c| c.to_ascii_lowercase())
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
            let use_keys = !column.numeric
                && (roots.len() >= 32 || children.iter().any(|group| group.len() >= 32));
            let text_keys = if !use_keys {
                Vec::new()
            } else {
                rows.iter()
                    .map(|row| {
                        row.cells
                            .get(sort_column as usize)
                            .map_or_else(String::new, |cell| cell.text.to_ascii_lowercase())
                    })
                    .collect::<Vec<_>>()
            };
            let compare_indices = |a: &usize, b: &usize| {
                compare(
                    &rows[*a],
                    &rows[*b],
                    sort_column as usize,
                    column.numeric,
                    descending,
                    if use_keys { &text_keys[*a] } else { "" },
                    if use_keys { &text_keys[*b] } else { "" },
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
