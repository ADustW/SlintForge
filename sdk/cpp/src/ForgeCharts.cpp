#include "ForgeCharts.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <limits>
#include <locale>
#include <optional>
#include <sstream>
#include <utility>

static constexpr double forge_chart_pi=3.14159265358979323846;
slint::Color forge_chart_color(std::size_t index) {
    static constexpr std::array<std::array<unsigned char,3>,7> colors{{{54,162,235},{255,99,132},{75,192,192},{255,205,86},{153,102,255},{255,159,64},{120,134,148}}};
    const auto& c=colors[index%colors.size()];return slint::Color::from_rgb_uint8(c[0],c[1],c[2]);
}
static std::string forge_chart_number(double value,int precision) {
    if(std::abs(value)<1e-10)value=0;
    std::ostringstream stream;stream.imbue(std::locale::classic());stream<<std::fixed<<std::setprecision(std::clamp(precision,0,6))<<value;
    auto text=stream.str();if(text.find('.')!=std::string::npos){while(text.back()=='0')text.pop_back();if(text.back()=='.')text.pop_back();}return text;
}
static std::pair<double,double> forge_chart_nice_range(double low,double high,bool zero,bool padding) {
    if(zero){low=std::min(low,0.);high=std::max(high,0.);}
    if(low==high){if(low==0)high=1;else{const auto delta=std::max(std::abs(low),1.)*0.1;low-=delta;high+=delta;}}
    else if(padding){const auto delta=(high-low)*0.05;low-=delta;high+=delta;}
    const auto raw=(high-low)/5;const auto power=std::pow(10.,std::floor(std::log10(raw)));const auto fraction=raw/power;
    const auto step=(fraction<=1?1.:fraction<=2?2.:fraction<=2.5?2.5:fraction<=5?5.:10.)*power;
    return {std::floor(low/step)*step,std::ceil(high/step)*step};
}
static std::vector<ForgeChartTickData> forge_chart_ticks(double low,double high,int precision) {
    std::vector<ForgeChartTickData> output;for(int i=0;i<=5;++i){const auto p=i/5.;output.push_back({p,forge_chart_number(low+(high-low)*p,precision)});}return output;
}
static slint::Color forge_chart_resolve_color(slint::Color color,std::size_t index) {return color.alpha()==0?forge_chart_color(index):color;}
static std::pair<double,double> forge_chart_polar_point(double angle,double radius) {return {0.5+radius*std::cos(angle*forge_chart_pi/180.),0.5+radius*std::sin(angle*forge_chart_pi/180.)};}
static ForgeChartLineData forge_chart_line(std::pair<double,double> a,std::pair<double,double> b,std::pair<double,double> previous,std::pair<double,double> next,bool smooth,slint::Color color) {
    ForgeChartLineData line;line.x=a.first;line.y=a.second;line.x1=b.first;line.y1=b.second;line.color=color;
    if(smooth){line.c1x=a.first+(b.first-previous.first)/6;line.c1y=a.second+(b.second-previous.second)/6;line.c2x=b.first-(next.first-a.first)/6;line.c2y=b.second-(next.second-a.second)/6;}
    else{line.c1x=a.first+(b.first-a.first)/3;line.c1y=a.second+(b.second-a.second)/3;line.c2x=a.first+2*(b.first-a.first)/3;line.c2y=a.second+2*(b.second-a.second)/3;}return line;
}
ForgeChartSceneData forge_prepare_chart(int kind,const std::vector<ForgeChartSeriesData>& series,const std::vector<ForgeChartSliceData>& slices,const std::vector<std::string>& categories,const ForgeChartOptionsData& options) {
    ForgeChartSceneData scene;if(kind<0||kind>6){scene.status=2;return scene;}
    if(kind==2||kind==3) {
        std::vector<std::size_t> values;double total=0,maximum=0;
        for(std::size_t i=0;i<slices.size();++i){const auto& s=slices[i];if(!s.hidden&&std::isfinite(s.value)&&s.value>0){values.push_back(i);total+=s.value;maximum=std::max(maximum,s.value);}}
        if(total<=0||!std::isfinite(total))return scene;
        scene.status=0;scene.y_max=maximum;scene.inner_radius=kind==2&&options.doughnut?0.23:0.;double angle=-90;
        for(const auto index:values) {
            const auto& s=slices[index];const auto sweep=kind==3?360./values.size():s.value/total*360.;const auto radius=kind==3?0.45*std::sqrt(s.value/maximum):0.45;
            const auto a=forge_chart_polar_point(angle,radius),b=forge_chart_polar_point(angle+sweep/2,radius),c=forge_chart_polar_point(angle+sweep,radius);
            ForgeChartMarkData mark;mark.kind=2;mark.x=0.5;mark.y=0.5;mark.radius=radius;mark.x1=a.first;mark.y1=a.second;mark.x2=b.first;mark.y2=b.second;mark.x3=c.first;mark.y3=c.second;
            mark.start_angle=angle;mark.sweep_angle=sweep;mark.color=forge_chart_resolve_color(s.color,index);mark.series_index=-1;mark.point_index=static_cast<int>(index);mark.value=s.value;
            mark.label=s.label+": "+forge_chart_number(s.value,options.precision)+" ("+forge_chart_number(s.value/total*100,1)+"%)";scene.marks.push_back(std::move(mark));angle+=sweep;
        }
        if(kind==3)scene.y_ticks=forge_chart_ticks(0,maximum,options.precision);return scene;
    }
    std::vector<std::size_t> visible;std::vector<const ForgeChartPointData*> all;
    for(std::size_t i=0;i<series.size();++i)if(!series[i].hidden){visible.push_back(i);for(const auto& p:series[i].points)if(std::isfinite(p.x)&&std::isfinite(p.y)&&(kind!=4||(std::isfinite(p.size)&&p.size>0)))all.push_back(&p);}
    if(all.empty())return scene;
    const bool categorical=kind==0||kind==6||(kind==1&&!categories.empty());std::size_t inferred=0;
    for(const auto p:all)if(p->x>=0&&p->x<10000)inferred=std::max(inferred,static_cast<std::size_t>(std::floor(p->x))+1);
    const auto count=categories.empty()?inferred:categories.size();
    if(categorical&&(count==0||count>10000||(kind==6&&count<3))){scene.status=2;return scene;}
    // 自动范围与绘制使用同一类别域，忽略没有对应类别的数据点。
    if(categorical){std::erase_if(all,[&](const auto p){return p->x<0||p->x>=count;});if(all.empty())return scene;}
    double xmin=std::numeric_limits<double>::infinity(),xmax=-xmin,ymin=xmin,ymax=xmax;
    for(const auto p:all){xmin=std::min(xmin,p->x);xmax=std::max(xmax,p->x);ymin=std::min(ymin,p->y);ymax=std::max(ymax,p->y);}
    if(kind==0&&options.stacked) {
        std::vector<double> positive(count,0),negative(count,0);
        for(const auto p:all){if(p->x<0||p->x>=count)continue;const auto i=static_cast<std::size_t>(std::floor(p->x));if(p->y>=0)positive[i]+=p->y;else negative[i]+=p->y;}
        ymin=std::min(0.,*std::min_element(negative.begin(),negative.end()));ymax=std::max(0.,*std::max_element(positive.begin(),positive.end()));
    }
    if(kind==6){ymin=0;ymax=std::max(ymax,0.);}
    if(options.auto_range) {
        const auto xr=categorical?std::pair<double,double>{0,static_cast<double>(count)}:forge_chart_nice_range(xmin,xmax,false,true);xmin=xr.first;xmax=xr.second;
        const auto yr=forge_chart_nice_range(ymin,ymax,options.include_zero||kind==0||kind==6,kind==4||kind==5);ymin=yr.first;ymax=yr.second;
    } else {xmin=categorical?0:options.x_min;xmax=categorical?static_cast<double>(count):options.x_max;ymin=options.y_min;ymax=options.y_max;}
    if(!std::isfinite(xmin)||!std::isfinite(xmax)||!std::isfinite(ymin)||!std::isfinite(ymax)||!std::isfinite(xmax-xmin)||!std::isfinite(ymax-ymin)||xmin>=xmax||ymin>=ymax||(kind==6&&ymin<0)){scene.status=2;return scene;}
    scene.status=0;scene.x_min=xmin;scene.x_max=xmax;scene.y_min=ymin;scene.y_max=ymax;scene.zero_y=1-(0-ymin)/(ymax-ymin);
    const auto xn=[&](double x){return categorical?(x+0.5)/count:(x-xmin)/(xmax-xmin);};const auto yn=[&](double y){return 1-(y-ymin)/(ymax-ymin);};
    scene.y_ticks=forge_chart_ticks(ymin,ymax,options.precision);
    if(categorical){const auto stride=(count+7)/8;for(std::size_t i=0;i<count;i+=stride)scene.x_ticks.push_back({(i+0.5)/count,i<categories.size()?categories[i]:std::to_string(i+1)});}
    else scene.x_ticks=forge_chart_ticks(xmin,xmax,options.precision);
    if(kind==6){scene.x_ticks.clear();for(std::size_t i=0;i<count;++i){const auto p=forge_chart_polar_point(-90+360.*i/count,0.51);scene.labels.push_back({p.first,p.second,i<categories.size()?categories[i]:std::to_string(i+1)});}}
    std::vector<double> positive(categorical?count:0,0),negative=positive;
    for(std::size_t order=0;order<visible.size();++order) {
        const auto si=visible[order];const auto& data=series[si];const auto color=forge_chart_resolve_color(data.color,si);const auto name=data.name.empty()?"数据集 "+std::to_string(si+1):data.name;
        std::vector<std::optional<ForgeChartMarkData>> projected(data.points.size());
        for(std::size_t pi=0;pi<data.points.size();++pi) {
            const auto& p=data.points[pi];
            if(!std::isfinite(p.x)||!std::isfinite(p.y)||(categorical&&(p.x<0||p.x>=count))||(kind==4&&(!std::isfinite(p.size)||p.size<=0)))continue;
            const auto category=categorical?static_cast<std::size_t>(std::floor(p.x)):0;
            const auto label=!p.label.empty()?p.label:categorical?(category<categories.size()?categories[category]:std::to_string(category+1)):forge_chart_number(p.x,options.precision);
            ForgeChartMarkData mark;mark.kind=1;mark.x=xn(p.x);mark.y=yn(p.y);mark.radius=kind==4?p.size:std::max(options.point_size,0.);
            mark.color=p.color.alpha()==0?color:p.color;mark.series_index=static_cast<int>(si);mark.point_index=static_cast<int>(pi);mark.value=p.y;mark.label=name+" · "+label+": "+forge_chart_number(p.y,options.precision);
            if(kind==0) {
                const auto base=options.stacked?(p.y>=0?positive[category]:negative[category]):0.;const auto end=base+p.y;
                if(options.stacked){if(p.y>=0)positive[category]=end;else negative[category]=end;}
                const auto parts=options.stacked?1:visible.size();const auto part=options.stacked?0:order;
                mark.kind=0;mark.x=(category+0.12+0.76*part/parts)/count;mark.width=0.76/parts/count;mark.y=std::min(yn(base),yn(end));mark.height=std::abs(yn(base)-yn(end));
            } else if(kind==6){const auto point=forge_chart_polar_point(-90+360.*p.x/count,0.45*std::clamp((p.y-ymin)/(ymax-ymin),0.,1.));mark.x=point.first;mark.y=point.second;}
            projected[pi]=std::move(mark);
        }
        const auto original=data.points.size();
        if(kind==1||kind==6) {
            const auto pairs=kind==6?original:original==0?0:original-1;
            for(std::size_t i=0;i<pairs;++i) {
                const auto j=(i+1)%original;if(!projected[i]||!projected[j])continue;const auto& a=*projected[i];const auto& b=*projected[j];
                const auto& previous=i>0&&projected[i-1]?*projected[i-1]:a;const auto& next=j+1<original&&projected[j+1]?*projected[j+1]:b;
                scene.lines.push_back(forge_chart_line({a.x,a.y},{b.x,b.y},{previous.x,previous.y},{next.x,next.y},options.smooth&&kind==1,color));
                if(options.fill_area||kind==6) {
                    ForgeChartMarkData area;area.kind=3;area.x=a.x;area.y=a.y;area.x1=b.x;area.y1=b.y;area.x2=kind==6?0.5:b.x;area.y2=kind==6?0.5:std::clamp(scene.zero_y,0.,1.);
                    area.x3=kind==6?0.5:a.x;area.y3=area.y2;area.color=color;scene.areas.push_back(area);
                }
            }
        }
        for(const auto& mark:projected)if(mark)scene.marks.push_back(*mark);
    }
    if(scene.marks.empty())scene.status=1;return scene;
}
int forge_chart_hit_test(const std::vector<ForgeChartMarkData>& marks,double x,double y,double width,double height,double inner_radius) {
    if(!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(width)||!std::isfinite(height)||width<=0||height<=0)return -1;
    for(std::size_t i=marks.size();i>0;--i) {
        const auto& m=marks[i-1];bool hit=false;
        if(m.kind==0)hit=x>=m.x&&x<=m.x+m.width&&y>=m.y&&y<=m.y+m.height;
        else if(m.kind==1)hit=std::hypot((x-m.x)*width,(y-m.y)*height)<=std::max(m.radius,4.)+4;
        else if(m.kind==2){const auto distance=std::hypot(x-0.5,y-0.5);auto angle=std::fmod(std::atan2(y-0.5,x-0.5)*180/forge_chart_pi-m.start_angle,360.);if(angle<0)angle+=360;hit=distance>=inner_radius&&distance<=m.radius&&angle<=m.sweep_angle+1e-7;}
        if(hit)return static_cast<int>(i-1);
    }
    return -1;
}
