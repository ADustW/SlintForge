// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#include "ForgePlot.hpp"
#include "plot-chart.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

static void forge_chart_require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
struct ForgePlotChartTestScene {
    void* plot;
    ForgePlotOptionsCore options{};
    ForgePlotChartTestScene(){options.width=720;options.height=480;options.font_size=14;options.background=0xfff9f9f9;options.foreground=0xff202020;options.grid_color=0x20000000;options.selection_color=0xff005fb8;options.antialias=1;options.legend=1;options.legend_placement=1;plot=forge_plot_core_new(&options);}
    ~ForgePlotChartTestScene(){forge_plot_core_free(plot);}
    ForgePlotSeriesCore series(int kind,const std::vector<double>& values,double inner=0,const std::vector<ForgePlotPointStyleCore>& styles={}){
        ForgePlotSeriesCore s{};s.id=17;s.kind=kind;s.y_axis=1;s.line_style=1;s.line_width=2;s.selectable=1;s.stack_group=-1;s.fill_target=-1;s.point_size=16;s.name="数据";s.color=0xff005fb8;s.fill_color=0x30005fb8;s.inner_radius=inner;s.point_styles=styles.data();s.point_style_count=styles.size();
        std::vector<ForgePlotPointCore> points;for(size_t i=0;i<values.size();++i){ForgePlotPointCore p{};p.x=static_cast<double>(i);p.y=values[i];points.push_back(p);}forge_plot_core_series(plot,&s,points.data(),points.size());return s;
    }
    ForgePlotRectCore rect(){ForgePlotRectCore r{};forge_plot_core_rects(plot,&r,1);return r;}
    void render(int status=0){forge_plot_core_render(plot);auto info=forge_plot_core_info(plot);if(info.status!=status)throw std::runtime_error(std::string("状态错误：")+info.error);}
};
int main(){try{
    const auto window=PlotChartTestWindow::create();
    const auto directory=std::filesystem::path("build/winui3/chart-exports");std::filesystem::create_directories(directory);
    for(int kind:{0,2,7,8,9,10}){ForgePlotChartTestScene p;auto spec=p.series(kind,{2,4,3});p.render();ForgePlot exported(p.options);std::vector<ForgePlotPointCore> points(3);for(size_t i=0;i<3;++i){points[i].x=static_cast<double>(i);points[i].y=i+1.;}exported.add_series(spec,points);forge_chart_require(exported.render().empty(),"导出场景渲染失败");for(const char* format:{"svg","png","jpg","bmp","pdf"}){const auto file=directory/(std::to_string(kind)+"."+format);const auto error=exported.export_file(file,format);if(!error.empty())throw std::runtime_error(error);forge_chart_require(std::filesystem::file_size(file)>100,"导出文件为空");}}

    for(int kind:{8,9}){ForgePlotChartTestScene p;p.series(kind,{5});p.render();auto r=p.rect();auto h=forge_plot_core_hit(p.plot,r.x+r.width*.5+20,r.y+r.height*.5,0);forge_chart_require(h.kind==0&&h.id==17&&h.index==0,"完整圆命中错误");}
    {ForgePlotChartTestScene p;p.series(8,{1,3},.6);p.render();auto r=p.rect();forge_chart_require(forge_plot_core_hit(p.plot,r.x+r.width*.5,r.y+r.height*.5,8).kind<0,"环形中心不应命中");forge_chart_require(forge_plot_core_measure(p.plot,17,0).kind<0,"饼图不应启用波形测量");}
    {ForgePlotChartTestScene p;auto s=p.series(8,{1,NAN,3},0,{{0,1,0,0},{2,0,0,0xffd13438}});p.render();auto r=p.rect();auto hit=forge_plot_core_hit(p.plot,r.x+r.width*.5+20,r.y+r.height*.5,0);forge_chart_require(hit.index==2,"隐藏数据改变了原始索引");bool found=false;for(int y=400;y<480;y+=2)for(int x=0;x<720;x+=4){auto h=forge_plot_core_hit(p.plot,x,y,0);if(h.kind==2&&h.index==0)found=true;}forge_chart_require(found,"隐藏扇区必须保留逐项图例");}
    {ForgePlotChartTestScene p;p.series(7,{1,2,3},0,{{0,0,-1,0},{1,0,NAN,0},{2,0,24,0xff00a78e}});p.render();forge_chart_require(forge_plot_core_info(p.plot).rendered_points==1,"非法气泡尺寸必须跳过");const double x=forge_plot_core_coordinate(p.plot,0,2,0),y=forge_plot_core_coordinate(p.plot,1,3,0);auto r=p.rect();forge_chart_require(y>=r.y&&y<=r.y+r.height&&forge_plot_core_hit(p.plot,x,y,0).index==2,"气泡范围与命中错误");}
    {ForgePlotChartTestScene p;p.series(10,{1,2});p.render(2);}
    {ForgePlotChartTestScene p;p.series(8,{});p.render(1);}
    {ForgePlotChartTestScene p;p.series(8,{NAN,-1,0});p.render(1);}
    {ForgePlotChartTestScene p;p.series(8,{1,2});auto s=p.series(0,{1,2});p.render(2);}
    {ForgePlotChartTestScene p;ForgePlotPanelCore a{};a.width=.5;a.height=1;forge_plot_core_panel(p.plot,&a);a.id=1;a.x=.5;forge_plot_core_panel(p.plot,&a);p.series(8,{1,2});ForgePlotSeriesCore s{};s.id=19;s.kind=10;s.x_axis=2;s.y_axis=3;s.selectable=1;s.fill_target=-1;s.stack_group=-1;s.line_width=2;ForgePlotPointCore pts[3]{};for(auto& v:pts)v.y=1;forge_plot_core_series(p.plot,&s,pts,3);p.render();}
    {ForgePlotChartTestScene p;auto s=p.series(0,{1,4,2,5});s.line_style=6;s.scatter_style=2;forge_plot_core_update_series(p.plot,&s);p.render();forge_chart_require(forge_plot_core_info(p.plot).rendered_points>4,"平滑曲线未细分");
        const std::string svg=forge_plot_core_render(p.plot);const auto fill=svg.rfind("fill=\"#005fb8\"");forge_chart_require(fill!=std::string::npos,"散点缺失");const auto start=svg.rfind("<path",fill);const auto marker=svg.substr(start,svg.find("/>",fill)-start);forge_chart_require(std::count(marker.begin(),marker.end(),'Z')==4,"插值顶点不应新增散点标记");}
    std::cout<<"FORGE_PLOT_CHARTS_OK kinds=6 exports=30 boundaries=10\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
