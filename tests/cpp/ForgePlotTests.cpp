#include "ForgePlotCore.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstdlib>

static void forge_plot_test_require(bool value,const char* message) {if(!value)throw std::runtime_error(message);}
static void forge_plot_test_near(double a,double b,double tolerance=1e-8){forge_plot_test_require(std::isfinite(a)&&std::isfinite(b)&&std::abs(a-b)<=tolerance,"坐标映射不一致");}
struct ForgePlotTestPlot {
    void* value=nullptr;
    ForgePlotTestPlot(bool adaptive=true){ForgePlotOptionsCore o{};o.width=900;o.height=540;o.font_size=12;o.background=0xff202020;o.foreground=0xffeeeeee;o.grid_color=0xff555555;o.adaptive=adaptive;o.antialias=1;o.font_family="Microsoft YaHei";value=forge_plot_core_new(&o);forge_plot_test_require(value!=nullptr,"场景创建失败");}
    ~ForgePlotTestPlot(){forge_plot_core_free(value);}
    ForgePlotTestPlot(const ForgePlotTestPlot&)=delete;
    ForgePlotTestPlot& operator=(const ForgePlotTestPlot&)=delete;
    void axis(int id,int side,int scale,double lo,double hi,bool automatic=false){ForgePlotAxisCore a{};a.id=id;a.side=side;a.scale=scale;a.minimum=lo;a.maximum=hi;a.auto_range=automatic;a.visible=1;a.grid=1;a.precision=2;forge_plot_core_axis(value,&a);}
    void series(int kind,const std::vector<ForgePlotPointCore>& points,int id=0,int style=1,int stack=-1){ForgePlotSeriesCore s{};s.id=id;s.kind=kind;s.y_axis=1;s.line_style=style;s.selectable=1;s.color=0xff123456;s.fill_color=0x40123456;s.fill_target=-1;s.line_width=2;s.point_size=6;s.bar_width=.8;s.stack_group=stack;s.name="数据";forge_plot_core_series(value,&s,points.data(),points.size());}
    std::string render(){const std::string svg=forge_plot_core_render(value);const auto info=forge_plot_core_info(value);forge_plot_test_require(info.status==0,info.error);forge_plot_test_require(svg.rfind("<svg",0)==0,"SVG 为空");return svg;}
};
static ForgePlotPointCore forge_plot_test_point(double x,double y){ForgePlotPointCore p{};p.x=x;p.y=y;return p;}
static ForgePlotHitCore forge_plot_test_legend_hit(void* plot,int id) {
    for(int y=0;y<540;y+=4)for(int x=0;x<900;x+=4){const auto hit=forge_plot_core_hit(plot,x,y,0);if(hit.kind==2&&hit.id==id)return hit;}
    throw std::runtime_error("图例没有可点击区域");
}

static void forge_plot_test_axes() {
    for(int sign:{1,-1}){ForgePlotTestPlot p;p.axis(0,0,1,sign>0?1:-100,sign>0?100:-1);p.axis(1,2,0,-5,5);p.series(0,{forge_plot_test_point(sign*10,0)});p.render();const double pixel=forge_plot_core_coordinate(p.value,0,sign*10,0);forge_plot_test_near(forge_plot_core_coordinate(p.value,0,pixel,1),sign*10);}
    ForgePlotTestPlot p;p.axis(0,0,0,0,10);p.axis(1,2,0,-2,2);p.series(0,{forge_plot_test_point(5,0)});p.render();for(double v:{0.,2.5,5.,10.})forge_plot_test_near(forge_plot_core_coordinate(p.value,0,forge_plot_core_coordinate(p.value,0,v,0),1),v);
    ForgePlotAxisCore a{};a.id=3;a.side=3;a.reversed=1;a.visible=1;a.minimum=100;a.maximum=200;forge_plot_core_axis(p.value,&a);p.render();forge_plot_test_near(forge_plot_core_coordinate(p.value,3,forge_plot_core_coordinate(p.value,3,125,0),1),125);
    ForgePlotTestPlot bad;bad.axis(0,0,1,-1,1);bad.axis(1,2,0,0,10);bad.series(0,{forge_plot_test_point(.5,2)});forge_plot_core_render(bad.value);forge_plot_test_require(forge_plot_core_info(bad.value).status==2,"跨零对数范围应拒绝");
    ForgePlotTestPlot error;auto point=forge_plot_test_point(1,5);point.error_x_minus=.5;point.error_x_plus=2;point.error_y_minus=3;point.error_y_plus=10;error.series(0,{point});error.render();ForgePlotRangeCore ranges[2];forge_plot_core_ranges(error.value,ranges,2);forge_plot_test_require(ranges[0].minimum<=.5&&ranges[0].maximum>=3&&ranges[1].minimum<=2&&ranges[1].maximum>=15,"自动范围未覆盖误差棒");
}
static void forge_plot_test_types() {
    const std::vector<ForgePlotPointCore> points{forge_plot_test_point(0,1),forge_plot_test_point(1,2),forge_plot_test_point(2,1)};
    for(int style=0;style<=5;++style){ForgePlotTestPlot p;p.series(0,points,0,style);p.render();}
    for(int kind:{0,1,2,6}){ForgePlotTestPlot p;p.series(kind,points);p.render();}
    for(int kind:{3,4,5}){auto point=forge_plot_test_point(1,5);point.low=1;point.q1=3;point.q3=7;point.high=10;point.open=4;point.close=6;const double outliers[]{-2,14};point.outliers=outliers;point.outlier_count=2;ForgePlotTestPlot p;p.series(kind,{point});p.render();forge_plot_test_require(forge_plot_core_info(p.value).rendered_points==1,"统计/金融点数量不正确");if(kind==3){const auto hit=forge_plot_core_hit(p.value,forge_plot_core_coordinate(p.value,0,1,0),forge_plot_core_coordinate(p.value,1,14,0),2);forge_plot_test_require(hit.kind==0&&hit.index==0&&hit.value==14,"统计箱离群值无法命中");}}
    ForgePlotTestPlot stack;stack.series(2,{forge_plot_test_point(0,10),forge_plot_test_point(1,-5)},0,1,0);stack.series(2,{forge_plot_test_point(0,20),forge_plot_test_point(1,-7)},1,1,0);stack.render();ForgePlotRangeCore ranges[2];forge_plot_core_ranges(stack.value,ranges,2);forge_plot_test_require(ranges[1].minimum<=-12&&ranges[1].maximum>=30,"堆叠范围未覆盖正负总和");
    ForgePlotTestPlot gap;gap.series(0,{forge_plot_test_point(0,1),forge_plot_test_point(1,2),forge_plot_test_point(2,NAN),forge_plot_test_point(3,1),forge_plot_test_point(4,2)});const auto svg=gap.render();forge_plot_test_require(svg.find("nan")==std::string::npos&&svg.find("inf")==std::string::npos,"输出含无效数值");
}
static void forge_plot_test_maps_items_exports(const std::filesystem::path& dir) {
    ForgePlotTestPlot p;ForgePlotMapCore m{};m.columns=2;m.rows=2;m.y_axis=1;m.x_max=2;m.y_max=2;m.auto_range=1;m.color_scale=1;m.name="色图";const double values[]{1,2,3,NAN};forge_plot_core_map(p.value,&m,values,4);
    for(int kind=0;kind<=7;++kind){ForgePlotItemCore item{};item.id=kind;item.kind=kind;item.y_axis=1;item.coordinates=1;item.anchor=-1;item.x=.2;item.y=.2;item.x2=.7;item.y2=.5;item.x3=.3;item.y3=.7;item.x4=.8;item.y4=.3;item.width=2;item.size=12;item.color=0xffee5566;item.fill_color=0x40ee5566;item.text="测试文字 & <图表>";forge_plot_core_item(p.value,&item);}
    ForgePlotItemCore image{};image.id=8;image.kind=8;image.coordinates=1;image.anchor=-1;image.x=.7;image.y=.7;image.x2=.8;image.y2=.8;image.image="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAIAAAACCAYAAABytg0kAAAAFUlEQVR4nGP8z8DQwMDAwMAEIkAYABglAYOd/VRoAAAAAElFTkSuQmCC";forge_plot_core_item(p.value,&image);
    const auto svg=p.render();forge_plot_test_require(svg.find("data:image/png;base64,")!=std::string::npos,"色图图像缺失");forge_plot_test_require(svg.find("&amp; &lt;")!=std::string::npos,"文字未转义");forge_plot_test_require(svg.size()<30000&&svg.find("optimizeSpeed")!=std::string::npos,"小色图未复用源单元像素");
    const auto hit=forge_plot_core_hit(p.value,forge_plot_core_coordinate(p.value,0,.5,0),forge_plot_core_coordinate(p.value,1,.5,0),2);forge_plot_test_require(hit.kind==3&&hit.index==0&&hit.value==1,"色图单元命中错误");
    forge_plot_test_require(forge_plot_core_export(p.value,(dir/"professional.svg").string().c_str(),0)==1,"SVG 导出失败");forge_plot_test_require(forge_plot_core_export(p.value,(dir/"professional.pdf").string().c_str(),1)==1,"PDF 导出失败");
    forge_plot_test_require(std::filesystem::file_size(dir/"professional.pdf")>1000,"PDF 输出为空");
    ForgePlotTestPlot cycle;cycle.series(0,{forge_plot_test_point(0,0)});ForgePlotItemCore item{};item.id=1;item.anchor=2;forge_plot_core_item(cycle.value,&item);item.id=2;item.anchor=1;forge_plot_core_item(cycle.value,&item);forge_plot_core_render(cycle.value);forge_plot_test_require(forge_plot_core_info(cycle.value).status==2,"锚点循环应拒绝");
}
static void forge_plot_test_selection() {
    ForgePlotTestPlot p;p.series(0,{forge_plot_test_point(0,1),forge_plot_test_point(1,3),forge_plot_test_point(2,2)});p.render();const double x=forge_plot_core_coordinate(p.value,0,1,0),y=forge_plot_core_coordinate(p.value,1,3,0);const auto hit=forge_plot_core_hit(p.value,x,y,3);forge_plot_test_require(hit.kind==0&&hit.id==0&&hit.index==1,"点命中未保留原始索引");
    ForgePlotRectCore r;forge_plot_core_rects(p.value,&r,1);const size_t count=forge_plot_core_select(p.value,r.x,r.y,r.width,r.height,nullptr,0);forge_plot_test_require(count==3,"框选数量错误");ForgePlotHitCore selected[3];forge_plot_core_select(p.value,r.x,r.y,r.width,r.height,selected,3);for(int i=0;i<3;++i)forge_plot_test_require(selected[i].index==i,"框选索引错误");
}
static void forge_plot_test_geometry_regressions() {
    ForgePlotTestPlot step;step.axis(0,0,0,0,2);step.axis(1,2,0,0,4);step.series(0,{forge_plot_test_point(0,1),forge_plot_test_point(2,3)},0,2);step.render();
    const double x=forge_plot_core_coordinate(step.value,0,1,0),y=forge_plot_core_coordinate(step.value,1,1,0);
    forge_plot_test_require(forge_plot_core_hit(step.value,x,y,2).kind==0,"阶梯线水平段无法命中");
    forge_plot_test_require(forge_plot_core_hit(step.value,x,forge_plot_core_coordinate(step.value,1,2,0),2).kind==-1,"阶梯线错误命中直线弦");
    ForgePlotTestPlot impulse;impulse.axis(0,0,0,0,2);impulse.axis(1,2,0,0,4);impulse.series(0,{forge_plot_test_point(1,3)},0,5);impulse.render();
    forge_plot_test_require(forge_plot_core_hit(impulse.value,forge_plot_core_coordinate(impulse.value,0,1,0),forge_plot_core_coordinate(impulse.value,1,1.5,0),2).kind==0,"脉冲线中段无法命中");
    ForgePlotTestPlot sorted;sorted.series(0,{forge_plot_test_point(2,2),forge_plot_test_point(0,0),forge_plot_test_point(1,1)});sorted.render();
    forge_plot_test_require(forge_plot_core_hit(sorted.value,forge_plot_core_coordinate(sorted.value,0,2,0),forge_plot_core_coordinate(sorted.value,1,2,0),2).index==0,"排序改变原始索引");
    const ForgePlotSelectionCore selected{0,0,0};forge_plot_core_selection(sorted.value,&selected,1);forge_plot_test_require(sorted.render().find("#ffb020")!=std::string::npos,"选择没有可见标记");
    ForgePlotTestPlot polar;polar.series(6,{forge_plot_test_point(0,2),forge_plot_test_point(90,2),forge_plot_test_point(180,2),forge_plot_test_point(270,2)});const auto svg=polar.render();
    ForgePlotRangeCore ranges[2];forge_plot_core_ranges(polar.value,ranges,2);forge_plot_test_near(ranges[0].minimum,0);forge_plot_test_near(ranges[0].maximum,360);forge_plot_test_near(ranges[1].minimum,0);
    ForgePlotRectCore rect;forge_plot_core_rects(polar.value,&rect,1);const double radius=std::min(rect.width,rect.height)*.5*2/ranges[1].maximum,cx=rect.x+rect.width*.5,cy=rect.y+rect.height*.5;
    const auto hit=forge_plot_core_hit(polar.value,cx,cy-radius,2);if(hit.kind!=0||hit.index!=1)std::cerr<<"POLAR_HIT kind="<<hit.kind<<" index="<<hit.index<<" range="<<ranges[1].minimum<<','<<ranges[1].maximum<<" point="<<cx<<','<<cy-radius<<" pixel="<<hit.pixel_x<<','<<hit.pixel_y<<'\n';forge_plot_test_require(hit.kind==0&&hit.index==1,"极坐标 90 度未映射到上方");
    const auto cursor=forge_plot_core_hit(polar.value,cx,cy-radius,-1);forge_plot_test_near(cursor.x,90);forge_plot_test_near(cursor.y,2);forge_plot_test_require(forge_plot_core_hit(polar.value,rect.x,rect.y,-1).panel==-1,"极坐标圆外返回绘图区");forge_plot_test_require(svg.find("<circle")!=std::string::npos,"极坐标缺少圆形裁剪");
    ForgePlotTestPlot prices;auto price=forge_plot_test_point(1,0);price.low=90;price.high=110;price.open=95;price.close=105;prices.series(4,{price});prices.render();forge_plot_core_ranges(prices.value,ranges,2);forge_plot_test_require(ranges[1].minimum>80,"金融轴错误包含缺省 y=0");
    ForgePlotTestPlot log_bars;log_bars.axis(0,0,0,0,2);log_bars.axis(1,2,1,1,100);log_bars.series(2,{forge_plot_test_point(1,10)});log_bars.render();forge_plot_test_require(forge_plot_core_info(log_bars.value).rendered_points==1,"零基线的对数柱未裁剪到可见范围");forge_plot_test_require(forge_plot_core_hit(log_bars.value,forge_plot_core_coordinate(log_bars.value,0,1,0),forge_plot_core_coordinate(log_bars.value,1,5,0),2).kind==0,"对数柱缺少可见几何命中");
    forge_plot_core_rects(prices.value,&rect,1);forge_plot_test_require(forge_plot_core_select(prices.value,rect.x,rect.y,rect.width,rect.height,nullptr,0)==1,"蜡烛图框选错误使用缺省 y");const auto candle=forge_plot_core_hit(prices.value,forge_plot_core_coordinate(prices.value,0,1,0),forge_plot_core_coordinate(prices.value,1,100,0),2);forge_plot_test_require(candle.kind==0&&candle.value==105,"金融提示应使用收盘值");
    ForgePlotOptionsCore options{};options.width=900;options.height=540;options.font_size=12;options.legend=1;options.adaptive=1;ForgePlotTestPlot legend;forge_plot_core_begin_view(legend.value,&options);ForgePlotSeriesCore item{};item.id=0;item.y_axis=1;item.fill_target=-1;item.stack_group=-1;item.name="Measurement / asymmetric errors";auto point=forge_plot_test_point(0,1);forge_plot_core_series(legend.value,&item,&point,1);item.id=1;item.name="Lower reference";forge_plot_core_series(legend.value,&item,&point,1);legend.render();const auto region=forge_plot_test_legend_hit(legend.value,0);const auto label=forge_plot_core_hit(legend.value,region.pixel_x+120,region.pixel_y+6,0);forge_plot_test_require(label.kind==2&&label.id==0,"长图例的文字与命中区重叠");
}
static void forge_plot_test_panels_and_tickers() {
    ForgePlotTestPlot p;
    for(int panel=0;panel<2;++panel){ForgePlotPanelCore r{};r.id=panel;r.x=panel*.5;r.width=.5;r.height=1;forge_plot_core_panel(p.value,&r);for(int side=0;side<2;++side){ForgePlotAxisCore axis{};axis.id=panel*2+side;axis.panel=panel;axis.side=side?2:0;axis.auto_range=1;axis.visible=1;axis.grid=1;forge_plot_core_axis(p.value,&axis);}ForgePlotSeriesCore series{};series.id=panel;series.kind=2;series.x_axis=panel*2;series.y_axis=panel*2+1;series.stack_group=0;series.bar_width=.8;series.selectable=1;const auto point=forge_plot_test_point(0,panel?10:100);forge_plot_core_series(p.value,&series,&point,1);}
    p.render();const auto hit=forge_plot_core_hit(p.value,forge_plot_core_coordinate(p.value,2,0,0),forge_plot_core_coordinate(p.value,3,5,0),1);forge_plot_test_require(hit.kind==0&&hit.id==1,"不同绘图区的堆叠互相影响");
    for(int ticker=1;ticker<=4;++ticker){ForgePlotTestPlot plot;ForgePlotAxisCore axis{};axis.id=0;axis.side=0;axis.visible=1;axis.auto_range=1;axis.ticker=ticker;axis.time_origin=1790985600;axis.precision=1;axis.format="%Y-%m-%d %H:%M:%S";ForgePlotTickCore ticks[2]{{0,"起点"},{3600,"终点"}};if(ticker==1){axis.ticks=ticks;axis.tick_count=2;}forge_plot_core_axis(plot.value,&axis);plot.axis(1,2,0,0,1,true);plot.series(0,{forge_plot_test_point(0,0),forge_plot_test_point(3600,1)});const auto svg=plot.render();forge_plot_test_require(svg.find(ticker==1?"起点":ticker==2?"2026-10-03":ticker==3?":00:":"π")!=std::string::npos,"刻度格式缺失");}
    ForgePlotTestPlot points;ForgePlotSeriesCore scatter{};scatter.id=0;scatter.y_axis=1;scatter.scatter_style=2;scatter.point_size=6;scatter.stack_group=-1;scatter.fill_target=-1;scatter.selectable=1;std::vector<ForgePlotPointCore> cloud;for(int i=0;i<100000;++i)cloud.push_back(forge_plot_test_point(i%1000,(i/1000)%100));forge_plot_core_series(points.value,&scatter,cloud.data(),cloud.size());points.render();forge_plot_test_require(forge_plot_core_info(points.value).rendered_points<30000,"散点未按二维空间降采样");ForgePlotRectCore rect;forge_plot_core_rects(points.value,&rect,1);forge_plot_test_require(forge_plot_core_select(points.value,rect.x,rect.y,rect.width,rect.height,nullptr,0)==100000,"散点框选丢失原始数据");
}
static void forge_plot_test_channel_gaps() {
    ForgePlotTestPlot p;p.axis(0,0,0,0,6);p.axis(1,2,0,0,8);
    ForgePlotSeriesCore series{};series.id=0;series.y_axis=1;series.kind=0;series.line_style=2;series.fill_color=0x80eeddcc;series.fill_target=1;series.stack_group=-1;
    const std::vector<ForgePlotPointCore> top{forge_plot_test_point(0,6),forge_plot_test_point(6,7)};
    forge_plot_core_series(p.value,&series,top.data(),top.size());
    series.id=1;series.fill_target=-1;series.fill_color=0;series.line_style=1;
    const std::vector<ForgePlotPointCore> lower{forge_plot_test_point(1,1),forge_plot_test_point(2,2),forge_plot_test_point(3,NAN),forge_plot_test_point(4,1),forge_plot_test_point(5,2)};
    forge_plot_core_series(p.value,&series,lower.data(),lower.size());const auto svg=p.render();
    const auto color=svg.find("#eeddcc");forge_plot_test_require(color!=std::string::npos,"系列间填充缺失");
    const auto start=svg.rfind("<path",color);const auto end=svg.find("/>",color);const auto path=svg.substr(start,end-start);
    forge_plot_test_require(std::count(path.begin(),path.end(),'M')==2,"系列间填充跨越参考曲线空值");
    ForgePlotTestPlot hidden;series.id=0;series.fill_target=1;series.fill_color=0x80eeddcc;forge_plot_core_series(hidden.value,&series,top.data(),top.size());series.id=1;series.fill_color=0;series.fill_target=-1;series.hidden=1;forge_plot_core_series(hidden.value,&series,lower.data(),lower.size());forge_plot_test_require(hidden.render().find("#eeddcc")==std::string::npos,"隐藏参考曲线仍产生填充");
}
static void forge_plot_test_dense_markers() {
    std::vector<ForgePlotPointCore> points;for(int i=0;i<500;++i)points.push_back(forge_plot_test_point(i*.01,5+std::sin(i*.01)*.1));
    ForgePlotTestPlot sampled(true),full(false);
    ForgePlotSeriesCore series{};series.y_axis=1;series.line_style=1;series.scatter_style=2;series.selectable=1;series.point_size=6;series.stack_group=-1;series.fill_target=-1;
    for(auto* plot:{&sampled,&full}){plot->axis(0,0,0,0,10);plot->axis(1,2,0,0,10);forge_plot_core_series(plot->value,&series,points.data(),points.size());plot->render();}
    forge_plot_test_require(forge_plot_core_info(sampled.value).svg_bytes<forge_plot_core_info(full.value).svg_bytes/2,"密集符号未按屏幕网格约束");
    forge_plot_test_require(forge_plot_core_info(sampled.value).rendered_points==points.size(),"符号去重不能删除折线顶点");
    const auto& point=points[123];const double x=forge_plot_core_coordinate(sampled.value,0,point.x,0),y=forge_plot_core_coordinate(sampled.value,1,point.y,0);
    forge_plot_test_require(forge_plot_core_hit(sampled.value,x,y,.1).index==123,"符号去重不能改变原始点命中");
}
static void forge_plot_test_streaming(const std::filesystem::path& dir) {
    ForgePlotTestPlot p;std::vector<ForgePlotPointCore> points;points.reserve(1000000);
    for(size_t i=0;i<1000000;++i)points.push_back(forge_plot_test_point(i*.001,std::sin(i*.001)));
    p.series(0,points);
    ForgePlotOptionsCore o{};o.width=900;o.height=540;o.font_size=12;o.background=0xff202020;o.foreground=0xffeeeeee;o.grid_color=0xff555555;o.adaptive=1;o.antialias=1;o.x_window_mode=1;o.x_window_size=10;
    forge_plot_core_begin_view(p.value,&o);p.axis(0,0,0,0,1,true);p.axis(1,2,0,0,1,true);p.render();
    ForgePlotRangeCore ranges[2],domains[2];forge_plot_core_ranges(p.value,ranges,2);forge_plot_core_domains(p.value,domains,2);
    forge_plot_test_near(ranges[0].maximum-ranges[0].minimum,10);forge_plot_test_near(domains[0].minimum,0);forge_plot_test_near(domains[0].maximum,999.999);
    std::ofstream csv(dir/"streaming-performance.csv");csv<<"history_points,append_points,append_ms,render_ms,rendered_points\n";
    for(int batch=0;batch<100;++batch){std::vector<ForgePlotPointCore> next;for(int i=0;i<32;++i){const double x=(1000000+batch*32+i)*.001;next.push_back(forge_plot_test_point(x,std::sin(x)));}
        const auto start=std::chrono::steady_clock::now();forge_plot_test_require(forge_plot_core_append(p.value,0,next.data(),next.size())!=0,"增量追加失败");const auto appended=std::chrono::steady_clock::now();p.render();const auto rendered=std::chrono::steady_clock::now();const auto info=forge_plot_core_info(p.value);
        forge_plot_test_require(info.input_points==1000000+(batch+1)*32,"无限历史被丢弃");forge_plot_test_require(info.rendered_points<5000,"窗口绘制未受屏幕像素约束");
        csv<<info.input_points<<",32,"<<std::chrono::duration<double,std::milli>(appended-start).count()<<','<<std::chrono::duration<double,std::milli>(rendered-appended).count()<<','<<info.rendered_points<<'\n';
    }
    const auto oldest=forge_plot_core_measure(p.value,0,0);const auto latest=forge_plot_core_measure(p.value,0,1003.199);forge_plot_test_require(oldest.index==0&&latest.index==1003199,"光标查询丢失历史或原始索引");
    forge_plot_core_begin_view(p.value,&o);p.axis(0,0,0,100,110);p.axis(1,2,0,-1,1);p.render();const auto next=forge_plot_test_point(1003.2,1);forge_plot_test_require(forge_plot_core_append(p.value,0,&next,1)!=0,"暂停视图后的追加失败");p.render();forge_plot_core_ranges(p.value,ranges,2);forge_plot_test_near(ranges[0].minimum,100);forge_plot_test_near(ranges[0].maximum,110);
    forge_plot_test_require(forge_plot_core_append(p.value,99,&next,1)==0,"不存在的曲线应拒绝追加");
    std::cout<<"PLOT_STREAMING_TESTS_OK history="<<forge_plot_core_info(p.value).input_points<<'\n';
}
static void forge_plot_test_legends(const std::filesystem::path& dir) {
    for(int position=0;position<8;++position)for(int style=0;style<3;++style){ForgePlotTestPlot p;p.series(0,{forge_plot_test_point(0,1),forge_plot_test_point(1,123.456)});
        ForgePlotOptionsCore o{};o.width=900;o.height=540;o.font_size=12;o.background=0xff202020;o.foreground=0xffeeeeee;o.grid_color=0xff555555;o.adaptive=1;o.legend=1;o.legend_position=position;o.legend_style=style;o.legend_latest_value=1;o.legend_precision=2;
        o.legend=0;forge_plot_core_begin_view(p.value,&o);p.axis(0,0,0,0,1,true);p.axis(1,2,0,0,1,true);p.render();ForgePlotRectCore without_legend{};forge_plot_core_rects(p.value,&without_legend,1);
        o.legend=1;forge_plot_core_begin_view(p.value,&o);const auto svg=p.render();forge_plot_test_require(svg.find("数据  123.46")!=std::string::npos,"图例最新值未正确格式化");
        ForgePlotRectCore with_legend{};forge_plot_core_rects(p.value,&with_legend,1);forge_plot_test_near(with_legend.x,without_legend.x);forge_plot_test_near(with_legend.y,without_legend.y);forge_plot_test_near(with_legend.width,without_legend.width);forge_plot_test_near(with_legend.height,without_legend.height);
        const std::string background="fill=\"#202020\"";const auto background_at=svg.find(background);forge_plot_test_require(background_at!=std::string::npos&&svg.find(background,background_at+background.size())==std::string::npos,"图例背景不能覆盖波形");
        bool hit=false;for(int y=0;y<540&&!hit;y+=4)for(int x=0;x<900&&!hit;x+=4)if(forge_plot_core_hit(p.value,x,y,0).kind==2)hit=true;forge_plot_test_require(hit,"图例位置与点击区域未对齐");
        forge_plot_test_require(forge_plot_core_export(p.value,(dir/("legend-"+std::to_string(position)+"-"+std::to_string(style)+".svg")).string().c_str(),0)!=0,"图例截图导出失败");
        if(position==2&&style==1){ForgePlotSeriesCore metadata{};metadata.id=0;metadata.y_axis=1;metadata.line_style=1;metadata.line_width=2;metadata.stack_group=-1;metadata.fill_target=-1;metadata.name="Very long measurement series name whose latest value must remain visible after truncation";forge_plot_test_require(forge_plot_core_update_series(p.value,&metadata)!=0,"更新图例名称失败");const auto long_name=p.render();forge_plot_test_require(long_name.find("123.46</text>")!=std::string::npos,"长图例名称截断时丢失最新值");}
    }std::cout<<"PLOT_LEGEND_TESTS_OK positions=8 styles=3\n";
}
static void forge_plot_test_annotation_placement(const std::filesystem::path& dir) {
    const auto overlaps=[](const auto& a,const auto& b){return a.x<b.x+b.width-.001&&a.x+a.width>b.x+.001&&a.y<b.y+b.height-.001&&a.y+a.height>b.y+.001;};
    const auto inside=[](const auto& a,const auto& b){return a.x>=b.x-.001&&a.y>=b.y-.001&&a.x+a.width<=b.x+b.width+.001&&a.y+a.height<=b.y+b.height+.001;};
    int cases=0;
    // 位置互相独立，遍历全部组合，检查同侧空间预留和区域外图例点击。
    for(int legend_area=0;legend_area<2;++legend_area)for(int measure_area=0;measure_area<2;++measure_area)for(int legend_position=0;legend_position<8;++legend_position)for(int measure_position=0;measure_position<8;++measure_position)for(int style=0;style<3;++style){
        ForgePlotTestPlot p;p.series(0,{forge_plot_test_point(0,1),forge_plot_test_point(1,2)});
        ForgePlotOptionsCore o{};o.width=900;o.height=540;o.font_size=12;o.adaptive=1;o.legend=1;o.legend_position=legend_position;o.legend_placement=legend_area;o.legend_style=style;o.measurement_enabled=1;o.measurement_placement=measure_area;o.measurement_position=measure_position;
        forge_plot_core_begin_view(p.value,&o);p.render();ForgePlotRectCore wave{};forge_plot_core_rects(p.value,&wave,1);const auto legend=forge_plot_core_annotation_rect(p.value,0),measure=forge_plot_core_annotation_rect(p.value,1);const ForgePlotRectCore component{0,0,0,900,540};
        forge_plot_test_require(inside(legend,component)&&inside(measure,component),"图例或测量框超出组件边界");
        forge_plot_test_require(legend_area?!overlaps(legend,wave):inside(legend,wave),"图例区域内外设置未生效");
        forge_plot_test_require(measure_area?!overlaps(measure,wave):inside(measure,wave),"测量区域内外设置未生效");
        forge_plot_test_require(!overlaps(legend,measure),"同侧图例与测量框重叠");
        const auto hit=forge_plot_core_hit(p.value,legend.x+10,legend.y+14,0);forge_plot_test_require(hit.kind==2&&hit.id==0,"区域内外图例未保持点击命中");
        if(legend_position==measure_position&&style==0)forge_plot_core_export(p.value,(dir/("placement-"+std::to_string(legend_area)+"-"+std::to_string(measure_area)+"-"+std::to_string(legend_position)+".svg")).string().c_str(),0);
        ++cases;
    }
    ForgePlotTestPlot panels;ForgePlotOptionsCore o{};o.width=900;o.height=540;o.font_size=12;o.measurement_enabled=1;o.measurement_series=7;o.measurement_position=3;o.measurement_placement=1;forge_plot_core_begin_view(panels.value,&o);
    for(int id=0;id<2;++id){ForgePlotPanelCore r{};r.id=id;r.y=id*.5;r.width=1;r.height=.5;forge_plot_core_panel(panels.value,&r);ForgePlotAxisCore a{};a.id=id*2;a.panel=id;a.auto_range=1;a.visible=1;forge_plot_core_axis(panels.value,&a);a.id+=1;a.side=2;forge_plot_core_axis(panels.value,&a);}
    ForgePlotSeriesCore s{};s.id=7;s.x_axis=2;s.y_axis=3;s.line_style=1;const auto point=forge_plot_test_point(1,2);forge_plot_core_series(panels.value,&s,&point,1);panels.render();const auto measure=forge_plot_core_annotation_rect(panels.value,1);ForgePlotRectCore rects[2];forge_plot_core_rects(panels.value,rects,2);
    forge_plot_test_require(measure.id==1&&!overlaps(measure,rects[0])&&!overlaps(measure,rects[1]),"测量框未跟随所选曲线绘图区");
    std::cout<<"PLOT_PLACEMENT_TESTS_OK combinations="<<cases<<" panels=2\n";
}
static void forge_plot_test_summary_extrema() {
    ForgePlotTestPlot p;std::vector<ForgePlotPointCore> points;for(int i=0;i<8192;++i)points.push_back(forge_plot_test_point(i,i==3267?1000:i==3268?-1000:0));p.series(0,points);p.axis(0,0,0,0,1000000);p.axis(1,2,0,-1100,1100);p.render();
    forge_plot_test_require(forge_plot_core_info(p.value).rendered_points<20,"块级抽样未约束输出");for(int index:{3267,3268}){const auto& pt=points[index];const auto hit=forge_plot_core_hit(p.value,forge_plot_core_coordinate(p.value,0,pt.x,0),forge_plot_core_coordinate(p.value,1,pt.y,0),1);forge_plot_test_require(hit.index==index,"块级抽样丢失峰谷特征或原始索引");}
    std::cout<<"PLOT_SUMMARY_EXTREMA_OK\n";
}
static void forge_plot_test_benchmark(const std::filesystem::path& dir) {
    std::ofstream out(dir/"core-performance.csv");out<<"points,adaptive,rendered_points,svg_bytes,render_ms,hit_1000_ms\n";
    for(size_t count:{100000u,1000000u})for(bool adaptive:{false,true}){
        std::vector<ForgePlotPointCore> points;points.reserve(count);const size_t spike=count/3;for(size_t i=0;i<count;++i)points.push_back(forge_plot_test_point(static_cast<double>(i),i==spike?1000:std::sin(i*.003)));
        ForgePlotTestPlot p(adaptive);p.series(0,points);const auto start=std::chrono::steady_clock::now();p.render();const auto end=std::chrono::steady_clock::now();const auto info=forge_plot_core_info(p.value);
        if(adaptive)forge_plot_test_require(info.rendered_points<5000,"降采样未按像素约束输出");
        const double x=forge_plot_core_coordinate(p.value,0,static_cast<double>(spike),0),y=forge_plot_core_coordinate(p.value,1,1000,0);const auto peak=forge_plot_core_hit(p.value,x,y,2);forge_plot_test_require(peak.kind==0&&peak.index==static_cast<int>(spike),"降采样丢失峰值或原始索引");
        const auto hit_start=std::chrono::steady_clock::now();for(int i=0;i<1000;++i)forge_plot_core_hit(p.value,x+i%10,y,3);const auto hit_end=std::chrono::steady_clock::now();
        const double ms=std::chrono::duration<double,std::milli>(end-start).count(),hit_ms=std::chrono::duration<double,std::milli>(hit_end-hit_start).count();out<<count<<','<<adaptive<<','<<info.rendered_points<<','<<info.svg_bytes<<','<<ms<<','<<hit_ms<<'\n';std::cout<<"PLOT_PERF points="<<count<<" adaptive="<<adaptive<<" rendered="<<info.rendered_points<<" render_ms="<<ms<<" hit_ms="<<hit_ms<<'\n';
    }
    std::ofstream maps(dir/"map-performance.csv");maps<<"cells,render_ms,svg_bytes\n";
    for(int size:{5,100,1000}){ForgePlotTestPlot p;ForgePlotMapCore map{};map.columns=map.rows=size;map.y_axis=1;map.x_max=map.y_max=size;map.auto_range=1;std::vector<double> values(static_cast<size_t>(size)*size);for(size_t i=0;i<values.size();++i)values[i]=std::sin(i*.01);forge_plot_core_map(p.value,&map,values.data(),values.size());const auto start=std::chrono::steady_clock::now();p.render();const double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();maps<<values.size()<<','<<ms<<','<<forge_plot_core_info(p.value).svg_bytes<<'\n';std::cout<<"PLOT_MAP_PERF cells="<<values.size()<<" render_ms="<<ms<<" svg_bytes="<<forge_plot_core_info(p.value).svg_bytes<<'\n';}
}
static void forge_plot_test_type_updates() {
    const std::vector<ForgePlotPointCore> original{forge_plot_test_point(2,2),forge_plot_test_point(0,0),forge_plot_test_point(1,1),forge_plot_test_point(NAN,NAN),forge_plot_test_point(3,3),forge_plot_test_point(1,2)};
    for(const auto [before,after]:{std::pair{0,1},std::pair{1,0},std::pair{0,2},std::pair{2,0}}){
        ForgePlotTestPlot plot;plot.series(before,original);plot.render();
        ForgePlotSeriesCore series{};series.kind=after;series.y_axis=1;series.line_style=1;series.selectable=1;series.color=0xff123456;series.fill_color=0x40123456;series.fill_target=-1;series.line_width=2;series.point_size=6;series.bar_width=.8;series.stack_group=-1;series.name="数据";
        forge_plot_test_require(forge_plot_core_update_series(plot.value,&series)!=0,"切换系列类型失败");
        ForgePlotTestPlot expected;expected.series(after,original);
        forge_plot_test_require(plot.render()==expected.render(),"切换系列类型后排序索引失效");
        const std::vector<ForgePlotPointCore> more{forge_plot_test_point(4,4),forge_plot_test_point(.5,2)};
        forge_plot_test_require(forge_plot_core_append(plot.value,0,more.data(),more.size())!=0,"切换类型后追加失败");
        auto appended=original;appended.insert(appended.end(),more.begin(),more.end());
        ForgePlotTestPlot appended_expected;appended_expected.series(after,appended);
        forge_plot_test_require(plot.render()==appended_expected.render(),"切换类型后追加数据的顺序或原始索引错误");
    }
    std::cout<<"PLOT_TYPE_UPDATE_OK transitions=4 appends=4 gaps=preserved\n";
}
int main(int argc,char** argv) {
    try{const auto configured=std::getenv("SLINTFORGE_PLOT_QA_DIR");const auto dir=std::filesystem::path(configured?configured:"build/plot-qa");std::filesystem::create_directories(dir);forge_plot_test_axes();forge_plot_test_types();forge_plot_test_maps_items_exports(dir);forge_plot_test_selection();forge_plot_test_geometry_regressions();forge_plot_test_panels_and_tickers();forge_plot_test_channel_gaps();forge_plot_test_dense_markers();forge_plot_test_streaming(dir);forge_plot_test_legends(dir);forge_plot_test_annotation_placement(dir);forge_plot_test_summary_extrema();forge_plot_test_type_updates();if(argc>1&&std::string(argv[1])=="--benchmark")forge_plot_test_benchmark(dir);std::cout<<"PLOT_CORE_TESTS_OK\n";return 0;}
    catch(const std::exception& e){std::cerr<<"PLOT_CORE_TESTS_FAILED "<<e.what()<<'\n';return 1;}
}
