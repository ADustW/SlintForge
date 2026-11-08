// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

use slintforge::{Error, NavItem, NavItemKind, NavTree};
use std::time::Instant;

fn review_plot(kind: i32, points: &[(f64, f64)]) -> slintforge::plot::Plot {
    use slintforge::plot::{raw, Plot};
    let options = raw::ForgePlotOptionsCore {
        width: 900.,
        height: 540.,
        font_size: 12.,
        foreground: 0xff202020,
        ..Default::default()
    };
    let series = review_plot_series(kind);
    let points = points
        .iter()
        .map(|&(x, y)| raw::ForgePlotPointCore {
            x,
            y,
            ..Default::default()
        })
        .collect::<Vec<_>>();
    let mut plot = unsafe { Plot::from_core(&options).unwrap() };
    unsafe {
        plot.add_series_core(&series, &points);
    }
    plot.render().unwrap();
    plot
}

fn review_plot_series(kind: i32) -> slintforge::plot::raw::ForgePlotSeriesCore {
    slintforge::plot::raw::ForgePlotSeriesCore {
        kind,
        y_axis: 1,
        line_style: 1,
        selectable: 1,
        stack_group: -1,
        fill_target: -1,
        line_width: 2.,
        ..Default::default()
    }
}

#[test]
fn plot_type_switches_rebuild_order_before_appending() {
    let original = [
        (2., 2.),
        (0., 0.),
        (1., 1.),
        (f64::NAN, f64::NAN),
        (3., 3.),
        (1., 2.),
    ];
    for (before, after) in [(0, 1), (1, 0), (0, 2), (2, 0)] {
        let mut plot = review_plot(before, &original);
        let series = review_plot_series(after);
        assert!(unsafe { plot.update_series_core(&series) });
        plot.render().unwrap();
        assert_eq!(
            plot.svg(),
            review_plot(after, &original).svg(),
            "切换 {before}->{after} 后连接顺序不正确"
        );
        assert!(plot.append_xy(0, &[(4., 4.), (0.5, 2.)]));
        plot.render().unwrap();
        let mut appended = original.to_vec();
        appended.extend([(4., 4.), (0.5, 2.)]);
        assert_eq!(
            plot.svg(),
            review_plot(after, &appended).svg(),
            "切换 {before}->{after} 后追加索引失效"
        );
    }
}

#[test]
fn plot_nan_points_do_not_pollute_ranges() {
    let plot=review_plot(0,&[(0.,1.),(1.,f64::NAN),(2.,3.)]);
    let ranges=plot.ranges();assert!(ranges[1].minimum>0. && ranges[1].maximum<4.);
}

#[test]
fn deep_navigation_tree_preserves_order_and_detects_disconnected_cycles() {
    let count = std::env::var("SLINTFORGE_NAV_REVIEW_DEPTH")
        .ok()
        .and_then(|v| v.parse::<usize>().ok())
        .unwrap_or(20000);
    let items = (0..count)
        .map(|i| {
            let parent = (i > 0).then(|| format!("node-{}", i - 1));
            NavItem::new(
                &format!("node-{i}"),
                parent.as_deref(),
                "节点",
                "",
                NavItemKind::Group,
            )
        })
        .collect();
    let start = Instant::now();
    let mut tree = NavTree::new(items).unwrap();
    let creation = start.elapsed();
    for i in 0..count {
        tree.set_expanded(&format!("node-{i}"), true).unwrap();
    }
    let start = Instant::now();
    let visible = tree.visible_items();
    eprintln!(
        "navigation-review nodes={count} creation_ms={:.3} flatten_ms={:.3}",
        creation.as_secs_f64() * 1000.,
        start.elapsed().as_secs_f64() * 1000.
    );
    assert_eq!(visible.len(), count);
    assert_eq!(visible.last().unwrap().depth, count - 1);
    assert_eq!(
        visible.last().unwrap().item.id,
        format!("node-{}", count - 1)
    );
    tree.set_expanded("node-1", false).unwrap();
    assert_eq!(tree.visible_items().len(), 2);
    let cyclic = vec![
        NavItem::new("root", None, "根", "", NavItemKind::Group),
        NavItem::new("a", Some("b"), "A", "", NavItemKind::Group),
        NavItem::new("b", Some("a"), "B", "", NavItemKind::Group),
    ];
    assert!(matches!(NavTree::new(cyclic), Err(Error::Cycle(_))));
}
