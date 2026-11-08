// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

pub fn merge_plot_selection(
    current: &[(i32, i32, i32)],
    next: &[(i32, i32, i32)],
    mode: i32,
) -> Vec<(i32, i32, i32)> {
    if mode == 0 {
        return merge_plot_selection(&[], next, 1);
    }
    let mut events = std::collections::BTreeMap::<(i32, i64), [i32; 2]>::new();
    for (source, list) in [current, next].iter().enumerate() {
        for &(id, first, last) in *list {
            if first < 0 || last < first {
                continue;
            }
            events.entry((id, first as i64)).or_default()[source] += 1;
            events.entry((id, last as i64 + 1)).or_default()[source] -= 1;
        }
    }
    let mut result = Vec::new();
    let mut active = [0, 0];
    let mut start = 0;
    let mut old_id = i32::MIN;
    let selected = |active: [i32; 2]| {
        if mode == 2 {
            (active[0] > 0) ^ (active[1] > 0)
        } else {
            active[0] > 0 || active[1] > 0
        }
    };
    for ((id, index), delta) in events {
        if id != old_id {
            active = [0, 0];
            old_id = id;
        }
        let previous = selected(active);
        active[0] += delta[0];
        active[1] += delta[1];
        let after = selected(active);
        if !previous && after {
            start = index;
        }
        if previous && !after {
            result.push((id, start as i32, (index - 1) as i32));
        }
    }
    result
}
