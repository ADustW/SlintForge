"""从 C 数据布局生成两端 Slint 薄适配层；数值算法只有共用引擎一份。"""
import argparse
import pathlib
import re

ROOT = pathlib.Path(__file__).resolve().parents[1]
RAW = (ROOT / "sdk/runtime-api/src/plot.rs").read_text(encoding="utf-8")
FIELDS = {name: re.findall(r"pub (\w+): ([^,]+),", body) for name, body in re.findall(r"pub struct (\w+) \{(.*?)\n\}", RAW, re.S)}

ENUMS = {"kind", "side", "scale", "ticker", "scatter_style", "coordinates", "legend_position", "legend_style", "legend_placement", "measurement_position", "measurement_placement", "x_window_mode"}
BOOLEANS = {"hidden", "reversed", "sorted", "logarithmic", "interpolate", "legend_latest_value", "measurement_enabled"}
SPECIAL = {
    "ForgePlotOptionsCore": {"legend": "!v.hide_legend", "adaptive": "!v.disable_adaptive", "antialias": "!v.disable_antialias"},
    "ForgePlotAxisCore": {"auto_range": "!v.manual_range", "visible": "!v.hidden", "grid": "!v.hide_grid", "tick_count": "ticks.len()", "ticks": "ticks.as_ptr()"},
    "ForgePlotSeriesCore": {"selectable": "!v.unselectable", "stack_group": "if v.stacked { v.stack_group } else { -1 }", "fill_target": "if v.channel_fill { v.fill_target } else { -1 }", "line_style": "if v.line_style as i32 == 0 { 1 } else { v.line_style as i32 - 1 }", "y_axis": "if v.y_axis == 0 && v.x_axis == 0 { 1 } else { v.y_axis }", "line_width": "if v.line_width > 0. { v.line_width as f64 } else { 2. }", "point_size": "if v.point_size > 0. { v.point_size as f64 } else { 6. }", "bar_width": "if v.bar_width > 0. { v.bar_width as f64 } else { 0.8 }"},
    "ForgePlotPointCore": {"label": "label_pointer", "outliers": "outlier_pointer", "outlier_count": "outlier_count"},
    "ForgePlotMapCore": {"auto_range": "!v.manual_range", "color_scale": "!v.hide_color_scale", "y_axis": "if v.y_axis == 0 && v.x_axis == 0 { 1 } else { v.y_axis }"},
    "ForgePlotItemCore": {"anchor": "if v.anchored { v.anchor } else { -1 }", "clip": "!v.unclipped", "selectable": "v.selectable", "image": "image.as_ptr()", "y_axis": "if v.y_axis == 0 && v.x_axis == 0 { 1 } else { v.y_axis }", "width": "if v.width > 0. { v.width as f64 } else { 1. }", "size": "if v.size > 0. { v.size as f64 } else { 12. }"},
}
CPP_SPECIAL = {
    "tick_count": "ticks.size()", "ticks": "ticks.data()",
    "stack_group": "v.stacked ? v.stack_group : -1", "fill_target": "v.channel_fill ? v.fill_target : -1",
    "line_style": "static_cast<int>(v.line_style) == 0 ? 1 : static_cast<int>(v.line_style) - 1",
    "y_axis": "v.y_axis == 0 && v.x_axis == 0 ? 1 : v.y_axis",
    "anchor": "v.anchored ? v.anchor : -1", "image": "image.c_str()",
    "line_width": "v.line_width > 0 ? v.line_width : 2", "point_size": "v.point_size > 0 ? v.point_size : 6", "bar_width": "v.bar_width > 0 ? v.bar_width : .8",
    "width": "v.width > 0 ? v.width : 1", "size": "v.size > 0 ? v.size : 12",
    "image_pixels": "nullptr", "image_byte_count": "0", "image_width": "0", "image_height": "0",
}
SPECIAL["ForgePlotItemCore"].update({"image_pixels":"std::ptr::null()","image_byte_count":"0","image_width":"0","image_height":"0"})
SPECIAL["ForgePlotAxisCore"]["time_origin"]="if v.time_origin.is_empty() {0.} else {v.time_origin.parse::<f64>().unwrap_or(f64::NAN)}"
CPP_SPECIAL["time_origin"]="forge_plot_ui_number(v.time_origin)"
def assignments(typ, lang):
    out = []
    for name, field_type in FIELDS[typ]:
        special = SPECIAL.get(typ, {}).get(name)
        if special:
            value = CPP_SPECIAL.get(name, special) if lang == "cpp" else special
            if lang == "rust" and field_type == "i32" and (value.startswith("!") or name == "selectable"):
                value = "(" + value + ") as i32"
        elif field_type.startswith("*const std::ffi::c_char"):
            value = name + (".as_ptr()" if lang == "rust" else ".c_str()")
        elif field_type == "u32":
            value = ("plot_color" if lang == "rust" else "forge_plot_ui_color") + "(v." + name + ")"
        elif field_type == "f64":
            value = "v." + name + (" as f64" if lang == "rust" else "")
        elif name in ENUMS or name in BOOLEANS:
            value = "v." + name + " as i32" if lang == "rust" else "static_cast<int>(v." + name + ")"
        else:
            value = "v." + name
        out.append("                " + (name + ": " + value + "," if lang == "rust" else "core." + name + " = " + value + ";"))
    return "\n".join(out)

def rust():
    out = '''// 由 tools/generate_plot_bindings.py 生成。
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
@POINT_EQUAL_RUST@
            }))))).unwrap_or(false);
            let reusable=equivalent && render_state.borrow().get(token).is_some();
            if std::env::var_os("SLINTFORGE_PLOT_TRACE").is_some(){eprintln!("PLOT_CACHE token={token} identical={identical} equivalent={equivalent} reusable={reusable} revision={} stream={} points={:?}",v.revision,v.stream_revision,inputs.0.iter().map(|s|(s.0,s.1.row_count())).collect::<Vec<_>>());}
            let calculation = (|| -> Result<(Plot, Option<String>), String> {
                let title = plot_cstring(v.title.as_str()); let font_family = plot_cstring(v.font_family.as_str());
                let options = ForgePlotOptionsCore {
@OPTIONS@
                };
                // 所有输入指针由本作用域持有，引擎在 add 调用内复制。
                let mut plot = if reusable {let mut plot=render_state.borrow_mut().take(token).unwrap();unsafe{plot.begin_view_core(&options);}plot} else {unsafe { Plot::from_core(&options)? }};
                if !reusable {
                for v in panels.iter() {
                    let title = plot_cstring(v.title.as_str());
                    let core = ForgePlotPanelCore {
@PANEL@
                    }; unsafe { plot.add_panel_core(&core); }
                }
                }
                for v in axes.iter() {
                    let label = plot_cstring(v.label.as_str()); let format = plot_cstring(v.format.as_str());
                    let labels = v.ticks.iter().map(|t| plot_cstring(t.label.as_str())).collect::<Vec<_>>();
                    let ticks = v.ticks.iter().zip(labels.iter()).map(|(t,l)| ForgePlotTickCore {value:t.value as f64,label:l.as_ptr()}).collect::<Vec<_>>();
                    let core = ForgePlotAxisCore {
@AXIS@
                    }; unsafe { plot.add_axis_core(&core); }
                }
                for v in series.iter() {
                    let name = plot_cstring(v.name.as_str());
                    let core = ForgePlotSeriesCore {
@SERIES@
                    };
                    if reusable {unsafe{plot.update_series_core(&core);}continue;}
                    let mut labels = Vec::new(); let mut outliers = Vec::new();
                    let mut points = Vec::with_capacity(v.points.row_count());
                    for v in v.points.iter() {
                        let label_pointer = if v.label.is_empty() { std::ptr::null() } else {labels.push(plot_cstring(v.label.as_str())); labels.last().unwrap().as_ptr()};
                        let outlier_count = v.outliers.row_count();
                        let outlier_pointer = if outlier_count == 0 {std::ptr::null()} else {outliers.push(v.outliers.iter().map(|x|x as f64).collect::<Vec<_>>());outliers.last().unwrap().as_ptr()};
                        points.push(ForgePlotPointCore {
@POINT@
                        });
                    }
                    unsafe { plot.add_series_core(&core,&points); }
                }
                if !reusable {
                for v in maps.iter() {
                    let name = plot_cstring(v.name.as_str());
                    let core = ForgePlotMapCore {
@MAP@
                    }; let values = v.values.iter().map(|x|x as f64).collect::<Vec<_>>();
                    unsafe { plot.add_map_core(&core,&values); }
                }
                for v in items.iter() {
                    let text = plot_cstring(v.text.as_str()); let image = plot_cstring(v.image_data.as_str());
                    let core = ForgePlotItemCore {
@ITEM@
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
@HIT@
            } out
        });
        let query = state.clone(); service.on_coordinate(move |token,axis,value,inverse| query.borrow().get(token).map(|p|p.coordinate(axis,value as f64,inverse) as f32).unwrap_or(f32::NAN));
        let query=state.clone();service.on_measure(move |token,id,x| {
            let mut out=<$hit>::default();out.kind=-1;out.id=-1;out.index=-1;
            if let Some(plot)=query.borrow().get(token){let v=plot.measure(id,x as f64);
@HIT@
            }out
        });
        let append=state.clone();service.on_append(move |token,id,model| {
            let mut labels=Vec::new();let mut outliers=Vec::new();let mut points=Vec::with_capacity(model.row_count());
            for v in model.iter(){
                let label_pointer=if v.label.is_empty(){std::ptr::null()}else{labels.push(plot_cstring(v.label.as_str()));labels.last().unwrap().as_ptr()};
                let outlier_count=v.outliers.row_count();let outlier_pointer=if outlier_count==0{std::ptr::null()}else{outliers.push(v.outliers.iter().map(|x|x as f64).collect::<Vec<_>>());outliers.last().unwrap().as_ptr()};
                points.push(ForgePlotPointCore{
@POINT@
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
'''
    for tag, typ in [("OPTIONS","Options"),("PANEL","Panel"),("AXIS","Axis"),("SERIES","Series"),("POINT","Point"),("MAP","Map"),("ITEM","Item")]:
        out = out.replace("@"+tag+"@", assignments("ForgePlot"+typ+"Core","rust"))
    equal=" && ".join("a."+name+".to_bits()==b."+name+".to_bits()" for name,typ in FIELDS["ForgePlotPointCore"] if typ=="f64")
    out=out.replace("@POINT_EQUAL_RUST@",equal+" && a.label==b.label && (a.outliers==b.outliers || (a.outliers.row_count()==b.outliers.row_count() && a.outliers.iter().zip(b.outliers.iter()).all(|(x,y)|x.to_bits()==y.to_bits())))")
    hit = []
    for field, typ in FIELDS["ForgePlotHitCore"]:
        hit.append("                out."+field+" = v."+field+(" as f32" if typ=="f64" else ".into()" if field=="label" else "")+";")
    return out.replace("@HIT@","\n".join(hit))

def cpp_loop(model, typ, prelude, epilogue):
    return """        for(size_t i=0;@MODEL@ && i<@MODEL@->row_count();++i){const auto item=@MODEL@->row_data(i);if(!item)continue;const auto& v=*item;
@PRELUDE@
            ForgePlot@TYPE@Core core{};
@FIELDS@
@EPILOGUE@
        }
""".replace("@MODEL@",model).replace("@TYPE@",typ).replace("@PRELUDE@",prelude).replace("@FIELDS@",assignments("ForgePlot"+typ+"Core","cpp")).replace("@EPILOGUE@",epilogue)

def cpp():
    out = '''// 由 tools/generate_plot_bindings.py 生成。
#pragma once
template<class Service,class Scene,class Axis,class Range,class Rect,class Hit,class Selection,class Export,class Handle>
void forge_bind_plot_services(const Handle& ui) {
    const auto state=std::make_shared<ForgePlotServiceState>();const auto& service=ui->template global<Service>();
    service.on_allocate([state](){return state->allocate();});service.on_release([state](int id){state->plots.erase(id);state->inputs.erase(id);state->point_inputs.erase(id);});
    service.on_prepare([state](int token,auto axes,auto panels,auto series,auto maps,auto items,auto selection,auto v){
        Scene result{};result.token=token;
        const std::string title(v.title),font_family(v.font_family);ForgePlotOptionsCore core{};
@OPTIONS@
        const auto input=std::make_pair(std::array<std::shared_ptr<void>,4>{nullptr,maps&&maps->row_count()?maps:nullptr,items&&items->row_count()?items:nullptr,panels&&panels->row_count()?panels:nullptr},v.revision);
        std::vector<std::pair<int,std::shared_ptr<void>>> point_inputs;for(size_t i=0;series&&i<series->row_count();++i){const auto s=series->row_data(i);if(s)point_inputs.emplace_back(s->id,s->points);}
        bool same_points=state->point_inputs.count(token)&&state->point_inputs[token]==point_inputs;
        if(!same_points&&state->point_inputs.count(token)&&state->point_inputs[token].size()==point_inputs.size()){
            same_points=true;size_t previous_index=0;
            for(size_t i=0;series&&i<series->row_count();++i){const auto s=series->row_data(i);if(!s)continue;const auto& cached=state->point_inputs[token][previous_index++];
                using PointModel=typename std::remove_cvref_t<decltype(s->points)>::element_type;
                const auto previous=std::static_pointer_cast<PointModel>(cached.second);const auto next=s->points;
                if(cached.first!=s->id){same_points=false;break;}if(previous==next)continue;
                if(!previous||!next){if((previous&&previous->row_count())||(next&&next->row_count())){same_points=false;break;}continue;}
                if(previous->row_count()!=next->row_count()){same_points=false;break;}
                for(size_t j=0;j<next->row_count()&&same_points;++j){const auto a=previous->row_data(j),b=next->row_data(j);if(!a||!b){same_points=a.has_value()==b.has_value();continue;}
@POINT_EQUAL_CPP@
                    if(a->outliers!=b->outliers){const size_t n=a->outliers?a->outliers->row_count():0,m=b->outliers?b->outliers->row_count():0;if(n!=m){same_points=false;break;}for(size_t k=0;k<n;++k){const auto x=a->outliers->row_data(k),y=b->outliers->row_data(k);if(x.has_value()!=y.has_value()||(x&&std::bit_cast<uint32_t>(*x)!=std::bit_cast<uint32_t>(*y))){same_points=false;break;}}}
                }
                if(!same_points)break;
            }
        }
        const bool reusable=state->inputs.count(token)&&state->inputs[token]==input&&same_points&&state->plots.count(token);
        auto plot=reusable?state->plots[token]:std::make_shared<ForgePlot>(core);if(reusable)plot->begin_view(core);
@LOOPS@
        std::vector<ForgePlotSelectionCore> selected;for(size_t i=0;selection&&i<selection->row_count();++i){const auto s=selection->row_data(i);if(s)selected.push_back({s->id,s->first,s->last});}plot->set_selection(selected);
        const auto error=plot->render();if(!error.empty()){result.status=2;result.error=error.c_str();state->plots[token]=plot;state->inputs[token]=input;state->point_inputs[token]=point_inputs;return result;}
        const auto stats=plot->info();result.status=stats.status;result.image=plot->display_image(v.render_scale);result.svg=plot->svg().c_str();
        result.input_points=static_cast<int>(std::min<uint64_t>(stats.input_points,2147483647));result.rendered_points=static_cast<int>(std::min<uint64_t>(stats.rendered_points,2147483647));
        result.path_count=static_cast<int>(std::min<uint64_t>(stats.path_count,2147483647));result.svg_bytes=static_cast<int>(std::min<uint64_t>(stats.svg_bytes,2147483647));
        if(result.image.size().width==0){result.status=2;result.error="SVG 加载失败";}
        std::vector<Range> ranges;for(const auto& v:plot->ranges()){Range r{};r.id=v.id;r.scale=v.scale;r.reversed=v.reversed!=0;r.minimum=static_cast<float>(v.minimum);r.maximum=static_cast<float>(v.maximum);ranges.push_back(r);}result.ranges=std::make_shared<slint::VectorModel<Range>>(std::move(ranges));
        std::vector<Range> domains;for(const auto& v:plot->domains()){Range r{};r.id=v.id;r.scale=v.scale;r.reversed=v.reversed!=0;r.minimum=static_cast<float>(v.minimum);r.maximum=static_cast<float>(v.maximum);domains.push_back(r);}result.domains=std::make_shared<slint::VectorModel<Range>>(std::move(domains));
        for(size_t i=0;i<result.ranges->row_count();++i){const auto r=result.ranges->row_data(i);if(r&&r->id==v.x_window_axis)result.window_range=*r;}
        for(size_t i=0;i<result.domains->row_count();++i){const auto r=result.domains->row_data(i);if(r&&r->id==v.x_window_axis)result.history_range=*r;}
        std::vector<Rect> rects;for(const auto& v:plot->rects()){Rect r{};r.id=v.id;r.x=static_cast<float>(v.x);r.y=static_cast<float>(v.y);r.width=static_cast<float>(v.width);r.height=static_cast<float>(v.height);rects.push_back(r);}result.rects=std::make_shared<slint::VectorModel<Rect>>(std::move(rects));
        for(int kind=0;kind<2;++kind){const auto v=plot->annotation_rect(kind);Rect r{};r.id=v.id;r.x=static_cast<float>(v.x);r.y=static_cast<float>(v.y);r.width=static_cast<float>(v.width);r.height=static_cast<float>(v.height);if(kind==0)result.legend_rect=r;else result.measurement_rect=r;}
        result.measurement_axis=v.x_window_axis;for(size_t i=0;series&&i<series->row_count();++i)if(const auto s=series->row_data(i);s&&s->id==v.measurement_series){result.measurement_axis=s->x_axis;break;}
        for(size_t i=0;i<result.ranges->row_count();++i)if(const auto r=result.ranges->row_data(i);r&&r->id==result.measurement_axis){result.measurement_range=*r;break;}
        state->plots[token]=std::move(plot);state->inputs[token]=input;state->point_inputs[token]=point_inputs;return result;
    });
    service.on_hit_test([state](int token,float x,float y,float tolerance){Hit out{};out.kind=-1;out.id=-1;out.index=-1;const auto it=state->plots.find(token);if(it!=state->plots.end()){const auto v=it->second->hit(x,y,tolerance);
@HIT@
        }return out;});
    service.on_coordinate([state](int token,int axis,float value,bool inverse){const auto it=state->plots.find(token);return it==state->plots.end()?std::numeric_limits<float>::quiet_NaN():static_cast<float>(it->second->coordinate(axis,value,inverse));});
    service.on_measure([state](int token,int id,float x){Hit out{};out.kind=-1;out.id=-1;out.index=-1;const auto it=state->plots.find(token);if(it!=state->plots.end()){const auto v=it->second->measure(id,x);
@HIT@
        }return out;});
    service.on_append([state](int token,int id,auto model){const auto it=state->plots.find(token);if(it==state->plots.end())return false;
        std::vector<ForgePlotPointCore> points;std::deque<std::string> labels;std::vector<std::vector<double>> outliers;
        for(size_t j=0;model&&j<model->row_count();++j){const auto item=model->row_data(j);if(!item)continue;const auto& v=*item;
            const char* label_pointer=nullptr;if(!v.label.empty()){labels.emplace_back(v.label);label_pointer=labels.back().c_str();}
            const size_t outlier_count=v.outliers?v.outliers->row_count():0;const double* outlier_pointer=nullptr;
            if(outlier_count){outliers.emplace_back();for(size_t k=0;k<outlier_count;++k){const auto value=v.outliers->row_data(k);outliers.back().push_back(value?*value:0);}outlier_pointer=outliers.back().data();}
            ForgePlotPointCore point{};
@APPEND_POINT@
            points.push_back(point);
        }return it->second->append(id,points);
    });
    service.on_select_rect([state](int token,float x,float y,float width,float height){std::vector<Selection> out;const auto it=state->plots.find(token);if(it!=state->plots.end())for(const auto& [id,first,last]:it->second->select_rect(x,y,width,height)){Selection r{};r.id=id;r.first=first;r.last=last;out.push_back(r);}return std::make_shared<slint::VectorModel<Selection>>(std::move(out));});
    service.on_merge_selection([](auto a,auto b,int mode){std::vector<std::tuple<int,int,int>> current,next;const auto append=[](const auto& model,auto& list){for(size_t i=0;model&&i<model->row_count();++i){const auto s=model->row_data(i);if(s)list.emplace_back(s->id,s->first,s->last);}};append(a,current);append(b,next);std::vector<Selection> out;for(const auto& [id,first,last]:forge_plot_merge_selection(current,next,mode)){Selection r{};r.id=id;r.first=first;r.last=last;out.push_back(r);}return std::make_shared<slint::VectorModel<Selection>>(std::move(out));});
    service.on_change_view([state](int token,auto axes,auto ranges,int panel,int mode,float x,float y,float w,float h,int directions){std::vector<Axis> out;const auto it=state->plots.find(token);
        for(size_t i=0;axes&&i<axes->row_count();++i){const auto value=axes->row_data(i);if(!value)continue;auto a=*value;const bool horizontal=static_cast<int>(a.side)<2;
            if((panel<0||a.panel==panel)&&(horizontal?directions%2!=0:directions>=2)){if(mode==3)a.manual_range=false;else if(it!=state->plots.end())for(size_t j=0;ranges&&j<ranges->row_count();++j){const auto r=ranges->row_data(j);if(r&&r->id==a.id){ForgePlotRangeCore range{r->id,r->scale,r->reversed?1:0,r->minimum,r->maximum};const auto next=it->second->view(range,mode,horizontal?x:y,horizontal?w:h);a.manual_range=true;a.minimum=static_cast<float>(next.minimum);a.maximum=static_cast<float>(next.maximum);break;}}}out.push_back(a);
        }return std::make_shared<slint::VectorModel<Axis>>(std::move(out));});
    service.on_toggle_series([](auto series,int id){using Item=std::remove_cvref_t<decltype(series->row_data(0).value())>;std::vector<Item> out;for(size_t i=0;series&&i<series->row_count();++i){const auto value=series->row_data(i);if(value){auto s=*value;if(s.id==id)s.hidden=!s.hidden;out.push_back(s);}}return std::make_shared<slint::VectorModel<Item>>(std::move(out));});
    service.on_scroll_view([state](int token,auto axes,int id,float position,float span){std::vector<Axis> out;const auto it=state->plots.find(token);
        for(size_t i=0;axes&&i<axes->row_count();++i){const auto value=axes->row_data(i);if(!value)continue;auto a=*value;if(a.id==id&&it!=state->plots.end())for(const auto& b:it->second->domains())if(b.id==id){const double width=std::max(.000001,static_cast<double>(span));a.manual_range=true;a.minimum=static_cast<float>(b.minimum+std::max(0.,b.maximum-b.minimum-width)*std::clamp(static_cast<double>(position),0.,1.));a.maximum=a.minimum+static_cast<float>(width);break;}out.push_back(a);}return std::make_shared<slint::VectorModel<Axis>>(std::move(out));
    });
    service.on_export([state](int token,slint::SharedString path,slint::SharedString format){Export result{};const auto it=state->plots.find(token);if(it==state->plots.end()){result.error="图表实例不存在";return result;}const auto error=it->second->export_file(std::filesystem::u8path(std::string(path)),std::string(format));result.ok=error.empty();result.error=error.c_str();return result;});
    service.set_connected(true);
}
'''
    out=out.replace("@OPTIONS@",assignments("ForgePlotOptionsCore","cpp").replace(".as_ptr()",".c_str()"))
    equal=" && ".join("std::bit_cast<uint32_t>(a->"+name+")==std::bit_cast<uint32_t>(b->"+name+")" for name,typ in FIELDS["ForgePlotPointCore"] if typ=="f64")
    out=out.replace("@POINT_EQUAL_CPP@","                    same_points="+equal+" && a->label==b->label;")
    loops="        if(!reusable){\n"+cpp_loop("panels","Panel","            const std::string title(v.title);","            plot->add_panel(core);")+"        }\n"
    loops+=cpp_loop("axes","Axis",'''            const std::string label(v.label),format(v.format);std::deque<std::string> labels;std::vector<ForgePlotTickCore> ticks;
            for(size_t t=0;v.ticks&&t<v.ticks->row_count();++t){const auto tick=v.ticks->row_data(t);if(tick){labels.emplace_back(tick->label);ticks.push_back({tick->value,labels.back().c_str()});}}''',"            plot->add_axis(core);")
    points=assignments("ForgePlotPointCore","cpp").replace("core.","point.")
    loops+=cpp_loop("series","Series","            const std::string name(v.name);",'''            if(reusable){plot->update_series(core);continue;}
            std::vector<ForgePlotPointCore> points;std::deque<std::string> labels;std::vector<std::vector<double>> outliers;
            points.reserve(v.points?v.points->row_count():0);
            for(size_t j=0;v.points&&j<v.points->row_count();++j){const auto item=v.points->row_data(j);if(!item)continue;const auto& v=*item;
                const char* label_pointer=nullptr;if(!v.label.empty()){labels.emplace_back(v.label);label_pointer=labels.back().c_str();}
                const size_t outlier_count=v.outliers?v.outliers->row_count():0;const double* outlier_pointer=nullptr;
                if(outlier_count){outliers.emplace_back();for(size_t k=0;k<outlier_count;++k){const auto value=v.outliers->row_data(k);outliers.back().push_back(value?*value:0);}outlier_pointer=outliers.back().data();}
                ForgePlotPointCore point{};
'''+points+'''
                points.push_back(point);
            }plot->add_series(core,points);''')
    loops+="        if(!reusable){\n"
    loops+=cpp_loop("maps","Map","            const std::string name(v.name);","            std::vector<double> values;for(size_t j=0;v.values&&j<v.values->row_count();++j){const auto value=v.values->row_data(j);values.push_back(value?*value:std::numeric_limits<double>::quiet_NaN());}plot->add_map(core,values);")
    loops+=cpp_loop("items","Item","            const std::string text(v.text),image(v.image_data);","            plot->add_item(core);")
    loops+="        }\n"
    out=out.replace("@LOOPS@",loops).replace("@APPEND_POINT@",points)
    hit=[]
    for field,typ in FIELDS["ForgePlotHitCore"]:
        hit.append("            out."+field+" = "+("static_cast<float>(v."+field+")" if typ=="f64" else "v."+field)+";")
    return out.replace("@HIT@","\n".join(hit))

if __name__=="__main__":
    parser=argparse.ArgumentParser();parser.add_argument("--check",action="store_true");args=parser.parse_args()
    for relative, text in [("sdk/rust/src/plot_bindings.rs",rust()),("sdk/cpp/include/ForgePlotBindings.hpp",cpp())]:
        path=ROOT/relative
        if args.check:
            if path.read_text(encoding="utf-8")!=text:raise SystemExit("PLOT_BINDINGS_STALE "+relative)
        else:path.write_text(text,encoding="utf-8")
    print("PLOT_BINDINGS_OK")
