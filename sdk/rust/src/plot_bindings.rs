// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

// 由 tools/generate_plot_bindings.py 生成。
#[macro_export]
macro_rules! bind_plot_services {
    ($ui:expr, $service:ty, $scene:ty, $axis:ty, $range:ty, $rect:ty, $hit:ty, $selection:ty, $export:ty) => {{
        use slint::{ComponentHandle, Model};
        $crate::runtime::initialize().expect("绘图运行库初始化失败");
        use $crate::plot::{Plot, PlotServiceState, plot_cstring, plot_color};
        use $crate::plot::raw::*;
        let state = std::rc::Rc::new(std::cell::RefCell::new(PlotServiceState::default()));
        let inputs_cache = std::rc::Rc::new(std::cell::RefCell::new(std::collections::HashMap::new()));
        let service = $ui.global::<$service>();
        let allocate = state.clone(); service.on_allocate(move || allocate.borrow_mut().allocate());
        let release = state.clone(); let release_cache = inputs_cache.clone();
        service.on_release(move |id| {release.borrow_mut().release(id);release_cache.borrow_mut().remove(&id);});
        let render_state = state.clone();
        service.on_prepare(move |token, axes, panels, series, maps, items, selection, v| {
            let mut result = <$scene>::default(); result.token = token;
            let inputs=(series.iter().map(|s|(s.id,s.points)).collect::<Vec<_>>(),if maps.row_count()==0{Default::default()}else{maps.clone()},if items.row_count()==0{Default::default()}else{items.clone()},if panels.row_count()==0{Default::default()}else{panels.clone()},v.revision);
            let identical=inputs_cache.borrow().get(&token)==Some(&inputs);
            // Slint 常量数组可能生成新的模型句柄；内容相同的静态基线仍保留追加历史。
            let equivalent=identical||inputs_cache.borrow().get(&token).map(|old|old.1==inputs.1&&old.2==inputs.2&&old.3==inputs.3&&old.4==inputs.4&&old.0.len()==inputs.0.len()&&old.0.iter().zip(&inputs.0).all(|(a,b)|a.0==b.0&&(a.1==b.1||(a.1.row_count()==b.1.row_count()&&a.1.iter().zip(b.1.iter()).all(|(a,b)|{
a.x.to_bits()==b.x.to_bits() && a.y.to_bits()==b.y.to_bits() && a.low.to_bits()==b.low.to_bits() && a.high.to_bits()==b.high.to_bits() && a.open.to_bits()==b.open.to_bits() && a.close.to_bits()==b.close.to_bits() && a.q1.to_bits()==b.q1.to_bits() && a.q3.to_bits()==b.q3.to_bits() && a.error_x_minus.to_bits()==b.error_x_minus.to_bits() && a.error_x_plus.to_bits()==b.error_x_plus.to_bits() && a.error_y_minus.to_bits()==b.error_y_minus.to_bits() && a.error_y_plus.to_bits()==b.error_y_plus.to_bits() && a.label==b.label && (a.outliers==b.outliers || (a.outliers.row_count()==b.outliers.row_count() && a.outliers.iter().zip(b.outliers.iter()).all(|(x,y)|x.to_bits()==y.to_bits())))
            }))))).unwrap_or(false);
            let reusable=equivalent && render_state.borrow().get(token).is_some();
            if std::env::var_os("SLINTFORGE_PLOT_TRACE").is_some(){eprintln!("PLOT_CACHE token={token} identical={identical} equivalent={equivalent} reusable={reusable} revision={} stream={} points={:?}",v.revision,v.stream_revision,inputs.0.iter().map(|s|(s.0,s.1.row_count())).collect::<Vec<_>>());}
            let calculation = (|| -> Result<(Plot, Option<String>), String> {
                let title = plot_cstring(v.title.as_str()); let font_family = plot_cstring(v.font_family.as_str());
                let options = ForgePlotOptionsCore {
                width: v.width as f64,
                height: v.height as f64,
                font_size: v.font_size as f64,
                background: plot_color(v.background),
                foreground: plot_color(v.foreground),
                grid_color: plot_color(v.grid_color),
                selection_color: plot_color(v.selection_color),
                legend: (!v.hide_legend) as i32,
                adaptive: (!v.disable_adaptive) as i32,
                antialias: (!v.disable_antialias) as i32,
                max_points_per_pixel: v.max_points_per_pixel,
                legend_position: v.legend_position as i32,
                legend_style: v.legend_style as i32,
                legend_latest_value: v.legend_latest_value as i32,
                legend_precision: v.legend_precision,
                legend_placement: v.legend_placement as i32,
                measurement_enabled: v.measurement_enabled as i32,
                measurement_position: v.measurement_position as i32,
                measurement_placement: v.measurement_placement as i32,
                measurement_series: v.measurement_series,
                measurement_width: v.measurement_width as f64,
                measurement_height: v.measurement_height as f64,
                x_window_mode: v.x_window_mode as i32,
                x_window_axis: v.x_window_axis,
                x_window_size: v.x_window_size as f64,
                title: title.as_ptr(),
                font_family: font_family.as_ptr(),
                };
                // 所有输入指针由本作用域持有，引擎在 add 调用内复制。
                let mut plot = if reusable {let mut plot=render_state.borrow_mut().take(token).unwrap();unsafe{plot.begin_view_core(&options);}plot} else {unsafe { Plot::from_core(&options)? }};
                if !reusable {
                for v in panels.iter() {
                    let title = plot_cstring(v.title.as_str());
                    let category_labels=v.categories.iter().map(|v|plot_cstring(v.as_str())).collect::<Vec<_>>();
                    let categories=category_labels.iter().enumerate().map(|(i,v)|ForgePlotTickCore{value:i as f64,label:v.as_ptr()}).collect::<Vec<_>>();
                    let core = ForgePlotPanelCore {
                id: v.id,
                x: v.x as f64,
                y: v.y as f64,
                width: v.width as f64,
                height: v.height as f64,
                title: title.as_ptr(),
                categories: categories.as_ptr(),
                category_count: categories.len(),
                    }; unsafe { plot.add_panel_core(&core); }
                }
                }
                for v in axes.iter() {
                    let label = plot_cstring(v.label.as_str()); let format = plot_cstring(v.format.as_str());
                    let labels = v.ticks.iter().map(|t| plot_cstring(t.label.as_str())).collect::<Vec<_>>();
                    let ticks = v.ticks.iter().zip(labels.iter()).map(|(t,l)| ForgePlotTickCore {value:t.value as f64,label:l.as_ptr()}).collect::<Vec<_>>();
                    let core = ForgePlotAxisCore {
                id: v.id,
                panel: v.panel,
                side: v.side as i32,
                scale: v.scale as i32,
                ticker: v.ticker as i32,
                auto_range: (!v.manual_range) as i32,
                reversed: v.reversed as i32,
                visible: (!v.hidden) as i32,
                grid: (!v.hide_grid) as i32,
                precision: v.precision,
                minimum: v.minimum as f64,
                maximum: v.maximum as f64,
                tick_step: v.tick_step as f64,
                offset: v.offset as f64,
                time_origin: if v.time_origin.is_empty() {0.} else {v.time_origin.parse::<f64>().unwrap_or(f64::NAN)},
                label: label.as_ptr(),
                format: format.as_ptr(),
                ticks: ticks.as_ptr(),
                tick_count: ticks.len(),
                    }; unsafe { plot.add_axis_core(&core); }
                }
                for v in series.iter() {
                    let name = plot_cstring(v.name.as_str());
                    let point_styles=v.point_styles.iter().map(|v|ForgePlotPointStyleCore{index:v.index,hidden:v.hidden as i32,size:v.size as f64,color:plot_color(v.color)}).collect::<Vec<_>>();
                    let core = ForgePlotSeriesCore {
                id: v.id,
                kind: v.kind as i32,
                x_axis: v.x_axis,
                y_axis: if v.y_axis == 0 && v.x_axis == 0 { 1 } else { v.y_axis },
                line_style: if v.line_style as i32 == 0 { 1 } else { v.line_style as i32 - 1 },
                scatter_style: v.scatter_style as i32,
                hidden: v.hidden as i32,
                selectable: (!v.unselectable) as i32,
                stack_group: if v.stacked { v.stack_group } else { -1 },
                fill_target: if v.channel_fill { v.fill_target } else { -1 },
                layer: v.layer,
                sorted: v.sorted as i32,
                color: plot_color(v.color),
                fill_color: plot_color(v.fill_color),
                line_width: if v.line_width > 0. { v.line_width as f64 } else { 2. },
                point_size: if v.point_size > 0. { v.point_size as f64 } else { 6. },
                bar_width: if v.bar_width > 0. { v.bar_width as f64 } else { 0.8 },
                baseline: v.baseline as f64,
                name: name.as_ptr(),
                inner_radius: v.inner_radius as f64,
                point_styles: point_styles.as_ptr(),
                point_style_count: point_styles.len(),
                    };
                    if reusable {unsafe{plot.update_series_core(&core);}continue;}
                    let mut labels = Vec::new(); let mut outliers = Vec::new();
                    let mut points = Vec::with_capacity(v.points.row_count());
                    for v in v.points.iter() {
                        let label_pointer = if v.label.is_empty() { std::ptr::null() } else {labels.push(plot_cstring(v.label.as_str())); labels.last().unwrap().as_ptr()};
                        let outlier_count = v.outliers.row_count();
                        let outlier_pointer = if outlier_count == 0 {std::ptr::null()} else {outliers.push(v.outliers.iter().map(|x|x as f64).collect::<Vec<_>>());outliers.last().unwrap().as_ptr()};
                        points.push(ForgePlotPointCore {
                x: v.x as f64,
                y: v.y as f64,
                low: v.low as f64,
                high: v.high as f64,
                open: v.open as f64,
                close: v.close as f64,
                q1: v.q1 as f64,
                q3: v.q3 as f64,
                error_x_minus: v.error_x_minus as f64,
                error_x_plus: v.error_x_plus as f64,
                error_y_minus: v.error_y_minus as f64,
                error_y_plus: v.error_y_plus as f64,
                label: label_pointer,
                outliers: outlier_pointer,
                outlier_count: outlier_count,
                        });
                    }
                    unsafe { plot.add_series_core(&core,&points); }
                }
                if !reusable {
                for v in maps.iter() {
                    let name = plot_cstring(v.name.as_str());
                    let core = ForgePlotMapCore {
                id: v.id,
                panel: v.panel,
                x_axis: v.x_axis,
                y_axis: if v.y_axis == 0 && v.x_axis == 0 { 1 } else { v.y_axis },
                columns: v.columns,
                rows: v.rows,
                palette: v.palette,
                auto_range: (!v.manual_range) as i32,
                logarithmic: v.logarithmic as i32,
                hidden: v.hidden as i32,
                interpolate: v.interpolate as i32,
                color_scale: (!v.hide_color_scale) as i32,
                layer: v.layer,
                x_min: v.x_min as f64,
                x_max: v.x_max as f64,
                y_min: v.y_min as f64,
                y_max: v.y_max as f64,
                z_min: v.z_min as f64,
                z_max: v.z_max as f64,
                name: name.as_ptr(),
                    }; let values = v.values.iter().map(|x|x as f64).collect::<Vec<_>>();
                    unsafe { plot.add_map_core(&core,&values); }
                }
                for v in items.iter() {
                    let text = plot_cstring(v.text.as_str()); let image = plot_cstring(v.image_data.as_str());
                    let core = ForgePlotItemCore {
                id: v.id,
                kind: v.kind as i32,
                panel: v.panel,
                x_axis: v.x_axis,
                y_axis: if v.y_axis == 0 && v.x_axis == 0 { 1 } else { v.y_axis },
                coordinates: v.coordinates as i32,
                anchor: if v.anchored { v.anchor } else { -1 },
                anchor_part: v.anchor_part,
                clip: (!v.unclipped) as i32,
                layer: v.layer,
                selectable: (v.selectable) as i32,
                x: v.x as f64,
                y: v.y as f64,
                x2: v.x2 as f64,
                y2: v.y2 as f64,
                x3: v.x3 as f64,
                y3: v.y3 as f64,
                x4: v.x4 as f64,
                y4: v.y4 as f64,
                width: if v.width > 0. { v.width as f64 } else { 1. },
                size: if v.size > 0. { v.size as f64 } else { 12. },
                color: plot_color(v.color),
                fill_color: plot_color(v.fill_color),
                text: text.as_ptr(),
                image: image.as_ptr(),
                image_pixels: std::ptr::null(),
                image_byte_count: 0,
                image_width: 0,
                image_height: 0,
                    }; unsafe { plot.add_item_core(&core); }
                }
                }
                let selection=selection.iter().map(|s|ForgePlotSelectionCore{id:s.id,first:s.first,last:s.last}).collect::<Vec<_>>();
                plot.set_selection(&selection);let error=plot.render().err();Ok((plot,error))
            })();
            match calculation {
                Ok((plot,error)) => {
                    if let Some(error)=error {result.status=2;result.error=error.into();render_state.borrow_mut().store(token,plot);inputs_cache.borrow_mut().insert(token,inputs);return result;}
                    let stats = plot.stats(); result.status = stats.status;
                    result.input_points = stats.input_points.min(i32::MAX as u64) as i32;
                    result.rendered_points = stats.rendered_points.min(i32::MAX as u64) as i32;
                    result.path_count = stats.path_count.min(i32::MAX as u64) as i32;
                    result.svg_bytes = stats.svg_bytes.min(i32::MAX as u64) as i32;
                    result.svg = plot.svg().into();
                    match plot.display_image(v.render_scale) {Ok(image) => result.image = image,Err(e) => {result.status=2;result.error=e.into();}}
                    result.ranges = slint::ModelRc::new(slint::VecModel::from(plot.ranges().into_iter().map(|v| {
                        let mut item=<$range>::default();item.id=v.id;item.scale=v.scale;item.reversed=v.reversed!=0;item.minimum=v.minimum as f32;item.maximum=v.maximum as f32;item
                    }).collect::<Vec<_>>()));
                    result.domains = slint::ModelRc::new(slint::VecModel::from(plot.domains().into_iter().map(|v| {
                        let mut item=<$range>::default();item.id=v.id;item.scale=v.scale;item.reversed=v.reversed!=0;item.minimum=v.minimum as f32;item.maximum=v.maximum as f32;item
                    }).collect::<Vec<_>>()));
                    if let Some(range)=result.ranges.iter().find(|r|r.id==v.x_window_axis){result.window_range=range;}
                    if let Some(range)=result.domains.iter().find(|r|r.id==v.x_window_axis){result.history_range=range;}
                    result.rects = slint::ModelRc::new(slint::VecModel::from(plot.rects().into_iter().map(|v| {
                        let mut item=<$rect>::default();item.id=v.id;item.x=v.x as f32;item.y=v.y as f32;item.width=v.width as f32;item.height=v.height as f32;item
                    }).collect::<Vec<_>>()));
                    for kind in 0..2 {let v=plot.annotation_rect(kind);let mut item=<$rect>::default();item.id=v.id;item.x=v.x as f32;item.y=v.y as f32;item.width=v.width as f32;item.height=v.height as f32;if kind==0{result.legend_rect=item;}else{result.measurement_rect=item;}}
                    result.measurement_axis=series.iter().find(|s|s.id==v.measurement_series).map(|s|s.x_axis).unwrap_or(v.x_window_axis);
                    result.measurement_range=result.ranges.iter().find(|r|r.id==result.measurement_axis).unwrap_or_default();
                    render_state.borrow_mut().store(token,plot);
                    inputs_cache.borrow_mut().insert(token,inputs);
                }
                Err(error) => {result.status=2;result.error=error.into();render_state.borrow_mut().release(token);inputs_cache.borrow_mut().remove(&token);}
            } result
        });
        let query = state.clone(); service.on_hit_test(move |token,x,y,tolerance| {
            let state = query.borrow(); let mut out=<$hit>::default();out.kind=-1;out.id=-1;out.index=-1;
            if let Some(plot)=state.get(token) {let v=plot.hit(x as f64,y as f64,tolerance as f64);
                out.kind = v.kind;
                out.id = v.id;
                out.index = v.index;
                out.panel = v.panel;
                out.x = v.x as f32;
                out.y = v.y as f32;
                out.value = v.value as f32;
                out.pixel_x = v.pixel_x as f32;
                out.pixel_y = v.pixel_y as f32;
                out.distance = v.distance as f32;
                out.label = v.label.into();
            } out
        });
        let query = state.clone(); service.on_coordinate(move |token,axis,value,inverse| query.borrow().get(token).map(|p|p.coordinate(axis,value as f64,inverse) as f32).unwrap_or(f32::NAN));
        let query=state.clone();service.on_measure(move |token,id,x| {
            let mut out=<$hit>::default();out.kind=-1;out.id=-1;out.index=-1;
            if let Some(plot)=query.borrow().get(token){let v=plot.measure(id,x as f64);
                out.kind = v.kind;
                out.id = v.id;
                out.index = v.index;
                out.panel = v.panel;
                out.x = v.x as f32;
                out.y = v.y as f32;
                out.value = v.value as f32;
                out.pixel_x = v.pixel_x as f32;
                out.pixel_y = v.pixel_y as f32;
                out.distance = v.distance as f32;
                out.label = v.label.into();
            }out
        });
        let append=state.clone();service.on_append(move |token,id,model| {
            let mut labels=Vec::new();let mut outliers=Vec::new();let mut points=Vec::with_capacity(model.row_count());
            for v in model.iter(){
                let label_pointer=if v.label.is_empty(){std::ptr::null()}else{labels.push(plot_cstring(v.label.as_str()));labels.last().unwrap().as_ptr()};
                let outlier_count=v.outliers.row_count();let outlier_pointer=if outlier_count==0{std::ptr::null()}else{outliers.push(v.outliers.iter().map(|x|x as f64).collect::<Vec<_>>());outliers.last().unwrap().as_ptr()};
                points.push(ForgePlotPointCore{
                x: v.x as f64,
                y: v.y as f64,
                low: v.low as f64,
                high: v.high as f64,
                open: v.open as f64,
                close: v.close as f64,
                q1: v.q1 as f64,
                q3: v.q3 as f64,
                error_x_minus: v.error_x_minus as f64,
                error_x_plus: v.error_x_plus as f64,
                error_y_minus: v.error_y_minus as f64,
                error_y_plus: v.error_y_plus as f64,
                label: label_pointer,
                outliers: outlier_pointer,
                outlier_count: outlier_count,
                });
            }
            append.borrow_mut().get_mut(token).map(|p|unsafe{p.append_core(id,&points)}).unwrap_or(false)
        });
        let query = state.clone(); service.on_select_rect(move |token,x,y,width,height| {
            let ranges=query.borrow().get(token).map(|p|p.select_rect(x as f64,y as f64,width as f64,height as f64)).unwrap_or_default();
            slint::ModelRc::new(slint::VecModel::from(ranges.into_iter().map(|(id,first,last)| {let mut r=<$selection>::default();r.id=id;r.first=first;r.last=last;r}).collect::<Vec<_>>()))
        });
        service.on_merge_selection(move |a,b,mode| {
            let a=a.iter().map(|s|(s.id,s.first,s.last)).collect::<Vec<_>>();let b=b.iter().map(|s|(s.id,s.first,s.last)).collect::<Vec<_>>();
            let ranges=$crate::plot::merge_plot_selection(&a,&b,mode);
            slint::ModelRc::new(slint::VecModel::from(ranges.into_iter().map(|(id,first,last)| {let mut r=<$selection>::default();r.id=id;r.first=first;r.last=last;r}).collect::<Vec<_>>()))
        });
        service.on_axis_index(|axes,id| axes.iter().position(|axis| axis.id == id).map(|index| index as i32).unwrap_or(-1));
        let query = state.clone(); service.on_change_view(move |token,axes,ranges,panel,mode,x,y,w,h,directions| {
            let state=query.borrow();let plot=state.get(token);
            slint::ModelRc::new(slint::VecModel::from(axes.iter().map(|mut a| {
                let horizontal=(a.side as i32) < 2;
                if (panel<0||a.panel==panel) && (if horizontal {directions%2!=0}else{directions>=2}) {
                    if mode==3 {a.manual_range=false;} else if let Some(plot)=plot {
                        if let Some(r)=ranges.iter().find(|r|r.id==a.id) {
                            let range=ForgePlotRangeCore{id:r.id,scale:r.scale,reversed:r.reversed as i32,minimum:r.minimum as f64,maximum:r.maximum as f64};
                            let result=plot.view(&range,mode,if horizontal{x}else{y},if horizontal{w}else{h});
                            a.manual_range=true;a.minimum=result.minimum as f32;a.maximum=result.maximum as f32;
                        }
                    }
                } a
            }).collect::<Vec<_>>()))
        });
        service.on_cartesian_panel(|series,axes,panel| series.row_count()==0||series.iter().any(|s|{let kind=s.kind as i32;(kind<=5||kind==7)&&(panel<0||axes.iter().find(|a|a.id==s.x_axis).map(|a|a.panel).unwrap_or(s.x_axis/2)==panel)}));
        service.on_cartesian_series(|series,id| series.iter().any(|s|s.id==id&&{let kind=s.kind as i32;kind<=5||kind==7}));
        service.on_toggle_point(move |series,id,index| slint::ModelRc::new(slint::VecModel::from(series.iter().map(|mut s| {
            if s.id==id {let mut styles=s.point_styles.iter().collect::<Vec<_>>();if let Some(style)=styles.iter_mut().find(|v|v.index==index){style.hidden=!style.hidden;}else{let mut style=styles.first().cloned().unwrap_or_default();style.index=index;style.hidden=true;style.size=if s.point_size>0.{s.point_size}else{6.};style.color=Default::default();styles.push(style);}s.point_styles=slint::ModelRc::new(slint::VecModel::from(styles));}s
        }).collect::<Vec<_>>())));

    service.on_toggle_series(move |series,id| slint::ModelRc::new(slint::VecModel::from(series.iter().map(|mut s| {if s.id==id{s.hidden=!s.hidden;}s}).collect::<Vec<_>>())));
        let query=state.clone();service.on_scroll_view(move |token,axes,id,position,span|{
            let bounds=query.borrow().get(token).and_then(|p|p.domains().into_iter().find(|r|r.id==id));
            slint::ModelRc::new(slint::VecModel::from(axes.iter().map(|mut a|{if a.id==id{if let Some(b)=bounds{let width=(span as f64).max(0.000001);a.manual_range=true;a.minimum=(b.minimum+(b.maximum-b.minimum-width).max(0.)*(position as f64).clamp(0.,1.)) as f32;a.maximum=a.minimum+width as f32;}}a}).collect::<Vec<_>>()))
        });
        service.on_export(move |token,path,format| {
            let mut result=<$export>::default();
            match state.borrow().get(token).ok_or_else(||"图表实例不存在".to_string()).and_then(|p|p.export(std::path::Path::new(path.as_str()),format.as_str())) {Ok(())=>result.ok=true,Err(e)=>result.error=e.into()};result
        });
        service.set_connected(true);
    }};
}
