// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

use slintforge::grids::*;
use std::time::Instant;
fn main() -> Result<(), Box<dyn std::error::Error>> {
    let directory = std::env::current_dir()?.join("build/plot-qa/grid-review");
    std::fs::create_dir_all(&directory)?;
    let columns = vec![
        GridColumnData {
            id: "text".into(),
            title: "名称".into(),
            sortable: true,
            ..Default::default()
        },
        GridColumnData {
            id: "value".into(),
            numeric: true,
            sortable: true,
            ..Default::default()
        },
    ];
    let mut csv = String::from("host,rows,shape,operation,ms,visible\n");
    for count in [10000, 100000] {
        for tree in [false, true] {
            let rows = (0..count)
                .map(|i| {
                    let key = ((count - i) * 17).to_string();
                    GridRowData {
                        id: i.to_string(),
                        parent_id: if tree && i > 0 {
                            ((i - 1) / 4).to_string()
                        } else {
                            String::new()
                        },
                        expanded: true,
                        disabled: i % 23 == 0,
                        cells: vec![
                            GridCellData {
                                text: format!(
                                    "{}{key}{}",
                                    if i % 2 == 0 { "a" } else { "A" },
                                    "X".repeat(70)
                                ),
                                value: 0.,
                            },
                            GridCellData {
                                text: key,
                                value: (count - i) as f64,
                            },
                        ],
                        ..Default::default()
                    }
                })
                .collect::<Vec<_>>();
            for operation in 0..4 {
                let start = Instant::now();
                let scene = prepare_grid(
                    &rows,
                    &columns,
                    tree,
                    if operation == 0 {
                        -1
                    } else if operation == 1 {
                        0
                    } else {
                        1
                    },
                    operation == 2,
                    if operation == 3 { "A17" } else { "" },
                );
                let ms = start.elapsed().as_secs_f64() * 1000.;
                assert_ne!(scene.status, 2);
                if operation < 3 {
                    assert_eq!(scene.rows.len(), rows.len());
                }
                csv += &format!(
                    "rust,{count},{},{operation},{ms},{}\n",
                    if tree { "tree" } else { "flat" },
                    scene.rows.len()
                );
                println!(
                    "GRID_REVIEW_PERF host=rust rows={count} tree={tree} op={operation} ms={ms:.3}"
                );
            }
        }
    }
    std::fs::write(
        directory.join(
            std::env::args()
                .nth(1)
                .unwrap_or("rust-performance.csv".into()),
        ),
        csv,
    )?;
    println!("GRID_REVIEW_TESTS_OK host=rust");
    Ok(())
}
