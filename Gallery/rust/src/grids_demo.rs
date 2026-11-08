// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

use crate::{
    ForgeGridCell, ForgeGridColumn, ForgeGridEditResult, ForgeGridRow, ForgeGridScene,
    ForgeGridServices, ForgeGridViewColumn, ForgeGridViewRow, GalleryGridDemoModels, GalleryWindow,
};
use slint::{ComponentHandle, Model, ModelRc, VecModel};

pub fn connect(ui: &GalleryWindow) {
    slintforge::bind_grid_services!(
        ui,
        ForgeGridServices,
        ForgeGridScene,
        ForgeGridViewRow,
        ForgeGridViewColumn,
        ForgeGridRow,
        ForgeGridCell,
        ForgeGridColumn,
        ForgeGridEditResult
    );
    let weak = ui.as_weak();
    ui.global::<GalleryGridDemoModels>()
        .on_action(move |tree, action, rows| {
            let ui = weak.upgrade().expect("窗口存在");
            let models = ui.global::<GalleryGridDemoModels>();
            let seed = if tree {
                models.get_tree_seed()
            } else {
                models.get_flat_seed()
            };
            let seed = seed.row_data(0).unwrap();
            let make_row = |index: usize, large: bool| {
                let mut row = seed.clone();
                let group = index / 1000;
                let is_root = index % 1000 == 0;
                row.id = if tree && large {
                    if is_root {
                        format!("group-{group}")
                    } else {
                        format!("group-{group}/{index}")
                    }
                } else {
                    format!("new-{index}")
                }
                .into();
                row.parent_id = if tree && large && !is_root {
                    format!("group-{group}").into()
                } else {
                    "".into()
                };
                row.expanded = true;
                row.selected = false;
                row.disabled = false;
                let mut cells = row.cells.iter().collect::<Vec<_>>();
                cells[0].text = format!(
                    "{} {}",
                    if tree && large && is_root {
                        models.get_group_label()
                    } else {
                        models.get_new_label()
                    },
                    index + 1
                )
                .into();
                cells[1].value = (index % 200) as f32;
                cells[1].text = format!("{}", index % 200).into();
                row.cells = ModelRc::new(VecModel::from(cells));
                row
            };
            let mut result = rows.iter().collect::<Vec<_>>();
            match action.as_str() {
                "large" => result = (0..10000).map(|i| make_row(i, true)).collect(),
                "append" => {
                    let mut index = result.len();
                    while result
                        .iter()
                        .any(|r| r.id.as_str() == format!("new-{index}"))
                    {
                        index += 1;
                    }
                    result.push(make_row(index, false));
                }
                "update" if !result.is_empty() => {
                    let mut cells = result[0].cells.iter().collect::<Vec<_>>();
                    if let Some(cell) = cells.get_mut(1) {
                        cell.value += 10.;
                        cell.text = format!("{}", cell.value).into();
                    }
                    result[0].cells = ModelRc::new(VecModel::from(cells));
                }
                _ => {}
            }
            ModelRc::new(VecModel::from(result))
        });
}

pub fn verify(ui: &GalleryWindow) {
    let models = ui.global::<GalleryGridDemoModels>();
    let columns = ModelRc::new(VecModel::from(vec![
        ForgeGridColumn {
            id: "name".into(),
            editable: true,
            ..Default::default()
        },
        ForgeGridColumn {
            id: "count".into(),
            numeric: true,
            editable: true,
            ..Default::default()
        },
    ]));
    for tree in [false, true] {
        let seed = if tree {
            models.get_tree_seed()
        } else {
            models.get_flat_seed()
        };
        let appended = models.invoke_action(tree, "append".into(), seed.clone());
        assert_eq!(appended.row_count(), seed.row_count() + 1);
        let large = models.invoke_action(tree, "large".into(), seed);
        let scene = ui.global::<ForgeGridServices>().invoke_prepare(
            large,
            columns.clone(),
            tree,
            -1,
            false,
            "".into(),
            0,
        );
        assert_eq!(scene.status, 0);
        assert_eq!(scene.rows.row_count(), 10000);
    }
    let edited = ui.global::<ForgeGridServices>().invoke_commit(
        models.get_flat_seed(),
        columns,
        0,
        1,
        "3.5".into(),
    );
    assert_eq!(edited.status, 0);
    let updated = models.invoke_action(false, "update".into(), edited.rows);
    let cell = updated.row_data(0).unwrap().cells.row_data(1).unwrap();
    assert_eq!(cell.value, 13.5);
    assert_eq!(cell.text, "13.5");
}
