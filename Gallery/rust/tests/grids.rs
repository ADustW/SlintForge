use slint::platform::{Key, PointerEventButton, WindowEvent};
use slint::{ComponentHandle, LogicalPosition, Model, ModelRc, VecModel};
use std::time::Duration;

slint::slint! {
    import { ForgeDataGrid } from "../../../ui/data/data-grid.slint";
    import { ForgeGridScene, ForgeGridRow, ForgeGridCell, ForgeGridColumn, ForgeGridSelectionMode } from "../../../ui/types/grids.slint";
    export { ForgeGridServices, ForgeGridScene, ForgeGridViewRow, ForgeGridViewColumn, ForgeGridRow,
        ForgeGridCell, ForgeGridColumn, ForgeGridEditResult, ForgeGridSelectionMode } from "../../../ui/types/grids.slint";
    export component GridTestWindow inherits Window {
        width: 780px; height: 490px;
        in-out property<[ForgeGridRow]> rows <=> grid.rows;
        in-out property<[ForgeGridColumn]> columns <=> grid.columns;
        in-out property<bool> tree: false;
        in-out property<bool> enabled: true;
        in-out property<string> filter <=> grid.filter;
        in-out property<ForgeGridSelectionMode> selection-mode: ForgeGridSelectionMode.multiple;
        out property<ForgeGridScene> scene: grid.scene;
        out property<string> current-id: grid.current-row-id;
        out property<int> current: grid.current-row;
        out property<int> selected: grid.selected-row-count;
        out property<bool> editing: grid.editing;
        out property<string> error: grid.edit-message;
        out property<length> scroll-x: grid.scroll-x;
        out property<length> scroll-y: grid.scroll-y;
        callback select(int, bool, bool); select(index, toggle, range) => { grid.select-row(index, toggle, range); }
        callback all(); all => { grid.select-all(); }
        callback clear(); clear => { grid.clear-selection(); }
        callback toggle(int); toggle(index) => { grid.toggle-row(index); }
        callback expand(bool); expand(value) => { grid.expand-all(value); }
        callback edit(int, int); edit(row, column) => { grid.begin-edit(row, column); }
        callback cancel(); cancel => { grid.cancel-edit(); }
        callback remove(); remove => { grid.delete-selected(); }
        callback refresh(); refresh => { grid.refresh(); }
        grid := ForgeDataGrid {
            x: 20px; y: 20px; width: 740px; height: 450px;
            tree: root.tree; enabled: root.enabled; selection-mode: root.selection-mode;
        }
    }
}

fn tick() {
    i_slint_backend_testing::mock_elapsed_time(Duration::from_millis(50));
    slint::platform::update_timers_and_animations();
}
fn down(ui: &GridTestWindow, text: impl Into<slint::SharedString>) {
    ui.window()
        .dispatch_event(WindowEvent::KeyPressed { text: text.into() });
}
fn up(ui: &GridTestWindow, text: impl Into<slint::SharedString>) {
    ui.window()
        .dispatch_event(WindowEvent::KeyReleased { text: text.into() });
}
fn key(ui: &GridTestWindow, text: impl Into<slint::SharedString>) {
    let text = text.into();
    down(ui, text.clone());
    up(ui, text);
    tick();
}
fn click(ui: &GridTestWindow, x: f32, y: f32) {
    let position = LogicalPosition::new(x, y);
    ui.window()
        .dispatch_event(WindowEvent::PointerMoved { position });
    ui.window().dispatch_event(WindowEvent::PointerPressed {
        position,
        button: PointerEventButton::Left,
    });
    ui.window().dispatch_event(WindowEvent::PointerReleased {
        position,
        button: PointerEventButton::Left,
    });
    tick();
}
fn column(id: &str, width: f32, numeric: bool, frozen: bool) -> ForgeGridColumn {
    ForgeGridColumn {
        id: id.into(),
        title: id.into(),
        width,
        min_width: 80.,
        max_width: 800.,
        numeric,
        sortable: true,
        editable: true,
        frozen,
        ..Default::default()
    }
}
fn row(id: &str, parent: &str, number: f32) -> ForgeGridRow {
    ForgeGridRow {
        id: id.into(),
        parent_id: parent.into(),
        expanded: true,
        cells: ModelRc::new(VecModel::from(vec![
            ForgeGridCell {
                text: id.into(),
                value: 0.,
            },
            ForgeGridCell {
                text: format!("{number}").into(),
                value: number,
            },
            ForgeGridCell {
                text: "owner".into(),
                value: 0.,
            },
            ForgeGridCell {
                text: "long notes".into(),
                value: 0.,
            },
        ])),
        ..Default::default()
    }
}
fn setup(rows: Vec<ForgeGridRow>, tree: bool) -> GridTestWindow {
    let ui = GridTestWindow::new().unwrap();
    ui.set_columns(ModelRc::new(VecModel::from(vec![
        column("name", 180., false, true),
        column("count", 90., true, false),
        column("owner", 120., false, false),
        column("notes", 420., false, false),
    ])));
    ui.set_rows(ModelRc::new(VecModel::from(rows)));
    ui.set_tree(tree);
    assert_eq!(ui.get_scene().status, 3);
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
    ui.show().unwrap();
    tick();
    assert_eq!(ui.get_scene().status, 0);
    ui
}
fn replace_editor(ui: &GridTestWindow, text: &str) {
    ui.window().dispatch_event(WindowEvent::PointerMoved {
        position: LogicalPosition::new(330., 80.),
    });
    tick();
    down(ui, Key::Control);
    key(ui, "a");
    up(ui, Key::Control);
    key(ui, text);
}

#[test]
fn grid_pointer_sort_multi_selection_resize_frozen_scroll_and_editing() {
    i_slint_backend_testing::init_no_event_loop();
    let mut rows = vec![
        row("a", "", 20.),
        row("b", "", 2.),
        row("c", "", 30.),
        row("d", "", 8.),
        row("e", "", 5.),
    ];
    rows[2].disabled = true;
    let ui = setup(rows, false);
    click(&ui, 140., 79.);
    assert_eq!(ui.get_current_id(), "a");
    assert_eq!(ui.get_selected(), 1);
    down(&ui, Key::Control);
    click(&ui, 140., 115.);
    up(&ui, Key::Control);
    assert_eq!(ui.get_selected(), 2);
    down(&ui, Key::Shift);
    click(&ui, 140., 187.);
    up(&ui, Key::Shift);
    assert_eq!(ui.get_selected(), 2);
    assert!(!ui.get_rows().row_data(2).unwrap().selected);
    ui.invoke_select(0, false, false);
    tick();
    click(&ui, 330., 39.);
    assert_eq!(ui.get_scene().rows.row_data(0).unwrap().id, "b");
    assert_eq!(ui.get_current_id(), "a");
    assert_eq!(ui.get_current(), 3);
    click(&ui, 330., 39.);
    assert_eq!(ui.get_current(), 1);
    down(&ui, Key::Control);
    key(&ui, "a");
    key(&ui, "a");
    up(&ui, Key::Control);
    assert_eq!(ui.get_selected(), 4);
    let position = LogicalPosition::new(282., 39.);
    ui.window()
        .dispatch_event(WindowEvent::PointerMoved { position });
    ui.window().dispatch_event(WindowEvent::PointerPressed {
        position,
        button: PointerEventButton::Left,
    });
    let position = LogicalPosition::new(302., 39.);
    ui.window()
        .dispatch_event(WindowEvent::PointerMoved { position });
    tick();
    let position = LogicalPosition::new(322., 39.);
    ui.window()
        .dispatch_event(WindowEvent::PointerMoved { position });
    ui.window().dispatch_event(WindowEvent::PointerReleased {
        position,
        button: PointerEventButton::Left,
    });
    tick();
    assert!(
        (ui.get_columns().row_data(0).unwrap().width - 220.).abs() < 1.,
        "拖动后的宽度: {}",
        ui.get_columns().row_data(0).unwrap().width
    );
    assert_eq!(ui.get_scene().columns.row_data(0).unwrap().width, 220.);
    ui.window().dispatch_event(WindowEvent::PointerScrolled {
        position: LogicalPosition::new(600., 220.),
        delta_x: -400.,
        delta_y: 0.,
    });
    tick();
    assert!(ui.get_scroll_x() < 0.);
    click(&ui, 150., 115.);
    assert_eq!(ui.get_current_id(), "a");
    assert_eq!(ui.get_selected(), 1);
    i_slint_backend_testing::mock_elapsed_time(Duration::from_millis(600));
    click(&ui, 150., 115.);
    click(&ui, 150., 115.);
    assert!(ui.get_editing(), "双击单元格应进入编辑");
    ui.invoke_cancel();
    tick();
    ui.invoke_edit(1, 1);
    tick();
    assert!(ui.get_editing());
    replace_editor(&ui, "NaN");
    key(&ui, Key::Return);
    assert!(ui.get_editing());
    assert!(!ui.get_error().is_empty());
    assert_eq!(
        ui.get_rows()
            .row_data(0)
            .unwrap()
            .cells
            .row_data(1)
            .unwrap()
            .value,
        20.
    );
    replace_editor(&ui, "3.5");
    key(&ui, Key::Return);
    assert!(!ui.get_editing());
    assert_eq!(
        ui.get_rows()
            .row_data(0)
            .unwrap()
            .cells
            .row_data(1)
            .unwrap()
            .value,
        3.5
    );
    ui.invoke_edit(ui.get_current(), 1);
    tick();
    replace_editor(&ui, "99");
    key(&ui, Key::Escape);
    assert!(!ui.get_editing());
    assert_eq!(
        ui.get_rows()
            .row_data(0)
            .unwrap()
            .cells
            .row_data(1)
            .unwrap()
            .value,
        3.5
    );
    // 同一轮事件中替换列模型并开始编辑，延后执行的 changed 回调不应取消有效编辑。
    ui.set_columns(ModelRc::new(VecModel::from(
        ui.get_columns().iter().collect::<Vec<_>>(),
    )));
    ui.invoke_edit(ui.get_current(), 1);
    tick();
    assert!(ui.get_editing());
    replace_editor(&ui, "6.5");
    key(&ui, Key::Escape);
    assert!(!ui.get_editing());
    ui.set_enabled(false);
    ui.invoke_all();
    ui.invoke_remove();
    click(&ui, 140., 223.);
    key(&ui, Key::Home);
    assert_eq!(ui.get_current_id(), "a");
    assert_eq!(ui.get_rows().row_count(), 5);
    ui.set_enabled(true);
    ui.set_filter("missing".into());
    assert_eq!(ui.get_scene().status, 1);
    ui.set_filter("a".into());
    assert_eq!(ui.get_scene().rows.row_count(), 1);
    assert_eq!(ui.get_selected(), 1);
    ui.invoke_remove();
    assert_eq!(ui.get_rows().row_count(), 4);
}

#[test]
fn tree_grid_expansion_parent_keyboard_filter_model_updates_and_large_scrolling() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = setup(
        vec![
            row("child", "root", 2.),
            row("grand", "child", 1.),
            row("root", "", 5.),
            row("other", "", 3.),
        ],
        true,
    );
    assert_eq!(ui.get_scene().rows.row_data(0).unwrap().id, "root");
    ui.set_filter(" \t\n".into());
    tick();
    click(&ui, 118., 79.);
    assert_eq!(ui.get_scene().rows.row_count(), 2);
    ui.set_filter("".into());
    tick();
    ui.invoke_select(0, false, false);
    key(&ui, Key::RightArrow);
    assert_eq!(ui.get_scene().rows.row_count(), 4);
    key(&ui, Key::RightArrow);
    assert_eq!(ui.get_current_id(), "child");
    key(&ui, Key::LeftArrow);
    assert_eq!(ui.get_scene().rows.row_count(), 3);
    key(&ui, Key::LeftArrow);
    assert_eq!(ui.get_current_id(), "root");
    ui.invoke_expand(false);
    ui.set_filter("grand".into());
    assert_eq!(ui.get_scene().rows.row_count(), 3);
    assert_eq!(ui.get_scene().rows.row_data(2).unwrap().depth, 2);
    ui.set_filter("".into());
    assert_eq!(ui.get_scene().rows.row_count(), 2);
    ui.invoke_select(0, false, false);
    ui.invoke_remove();
    assert_eq!(ui.get_rows().row_count(), 1);
    assert_eq!(ui.get_rows().row_data(0).unwrap().id, "other");
    ui.set_tree(false);
    let cells = ModelRc::new(VecModel::from(vec![ForgeGridCell {
        text: "updated".into(),
        value: 0.,
    }]));
    ui.set_rows(ModelRc::new(VecModel::from(vec![ForgeGridRow {
        id: "updated".into(),
        cells: cells.clone(),
        ..Default::default()
    }])));
    cells.set_row_data(
        0,
        ForgeGridCell {
            text: "nested update".into(),
            value: 0.,
        },
    );
    ui.invoke_refresh();
    assert_eq!(
        ui.get_scene()
            .rows
            .row_data(0)
            .unwrap()
            .cells
            .row_data(0)
            .unwrap()
            .text,
        "nested update"
    );
    ui.set_rows(ModelRc::new(VecModel::from(
        (0..10000)
            .map(|i| row(&format!("r{i}"), "", i as f32))
            .collect::<Vec<_>>(),
    )));
    ui.invoke_select(0, false, false);
    key(&ui, Key::End);
    assert_eq!(ui.get_current_id(), "r9999");
    assert!(ui.get_scroll_y() < -300000.);
    key(&ui, Key::Home);
    assert_eq!(ui.get_current_id(), "r0");
    assert_eq!(ui.get_scroll_y(), 0.);
    ui.set_rows(ModelRc::new(VecModel::from(Vec::<ForgeGridRow>::new())));
    assert_eq!(ui.get_scene().status, 1);
}
