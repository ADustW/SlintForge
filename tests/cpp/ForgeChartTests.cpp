#include "ForgeCharts.hpp"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>

static ForgeChartSeriesData forge_chart_test_points(std::initializer_list<std::pair<double,double>> values) {
    ForgeChartSeriesData result;result.name="测试数据";for(const auto& p:values)result.points.push_back({p.first,p.second,15,"",{}});return result;
}
static void forge_chart_test_near(double a,double b){assert(std::abs(a-b)<1e-8);}
int main() {
    const std::vector<std::string> categories{"甲","乙","丙"};
    auto data=std::vector<ForgeChartSeriesData>{forge_chart_test_points({{0,10},{1,-5}}),forge_chart_test_points({{0,20},{1,-7}})};
    ForgeChartOptionsData options;options.stacked=true;
    auto bars=forge_prepare_chart(0,data,{},{"甲","乙"},options);
    assert(bars.status==0&&bars.marks.size()==4);forge_chart_test_near(bars.y_min,-20);forge_chart_test_near(bars.y_max,30);forge_chart_test_near(bars.zero_y,0.6);
    forge_chart_test_near(bars.marks[0].height,0.2);forge_chart_test_near(bars.marks[2].height,0.4);forge_chart_test_near(bars.marks[3].height,0.14);
    assert(forge_chart_hit_test(bars.marks,bars.marks[0].x+0.05,0.5,500,300,0)==0);
    data[1].hidden=true;assert(forge_prepare_chart(0,data,{},{"甲","乙"},options).marks.size()==2);
    options.smooth=true;options.fill_area=true;
    const auto line=forge_prepare_chart(1,{forge_chart_test_points({{0,0},{1,10},{std::numeric_limits<double>::quiet_NaN(),5},{3,4}})},{},{},options);
    assert(line.marks.size()==3&&line.lines.size()==1&&line.areas.size()==1&&line.x_min<0&&line.x_max>3);
    options.auto_range=false;options.x_min=2;options.x_max=2;assert(forge_prepare_chart(5,data,{},{},options).status==2);
    options=ForgeChartOptionsData{};assert(forge_prepare_chart(1,{},{},{},options).status==1);
    const auto constant=forge_prepare_chart(5,{forge_chart_test_points({{2,4}})},{},{},options);assert(constant.x_min<2&&constant.x_max>2&&constant.y_min<4&&constant.y_max>4);
    std::vector<ForgeChartSliceData> slices{{"甲",1,{},false},{"乙",3,{},false},{"负值",-4,{},false}};
    options.doughnut=true;const auto pie=forge_prepare_chart(2,{},slices,{},options);assert(pie.marks.size()==2);
    forge_chart_test_near(pie.marks[0].sweep_angle,90);forge_chart_test_near(pie.marks[1].sweep_angle,270);assert(forge_chart_hit_test(pie.marks,0.5,0.5,300,300,pie.inner_radius)==-1);
    options.doughnut=false;const auto full=forge_prepare_chart(2,{},{{"甲",1,{},false}},{},options);
    forge_chart_test_near(full.marks[0].sweep_angle,360);forge_chart_test_near(full.marks[0].x1,full.marks[0].x3);forge_chart_test_near(full.marks[0].y1,full.marks[0].y3);
    const auto polar=forge_prepare_chart(3,{},{{"甲",1,{},false},{"乙",4,{},false}},{},options);
    forge_chart_test_near(polar.marks[0].sweep_angle,180);forge_chart_test_near(polar.marks[0].radius/polar.marks[1].radius,0.5);
    assert(forge_chart_hit_test(polar.marks,0.8,0.5,300,300,0)==-1);assert(forge_chart_hit_test(polar.marks,0.2,0.5,300,300,0)==1);
    assert(forge_prepare_chart(2,{},{{"零",0,{},false}},{},options).status==1);
    const auto bubble=forge_prepare_chart(4,{forge_chart_test_points({{0,0},{10,10}})},{},{},options);const auto& p=bubble.marks[0];
    assert(forge_chart_hit_test(bubble.marks,p.x+10./400,p.y,400,200,0)==0);assert(forge_chart_hit_test(bubble.marks,p.x+30./400,p.y,400,200,0)==-1);
    const auto radar=forge_prepare_chart(6,{forge_chart_test_points({{0,1},{1,2},{2,3}})},{},categories,options);
    assert(radar.marks.size()==3&&radar.lines.size()==3&&radar.areas.size()==3&&radar.labels.size()==3);
    forge_chart_test_near(radar.lines[2].x1,radar.lines[0].x);forge_chart_test_near(radar.lines[2].y1,radar.lines[0].y);
    assert(forge_chart_hit_test(radar.marks,0.5,0.5,0,200,0)==-1);
    assert(forge_prepare_chart(6,{forge_chart_test_points({{0,1},{1,2}})},{},{},options).status==2);
    // 没有对应类别的数据不能参与自动范围。
    for(const int kind:{0,1}) {
        const auto bounded=forge_prepare_chart(kind,{forge_chart_test_points({{-1,10000},{0,2},{1,4},{2,-10000}})},{},{"A","B"},options);
        assert(bounded.status==0&&bounded.marks.size()==2&&bounded.y_max<=10&&bounded.y_min>=-1);
    }
    assert(forge_prepare_chart(0,{forge_chart_test_points({{9,3}})},{},{"A","B"},options).status==1);
    options.auto_range=false;options.x_min=-std::numeric_limits<double>::max();options.x_max=std::numeric_limits<double>::max();options.y_min=-1;options.y_max=1;
    assert(forge_prepare_chart(5,{forge_chart_test_points({{0,0}})},{},{},options).status==2);
    options.x_min=-1;options.x_max=1;options.y_min=-std::numeric_limits<double>::max();options.y_max=std::numeric_limits<double>::max();
    assert(forge_prepare_chart(5,{forge_chart_test_points({{0,0}})},{},{},options).status==2);
    options=ForgeChartOptionsData{};
    const auto directory=std::filesystem::path("build/charts-qa");std::filesystem::create_directories(directory);
    std::ofstream output(directory/"cpp-geometry.csv");output.imbue(std::locale::classic());output<<std::fixed<<std::setprecision(6);
    const std::vector<ForgeChartSeriesData> contract{forge_chart_test_points({{0,20},{1,60},{2,40}}),forge_chart_test_points({{0,30},{1,10},{2,50}})};
    options.stacked=true;options.smooth=true;options.fill_area=true;slices.resize(2);
    for(int kind=0;kind<=6;++kind) {
        const auto scene=forge_prepare_chart(kind,contract,slices,categories,options);assert(scene.status==0);
        output<<kind<<','<<scene.status<<','<<scene.marks.size()<<','<<scene.lines.size()<<','<<scene.areas.size()<<','<<scene.x_min<<','<<scene.x_max<<','<<scene.y_min<<','<<scene.y_max<<'\n';
        for(const auto& m:scene.marks)output<<m.kind<<','<<m.series_index<<','<<m.point_index<<','<<m.x<<','<<m.y<<','<<m.width<<','<<m.height<<','<<m.radius<<','<<m.start_angle<<','<<m.sweep_angle<<'\n';
    }
}
