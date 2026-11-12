// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

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
use slintforge_runtime_api::models::*;
use slintforge_runtime_api::{Arena, Wire};
pub fn prepare_grid(
    rows: &[GridRowData],
    columns: &[GridColumnData],
    tree: bool,
    sort_column: i32,
    descending: bool,
    filter: &str,
) -> GridSceneData {
    let mut arena = Arena::default();
    let filter = filter.to_owned();
    let rows = rows.to_vec();
    let columns = columns.to_vec();
    let args = ArgsGridPrepare {
        rows: rows.view(&mut arena),
        columns: columns.view(&mut arena),
        tree: u8::from(tree),
        sort_column,
        descending: u8::from(descending),
        filter: filter.view(&mut arena),
    };
    crate::runtime::value_or::<GridSceneData, _>(OP_GRIDPREPARE, &args, GridSceneData::default)
}
pub fn update_grid(
    rows: &[GridRowData],
    visible: &[i32],
    operation: i32,
    target: i32,
    anchor: i32,
) -> Vec<GridRowData> {
    let mut arena = Arena::default();
    let rows = rows.to_vec();
    let args = ArgsGridUpdate {
        rows: rows.view(&mut arena),
        visible: visible.to_vec().view(&mut arena),
        operation,
        target,
        anchor,
    };
    crate::runtime::value_or::<Vec<GridRowData>, _>(OP_GRIDUPDATE, &args, || rows.clone())
}
pub fn commit_grid(
    rows: &[GridRowData],
    columns: &[GridColumnData],
    row: i32,
    column: i32,
    text: &str,
) -> (i32, Vec<GridRowData>) {
    let mut arena = Arena::default();
    let rows = rows.to_vec();
    let columns = columns.to_vec();
    let text = text.to_owned();
    let args = ArgsGridCommit {
        rows: rows.view(&mut arena),
        columns: columns.view(&mut arena),
        row,
        column,
        text: text.view(&mut arena),
    };
    // 失败时按“不可编辑”（1）拒绝提交，保留原行。
    crate::runtime::value_or::<(i32, Vec<GridRowData>), _>(OP_GRIDCOMMIT, &args, || (1, rows.clone()))
}
pub fn step_grid(disabled: &[bool], current: i32, delta: i32) -> i32 {
    let mut arena = Arena::default();
    let disabled = disabled.to_vec();
    let args = ArgsGridStep {
        disabled: disabled.view(&mut arena),
        current,
        delta,
    };
    crate::runtime::value_or::<i32, _>(OP_GRIDSTEP, &args, || current)
}
#[macro_export]
macro_rules! bind_grid_services {
    ($ui:expr, $service:ty, $scene:ident, $view_row:ident, $view_column:ident, $row:ident, $cell:ident, $column:ident, $edit:ident) => {{
        use slint::{ComponentHandle, Model};
        $crate::runtime::initialize().expect("表格运行库初始化失败");
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
