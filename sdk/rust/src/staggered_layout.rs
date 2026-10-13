/// 瀑布流中单张卡片的稳定 ID 与高度，单位为逻辑像素。
#[derive(Clone, Debug, PartialEq)]
pub struct StaggeredInput {
    pub id: String,
    pub height: f32,
}

/// 按当前最短列计算得到的卡片位置。
#[derive(Clone, Debug, PartialEq)]
pub struct StaggeredPlacement {
    pub id: String,
    pub column: usize,
    pub top: f32,
}

#[derive(Clone, Debug, PartialEq)]
pub struct StaggeredLayout {
    pub placements: Vec<StaggeredPlacement>,
    pub content_height: f32,
}

impl StaggeredLayout {
    pub fn arrange(items: &[StaggeredInput], columns: usize, gap: f32, padding: f32) -> Self {
        let mut arena = slintforge_runtime_api::Arena::default();
        use slintforge_runtime_api::Wire;
        let items = items.to_vec();
        let args = slintforge_runtime_api::models::ArgsArrange {
            items: items.view(&mut arena),
            columns,
            gap,
            padding,
        };
        crate::runtime::value::<StaggeredLayout, _>(
            slintforge_runtime_api::models::OP_ARRANGE,
            &args,
        )
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn shortest_column_and_stable_ties() {
        let items = [100.0, 60.0, 80.0, 50.0, 70.0]
            .into_iter()
            .enumerate()
            .map(|(n, height)| StaggeredInput {
                id: format!("card-{n}"),
                height,
            })
            .collect::<Vec<_>>();
        let result = StaggeredLayout::arrange(&items, 2, 10.0, 12.0);
        assert_eq!(
            result
                .placements
                .iter()
                .map(|place| (place.id.as_str(), place.column, place.top))
                .collect::<Vec<_>>(),
            vec![
                ("card-0", 0, 12.0),
                ("card-1", 1, 12.0),
                ("card-2", 1, 82.0),
                ("card-3", 0, 122.0),
                ("card-4", 1, 172.0),
            ]
        );
        assert_eq!(result.content_height, 254.0);
        assert_eq!(
            StaggeredLayout::arrange(&[], 0, 10.0, 12.0).content_height,
            24.0
        );
    }

    #[test]
    fn oversized_column_count_and_nonfinite_values_are_bounded() {
        let items = [
            StaggeredInput {
                id: "bad".into(),
                height: f32::NAN,
            },
            StaggeredInput {
                id: "good".into(),
                height: 30.0,
            },
            StaggeredInput {
                id: "infinite".into(),
                height: f32::INFINITY,
            },
        ];
        let result = StaggeredLayout::arrange(&items, usize::MAX, f32::NAN, -5.0);
        assert_eq!(
            result
                .placements
                .iter()
                .map(|p| p.column)
                .collect::<Vec<_>>(),
            [0, 0, 1]
        );
        assert_eq!(result.content_height, 30.0);
        assert_eq!(
            StaggeredLayout::arrange(&[], usize::MAX, 12.0, 5.0).content_height,
            10.0
        );
    }
}
