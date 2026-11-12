// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

// 由 tools/generate_plot_bindings.py 生成。
#pragma once
template<class Service,class Scene,class Axis,class Range,class Rect,class Hit,class Selection,class Export,class Handle>
void forge_bind_plot_services(const Handle& ui) {
    const auto state=std::make_shared<ForgePlotServiceState>();const auto& service=ui->template global<Service>();
    service.on_allocate([state](){return state->allocate();});service.on_release([state](int id){state->plots.erase(id);state->inputs.erase(id);state->point_inputs.erase(id);});
    service.on_prepare([state](int token,auto axes,auto panels,auto series,auto maps,auto items,auto selection,auto v){
        Scene result{};result.token=token;
        const std::string title(v.title),font_family(v.font_family);ForgePlotOptionsCore core{};
                core.width = v.width;
                core.height = v.height;
                core.font_size = v.font_size;
                core.background = forge_plot_ui_color(v.background);
                core.foreground = forge_plot_ui_color(v.foreground);
                core.grid_color = forge_plot_ui_color(v.grid_color);
                core.selection_color = forge_plot_ui_color(v.selection_color);
                core.legend = !v.hide_legend;
                core.adaptive = !v.disable_adaptive;
                core.antialias = !v.disable_antialias;
                core.max_points_per_pixel = v.max_points_per_pixel;
                core.legend_position = static_cast<int>(v.legend_position);
                core.legend_style = static_cast<int>(v.legend_style);
                core.legend_latest_value = static_cast<int>(v.legend_latest_value);
                core.legend_precision = v.legend_precision;
                core.legend_placement = static_cast<int>(v.legend_placement);
                core.measurement_enabled = static_cast<int>(v.measurement_enabled);
                core.measurement_position = static_cast<int>(v.measurement_position);
                core.measurement_placement = static_cast<int>(v.measurement_placement);
                core.measurement_series = v.measurement_series;
                core.measurement_width = v.measurement_width;
                core.measurement_height = v.measurement_height;
                core.x_window_mode = static_cast<int>(v.x_window_mode);
                core.x_window_axis = v.x_window_axis;
                core.x_window_size = v.x_window_size;
                core.title = title.c_str();
                core.font_family = font_family.c_str();
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
                    same_points=std::bit_cast<uint32_t>(a->x)==std::bit_cast<uint32_t>(b->x) && std::bit_cast<uint32_t>(a->y)==std::bit_cast<uint32_t>(b->y) && std::bit_cast<uint32_t>(a->low)==std::bit_cast<uint32_t>(b->low) && std::bit_cast<uint32_t>(a->high)==std::bit_cast<uint32_t>(b->high) && std::bit_cast<uint32_t>(a->open)==std::bit_cast<uint32_t>(b->open) && std::bit_cast<uint32_t>(a->close)==std::bit_cast<uint32_t>(b->close) && std::bit_cast<uint32_t>(a->q1)==std::bit_cast<uint32_t>(b->q1) && std::bit_cast<uint32_t>(a->q3)==std::bit_cast<uint32_t>(b->q3) && std::bit_cast<uint32_t>(a->error_x_minus)==std::bit_cast<uint32_t>(b->error_x_minus) && std::bit_cast<uint32_t>(a->error_x_plus)==std::bit_cast<uint32_t>(b->error_x_plus) && std::bit_cast<uint32_t>(a->error_y_minus)==std::bit_cast<uint32_t>(b->error_y_minus) && std::bit_cast<uint32_t>(a->error_y_plus)==std::bit_cast<uint32_t>(b->error_y_plus) && a->label==b->label;
                    if(a->outliers!=b->outliers){const size_t n=a->outliers?a->outliers->row_count():0,m=b->outliers?b->outliers->row_count():0;if(n!=m){same_points=false;break;}for(size_t k=0;k<n;++k){const auto x=a->outliers->row_data(k),y=b->outliers->row_data(k);if(x.has_value()!=y.has_value()||(x&&std::bit_cast<uint32_t>(*x)!=std::bit_cast<uint32_t>(*y))){same_points=false;break;}}}
                }
                if(!same_points)break;
            }
        }
        const bool reusable=state->inputs.count(token)&&state->inputs[token]==input&&same_points&&state->plots.count(token);
        auto plot=reusable?state->plots[token]:std::make_shared<ForgePlot>(core);if(reusable)plot->begin_view(core);
        if(!reusable){
        for(size_t i=0;panels && i<panels->row_count();++i){const auto item=panels->row_data(i);if(!item)continue;const auto& v=*item;
            const std::string title(v.title);std::deque<std::string> category_labels;std::vector<ForgePlotTickCore> categories;for(size_t i=0;v.categories&&i<v.categories->row_count();++i)if(auto label=v.categories->row_data(i)){category_labels.emplace_back(*label);categories.push_back({static_cast<double>(i),category_labels.back().c_str()});}
            ForgePlotPanelCore core{};
                core.id = v.id;
                core.x = v.x;
                core.y = v.y;
                core.width = v.width;
                core.height = v.height;
                core.title = title.c_str();
                core.categories = categories.data();
                core.category_count = categories.size();
            plot->add_panel(core);
        }
        }
        for(size_t i=0;axes && i<axes->row_count();++i){const auto item=axes->row_data(i);if(!item)continue;const auto& v=*item;
            const std::string label(v.label),format(v.format);std::deque<std::string> labels;std::vector<ForgePlotTickCore> ticks;
            for(size_t t=0;v.ticks&&t<v.ticks->row_count();++t){const auto tick=v.ticks->row_data(t);if(tick){labels.emplace_back(tick->label);ticks.push_back({tick->value,labels.back().c_str()});}}
            ForgePlotAxisCore core{};
                core.id = v.id;
                core.panel = v.panel;
                core.side = static_cast<int>(v.side);
                core.scale = static_cast<int>(v.scale);
                core.ticker = static_cast<int>(v.ticker);
                core.auto_range = !v.manual_range;
                core.reversed = static_cast<int>(v.reversed);
                core.visible = !v.hidden;
                core.grid = !v.hide_grid;
                core.precision = v.precision;
                core.minimum = v.minimum;
                core.maximum = v.maximum;
                core.tick_step = v.tick_step;
                core.offset = v.offset;
                core.time_origin = forge_plot_ui_number(v.time_origin);
                core.label = label.c_str();
                core.format = format.c_str();
                core.ticks = ticks.data();
                core.tick_count = ticks.size();
            plot->add_axis(core);
        }
        for(size_t i=0;series && i<series->row_count();++i){const auto item=series->row_data(i);if(!item)continue;const auto& v=*item;
            const std::string name(v.name);std::vector<ForgePlotPointStyleCore> point_styles;for(size_t i=0;v.point_styles&&i<v.point_styles->row_count();++i)if(auto style=v.point_styles->row_data(i))point_styles.push_back({style->index,static_cast<int>(style->hidden),style->size,forge_plot_ui_color(style->color)});
            ForgePlotSeriesCore core{};
                core.id = v.id;
                core.kind = static_cast<int>(v.kind);
                core.x_axis = v.x_axis;
                core.y_axis = v.y_axis == 0 && v.x_axis == 0 ? 1 : v.y_axis;
                core.line_style = static_cast<int>(v.line_style) == 0 ? 1 : static_cast<int>(v.line_style) - 1;
                core.scatter_style = static_cast<int>(v.scatter_style);
                core.hidden = static_cast<int>(v.hidden);
                core.selectable = !v.unselectable;
                core.stack_group = v.stacked ? v.stack_group : -1;
                core.fill_target = v.channel_fill ? v.fill_target : -1;
                core.layer = v.layer;
                core.sorted = static_cast<int>(v.sorted);
                core.color = forge_plot_ui_color(v.color);
                core.fill_color = forge_plot_ui_color(v.fill_color);
                core.line_width = v.line_width > 0 ? v.line_width : 2;
                core.point_size = v.point_size > 0 ? v.point_size : 6;
                core.bar_width = v.bar_width > 0 ? v.bar_width : .8;
                core.baseline = v.baseline;
                core.name = name.c_str();
                core.inner_radius = v.inner_radius;
                core.point_styles = point_styles.data();
                core.point_style_count = point_styles.size();
            if(reusable){plot->update_series(core);continue;}
            std::vector<ForgePlotPointCore> points;std::deque<std::string> labels;std::vector<std::vector<double>> outliers;
            points.reserve(v.points?v.points->row_count():0);
            for(size_t j=0;v.points&&j<v.points->row_count();++j){const auto item=v.points->row_data(j);if(!item)continue;const auto& v=*item;
                const char* label_pointer=nullptr;if(!v.label.empty()){labels.emplace_back(v.label);label_pointer=labels.back().c_str();}
                const size_t outlier_count=v.outliers?v.outliers->row_count():0;const double* outlier_pointer=nullptr;
                if(outlier_count){outliers.emplace_back();for(size_t k=0;k<outlier_count;++k){const auto value=v.outliers->row_data(k);outliers.back().push_back(value?*value:0);}outlier_pointer=outliers.back().data();}
                ForgePlotPointCore point{};
                point.x = v.x;
                point.y = v.y;
                point.low = v.low;
                point.high = v.high;
                point.open = v.open;
                point.close = v.close;
                point.q1 = v.q1;
                point.q3 = v.q3;
                point.error_x_minus = v.error_x_minus;
                point.error_x_plus = v.error_x_plus;
                point.error_y_minus = v.error_y_minus;
                point.error_y_plus = v.error_y_plus;
                point.label = label_pointer;
                point.outliers = outlier_pointer;
                point.outlier_count = outlier_count;
                points.push_back(point);
            }plot->add_series(core,points);
        }
        if(!reusable){
        for(size_t i=0;maps && i<maps->row_count();++i){const auto item=maps->row_data(i);if(!item)continue;const auto& v=*item;
            const std::string name(v.name);
            ForgePlotMapCore core{};
                core.id = v.id;
                core.panel = v.panel;
                core.x_axis = v.x_axis;
                core.y_axis = v.y_axis == 0 && v.x_axis == 0 ? 1 : v.y_axis;
                core.columns = v.columns;
                core.rows = v.rows;
                core.palette = v.palette;
                core.auto_range = !v.manual_range;
                core.logarithmic = static_cast<int>(v.logarithmic);
                core.hidden = static_cast<int>(v.hidden);
                core.interpolate = static_cast<int>(v.interpolate);
                core.color_scale = !v.hide_color_scale;
                core.layer = v.layer;
                core.x_min = v.x_min;
                core.x_max = v.x_max;
                core.y_min = v.y_min;
                core.y_max = v.y_max;
                core.z_min = v.z_min;
                core.z_max = v.z_max;
                core.name = name.c_str();
            std::vector<double> values;for(size_t j=0;v.values&&j<v.values->row_count();++j){const auto value=v.values->row_data(j);values.push_back(value?*value:std::numeric_limits<double>::quiet_NaN());}plot->add_map(core,values);
        }
        for(size_t i=0;items && i<items->row_count();++i){const auto item=items->row_data(i);if(!item)continue;const auto& v=*item;
            const std::string text(v.text),image(v.image_data);
            ForgePlotItemCore core{};
                core.id = v.id;
                core.kind = static_cast<int>(v.kind);
                core.panel = v.panel;
                core.x_axis = v.x_axis;
                core.y_axis = v.y_axis == 0 && v.x_axis == 0 ? 1 : v.y_axis;
                core.coordinates = static_cast<int>(v.coordinates);
                core.anchor = v.anchored ? v.anchor : -1;
                core.anchor_part = v.anchor_part;
                core.clip = !v.unclipped;
                core.layer = v.layer;
                core.selectable = v.selectable;
                core.x = v.x;
                core.y = v.y;
                core.x2 = v.x2;
                core.y2 = v.y2;
                core.x3 = v.x3;
                core.y3 = v.y3;
                core.x4 = v.x4;
                core.y4 = v.y4;
                core.width = v.width > 0 ? v.width : 1;
                core.size = v.size > 0 ? v.size : 12;
                core.color = forge_plot_ui_color(v.color);
                core.fill_color = forge_plot_ui_color(v.fill_color);
                core.text = text.c_str();
                core.image = image.c_str();
                core.image_pixels = nullptr;
                core.image_byte_count = 0;
                core.image_width = 0;
                core.image_height = 0;
            plot->add_item(core);
        }
        }

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
            out.kind = v.kind;
            out.id = v.id;
            out.index = v.index;
            out.panel = v.panel;
            out.x = static_cast<float>(v.x);
            out.y = static_cast<float>(v.y);
            out.value = static_cast<float>(v.value);
            out.pixel_x = static_cast<float>(v.pixel_x);
            out.pixel_y = static_cast<float>(v.pixel_y);
            out.distance = static_cast<float>(v.distance);
            out.label = v.label;
        }return out;});
    service.on_coordinate([state](int token,int axis,float value,bool inverse){const auto it=state->plots.find(token);return it==state->plots.end()?std::numeric_limits<float>::quiet_NaN():static_cast<float>(it->second->coordinate(axis,value,inverse));});
    service.on_measure([state](int token,int id,float x){Hit out{};out.kind=-1;out.id=-1;out.index=-1;const auto it=state->plots.find(token);if(it!=state->plots.end()){const auto v=it->second->measure(id,x);
            out.kind = v.kind;
            out.id = v.id;
            out.index = v.index;
            out.panel = v.panel;
            out.x = static_cast<float>(v.x);
            out.y = static_cast<float>(v.y);
            out.value = static_cast<float>(v.value);
            out.pixel_x = static_cast<float>(v.pixel_x);
            out.pixel_y = static_cast<float>(v.pixel_y);
            out.distance = static_cast<float>(v.distance);
            out.label = v.label;
        }return out;});
    service.on_append([state](int token,int id,auto model){const auto it=state->plots.find(token);if(it==state->plots.end())return false;
        std::vector<ForgePlotPointCore> points;std::deque<std::string> labels;std::vector<std::vector<double>> outliers;
        for(size_t j=0;model&&j<model->row_count();++j){const auto item=model->row_data(j);if(!item)continue;const auto& v=*item;
            const char* label_pointer=nullptr;if(!v.label.empty()){labels.emplace_back(v.label);label_pointer=labels.back().c_str();}
            const size_t outlier_count=v.outliers?v.outliers->row_count():0;const double* outlier_pointer=nullptr;
            if(outlier_count){outliers.emplace_back();for(size_t k=0;k<outlier_count;++k){const auto value=v.outliers->row_data(k);outliers.back().push_back(value?*value:0);}outlier_pointer=outliers.back().data();}
            ForgePlotPointCore point{};
                point.x = v.x;
                point.y = v.y;
                point.low = v.low;
                point.high = v.high;
                point.open = v.open;
                point.close = v.close;
                point.q1 = v.q1;
                point.q3 = v.q3;
                point.error_x_minus = v.error_x_minus;
                point.error_x_plus = v.error_x_plus;
                point.error_y_minus = v.error_y_minus;
                point.error_y_plus = v.error_y_plus;
                point.label = label_pointer;
                point.outliers = outlier_pointer;
                point.outlier_count = outlier_count;
            points.push_back(point);
        }return it->second->append(id,points);
    });
    service.on_select_rect([state](int token,float x,float y,float width,float height){std::vector<Selection> out;const auto it=state->plots.find(token);if(it!=state->plots.end())for(const auto& [id,first,last]:it->second->select_rect(x,y,width,height)){Selection r{};r.id=id;r.first=first;r.last=last;out.push_back(r);}return std::make_shared<slint::VectorModel<Selection>>(std::move(out));});
    service.on_merge_selection([](auto a,auto b,int mode){std::vector<std::tuple<int,int,int>> current,next;const auto append=[](const auto& model,auto& list){for(size_t i=0;model&&i<model->row_count();++i){const auto s=model->row_data(i);if(s)list.emplace_back(s->id,s->first,s->last);}};append(a,current);append(b,next);std::vector<Selection> out;for(const auto& [id,first,last]:forge_plot_merge_selection(current,next,mode)){Selection r{};r.id=id;r.first=first;r.last=last;out.push_back(r);}return std::make_shared<slint::VectorModel<Selection>>(std::move(out));});
    service.on_axis_index([](auto axes,int id){for(size_t i=0;axes&&i<axes->row_count();++i)if(auto axis=axes->row_data(i);axis&&axis->id==id)return static_cast<int>(i);return -1;});
    service.on_change_view([state](int token,auto axes,auto ranges,int panel,int mode,float x,float y,float w,float h,int directions){std::vector<Axis> out;const auto it=state->plots.find(token);
        for(size_t i=0;axes&&i<axes->row_count();++i){const auto value=axes->row_data(i);if(!value)continue;auto a=*value;const bool horizontal=static_cast<int>(a.side)<2;
            if((panel<0||a.panel==panel)&&(horizontal?directions%2!=0:directions>=2)){if(mode==3)a.manual_range=false;else if(it!=state->plots.end())for(size_t j=0;ranges&&j<ranges->row_count();++j){const auto r=ranges->row_data(j);if(r&&r->id==a.id){ForgePlotRangeCore range{r->id,r->scale,r->reversed?1:0,r->minimum,r->maximum};const auto next=it->second->view(range,mode,horizontal?x:y,horizontal?w:h);a.manual_range=true;a.minimum=static_cast<float>(next.minimum);a.maximum=static_cast<float>(next.maximum);break;}}}out.push_back(a);
        }return std::make_shared<slint::VectorModel<Axis>>(std::move(out));});
    service.on_cartesian_panel([](auto series,auto axes,int panel){if(!series||!series->row_count())return true;for(size_t i=0;series&&i<series->row_count();++i)if(auto s=series->row_data(i)){const int kind=static_cast<int>(s->kind);if(kind>5&&kind!=7)continue;int id=s->x_axis/2;for(size_t j=0;axes&&j<axes->row_count();++j)if(auto a=axes->row_data(j);a&&a->id==s->x_axis){id=a->panel;break;}if(panel<0||id==panel)return true;}return false;});
    service.on_cartesian_series([](auto series,int id){for(size_t i=0;series&&i<series->row_count();++i)if(auto s=series->row_data(i);s&&s->id==id){const int kind=static_cast<int>(s->kind);return kind<=5||kind==7;}return false;});
    service.on_toggle_point([](auto series,int id,int index){using Item=std::remove_cvref_t<decltype(series->row_data(0).value())>;std::vector<Item> out;
        for(size_t i=0;series&&i<series->row_count();++i)if(auto value=series->row_data(i)){auto s=*value;if(s.id==id){using Style=std::remove_cvref_t<decltype(s.point_styles->row_data(0).value())>;std::vector<Style> styles;bool found=false;for(size_t j=0;s.point_styles&&j<s.point_styles->row_count();++j)if(auto v=s.point_styles->row_data(j)){if(v->index==index){v->hidden=!v->hidden;found=true;}styles.push_back(*v);}if(!found){Style v{};v.index=index;v.hidden=true;v.size=s.point_size>0?s.point_size:6;styles.push_back(v);}s.point_styles=std::make_shared<slint::VectorModel<Style>>(std::move(styles));}out.push_back(s);}return std::make_shared<slint::VectorModel<Item>>(std::move(out));});
    service.on_toggle_series([](auto series,int id){using Item=std::remove_cvref_t<decltype(series->row_data(0).value())>;std::vector<Item> out;for(size_t i=0;series&&i<series->row_count();++i){const auto value=series->row_data(i);if(value){auto s=*value;if(s.id==id)s.hidden=!s.hidden;out.push_back(s);}}return std::make_shared<slint::VectorModel<Item>>(std::move(out));});
    service.on_scroll_view([state](int token,auto axes,int id,float position,float span){std::vector<Axis> out;const auto it=state->plots.find(token);
        for(size_t i=0;axes&&i<axes->row_count();++i){const auto value=axes->row_data(i);if(!value)continue;auto a=*value;if(a.id==id&&it!=state->plots.end())for(const auto& b:it->second->domains())if(b.id==id){const double width=std::max(.000001,static_cast<double>(span));a.manual_range=true;a.minimum=static_cast<float>(b.minimum+std::max(0.,b.maximum-b.minimum-width)*std::clamp(static_cast<double>(position),0.,1.));a.maximum=a.minimum+static_cast<float>(width);break;}out.push_back(a);}return std::make_shared<slint::VectorModel<Axis>>(std::move(out));
    });
    service.on_export([state](int token,slint::SharedString path,slint::SharedString format){Export result{};const auto it=state->plots.find(token);if(it==state->plots.end()){result.error="图表实例不存在";return result;}const auto error=it->second->export_file(std::filesystem::u8path(std::string(path)),std::string(format));result.ok=error.empty();result.error=error.c_str();return result;});
    service.set_connected(true);
}
