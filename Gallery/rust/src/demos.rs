use crate::{
    ForgeQRCodeResult, ForgeSyntaxLine, ForgeTreeItem, ForgeWidgetServices, GalleryDemoDialog,
    GalleryDemoModels, GalleryDemoWindow, GalleryWindow,
};
use slint::{ComponentHandle, Model, ModelRc, Rgba8Pixel, SharedPixelBuffer, VecModel};
use std::cell::RefCell;
use std::rc::Rc;

fn tree_model() -> Vec<ForgeTreeItem> {
    [
        ("project", "", "SlintForge", true, true, true),
        ("project/sdk", "project", "SDK", true, true, true),
        (
            "project/sdk/rust",
            "project/sdk",
            "Rust",
            false,
            false,
            true,
        ),
        ("project/sdk/cpp", "project/sdk", "C++", false, false, true),
        ("project/gallery", "project", "Gallery", true, false, true),
        (
            "project/gallery/main",
            "project/gallery",
            "main-screen.slint",
            false,
            false,
            true,
        ),
        (
            "project/readonly",
            "project",
            "readonly.txt",
            false,
            false,
            false,
        ),
    ]
    .into_iter()
    .map(
        |(id, parent, label, folder, expanded, enabled)| ForgeTreeItem {
            id: id.into(),
            parent_id: parent.into(),
            label: label.into(),
            folder,
            expanded,
            enabled,
            selected: false,
            visible: true,
            depth: 0,
        },
    )
    .collect()
}

fn refresh_tree(ui: &GalleryWindow, items: &mut [ForgeTreeItem]) {
    let source = items.to_vec();
    for item in items.iter_mut() {
        item.depth = 0;
        item.visible = true;
        let mut parent = item.parent_id.clone();
        while !parent.is_empty() {
            let Some(node) = source.iter().find(|node| node.id == parent) else {
                break;
            };
            item.depth += 1;
            item.visible &= node.expanded;
            parent = node.parent_id.clone();
        }
    }
    let summary = items
        .iter()
        .filter(|node| node.selected && !node.folder)
        .map(|node| node.label.as_str())
        .collect::<Vec<_>>()
        .join(", ");
    let global = ui.global::<GalleryDemoModels>();
    global.set_tree_selection(summary.into());
    global.set_tree_items(ModelRc::new(VecModel::from(items.to_vec())));
}

pub fn connect(ui: &GalleryWindow) {
    crate::plot_demo::connect(ui);
    crate::grids_demo::connect(ui);
    slintforge::bind_chart_services!(ui, crate::ForgeChartServices, crate::ForgeChartScene, crate::ForgeChartMark, crate::ForgeChartLine, crate::ForgeChartTick, crate::ForgeChartLabel);
    let model = Rc::new(RefCell::new(tree_model()));
    refresh_tree(ui, &mut model.borrow_mut());
    let mut pixels = SharedPixelBuffer::<Rgba8Pixel>::new(600, 360);
    for y in 0..360 {
        for x in 0..600 {
            let checker = ((x / 45 + y / 45) % 2) as u8;
            pixels.make_mut_slice()[(y * 600 + x) as usize] = Rgba8Pixel {
                r: (40 + x * 150 / 600) as u8,
                g: (70 + y * 100 / 360) as u8,
                b: 100 + checker * 100,
                a: 255,
            };
        }
    }
    ui.global::<GalleryDemoModels>()
        .set_backdrop_image(slint::Image::from_rgba8(pixels));
    slintforge::bind_widget_services!(ui, ForgeWidgetServices, ForgeQRCodeResult, ForgeSyntaxLine);
    {
        let weak = ui.as_weak();
        ui.global::<GalleryDemoModels>().on_add_tag(move |value| {
            if let Some(ui) = weak.upgrade() {
                let value = value.as_str().trim();
                let global = ui.global::<GalleryDemoModels>();
                let model = global.get_tags();
                let mut tags = (0..model.row_count())
                    .filter_map(|row| model.row_data(row))
                    .collect::<Vec<_>>();
                if !value.is_empty()
                    && tags.len() < 20
                    && !tags.iter().any(|tag| tag.as_str() == value)
                {
                    tags.push(value.into());
                }
                global.set_tags(ModelRc::new(VecModel::from(tags)));
            }
        });
    }
    {
        let weak = ui.as_weak();
        ui.global::<GalleryDemoModels>()
            .on_remove_tag(move |index| {
                if let Some(ui) = weak.upgrade() {
                    let global = ui.global::<GalleryDemoModels>();
                    let model = global.get_tags();
                    let tags = (0..model.row_count())
                        .filter(|row| *row as i32 != index)
                        .filter_map(|row| model.row_data(row))
                        .collect::<Vec<_>>();
                    global.set_tags(ModelRc::new(VecModel::from(tags)));
                }
            });
    }
    {
        let weak = ui.as_weak();
        let model = model.clone();
        ui.global::<GalleryDemoModels>()
            .on_expand_tree(move |id, value| {
                if let Some(ui) = weak.upgrade() {
                    let mut items = model.borrow_mut();
                    if let Some(item) = items
                        .iter_mut()
                        .find(|item| item.id == id && item.enabled && item.folder)
                    {
                        item.expanded = value;
                    }
                    refresh_tree(&ui, &mut items);
                }
            });
    }
    {
        let weak = ui.as_weak();
        let model = model.clone();
        ui.global::<GalleryDemoModels>()
            .on_select_tree(move |id, selected, multi| {
                if let Some(ui) = weak.upgrade() {
                    let mut items = model.borrow_mut();
                    if !items.iter().any(|item| item.id == id && item.enabled) {
                        return;
                    }
                    let prefix = format!("{id}/");
                    for item in items.iter_mut() {
                        if multi {
                            if item.enabled
                                && (item.id == id || item.id.as_str().starts_with(&prefix))
                            {
                                item.selected = selected;
                            }
                        } else {
                            item.selected = item.enabled && item.id == id && selected;
                        }
                    }
                    if multi {
                        for index in (0..items.len()).rev() {
                            if !items[index].folder {
                                continue;
                            }
                            let children = items
                                .iter()
                                .filter(|child| child.parent_id == items[index].id && child.enabled)
                                .collect::<Vec<_>>();
                            let checked =
                                !children.is_empty() && children.iter().all(|child| child.selected);
                            items[index].selected = checked;
                        }
                    }
                    refresh_tree(&ui, &mut items);
                }
            });
    }
    let auxiliary = Rc::new(RefCell::new((
        None::<GalleryDemoWindow>,
        None::<GalleryDemoDialog>,
    )));
    {
        let auxiliary = auxiliary.clone();
        ui.global::<GalleryDemoModels>()
            .on_show_window(move |dialog| {
                let mut windows = auxiliary.borrow_mut();
                if dialog {
                    if windows.1.is_none() {
                        let window = GalleryDemoDialog::new().unwrap();
                        let weak = window.as_weak();
                        window.on_ok_clicked(move || {
                            if let Some(window) = weak.upgrade() {
                                let _ = window.hide();
                            }
                        });
                        let weak = window.as_weak();
                        window.on_cancel_clicked(move || {
                            if let Some(window) = weak.upgrade() {
                                let _ = window.hide();
                            }
                        });
                        windows.1 = Some(window);
                    }
                    windows.1.as_ref().unwrap().show().unwrap();
                } else {
                    if windows.0.is_none() {
                        let window = GalleryDemoWindow::new().unwrap();
                        let weak = window.as_weak();
                        window.on_dismissed(move || {
                            if let Some(window) = weak.upgrade() {
                                let _ = window.hide();
                            }
                        });
                        windows.0 = Some(window);
                    }
                    windows.0.as_ref().unwrap().show().unwrap();
                }
            });
    }
    ui.global::<GalleryDemoModels>().on_close_windows(move || {
        let windows = auxiliary.borrow();
        if let Some(window) = &windows.0 {
            let _ = window.hide();
        }
        if let Some(window) = &windows.1 {
            let _ = window.hide();
        }
    });
    let weak = ui.as_weak();
    ui.global::<GalleryDemoModels>()
        .on_export_image(move |image| {
            weak.upgrade()
                .map(|ui| ui.invoke_export_capture(image))
                .unwrap_or_default()
        });
}
