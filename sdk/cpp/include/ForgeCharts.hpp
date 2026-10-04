#pragma once
#include <slint.h>
#include <string>
#include <vector>

// 第一方公共类型均带模块前缀；数据与 Slint 生成类型保持独立。
struct ForgeChartPointData { double x=0, y=0, size=0; std::string label; slint::Color color; };
struct ForgeChartSeriesData { std::string name; slint::Color color; std::vector<ForgeChartPointData> points; bool hidden=false; };
struct ForgeChartSliceData { std::string label; double value=0; slint::Color color; bool hidden=false; };
struct ForgeChartOptionsData {
    bool auto_range=true; double x_min=0,x_max=10,y_min=0,y_max=100;
    bool include_zero=false,stacked=false,smooth=false,fill_area=false,doughnut=false;
    double point_size=5; int precision=2;
};
struct ForgeChartMarkData {
    int kind=0; double x=0,y=0,width=0,height=0,radius=0;
    double x1=0,y1=0,x2=0,y2=0,x3=0,y3=0,start_angle=0,sweep_angle=0;
    slint::Color color; int series_index=0,point_index=0; double value=0; std::string label;
};
struct ForgeChartLineData {
    double x=0,y=0,x1=0,y1=0,c1x=0,c1y=0,c2x=0,c2y=0; slint::Color color;
};
struct ForgeChartTickData { double position=0; std::string label; };
struct ForgeChartLabelData { double x=0,y=0; std::string text; };
struct ForgeChartSceneData {
    int status=1; double x_min=0,x_max=1,y_min=0,y_max=1,zero_y=1,inner_radius=0;
    std::vector<ForgeChartMarkData> marks,areas; std::vector<ForgeChartLineData> lines;
    std::vector<ForgeChartTickData> x_ticks,y_ticks; std::vector<ForgeChartLabelData> labels;
};
slint::Color forge_chart_color(std::size_t index);
ForgeChartSceneData forge_prepare_chart(int kind, const std::vector<ForgeChartSeriesData>& series,
    const std::vector<ForgeChartSliceData>& slices, const std::vector<std::string>& categories, const ForgeChartOptionsData& options);
int forge_chart_hit_test(const std::vector<ForgeChartMarkData>& marks, double x, double y, double width, double height, double inner_radius);

template<class Service,class Scene,class Mark,class Line,class Tick,class Label,class Handle>
void forge_bind_chart_services(const Handle& ui) {
    const auto& service=ui->template global<Service>();
    service.on_prepare([](int kind,auto model,auto slices_model,auto categories_model,auto options) {
        std::vector<ForgeChartSeriesData> series;
        for (std::size_t i=0;model && i<model->row_count();++i) {
            const auto item=model->row_data(i); if (!item) continue;
            ForgeChartSeriesData data;data.name=std::string(item->name);data.color=item->color;data.hidden=item->hidden;
            for (std::size_t p=0;item->points && p<item->points->row_count();++p) {
                const auto point=item->points->row_data(p);if (!point) continue;
                data.points.push_back({point->x,point->y,point->size,std::string(point->label),point->color});
            }
            series.push_back(std::move(data));
        }
        std::vector<ForgeChartSliceData> slices;
        for (std::size_t i=0;slices_model && i<slices_model->row_count();++i) {
            const auto item=slices_model->row_data(i);if (item) slices.push_back({std::string(item->label),item->value,item->color,item->hidden});
        }
        std::vector<std::string> categories;
        for (std::size_t i=0;categories_model && i<categories_model->row_count();++i) {
            const auto item=categories_model->row_data(i);if (item) categories.push_back(std::string(*item));
        }
        ForgeChartOptionsData settings;
        settings.auto_range=options.auto_range;settings.x_min=options.x_min;settings.x_max=options.x_max;settings.y_min=options.y_min;settings.y_max=options.y_max;
        settings.include_zero=options.include_zero;settings.stacked=options.stacked;settings.smooth=options.smooth;settings.fill_area=options.fill_area;
        settings.doughnut=options.doughnut;settings.point_size=options.point_size;settings.precision=options.precision;
        const auto source=forge_prepare_chart(kind,series,slices,categories,settings);
        const auto convert_mark=[](const ForgeChartMarkData& p) {
            Mark mark{};mark.kind=p.kind;mark.x=static_cast<float>(p.x);mark.y=static_cast<float>(p.y);mark.width=static_cast<float>(p.width);mark.height=static_cast<float>(p.height);mark.radius=static_cast<float>(p.radius);
            mark.x1=static_cast<float>(p.x1);mark.y1=static_cast<float>(p.y1);mark.x2=static_cast<float>(p.x2);mark.y2=static_cast<float>(p.y2);mark.x3=static_cast<float>(p.x3);mark.y3=static_cast<float>(p.y3);
            mark.start_angle=static_cast<float>(p.start_angle);mark.sweep_angle=static_cast<float>(p.sweep_angle);mark.color=p.color;
            mark.series_index=p.series_index;mark.point_index=p.point_index;mark.value=static_cast<float>(p.value);mark.label=p.label.c_str();return mark;
        };
        Scene result{};result.status=source.status;result.x_min=static_cast<float>(source.x_min);result.x_max=static_cast<float>(source.x_max);
        result.y_min=static_cast<float>(source.y_min);result.y_max=static_cast<float>(source.y_max);result.zero_y=static_cast<float>(source.zero_y);result.inner_radius=static_cast<float>(source.inner_radius);
        std::vector<Mark> marks,areas;for(const auto& p:source.marks) marks.push_back(convert_mark(p));for(const auto& p:source.areas) areas.push_back(convert_mark(p));
        result.marks=std::make_shared<slint::VectorModel<Mark>>(std::move(marks));result.areas=std::make_shared<slint::VectorModel<Mark>>(std::move(areas));
        std::vector<Line> lines;for(const auto& p:source.lines) {
            Line line{};line.x=static_cast<float>(p.x);line.y=static_cast<float>(p.y);line.x1=static_cast<float>(p.x1);line.y1=static_cast<float>(p.y1);
            line.c1x=static_cast<float>(p.c1x);line.c1y=static_cast<float>(p.c1y);line.c2x=static_cast<float>(p.c2x);line.c2y=static_cast<float>(p.c2y);line.color=p.color;lines.push_back(line);
        }
        result.lines=std::make_shared<slint::VectorModel<Line>>(std::move(lines));
        const auto ticks=[](const std::vector<ForgeChartTickData>& input) {
            std::vector<Tick> output;for(const auto& p:input){Tick item{};item.position=static_cast<float>(p.position);item.label=p.label.c_str();output.push_back(item);}
            return std::make_shared<slint::VectorModel<Tick>>(std::move(output));
        };
        result.x_ticks=ticks(source.x_ticks);result.y_ticks=ticks(source.y_ticks);
        std::vector<Label> labels;for(const auto& p:source.labels){Label item{};item.x=static_cast<float>(p.x);item.y=static_cast<float>(p.y);item.text=p.text.c_str();labels.push_back(item);}
        result.labels=std::make_shared<slint::VectorModel<Label>>(std::move(labels));return result;
    });
    service.on_hit_test([](std::shared_ptr<slint::Model<Mark>> model,float x,float y,float width,float height,float inner) {
        std::vector<ForgeChartMarkData> marks;
        for(std::size_t i=0;model && i<model->row_count();++i) {
            const auto p=model->row_data(i);if(!p)continue;ForgeChartMarkData mark;
            mark.kind=p->kind;mark.x=p->x;mark.y=p->y;mark.width=p->width;mark.height=p->height;mark.radius=p->radius;mark.start_angle=p->start_angle;mark.sweep_angle=p->sweep_angle;marks.push_back(mark);
        }
        return forge_chart_hit_test(marks,x,y,width,height,inner);
    });
    service.set_connected(true);
}
