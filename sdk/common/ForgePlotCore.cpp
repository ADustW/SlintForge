// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#include "ForgePlotCore.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <vector>
#include <charconv>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <wincodec.h>
#include <wrl/client.h>
#endif

static constexpr double forge_plot_pi = 3.14159265358979323846;
static std::string forge_plot_string(const char* s) { return s ? s : ""; }
static std::string forge_plot_number(double v, int precision = 3) {
    if (!std::isfinite(v)) return "0";
    if (std::abs(v) < 0.5 * std::pow(10., -precision)) v = 0;
    // 坐标序列使用无 locale/流分配的固定精度格式，保留原有小数位和零值规范。
    char buffer[512];const auto converted=std::to_chars(buffer,buffer+sizeof(buffer),v,std::chars_format::fixed,precision);
    if(converted.ec==std::errc{})return std::string(buffer,converted.ptr);
    std::ostringstream s; s.imbue(std::locale::classic());
    s << std::fixed << std::setprecision(precision) << v; return s.str();
}
static std::string forge_plot_xml(const std::string& text) {
    std::string out; for (char c : text) {
        if (c == '&') out += "&amp;"; else if (c == '<') out += "&lt;";
        else if (c == '>') out += "&gt;"; else if (c == '"') out += "&quot;";
        else if (static_cast<unsigned char>(c) >= 32 || c == '\n') out += c;
    } return out;
}
static std::string forge_plot_color(uint32_t c) {
    char s[8]; std::snprintf(s, sizeof(s), "#%06x", c & 0xffffffu); return s;
}
static uint32_t forge_plot_palette(size_t i) {
    static const uint32_t colors[]{0xff36a2eb,0xffff6384,0xff4bc0c0,0xffffcd56,0xff9966ff,0xffff9f40,0xff788694};
    return colors[i % 7];
}
// SVG 使用显式回退链；系统控件的字体回退不会自动传递到离屏图像。
static bool forge_plot_has_cjk(const std::string& text) {
    for(size_t i=0;i<text.size();){
        unsigned code=static_cast<unsigned char>(text[i++]);int remaining=0;
        if(code>=0xf0){code&=7;remaining=3;}else if(code>=0xe0){code&=15;remaining=2;}else if(code>=0xc0){code&=31;remaining=1;}
        while(remaining--&&i<text.size())code=(code<<6)|(static_cast<unsigned char>(text[i++])&63);
        if((code>=0x2e80&&code<=0x9fff)||(code>=0xf900&&code<=0xfaff)||(code>=0x20000&&code<=0x3134f))return true;
    }
    return false;
}
static std::string forge_plot_font_families(std::string family) {
    if(family.empty())family="Segoe UI Variable";
    return family+", Segoe UI, Microsoft YaHei UI, Microsoft YaHei, Noto Sans CJK SC, sans-serif";
}
static double forge_plot_transform(double v, int scale) {
    if (scale == 0) return v;
    return v > 0 ? std::log10(v) : v < 0 ? -std::log10(-v) : std::numeric_limits<double>::quiet_NaN();
}
static double forge_plot_inverse(double v, int scale, double domain) {
    return scale == 0 ? v : domain > 0 ? std::pow(10., v) : -std::pow(10., -v);
}

struct ForgePlotCoreAxisState {
    ForgePlotAxisCore spec{};
    std::string label, format;
    std::vector<std::pair<double,std::string>> ticks;
    double minimum=0, maximum=1;
    double domain_minimum=0, domain_maximum=1;
};
struct ForgePlotCoreBounds {
    std::array<double,3> minimum{INFINITY,INFINITY,INFINITY},maximum{-INFINITY,-INFINITY,-INFINITY};
    void include(double value){if(!std::isfinite(value))return;minimum[0]=std::min(minimum[0],value);maximum[0]=std::max(maximum[0],value);if(value==0)return;const size_t domain=value>0?1:2;minimum[domain]=std::min(minimum[domain],value);maximum[domain]=std::max(maximum[domain],value);}
};
struct ForgePlotCoreSampleBlock {size_t minimum=0,maximum=0;bool valid=true;};
static constexpr std::array<size_t,6> forge_plot_block_spans{64,1024,16384,262144,4194304,67108864};
struct ForgePlotCoreSeriesState {
    ForgePlotSeriesCore spec{};
    std::string name;
    std::vector<ForgePlotPointCore> points;
    std::vector<size_t> order;
    std::unordered_map<size_t,std::string> labels;
    std::unordered_map<size_t,std::vector<double>> outliers;
    std::unordered_map<size_t,ForgePlotPointStyleCore> styles;
    bool monotone=true, binary_searchable=true;
    ForgePlotCoreBounds x_bounds,y_bounds;
    size_t latest_valid=std::numeric_limits<size_t>::max();
    std::array<std::vector<ForgePlotCoreSampleBlock>,6> sample_blocks;
    bool simple_samples=true;
};
struct ForgePlotCoreMapState { ForgePlotMapCore spec{}; std::string name; std::vector<double> values; };
struct ForgePlotCoreItemState { ForgePlotItemCore spec{}; std::string text,image; int image_width=0,image_height=0;std::vector<uint8_t> pixels; };
struct ForgePlotCorePanelState { ForgePlotPanelCore spec{}; std::string title; std::vector<std::string> categories; ForgePlotRectCore rect{}; bool polar=false; int system=0; };
struct ForgePlotCoreDraw {
    int panel=-1, layer=0;
    bool clip=false,nearest=false;
    uint32_t stroke=0, fill=0;
    double width=1, x=0,y=0,size=12,rotation=0;
    std::string path,text,image;
    double image_width=0,image_height=0;
    int raster_width=0,raster_height=0;
    std::vector<uint8_t> raster;
};
struct ForgePlotCoreHitState { ForgePlotHitCore hit{}; double x2=0,y2=0,radius=4; bool segment=false,rect=false,clip=true,sector=false; double cx=0,cy=0,inner=0,outer=0,start=0,sweep=0; };
struct ForgePlotCoreState {
    ForgePlotOptionsCore options{};
    std::string title,font,error,svg;
    int status=0;
    uint64_t input_points=0,rendered_points=0;
    std::vector<ForgePlotCoreAxisState> axes;
    std::vector<ForgePlotCorePanelState> panels;
    std::vector<ForgePlotCoreSeriesState> series;
    std::vector<ForgePlotCoreMapState> maps;
    std::vector<ForgePlotCoreItemState> items;
    std::vector<ForgePlotSelectionCore> selection;
    std::vector<ForgePlotCoreDraw> draw;
    std::vector<ForgePlotCoreHitState> hits;
    std::unordered_map<int64_t,std::vector<size_t>> hit_cells;
    ForgePlotRectCore legend_area{-1,0,0,0,0},measurement_area{-1,0,0,0,0};
};

static ForgePlotCoreAxisState* forge_plot_axis_find(ForgePlotCoreState& p,int id) {
    for(auto& a:p.axes) if(a.spec.id==id) return &a; return nullptr;
}
static const ForgePlotCoreAxisState* forge_plot_axis_find(const ForgePlotCoreState& p,int id) {
    for(const auto& a:p.axes) if(a.spec.id==id) return &a; return nullptr;
}
static const ForgePlotCorePanelState* forge_plot_panel_find(const ForgePlotCoreState& p,int id) {
    for(const auto& a:p.panels) if(a.spec.id==id) return &a; return nullptr;
}
static bool forge_plot_domain(const ForgePlotCoreAxisState& a,double v) {
    return std::isfinite(v) && (a.spec.scale==0 || (a.minimum>0 ? v>0 : v<0));
}
static double forge_plot_axis_normal(const ForgePlotCoreAxisState& a,double v) {
    if(!forge_plot_domain(a,v)) return std::numeric_limits<double>::quiet_NaN();
    const double lo=forge_plot_transform(a.minimum,a.spec.scale),hi=forge_plot_transform(a.maximum,a.spec.scale);
    const double t=(forge_plot_transform(v,a.spec.scale)-lo)/(hi-lo);
    return a.spec.reversed ? 1-t : t;
}
static double forge_plot_axis_pixel(const ForgePlotCoreState& p,const ForgePlotCoreAxisState& a,double v) {
    const auto* panel=forge_plot_panel_find(p,a.spec.panel); if(!panel) return std::numeric_limits<double>::quiet_NaN();
    const auto& r=panel->rect; const double t=forge_plot_axis_normal(a,v);
    return a.spec.side<2 ? r.x+t*r.width : r.y+(1-t)*r.height;
}
static double forge_plot_axis_value(const ForgePlotCoreState& p,const ForgePlotCoreAxisState& a,double pixel) {
    const auto* panel=forge_plot_panel_find(p,a.spec.panel); if(!panel) return std::numeric_limits<double>::quiet_NaN();
    const auto& r=panel->rect;
    double t=a.spec.side<2 ? (pixel-r.x)/r.width : 1-(pixel-r.y)/r.height;
    if(a.spec.reversed) t=1-t;
    const double lo=forge_plot_transform(a.minimum,a.spec.scale),hi=forge_plot_transform(a.maximum,a.spec.scale);
    return forge_plot_inverse(lo+t*(hi-lo),a.spec.scale,a.minimum);
}
static std::pair<double,double> forge_plot_polar_point(const ForgePlotCorePanelState& panel,const ForgePlotCoreAxisState& ax,const ForgePlotCoreAxisState& ay,double angle,double radius) {
    const auto& r=panel.rect;const double a=forge_plot_axis_normal(ax,angle)*2*forge_plot_pi;
    const double distance=forge_plot_axis_normal(ay,radius)*std::min(r.width,r.height)*.5;
    return {r.x+r.width*.5+distance*std::cos(a),r.y+r.height*.5-distance*std::sin(a)};
}
static bool forge_plot_inside(const ForgePlotCorePanelState& panel,double x,double y) {
    const auto& r=panel.rect;if(x<r.x||x>r.x+r.width||y<r.y||y>r.y+r.height)return false;
    return !panel.polar||std::hypot(x-r.x-r.width*.5,y-r.y-r.height*.5)<=std::min(r.width,r.height)*.5+.1;
}
static const ForgePlotPointStyleCore* forge_plot_point_style(const ForgePlotCoreSeriesState& s,size_t index) {
    const auto found=s.styles.find(index);return found==s.styles.end()?nullptr:&found->second;
}
static bool forge_plot_point_hidden(const ForgePlotCoreSeriesState& s,size_t index) {
    const auto* style=forge_plot_point_style(s,index);return style&&style->hidden;
}
static void forge_plot_copy_styles(ForgePlotCoreSeriesState& s) {
    s.styles.clear();if(s.spec.point_styles)for(size_t i=0;i<s.spec.point_style_count;++i){const auto& v=s.spec.point_styles[i];if(v.index>=0)s.styles[static_cast<size_t>(v.index)]=v;}
    s.spec.point_styles=nullptr;
}
static bool forge_plot_point_valid(const ForgePlotCoreSeriesState& s,const ForgePlotPointCore& pt) {
    if(!std::isfinite(pt.x)||!std::isfinite(pt.y)) return false;
    if(s.spec.kind==3) return std::isfinite(pt.low)&&std::isfinite(pt.high)&&std::isfinite(pt.q1)&&std::isfinite(pt.q3)&&pt.low<=pt.q1&&pt.q1<=pt.y&&pt.y<=pt.q3&&pt.q3<=pt.high;
    if(s.spec.kind==4||s.spec.kind==5) return std::isfinite(pt.low)&&std::isfinite(pt.high)&&std::isfinite(pt.open)&&std::isfinite(pt.close)&&pt.low<=std::min(pt.open,pt.close)&&std::max(pt.open,pt.close)<=pt.high;
    return true;
}
static void forge_plot_series_order(ForgePlotCoreSeriesState& s) {
    // 类型切换后，参数曲线恢复输入顺序；折线重新建立各断点段的稳定排序。
    s.order.clear();
    if(s.spec.kind!=0||s.monotone)return;
    s.order.resize(s.points.size());std::iota(s.order.begin(),s.order.end(),0);
    size_t start=0;
    while(start<s.points.size()){
        if(!forge_plot_point_valid(s,s.points[start])){++start;continue;}
        size_t end=start+1;while(end<s.points.size()&&forge_plot_point_valid(s,s.points[end]))++end;
        std::stable_sort(s.order.begin()+start,s.order.begin()+end,[&](size_t a,size_t b){return s.points[a].x<s.points[b].x;});
        start=end;
    }
}
static void forge_plot_series_bounds(ForgePlotCoreSeriesState& s,size_t index) {
    const auto& pt=s.points[index];const bool valid=forge_plot_point_valid(s,pt);
    if(pt.error_x_minus!=0||pt.error_x_plus!=0||pt.error_y_minus!=0||pt.error_y_plus!=0)s.simple_samples=false;
    if(s.spec.kind==0)for(size_t level=0;level<forge_plot_block_spans.size();++level){const size_t block=index/forge_plot_block_spans[level];auto& blocks=s.sample_blocks[level];
        if(blocks.size()<=block)blocks.push_back({index,index,valid});else{auto& summary=blocks[block];summary.valid=summary.valid&&valid;if(valid){if(pt.y<s.points[summary.minimum].y)summary.minimum=index;if(pt.y>s.points[summary.maximum].y)summary.maximum=index;}}
    }
    if(!valid)return;s.latest_valid=index;
    s.x_bounds.include(pt.x);s.x_bounds.include(pt.x-std::max(0.,pt.error_x_minus));s.x_bounds.include(pt.x+std::max(0.,pt.error_x_plus));
    if(s.spec.kind>=2&&s.spec.kind<=5){s.x_bounds.include(pt.x-s.spec.bar_width*.5);s.x_bounds.include(pt.x+s.spec.bar_width*.5);}
    if(s.spec.kind<4||s.spec.kind>=6){s.y_bounds.include(pt.y);s.y_bounds.include(pt.y-std::max(0.,pt.error_y_minus));s.y_bounds.include(pt.y+std::max(0.,pt.error_y_plus));}
    if(s.spec.kind>=3&&s.spec.kind<=5){s.y_bounds.include(pt.low);s.y_bounds.include(pt.high);}
    if(s.spec.kind==2)s.y_bounds.include(s.spec.baseline);
    const auto outliers=s.outliers.find(index);if(outliers!=s.outliers.end())for(double v:outliers->second)s.y_bounds.include(v);
}
static void forge_plot_path_point(std::string& path,char command,double x,double y) {
    path+=command;path+=forge_plot_number(std::clamp(x,-1e8,1e8));path+=' ';path+=forge_plot_number(std::clamp(y,-1e8,1e8));path+=' ';
}
static void forge_plot_line(std::string& path,double x,double y,double x2,double y2) {
    if(!std::isfinite(x+y+x2+y2)) return;
    forge_plot_path_point(path,'M',x,y);forge_plot_path_point(path,'L',x2,y2);
}
static void forge_plot_rectangle(std::string& path,double x,double y,double w,double h) {
    forge_plot_path_point(path,'M',x,y);forge_plot_path_point(path,'L',x+w,y);
    forge_plot_path_point(path,'L',x+w,y+h);forge_plot_path_point(path,'L',x,y+h);path+="Z ";
}
static void forge_plot_ellipse(std::string& path,double x,double y,double rx,double ry) {
    constexpr double k=.5522847498307936;
    forge_plot_path_point(path,'M',x+rx,y);
    for(int i=0;i<4;++i) {
        const double a=i*forge_plot_pi/2,b=(i+1)*forge_plot_pi/2;
        path+='C';path+=forge_plot_number(x+rx*(std::cos(a)-k*std::sin(a)))+' '+forge_plot_number(y+ry*(std::sin(a)+k*std::cos(a)))+' ';
        path+=forge_plot_number(x+rx*(std::cos(b)+k*std::sin(b)))+' '+forge_plot_number(y+ry*(std::sin(b)-k*std::cos(b)))+' ';
        path+=forge_plot_number(x+rx*std::cos(b))+' '+forge_plot_number(y+ry*std::sin(b))+' ';
    } path+="Z ";
}
static void forge_plot_add_path(ForgePlotCoreState& p,std::string path,uint32_t stroke,uint32_t fill,double width,int panel,int layer=0,bool clip=true) {
    if(path.empty())return;
    ForgePlotCoreDraw d;d.path=std::move(path);d.stroke=stroke;d.fill=fill;d.width=std::clamp(width,.1,64.);d.panel=panel;d.layer=layer;d.clip=clip;p.draw.push_back(std::move(d));
}
static double forge_plot_text_width(const std::string& text,double size) {
    double width=0;for(size_t i=0;i<text.size();){const unsigned char c=static_cast<unsigned char>(text[i]);if(c<128){width+=size*.55;++i;}else{width+=size*(c>=0xe0?1:.65);i+=c>=0xf0?4:c>=0xe0?3:2;}}return width;
}
#ifdef _WIN32
static std::wstring forge_plot_wide(const std::string& text);
#endif
class ForgePlotLegendMetrics {
#ifdef _WIN32
    HDC dc_=nullptr;HFONT font_=nullptr;HGDIOBJ previous_=nullptr;
#endif
public:
    ForgePlotLegendMetrics(const ForgePlotLegendMetrics&)=delete;
    ForgePlotLegendMetrics& operator=(const ForgePlotLegendMetrics&)=delete;
    explicit ForgePlotLegendMetrics(std::string family) {
#ifdef _WIN32
        family=family.substr(0,family.find(','));
        dc_=CreateCompatibleDC(nullptr);if(dc_){font_=CreateFontW(-128,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_TT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH,forge_plot_wide(family).c_str());if(font_)previous_=SelectObject(dc_,font_);}
#else
        (void)family;
#endif
    }
    ~ForgePlotLegendMetrics() {
#ifdef _WIN32
        if(dc_&&previous_)SelectObject(dc_,previous_);if(font_)DeleteObject(font_);if(dc_)DeleteDC(dc_);
#endif
    }
    double width(const std::string& text,double size) const {
#ifdef _WIN32
        // 按实际字体度量，避免透明图例右侧因估算和安全余量而留下大空隙。
        const auto wide=forge_plot_wide(text);SIZE extent{};if(dc_&&font_&&GetTextExtentPoint32W(dc_,wide.c_str(),static_cast<int>(wide.size()),&extent))return extent.cx*size/128.;
#endif
        return forge_plot_text_width(text,size);
    }
    bool supports(const std::string& text) const {
#ifdef _WIN32
        const auto wide=forge_plot_wide(text);std::vector<WORD> glyphs(wide.size());
        return dc_&&font_&&GetGlyphIndicesW(dc_,wide.data(),static_cast<int>(wide.size()),glyphs.data(),GGI_MARK_NONEXISTING_GLYPHS)!=GDI_ERROR&&std::none_of(glyphs.begin(),glyphs.end(),[](WORD glyph){return glyph==0xffff;});
#else
        (void)text;return true;
#endif
    }
};
struct ForgePlotCoreLegendEntry {size_t index=0;double x=0,y=0,width=0;std::string text;int point=-1;};
struct ForgePlotCoreLegendLayout {
    std::vector<ForgePlotCoreLegendEntry> entries;
    ForgePlotRectCore rect{};
    double x=0,y=0,width=0,height=0;
};
static ForgePlotCoreLegendLayout forge_plot_legend_layout(const ForgePlotCoreState& p) {
    ForgePlotCoreLegendLayout layout;if(!p.options.legend||p.series.empty())return layout;
    const int position=std::clamp(p.options.legend_position,0,7);const bool vertical=position>=2;
    // 区域内按绘图区边界排列；区域外尺寸用于预留独立布局带。
    constexpr double gap=6,padding=4;
    const double title_height=p.title.empty()?0:36;
    layout.rect=p.panels.empty()||p.panels[0].rect.width<1?ForgePlotRectCore{0,84,title_height+24,p.options.width-108,p.options.height-title_height-110}:p.panels[0].rect;
    const auto& rect=layout.rect;
    const ForgePlotLegendMetrics metrics(p.font);
    const double size=std::clamp(p.options.font_size,6.,96.),row_height=std::max(p.options.legend_style==2?20.:24.,size*1.6);
    const bool outside=p.options.legend_placement==1;
    const double usable_width=std::max(1.,outside&&(position==2||position==3)?std::min(260.,p.options.width*.28)-2*padding:rect.width-2*(gap+padding));
    const double available=vertical?std::min(260.,usable_width):usable_width;
    double x=padding,y=0;
    for(size_t i=0;i<p.series.size();++i){
        const auto& series=p.series[i];const bool slices=series.spec.kind==8||series.spec.kind==9;const auto* axis=forge_plot_axis_find(p,series.spec.x_axis);const auto* panel=axis?forge_plot_panel_find(p,axis->spec.panel):nullptr;
        for(size_t item=0;item<(slices?series.points.size():1);++item){const int point=slices?static_cast<int>(item):-1;
        auto text=slices?(panel&&item<panel->categories.size()?panel->categories[item]:series.labels.contains(item)?series.labels.at(item):std::to_string(item+1)):series.name;for(auto& c:text)if(c=='\n'||c=='\r'||c=='\t')c=' ';
        std::string suffix;if(p.options.legend_latest_value&&series.latest_valid!=std::numeric_limits<size_t>::max()){const auto& pt=series.points[series.latest_valid];suffix="  "+forge_plot_number(series.spec.kind==4||series.spec.kind==5?pt.close:pt.y,std::clamp(p.options.legend_precision,0,10));}
        const double text_limit=std::max(0.,available-28-metrics.width(suffix,size));bool shortened=false;
        while(!text.empty()&&metrics.width(text+(shortened?"…":""),size)>text_limit){size_t finish=text.size()-1;while(finish&&((static_cast<unsigned char>(text[finish])&0xc0)==0x80))--finish;text.resize(finish);shortened=true;}
        if(shortened)text+="…";
        text+=suffix;
        const double width=std::min(available,metrics.width(text,size)+28);
        if(!vertical&&x>padding&&x+width>available+padding){x=padding;y+=row_height;}
        layout.entries.push_back({i,x,y+size+5,width,std::move(text),point});layout.width=std::max(layout.width,x+width+padding);layout.height=std::max(layout.height,y+row_height+2*padding);
        if(vertical)y+=row_height;else x+=width+8;
        }
    }
    if(!outside)layout.height=std::min(layout.height,std::max(1.,rect.height-2*gap));
    layout.x=position==0||position==1?rect.x+(rect.width-layout.width)*.5:position==3||position==4||position==5?rect.x+rect.width-layout.width-gap:rect.x+gap;
    layout.y=position==2||position==3?rect.y+(rect.height-layout.height)*.5:position==0||position==5||position==7?rect.y+rect.height-layout.height-gap:rect.y+gap;
    return layout;
}
static int forge_plot_annotation_side(int position) {return position<4?position:position==4||position==6?1:0;}
static ForgePlotRectCore forge_plot_annotation_anchor(const ForgePlotRectCore& rect,double width,double height,int position) {
    const double gap=6;
    return {rect.id,position==0||position==1?rect.x+(rect.width-width)*.5:position==3||position==4||position==5?rect.x+rect.width-width-gap:rect.x+gap,
        position==2||position==3?rect.y+(rect.height-height)*.5:position==0||position==5||position==7?rect.y+rect.height-height-gap:rect.y+gap,width,height};
}
static bool forge_plot_annotations_overlap(const ForgePlotRectCore& a,const ForgePlotRectCore& b) {
    return a.width>0&&b.width>0&&a.x<b.x+b.width+6&&a.x+a.width+6>b.x&&a.y<b.y+b.height+6&&a.y+a.height+6>b.y;
}
static bool forge_plot_layout_annotations(ForgePlotCoreState& p) {
    std::vector<std::array<double,4>> margins;
    for(const auto& panel:p.panels){std::array<double,4> margin{22,panel.title.empty()?18.:38.,22,22};
        for(const auto& axis:p.axes)if(!panel.system&&axis.spec.panel==panel.spec.id&&axis.spec.visible)margin[axis.spec.side]+=42+(axis.label.empty()?0:20)+std::max(0.,axis.spec.offset);
        for(const auto& map:p.maps)if(map.spec.panel==panel.spec.id&&!map.spec.hidden&&map.spec.color_scale)margin[3]+=76;
        margins.push_back(margin);
    }
    const auto* measurement_panel=&p.panels.front();
    const ForgePlotCoreSeriesState* measured_series=nullptr;
    for(const auto& series:p.series)if(series.spec.id==p.options.measurement_series){measured_series=&series;const auto* axis=forge_plot_axis_find(p,series.spec.x_axis);if(axis)measurement_panel=forge_plot_panel_find(p,axis->spec.panel);break;}
    const double measurement_font=std::clamp(p.options.font_size,6.,96.);const ForgePlotLegendMetrics measurement_metrics(p.font);
    double content_width=100;
    if(measured_series){const auto& s=*measured_series;const auto number=[](double value){return forge_plot_number(std::isfinite(value)?value:0,3);};
        const auto widest=[&](double minimum,double maximum){const auto a=number(minimum),b=number(maximum);return measurement_metrics.width(a,measurement_font)>measurement_metrics.width(b,measurement_font)?a:b;};
        const auto x=widest(s.x_bounds.minimum[0],s.x_bounds.maximum[0]),y=widest(s.y_bounds.minimum[0],s.y_bounds.maximum[0]);
        content_width=std::max({measurement_metrics.width(s.name,measurement_font),measurement_metrics.width("A: X="+x+"  Y="+y,measurement_font),measurement_metrics.width("ΔX="+number(p.options.x_window_mode>0?p.options.x_window_size:s.x_bounds.maximum[0]-s.x_bounds.minimum[0])+"  ΔY="+number(s.y_bounds.maximum[0]-s.y_bounds.minimum[0]),measurement_font)});
    }
    const double automatic_width=std::clamp(std::ceil(content_width+16),100.,260.),automatic_height=std::ceil(4*measurement_font*1.2+18);
    const int measurement_id=measurement_panel->spec.id;
    const int legend_position=std::clamp(p.options.legend_position,0,7),measurement_position=std::clamp(p.options.measurement_position,0,7);
    const bool legend_outside=p.options.legend_placement==1,measurement_outside=p.options.measurement_placement==1;
    std::array<double,4> docks{};ForgePlotCoreLegendLayout legend;double measurement_width=0,measurement_height=0;
    const auto calculate=[&](){
        const double width=p.options.width-docks[2]-docks[3],height=p.options.height-(p.title.empty()?0:36)-docks[0]-docks[1];
        for(size_t i=0;i<p.panels.size();++i){auto& panel=p.panels[i];const auto& m=margins[i];panel.rect={panel.spec.id,docks[2]+panel.spec.x*width+m[2],(p.title.empty()?0.:36.)+docks[1]+panel.spec.y*height+m[1],panel.spec.width*width-m[2]-m[3],panel.spec.height*height-m[1]-m[0]};
            if(panel.rect.width<30||panel.rect.height<30){p.error="绘图区空间不足，需增大组件尺寸或减少外部信息框";return false;}}
        return true;
    };
    // 图例换行会影响区域外高度，收敛布局带后统一计算绘图区与信息框。
    for(int pass=0;pass<8;++pass){if(!calculate())return false;legend=forge_plot_legend_layout(p);const auto& rect=forge_plot_panel_find(p,measurement_id)->rect;
        if(p.options.measurement_enabled&&measurement_panel->system==0){
            const double preferred_width=p.options.measurement_width>0?p.options.measurement_width:automatic_width,preferred_height=p.options.measurement_height>0?p.options.measurement_height:automatic_height;
            measurement_width=std::min(preferred_width,measurement_outside&&(measurement_position==2||measurement_position==3)?p.options.width*.28:std::max(1.,rect.width-12));
            measurement_height=std::min(preferred_height,measurement_outside?p.options.height*.35:std::max(1.,rect.height-12));
        }
        std::array<double,4> next{};
        if(legend_outside&&!legend.entries.empty()){const int side=forge_plot_annotation_side(legend_position);next[side]+=6+(side<2?legend.height:legend.width);}
        if(measurement_outside&&p.options.measurement_enabled){const int side=forge_plot_annotation_side(measurement_position);next[side]+=6+(side<2?measurement_height:measurement_width);}
        double difference=0;for(int side=0;side<4;++side)difference=std::max(difference,std::abs(next[side]-docks[side]));if(difference<.01)break;docks=next;
    }
    if(!calculate())return false;
    ForgePlotRectCore axes_bounds{-1,std::numeric_limits<double>::infinity(),std::numeric_limits<double>::infinity(),0,0};double right=0,bottom=0;
    for(size_t i=0;i<p.panels.size();++i){const auto& rect=p.panels[i].rect;const auto& m=margins[i];axes_bounds.x=std::min(axes_bounds.x,rect.x-(m[2]-22));axes_bounds.y=std::min(axes_bounds.y,rect.y-(m[1]-18));right=std::max(right,rect.x+rect.width+m[3]-22);bottom=std::max(bottom,rect.y+rect.height+m[0]-22);}
    axes_bounds.width=right-axes_bounds.x;axes_bounds.height=bottom-axes_bounds.y;
    std::array<double,4> offsets{};
    const auto outside_box=[&](const ForgePlotRectCore& rect,double width,double height,int position){auto box=forge_plot_annotation_anchor(rect,width,height,position);const int side=forge_plot_annotation_side(position);
        if(side==0)box.y=axes_bounds.y+axes_bounds.height+6+offsets[side];else if(side==1)box.y=axes_bounds.y-6-height-offsets[side];else if(side==2)box.x=axes_bounds.x-6-width-offsets[side];else box.x=axes_bounds.x+axes_bounds.width+6+offsets[side];
        offsets[side]+=6+(side<2?height:width);return box;
    };
    p.legend_area={-1,0,0,0,0};p.measurement_area={-1,0,0,0,0};
    if(!legend.entries.empty()){const auto& rect=p.panels.front().rect;p.legend_area=legend_outside?outside_box(rect,legend.width,legend.height,legend_position):forge_plot_annotation_anchor(rect,legend.width,legend.height,legend_position);}
    if(p.options.measurement_enabled&&measurement_panel->system==0){const auto& rect=forge_plot_panel_find(p,measurement_id)->rect;p.measurement_area=measurement_outside?outside_box(rect,measurement_width,measurement_height,measurement_position):forge_plot_annotation_anchor(rect,measurement_width,measurement_height,measurement_position);
        // 区域内同位置的两个信息框向内错开，保持各自可读。
        if(!legend_outside&&!measurement_outside&&p.legend_area.id==rect.id&&forge_plot_annotations_overlap(p.legend_area,p.measurement_area)){
            auto candidate=p.measurement_area;candidate.y=measurement_position==0||measurement_position==5||measurement_position==7?p.legend_area.y-candidate.height-6:p.legend_area.y+p.legend_area.height+6;
            if(candidate.y>=rect.y+6&&candidate.y+candidate.height<=rect.y+rect.height-6)p.measurement_area=candidate;
        }
    }
    return true;
}
static void forge_plot_add_text(ForgePlotCoreState& p,double x,double y,std::string text,uint32_t color,double size,int layer=0,int panel=-1,bool clip=false,double rotation=0) {
    size=std::clamp(size,6.,96.);size_t start=0;
    do {
        const size_t end=text.find('\n',start); ForgePlotCoreDraw d;d.x=x;d.y=y;d.size=size;d.fill=color;d.layer=layer;d.panel=panel;d.clip=clip;d.rotation=rotation;d.text=text.substr(start,end-start);p.draw.push_back(std::move(d));
        if(end==std::string::npos)break;start=end+1;y+=size*1.2;
    } while(start<=text.size());
}
static std::string forge_plot_tick_label(double v,const ForgePlotCoreAxisState& a) {
    if(a.spec.ticker==2) {
        v+=a.spec.time_origin;
        if(!std::isfinite(v)||v < -62135596800. ||v > 253402300799.)return "";
        const std::time_t seconds=static_cast<std::time_t>(v);std::tm tm{};
#ifdef _WIN32
        if(gmtime_s(&tm,&seconds)!=0)return "";
#else
        if(!gmtime_r(&seconds,&tm))return "";
#endif
        char buf[128]{};std::strftime(buf,sizeof(buf),a.format.empty()?"%H:%M:%S":a.format.c_str(),&tm);return buf;
    }
    if(a.spec.ticker==3) {
        const double value=std::abs(v);if(value>9e15)return forge_plot_number(v,0);
        const auto seconds=static_cast<uint64_t>(value);char buf[64];
        std::snprintf(buf,sizeof(buf),"%s%llu:%02llu:%02llu",v<0?"-":"",static_cast<unsigned long long>(seconds/3600),static_cast<unsigned long long>((seconds/60)%60),static_cast<unsigned long long>(seconds%60));return buf;
    }
    if(a.spec.ticker==4) {
        const double n=v/forge_plot_pi;if(std::abs(n)<1e-10)return "0";
        if(std::abs(n-1)<1e-10)return "π";if(std::abs(n+1)<1e-10)return "-π";
        return forge_plot_number(n,std::clamp(a.spec.precision,0,6))+"π";
    }
    if(std::abs(v)>=1e7||(v!=0&&std::abs(v)<.0001)) {
        std::ostringstream s;s.imbue(std::locale::classic());s<<std::scientific<<std::setprecision(std::clamp(a.spec.precision,0,8))<<v;return s.str();
    }
    return forge_plot_number(v,std::clamp(a.spec.precision,0,10));
}
static std::vector<std::pair<double,std::string>> forge_plot_ticks(const ForgePlotCoreAxisState& a,double pixels) {
    if(a.spec.ticker==1||!a.ticks.empty())return a.ticks;
    std::vector<std::pair<double,std::string>> result;
    const int count=std::clamp(static_cast<int>(pixels/95),2,12);
    double lo=forge_plot_transform(a.minimum,a.spec.scale),hi=forge_plot_transform(a.maximum,a.spec.scale),step=(hi-lo)/count;
    if(a.spec.tick_step>0&&std::isfinite(a.spec.tick_step))step=a.spec.tick_step;
    else {
        const double magnitude=std::pow(10.,std::floor(std::log10(step)));const double n=step/magnitude;
        step=(n<=1?1:n<=2?2:n<=2.5?2.5:n<=5?5:10)*magnitude;
    }
    if(!(step>0)||!std::isfinite(step))return result;
    const double start=std::ceil(lo/step)*step;
    for(int i=0;i<200;++i) {const double t=start+i*step;if(t>hi+step*1e-8)break;const double v=forge_plot_inverse(t,a.spec.scale,a.minimum);result.emplace_back(v,forge_plot_tick_label(v,a));}
    return result;
}

static bool forge_plot_layout(ForgePlotCoreState& p) {
    if(!std::isfinite(p.options.width+p.options.height+p.options.font_size)||p.options.width<120||p.options.height<100||p.options.width>16384||p.options.height>16384){p.error="绘图尺寸须在有效范围内";return false;}
    if(p.panels.empty()){ForgePlotCorePanelState v;v.spec.id=0;v.spec.width=1;v.spec.height=1;p.panels.push_back(v);}
    if(p.axes.empty()) {
        for(const auto& panel:p.panels) {ForgePlotCoreAxisState x,y;x.spec.id=panel.spec.id*2;x.spec.panel=panel.spec.id;x.spec.visible=1;x.spec.grid=1;x.spec.auto_range=1;x.spec.precision=2;y=x;y.spec.id=x.spec.id+1;y.spec.side=2;p.axes.push_back(x);p.axes.push_back(y);}
    }
    std::map<int,int> panel_ids,axis_ids;
    for(auto& panel:p.panels) {
        if(panel_ids[panel.spec.id]++||!std::isfinite(panel.spec.x+panel.spec.y+panel.spec.width+panel.spec.height)||panel.spec.x<0||panel.spec.y<0||panel.spec.width<=0||panel.spec.height<=0||panel.spec.x+panel.spec.width>1.000001||panel.spec.y+panel.spec.height>1.000001){p.error="绘图区 ID 或比例矩形无效";return false;}
        panel.polar=false;panel.system=-1;
        for(const auto& s:p.series){const auto* axis=forge_plot_axis_find(p,s.spec.x_axis);if(axis&&axis->spec.panel==panel.spec.id){
            const int system=s.spec.kind>=8?s.spec.kind:s.spec.kind==6?6:0;
            if(panel.system>=0&&panel.system!=system){p.error="不同坐标体系须使用不同绘图区";return false;}panel.system=system;
        }}
        if(panel.system<0)panel.system=0;panel.polar=panel.system==6;
        for(const auto& m:p.maps)if(!m.spec.hidden&&m.spec.panel==panel.spec.id&&panel.system){p.error="色图须使用笛卡尔绘图区";return false;}
        if(panel.system==8||panel.system==9){size_t count=0;for(const auto& s:p.series){const auto* axis=forge_plot_axis_find(p,s.spec.x_axis);if(axis&&axis->spec.panel==panel.spec.id)++count;}if(count>1){p.error="饼图和极区面积图的多个系列须使用独立绘图区";return false;}}
        double left=22,right=22,top=panel.title.empty()?18:38,bottom=22;
        for(const auto& a:p.axes)if(!panel.system&&a.spec.panel==panel.spec.id&&a.spec.visible){const double extent=42+(a.label.empty()?0:20)+std::max(0.,a.spec.offset);if(a.spec.side==0)bottom+=extent;else if(a.spec.side==1)top+=extent;else if(a.spec.side==2)left+=extent;else right+=extent;}
        for(const auto& m:p.maps)if(m.spec.panel==panel.spec.id&&!m.spec.hidden&&m.spec.color_scale)right+=76;
        const double title_height=p.title.empty()?0:36;
        const double usable=p.options.height-title_height;
        panel.rect={panel.spec.id,panel.spec.x*p.options.width+left,title_height+panel.spec.y*usable+top,panel.spec.width*p.options.width-left-right,panel.spec.height*usable-top-bottom};
        if(panel.rect.width<30||panel.rect.height<30){p.error="绘图区空间不足，需增大组件尺寸或减少坐标轴";return false;}
    }
    for(auto& a:p.axes){if(axis_ids[a.spec.id]++||!forge_plot_panel_find(p,a.spec.panel)||a.spec.side<0||a.spec.side>3||a.spec.scale<0||a.spec.scale>1||!std::isfinite(a.spec.offset)||!std::isfinite(a.spec.time_origin)){p.error="坐标轴 ID、绘图区、类型或日期时间基准无效";return false;}}
    return forge_plot_layout_annotations(p);
}

static bool forge_plot_ranges(ForgePlotCoreState& p) {
    for(auto& a:p.axes) {
        double lo=std::numeric_limits<double>::infinity(),hi=-lo;
        const bool horizontal=a.spec.side<2;
        const bool polar=forge_plot_panel_find(p,a.spec.panel)->polar;
        const auto include=[&](double v){if(std::isfinite(v)&&(a.spec.scale==0||(a.spec.minimum<0&&a.spec.maximum<0?v<0:v>0))){lo=std::min(lo,v);hi=std::max(hi,v);}};
        for(const auto& s:p.series)if(!s.spec.hidden&&(horizontal?s.spec.x_axis:s.spec.y_axis)==a.spec.id) {
            const auto& bounds=horizontal?s.x_bounds:s.y_bounds;const size_t domain=a.spec.scale?(a.spec.minimum<0&&a.spec.maximum<0?2:1):0;
            if(s.styles.empty()){include(bounds.minimum[domain]);include(bounds.maximum[domain]);}
            else for(size_t i=0;i<s.points.size();++i){const auto& pt=s.points[i];if(forge_plot_point_hidden(s,i)||!forge_plot_point_valid(s,pt))continue;
                if(horizontal){include(pt.x-std::max(0.,pt.error_x_minus));include(pt.x+std::max(0.,pt.error_x_plus));if(s.spec.kind>=2&&s.spec.kind<=5){include(pt.x-s.spec.bar_width*.5);include(pt.x+s.spec.bar_width*.5);}}
                else{include(pt.y-std::max(0.,pt.error_y_minus));include(pt.y+std::max(0.,pt.error_y_plus));if(s.spec.kind>=3&&s.spec.kind<=5){include(pt.low);include(pt.high);}if(s.spec.kind==2)include(s.spec.baseline);const auto outliers=s.outliers.find(i);if(outliers!=s.outliers.end())for(double v:outliers->second)include(v);}
            }
            if(!horizontal&&s.spec.kind==2&&s.spec.stack_group>=0){std::map<double,std::pair<double,double>> sums;for(const auto& other:p.series)if(!other.spec.hidden&&other.spec.kind==2&&other.spec.stack_group==s.spec.stack_group&&other.spec.x_axis==s.spec.x_axis&&other.spec.y_axis==s.spec.y_axis)for(size_t i=0;i<other.points.size();++i)if(!forge_plot_point_hidden(other,i)&&forge_plot_point_valid(other,other.points[i])){const auto& pt=other.points[i];auto& sum=sums[pt.x];if(pt.y>=0)sum.first+=pt.y;else sum.second+=pt.y;}for(const auto& sum:sums){include(s.spec.baseline+sum.second.first);include(s.spec.baseline+sum.second.second);}}
        }
        for(const auto& m:p.maps)if(!m.spec.hidden&&(horizontal?m.spec.x_axis:m.spec.y_axis)==a.spec.id){include(horizontal?m.spec.x_min:m.spec.y_min);include(horizontal?m.spec.x_max:m.spec.y_max);}
        if(polar&&a.spec.auto_range&&horizontal){lo=0;hi=360;}
        if(polar&&a.spec.auto_range&&!horizontal&&a.spec.scale==0){lo=0;hi=std::max(hi,1.);}
        a.domain_minimum=std::isfinite(lo)?lo:(a.spec.scale?1:0);a.domain_maximum=std::isfinite(hi)?hi:(a.spec.scale?10:1);
        if(a.spec.auto_range) {
            if(!std::isfinite(lo)){lo=a.spec.scale?1:0;hi=a.spec.scale?10:1;}
            if(lo==hi){if(a.spec.scale){lo=lo>0?lo/2:lo*2;hi=hi>0?hi*2:hi/2;}else{const double delta=std::max(std::abs(lo)*.05,1.);lo-=delta;hi+=delta;}}
            else if(!polar||!horizontal){const double tl=forge_plot_transform(lo,a.spec.scale),th=forge_plot_transform(hi,a.spec.scale),pad=(th-tl)*.04;if(!polar)lo=forge_plot_inverse(tl-pad,a.spec.scale,lo);hi=forge_plot_inverse(th+pad,a.spec.scale,hi);}
            a.minimum=lo;a.maximum=hi;
            if(!forge_plot_panel_find(p,a.spec.panel)->system&&horizontal&&a.spec.id==p.options.x_window_axis&&p.options.x_window_mode>0){
                if(a.spec.scale||!std::isfinite(p.options.x_window_size)||p.options.x_window_size<=0){p.error="实时 X 窗口需要线性轴和正的有限跨度";return false;}
                a.maximum=std::max(a.domain_maximum,a.domain_minimum+p.options.x_window_size);a.minimum=a.maximum-p.options.x_window_size;
            }
        } else {a.minimum=a.spec.minimum;a.maximum=a.spec.maximum;}
        if(!std::isfinite(a.minimum)||!std::isfinite(a.maximum)||a.minimum>=a.maximum||(a.spec.scale&&((a.minimum<=0)!=(a.maximum<=0)||a.minimum==0||a.maximum==0))||!std::isfinite(forge_plot_transform(a.maximum,a.spec.scale)-forge_plot_transform(a.minimum,a.spec.scale))){p.error="坐标范围须有限、有序；对数轴须位于同一非零符号域";return false;}
    }
    return true;
}

static void forge_plot_draw_axes(ForgePlotCoreState& p) {
    for(const auto& panel:p.panels){const auto& r=panel.rect;std::string box;if(panel.system>=8){if(!panel.title.empty())forge_plot_add_text(p,r.x,r.y-16,panel.title,p.options.foreground,p.options.font_size+1,-99);continue;}if(panel.polar)forge_plot_ellipse(box,r.x+r.width*.5,r.y+r.height*.5,std::min(r.width,r.height)*.5,std::min(r.width,r.height)*.5);else forge_plot_rectangle(box,r.x,r.y,r.width,r.height);forge_plot_add_path(p,box,p.options.grid_color,0,1,panel.spec.id,-100,false);if(!panel.title.empty())forge_plot_add_text(p,r.x,r.y-16,panel.title,p.options.foreground,p.options.font_size+1,-99);
        if(panel.polar){const ForgePlotCoreAxisState* angular=nullptr;const ForgePlotCoreAxisState* radial=nullptr;for(const auto& a:p.axes)if(a.spec.panel==panel.spec.id){if(a.spec.side<2&&!angular)angular=&a;if(a.spec.side>=2&&!radial)radial=&a;}if(!angular||!radial)continue;
            const double cx=r.x+r.width*.5,cy=r.y+r.height*.5,radius=std::min(r.width,r.height)*.5;std::string grid;
            auto angular_ticks=*angular;if(angular_ticks.spec.tick_step<=0)angular_ticks.spec.tick_step=(angular->maximum-angular->minimum)/8;
            for(const auto& tick:forge_plot_ticks(angular_ticks,2*forge_plot_pi*radius)){if(tick.first<angular->minimum||tick.first>=angular->maximum)continue;const auto pos=forge_plot_polar_point(panel,*angular,*radial,tick.first,radial->maximum);if(angular->spec.grid)forge_plot_line(grid,cx,cy,pos.first,pos.second);if(angular->spec.visible){const double angle=forge_plot_axis_normal(*angular,tick.first)*2*forge_plot_pi;forge_plot_add_text(p,pos.first+16*std::cos(angle)-tick.second.size()*p.options.font_size*.25,pos.second-16*std::sin(angle)+p.options.font_size*.35,tick.second,p.options.foreground,p.options.font_size,-80);}}
            for(const auto& tick:forge_plot_ticks(*radial,radius)){const double rr=forge_plot_axis_normal(*radial,tick.first)*radius;if(!std::isfinite(rr)||rr<=0||rr>radius)continue;if(radial->spec.grid)forge_plot_ellipse(grid,cx,cy,rr,rr);if(radial->spec.visible)forge_plot_add_text(p,cx+4,cy-rr-3,tick.second,p.options.foreground,p.options.font_size,-80);}
            forge_plot_add_path(p,grid,p.options.grid_color,0,1,panel.spec.id,-90,true);if(!angular->label.empty())forge_plot_add_text(p,cx-angular->label.size()*p.options.font_size*.25,cy+radius+40,angular->label,p.options.foreground,p.options.font_size,-70);
        }
    }
    std::map<std::pair<int,int>,double> offsets;
    for(const auto& a:p.axes)if(a.spec.visible) {
        if(forge_plot_panel_find(p,a.spec.panel)->system)continue;
        const auto& r=forge_plot_panel_find(p,a.spec.panel)->rect;const bool horizontal=a.spec.side<2;
        double& offset=offsets[{a.spec.panel,a.spec.side}];const double o=offset+a.spec.offset;
        const double base=a.spec.side==0?r.y+r.height+o:a.spec.side==1?r.y-o:a.spec.side==2?r.x-o:r.x+r.width+o;
        std::string path;forge_plot_line(path,horizontal?r.x:base,horizontal?base:r.y,horizontal?r.x+r.width:base,horizontal?base:r.y+r.height);
        auto ticks=forge_plot_ticks(a,horizontal?r.width:r.height);const auto& categories=forge_plot_panel_find(p,a.spec.panel)->categories;
        if(horizontal&&!categories.empty()){ticks.clear();for(size_t i=0;i<categories.size();++i)ticks.emplace_back(static_cast<double>(i),categories[i]);}
        for(const auto& tick:ticks) {
            const double pixel=forge_plot_axis_pixel(p,a,tick.first);if(!std::isfinite(pixel)||(horizontal?(pixel<r.x-.1||pixel>r.x+r.width+.1):(pixel<r.y-.1||pixel>r.y+r.height+.1)))continue;
            const double sign=a.spec.side==0||a.spec.side==3?1:-1;
            forge_plot_line(path,horizontal?pixel:base,horizontal?base:pixel,horizontal?pixel:base+sign*5,horizontal?base+sign*5:pixel);
            if(a.spec.grid&&offset==0){std::string grid;forge_plot_line(grid,horizontal?pixel:r.x,horizontal?r.y:pixel,horizontal?pixel:r.x+r.width,horizontal?r.y+r.height:pixel);forge_plot_add_path(p,grid,p.options.grid_color,0,1,a.spec.panel,-90,true);}
            const double approx=forge_plot_text_width(tick.second,p.options.font_size)*.5;
            forge_plot_add_text(p,horizontal?pixel-approx:(a.spec.side==2?base-9-2*approx:base+9),horizontal?(a.spec.side==0?base+20:base-10):pixel+p.options.font_size*.35,tick.second,p.options.foreground,p.options.font_size,-80);
        }
        forge_plot_add_path(p,path,p.options.foreground,0,1,a.spec.panel,-70,false);
        ForgePlotCoreHitState axis_hit;axis_hit.hit={4,a.spec.id,-1,a.spec.panel,0,0,0,horizontal?r.x:base,horizontal?base:r.y,0,a.label.c_str()};axis_hit.x2=horizontal?r.x+r.width:base;axis_hit.y2=horizontal?base:r.y+r.height;axis_hit.segment=true;axis_hit.clip=false;p.hits.push_back(axis_hit);
        if(!a.label.empty()){const double text_width=forge_plot_text_width(a.label,p.options.font_size);if(horizontal)forge_plot_add_text(p,r.x+r.width*.5-text_width*.5,a.spec.side==0?base+42:base-30,a.label,p.options.foreground,p.options.font_size,-70);else forge_plot_add_text(p,a.spec.side==2?base-48:base+48,r.y+r.height*.5+text_width*.5,a.label,p.options.foreground,p.options.font_size,-70,-1,false,-90);}
        offset+=42+(a.label.empty()?0:20)+std::max(0.,a.spec.offset);
    }
    if(!p.title.empty())forge_plot_add_text(p,20,24,p.title,p.options.foreground,p.options.font_size+5,-70);
}

struct ForgePlotCoreSample { size_t index=0;double x=0,y=0;size_t sequence=0; };
static bool forge_plot_selected(const ForgePlotCoreState& p,int id,size_t index) {
    const auto it=std::upper_bound(p.selection.begin(),p.selection.end(),std::make_pair(id,index),[](const auto& key,const auto& range){return key<std::make_pair(range.id,static_cast<size_t>(range.first));});
    return it!=p.selection.begin()&&std::prev(it)->id==id&&index<=static_cast<size_t>(std::prev(it)->last);
}
static std::vector<ForgePlotCoreSample> forge_plot_samples(const ForgePlotCoreState& p,const ForgePlotCoreSeriesState& s,const ForgePlotCoreAxisState& ax,const ForgePlotCoreAxisState& ay) {
    std::vector<ForgePlotCoreSample> output;
    const auto& rect=forge_plot_panel_find(p,ax.spec.panel)->rect;
    size_t first=0,last=s.points.size();
    if(s.monotone&&s.binary_searchable&&s.spec.kind!=1&&s.spec.kind!=6&&!s.points.empty()) {
        const auto less=[](const ForgePlotPointCore& pt,double value){return pt.x<value;};
        const auto upper=[](double value,const ForgePlotPointCore& pt){return value<pt.x;};
        auto begin=std::lower_bound(s.points.begin(),s.points.end(),ax.minimum,less),end=std::upper_bound(s.points.begin(),s.points.end(),ax.maximum,upper);
        first=static_cast<size_t>(begin-s.points.begin());if(first) --first;last=static_cast<size_t>(end-s.points.begin());if(last<s.points.size())++last;
    }
    const double budget=std::clamp(p.options.max_points_per_pixel>0?p.options.max_points_per_pixel:4,1,16);
    const double column_width=std::max(1.,4./budget);
    const bool scatter_only=s.spec.kind<=1&&s.spec.line_style==0;
    std::unordered_set<int64_t> scatter_cells;
    const double scatter_cell=std::max(2.,s.spec.point_size*.75)*std::sqrt(4./budget);
    std::array<ForgePlotCoreSample,4> bucket{};int bucket_id=std::numeric_limits<int>::min();size_t count=0;
    const auto flush=[&](){if(!count)return;std::array<ForgePlotCoreSample,4> sorted=bucket;std::sort(sorted.begin(),sorted.end(),[](const auto& a,const auto& b){return a.sequence<b.sequence;});size_t prev=std::numeric_limits<size_t>::max();for(const auto& pt:sorted)if(pt.index!=prev){output.push_back(pt);prev=pt.index;}count=0;};
    const auto aggregate=[&](const ForgePlotCoreSample& sample){const int id=static_cast<int>(std::clamp(std::floor(sample.x/column_width),-1e7,1e7));if(id!=bucket_id){flush();bucket_id=id;bucket.fill(sample);count=1;}else{bucket[3]=sample;if(sample.y<bucket[1].y)bucket[1]=sample;if(sample.y>bucket[2].y)bucket[2]=sample;++count;}};
    for(size_t sequence=first;sequence<last;++sequence) {
        // 对完全位于同一个像素桶的有序数据块，直接使用增量维护的首/尾/峰/谷索引。
        // 有断点、误差棒、独立选择或对数符号变化的块回退到逐点处理。
        bool summarized=false;
        if(p.options.adaptive&&!scatter_only&&s.spec.kind==0&&s.monotone&&s.binary_searchable&&s.simple_samples&&s.styles.empty()&&s.order.empty()&&p.selection.empty()&&last-first>static_cast<size_t>(rect.width*budget)){
            for(size_t level=forge_plot_block_spans.size();level-->0;){const size_t span=forge_plot_block_spans[level];if(sequence%span||last-sequence<span)continue;const auto& block=s.sample_blocks[level][sequence/span];if(!block.valid||!forge_plot_domain(ay,s.points[block.minimum].y)||!forge_plot_domain(ay,s.points[block.maximum].y))continue;
                const double x1=forge_plot_axis_pixel(p,ax,s.points[sequence].x),x2=forge_plot_axis_pixel(p,ax,s.points[sequence+span-1].x);if(!std::isfinite(x1+x2)||std::floor(x1/column_width)!=std::floor(x2/column_width))continue;
                std::array<size_t,4> indices{sequence,block.minimum,block.maximum,sequence+span-1};std::sort(indices.begin(),indices.end());size_t previous=std::numeric_limits<size_t>::max();for(size_t index:indices)if(index!=previous){aggregate({index,forge_plot_axis_pixel(p,ax,s.points[index].x),forge_plot_axis_pixel(p,ay,s.points[index].y),index});previous=index;}
                sequence+=span-1;summarized=true;break;
            }
        }
        if(summarized)continue;
        const size_t i=s.order.empty()?sequence:s.order[sequence];
        const auto& pt=s.points[i];const double x=forge_plot_axis_pixel(p,ax,pt.x),y=forge_plot_axis_pixel(p,ay,pt.y);
        if(forge_plot_point_hidden(s,i)||!forge_plot_point_valid(s,pt)||!std::isfinite(x+y)){flush();if(!output.empty()&&std::isfinite(output.back().x))output.push_back({i,std::numeric_limits<double>::quiet_NaN(),0});bucket_id=std::numeric_limits<int>::min();continue;}
        ForgePlotCoreSample sample{i,x,y,sequence};
        // 独立点选择必须可见；大区间仍使用降采样，避免全选恢复百万个图元。
        if(!p.selection.empty()){const auto selected=std::lower_bound(p.selection.begin(),p.selection.end(),std::make_pair(s.spec.id,static_cast<int>(i)),[](const auto& range,const auto& key){return std::make_pair(range.id,range.first)<key;});if(selected!=p.selection.end()&&selected->id==s.spec.id&&selected->first==selected->last&&selected->first==static_cast<int>(i)){flush();output.push_back(sample);bucket_id=std::numeric_limits<int>::min();continue;}}
        if(p.options.adaptive&&scatter_only){if(x<rect.x-8||x>rect.x+rect.width+8||y<rect.y-8||y>rect.y+rect.height+8)continue;const auto cell_x=static_cast<uint32_t>(static_cast<int>(std::floor(x/scatter_cell))),cell_y=static_cast<uint32_t>(static_cast<int>(std::floor(y/scatter_cell)));const int64_t cell=static_cast<int64_t>((static_cast<uint64_t>(cell_x)<<32)|cell_y);if(scatter_cells.insert(cell).second)output.push_back(sample);continue;}
        if(!p.options.adaptive||(!s.monotone&&s.order.empty())||s.spec.kind>1||last-first<=static_cast<size_t>(rect.width*budget)){output.push_back(sample);continue;}
        aggregate(sample);
    }
    flush();return output;
}
static void forge_plot_symbol(std::string& path,int style,double x,double y,double radius) {
    if(style==0||radius<=0)return;
    if(style==1||style==2)forge_plot_ellipse(path,x,y,radius,radius);
    else if(style==3)forge_plot_rectangle(path,x-radius,y-radius,radius*2,radius*2);
    else if(style==4){forge_plot_path_point(path,'M',x,y-radius);forge_plot_path_point(path,'L',x+radius,y);forge_plot_path_point(path,'L',x,y+radius);forge_plot_path_point(path,'L',x-radius,y);path+="Z ";}
    else if(style==5||style==6){forge_plot_line(path,x-radius,y-radius,x+radius,y+radius);forge_plot_line(path,x-radius,y+radius,x+radius,y-radius);if(style==6){forge_plot_line(path,x-radius,y,x+radius,y);forge_plot_line(path,x,y-radius,x,y+radius);}}
    else if(style==7){forge_plot_path_point(path,'M',x,y-radius);forge_plot_path_point(path,'L',x+radius,y+radius);forge_plot_path_point(path,'L',x-radius,y+radius);path+="Z ";}
    else if(style==8){forge_plot_line(path,x-radius,y,x+radius,y);forge_plot_line(path,x,y-radius,x,y+radius);}
    else {for(int i=0;i<10;++i){const double a=-forge_plot_pi/2+i*forge_plot_pi/5,r=(i%2? .4:1)*radius;forge_plot_path_point(path,i?'L':'M',x+r*std::cos(a),y+r*std::sin(a));}path+="Z ";}
}
static void forge_plot_add_hit(ForgePlotCoreState& p,const ForgePlotCoreSeriesState& s,size_t index,double x,double y,double x2,double y2,bool segment,bool rect,double radius=4) {
    const auto& point=s.points[index];ForgePlotCoreHitState h;h.hit.kind=0;h.hit.id=s.spec.id;h.hit.index=static_cast<int>(index);h.hit.panel=forge_plot_axis_find(p,s.spec.x_axis)->spec.panel;
    h.hit.x=point.x;h.hit.y=s.spec.kind==4||s.spec.kind==5?point.close:point.y;h.hit.value=h.hit.y;h.hit.pixel_x=x;h.hit.pixel_y=y;h.x2=x2;h.y2=y2;h.segment=segment;h.rect=rect;h.radius=radius;
    const auto label=s.labels.find(index);h.hit.label=label==s.labels.end()?s.name.c_str():label->second.c_str();p.hits.push_back(h);
}

static std::vector<std::pair<double,double>> forge_plot_fill_vertices(const std::vector<ForgePlotCoreSample>& segment,int style) {
    std::vector<std::pair<double,double>> points;
    for(size_t j=0;j<segment.size();++j) {
        const auto& v=segment[j];
        if(j&&style==2)points.emplace_back(v.x,segment[j-1].y);
        else if(j&&style==3)points.emplace_back(segment[j-1].x,v.y);
        else if(j&&style==4){const double mid=(segment[j-1].x+v.x)*.5;points.emplace_back(mid,segment[j-1].y);points.emplace_back(mid,v.y);}
        points.emplace_back(v.x,v.y);
    }
    return points;
}
static std::vector<std::pair<double,double>> forge_plot_clip_fill(const std::vector<std::pair<double,double>>& points,double lo,double hi) {
    std::vector<std::pair<double,double>> clipped;
    const auto append=[&](const std::pair<double,double>& point){if(clipped.empty()||clipped.back()!=point)clipped.push_back(point);};
    for(size_t i=1;i<points.size();++i) {
        const auto [x,y]=points[i-1];const auto [x2,y2]=points[i];
        if(x==x2){if(x>=lo&&x<=hi){append(points[i-1]);append(points[i]);}continue;}
        double first=(lo-x)/(x2-x),last=(hi-x)/(x2-x);if(first>last)std::swap(first,last);
        first=std::max(0.,first);last=std::min(1.,last);if(first>last)continue;
        append({x+(x2-x)*first,y+(y2-y)*first});append({x+(x2-x)*last,y+(y2-y)*last});
    }
    return clipped;
}
// 圆弧转为三次贝塞尔，SVG、位图和 PDF 共用完全相同的路径。
static void forge_plot_arc(std::string& path,double cx,double cy,double radius,double start,double sweep) {
    const int count=std::max(1,static_cast<int>(std::ceil(std::abs(sweep)/(forge_plot_pi*.5))));
    for(int i=0;i<count;++i){const double a=start+sweep*i/count,b=start+sweep*(i+1)/count,k=4./3*std::tan((b-a)/4);
        path+="C "+forge_plot_number(cx+radius*(std::cos(a)-k*std::sin(a)))+" "+forge_plot_number(cy+radius*(std::sin(a)+k*std::cos(a)))+" "+forge_plot_number(cx+radius*(std::cos(b)+k*std::sin(b)))+" "+forge_plot_number(cy+radius*(std::sin(b)-k*std::cos(b)))+" "+forge_plot_number(cx+radius*std::cos(b))+" "+forge_plot_number(cy+radius*std::sin(b))+" ";
    }
}
static bool forge_plot_draw_radial(ForgePlotCoreState& p,const ForgePlotCoreSeriesState& s,size_t series_index,const ForgePlotCorePanelState& panel) {
    const auto& r=panel.rect;const double cx=r.x+r.width*.5,cy=r.y+r.height*.5;
    const double radius=std::max(1.,std::min(r.width,r.height)*.5-(s.spec.kind==10?36.:8.));
    const size_t n=panel.categories.empty()?s.points.size():panel.categories.size();
    if(s.points.empty())return true;
    if(s.spec.kind==10&&(n<3||s.points.size()!=n)){p.error="雷达图需要至少三个分类，所有系列点数须与分类数一致";return false;}
    if(!std::isfinite(s.spec.inner_radius)||s.spec.inner_radius<0||s.spec.inner_radius>=1){p.error="环形内径比例须在 [0, 1) 内";return false;}
    double sum=0,maximum=0;
    for(size_t i=0;i<s.points.size();++i)if(!forge_plot_point_hidden(s,i)&&std::isfinite(s.points[i].y)&&s.points[i].y>0){sum+=s.points[i].y;maximum=std::max(maximum,s.points[i].y);}
    if(!std::isfinite(sum)){p.error="径向图表数值总和超出有效范围";return false;}
    if(s.spec.kind==10){
        for(const auto& other:p.series)if(!other.spec.hidden&&other.spec.kind==10&&other.spec.x_axis==s.spec.x_axis)for(size_t i=0;i<other.points.size();++i)if(!forge_plot_point_hidden(other,i)&&std::isfinite(other.points[i].y))maximum=std::max(maximum,other.points[i].y);
        maximum=std::max(maximum,1.);
        bool first=true;for(size_t i=0;i<series_index;++i)if(!p.series[i].spec.hidden&&p.series[i].spec.kind==10&&p.series[i].spec.x_axis==s.spec.x_axis)first=false;
        if(first){std::string grid;for(int ring=1;ring<=4;++ring){for(size_t i=0;i<n;++i){const double a=-forge_plot_pi*.5+2*forge_plot_pi*i/n;forge_plot_path_point(grid,i?'L':'M',cx+radius*ring/4*std::cos(a),cy+radius*ring/4*std::sin(a));}grid+="Z ";}
            for(size_t i=0;i<n;++i){const double a=-forge_plot_pi*.5+2*forge_plot_pi*i/n;forge_plot_line(grid,cx,cy,cx+radius*std::cos(a),cy+radius*std::sin(a));const auto label=i<panel.categories.size()?panel.categories[i]:std::to_string(i+1);forge_plot_add_text(p,cx+(radius+18)*std::cos(a)-forge_plot_text_width(label,p.options.font_size)*.5,cy+(radius+18)*std::sin(a)+p.options.font_size*.35,label,p.options.foreground,p.options.font_size,-80);}
            forge_plot_add_path(p,grid,p.options.grid_color,0,1,panel.spec.id,-90,false);
        }
        std::string path,markers,selected;bool complete=true,active=false;
        for(size_t i=0;i<n;++i){const double v=s.points[i].y;if(forge_plot_point_hidden(s,i)||!std::isfinite(v)||v<0){complete=false;active=false;continue;}const double a=-forge_plot_pi*.5+2*forge_plot_pi*i/n,rr=radius*v/maximum,x=cx+rr*std::cos(a),y=cy+rr*std::sin(a);
            forge_plot_path_point(path,active?'L':'M',x,y);active=true;forge_plot_symbol(markers,2,x,y,3);if(s.spec.selectable)forge_plot_add_hit(p,s,i,x,y,x,y,false,false,5);if(forge_plot_selected(p,s.spec.id,i))forge_plot_symbol(selected,1,x,y,7);++p.rendered_points;
        }
        if(complete)path+="Z ";const uint32_t color=s.spec.color>>24?s.spec.color:forge_plot_palette(series_index);
        forge_plot_add_path(p,path,color,complete?s.spec.fill_color:0,std::max(1.,s.spec.line_width),panel.spec.id,s.spec.layer);
        forge_plot_add_path(p,markers,color,color,1,panel.spec.id,s.spec.layer+1);forge_plot_add_path(p,selected,p.options.selection_color,0,2,panel.spec.id,s.spec.layer+2);return true;
    }
    if(sum<=0)return true;
    double angle=-forge_plot_pi*.5;
    for(size_t i=0;i<s.points.size();++i){const double v=s.points[i].y;const double sweep=s.spec.kind==9?2*forge_plot_pi/s.points.size():(std::isfinite(v)&&v>0&&!forge_plot_point_hidden(s,i)?2*forge_plot_pi*(v/sum):0);
        if(forge_plot_point_hidden(s,i)||!std::isfinite(v)||v<=0){if(s.spec.kind==9)angle+=sweep;continue;}
        const double outer=s.spec.kind==9?radius*std::sqrt(v/maximum):radius,inner=s.spec.kind==8?outer*s.spec.inner_radius:0;
        std::string path;forge_plot_path_point(path,'M',cx+outer*std::cos(angle),cy+outer*std::sin(angle));forge_plot_arc(path,cx,cy,outer,angle,sweep);
        if(inner>0){forge_plot_path_point(path,'L',cx+inner*std::cos(angle+sweep),cy+inner*std::sin(angle+sweep));forge_plot_arc(path,cx,cy,inner,angle+sweep,-sweep);}else forge_plot_path_point(path,'L',cx,cy);path+="Z ";
        const auto* style=forge_plot_point_style(s,i);const uint32_t color=style&&(style->color>>24)?style->color:forge_plot_palette(i);
        const bool selected=forge_plot_selected(p,s.spec.id,i);forge_plot_add_path(p,path,selected?p.options.selection_color:p.options.background,color,selected?3:1,panel.spec.id,s.spec.layer);
        const double mid=angle+sweep*.5,rr=(inner+outer)*.5;
        if(s.spec.selectable){forge_plot_add_hit(p,s,i,cx+rr*std::cos(mid),cy+rr*std::sin(mid),cx,cy,false,false,outer*2);auto& hit=p.hits.back();hit.sector=true;hit.cx=cx;hit.cy=cy;hit.inner=inner;hit.outer=outer;hit.start=angle;hit.sweep=sweep;if(i<panel.categories.size())hit.hit.label=panel.categories[i].c_str();}
        ++p.rendered_points;angle+=sweep;
    }return true;
}
static std::vector<ForgePlotCoreSample> forge_plot_smooth_samples(const std::vector<ForgePlotCoreSample>& input) {
    std::vector<ForgePlotCoreSample> out;out.reserve(input.size()*8);
    for(size_t i=0;i<input.size();++i){if(!i||!std::isfinite(input[i].x+input[i].y+input[i-1].x+input[i-1].y)){out.push_back(input[i]);continue;}
        const auto& a=input[i-1];const auto& b=input[i];const auto& prev=i>1&&std::isfinite(input[i-2].x+input[i-2].y)?input[i-2]:a;const auto& next=i+1<input.size()&&std::isfinite(input[i+1].x+input[i+1].y)?input[i+1]:b;
        // 插值顶点只绘制曲线，散点、选择轮廓和误差棒仍对应真实样本。
        for(int step=1;step<=8;++step){const double t=step/8.,t2=t*t,t3=t2*t;const auto interpolate=[&](double p0,double p1,double p2,double p3){return .5*((2*p1)+(-p0+p2)*t+(2*p0-5*p1+4*p2-p3)*t2+(-p0+3*p1-3*p2+p3)*t3);};out.push_back({b.index,interpolate(prev.x,a.x,b.x,next.x),interpolate(prev.y,a.y,b.y,next.y),step==8?b.sequence:std::numeric_limits<size_t>::max()});}
    }return out;
}
static bool forge_plot_draw_series(ForgePlotCoreState& p) {
    std::map<std::tuple<int,int,int,double>,std::pair<double,double>> stacks;
    for(size_t si=0;si<p.series.size();++si) {
        const auto& s=p.series[si];if(s.spec.hidden)continue;
        const auto* ax=forge_plot_axis_find(p,s.spec.x_axis);const auto* ay=forge_plot_axis_find(p,s.spec.y_axis);
        if(!ax||!ay||ax->spec.side>=2||ay->spec.side<2||ax->spec.panel!=ay->spec.panel||s.spec.kind<0||s.spec.kind>10){p.error="系列的坐标轴、绘图区或图形类型无效";return false;}
        const auto& rect=forge_plot_panel_find(p,ax->spec.panel)->rect;const uint32_t color=s.spec.color>>24?s.spec.color:forge_plot_palette(si);
        if(s.spec.kind>=8){if(!forge_plot_draw_radial(p,s,si,*forge_plot_panel_find(p,ax->spec.panel)))return false;continue;}
        auto samples=forge_plot_samples(p,s,*ax,*ay);if(s.spec.line_style==6&&(s.spec.kind==0||s.spec.kind==1))samples=forge_plot_smooth_samples(samples);
        std::string path,symbols,errors,filled,selected;bool active=false;ForgePlotCoreSample previous{};
        // 密集折线的散点符号按屏幕网格去重；折线、误差、选择和原始索引命中不受影响。
        std::unordered_set<uint64_t> marker_cells;const double marker_cell=std::max(2.,s.spec.point_size*.75);
        double baseline=forge_plot_axis_pixel(p,*ay,s.spec.baseline);if(!std::isfinite(baseline))baseline=rect.y+rect.height;
        std::vector<std::vector<std::pair<double,double>>> channel_runs;
        if(s.spec.fill_target>=0&&(s.spec.fill_color>>24)) {
            const ForgePlotCoreSeriesState* target=nullptr;
            for(const auto& other:p.series)if(other.spec.id==s.spec.fill_target)target=&other;
            if(!target||target==&s||s.spec.kind!=0||target->spec.kind!=0||target->spec.x_axis!=s.spec.x_axis||target->spec.y_axis!=s.spec.y_axis){p.error="系列间填充须指定使用相同坐标轴的另一条折线";return false;}
            if(!target->spec.hidden){
                std::vector<ForgePlotCoreSample> run;
                const auto finish=[&](){if(run.size()>1)channel_runs.push_back(forge_plot_fill_vertices(run,target->spec.line_style));run.clear();};
                auto target_samples=forge_plot_samples(p,*target,*ax,*ay);if(target->spec.line_style==6)target_samples=forge_plot_smooth_samples(target_samples);
                for(const auto& v:target_samples){if(!std::isfinite(v.x+v.y)){finish();continue;}run.push_back(v);}finish();
            }
        }
        std::vector<ForgePlotCoreSample> segment;
        const auto fill_segment=[&](){if(segment.size()<2||!(s.spec.fill_color>>24)){segment.clear();return;}
            const auto source=forge_plot_fill_vertices(segment,s.spec.line_style);
            if(s.spec.fill_target>=0){
                // 仅填充两条连续曲线的重叠区间，空值和隐藏参考线不产生填充。
                for(const auto& target:channel_runs){
                    const double lo=std::max(std::min(source.front().first,source.back().first),std::min(target.front().first,target.back().first));
                    const double hi=std::min(std::max(source.front().first,source.back().first),std::max(target.front().first,target.back().first));if(lo>=hi)continue;
                    const auto a=forge_plot_clip_fill(source,lo,hi);auto b=forge_plot_clip_fill(target,lo,hi);if(a.size()<2||b.size()<2)continue;
                    if((a.front().first<a.back().first)!=(b.front().first<b.back().first))std::reverse(b.begin(),b.end());
                    for(size_t j=0;j<a.size();++j)forge_plot_path_point(filled,j?'L':'M',a[j].first,a[j].second);
                    for(auto it=b.rbegin();it!=b.rend();++it)forge_plot_path_point(filled,'L',it->first,it->second);filled+="Z ";
                }
            }else{
                for(size_t j=0;j<source.size();++j)forge_plot_path_point(filled,j?'L':'M',source[j].first,source[j].second);
                if(s.spec.kind==0){forge_plot_path_point(filled,'L',segment.back().x,baseline);forge_plot_path_point(filled,'L',segment.front().x,baseline);}filled+="Z ";
            }
            segment.clear();};
        for(const auto& sample:samples) {
            if(!std::isfinite(sample.x+sample.y)){fill_segment();active=false;continue;}
            const auto& pt=s.points[sample.index];double x=sample.x,y=sample.y;
            const bool original_sample=sample.sequence!=std::numeric_limits<size_t>::max();
            const bool chosen=forge_plot_selected(p,s.spec.id,sample.index);
            if(s.spec.kind==6){const auto pos=forge_plot_polar_point(*forge_plot_panel_find(p,ax->spec.panel),*ax,*ay,pt.x,pt.y);x=pos.first;y=pos.second;}
            const bool inside=x>=rect.x-10&&x<=rect.x+rect.width+10&&y>=rect.y-10&&y<=rect.y+rect.height+10;
            if(s.spec.kind==0||s.spec.kind==1||s.spec.kind==6) {
                if(s.spec.line_style!=0){if(s.spec.line_style==5)forge_plot_line(path,x,baseline,x,y);else if(!active)forge_plot_path_point(path,'M',x,y);else{if(s.spec.line_style==2)forge_plot_path_point(path,'L',x,previous.y);else if(s.spec.line_style==3)forge_plot_path_point(path,'L',previous.x,y);else if(s.spec.line_style==4){forge_plot_path_point(path,'L',(previous.x+x)*.5,previous.y);forge_plot_path_point(path,'L',(previous.x+x)*.5,y);}forge_plot_path_point(path,'L',x,y);}}
                if(original_sample&&inside&&s.spec.scatter_style!=0){
                    const auto cx=static_cast<uint32_t>(static_cast<int32_t>(std::floor(x/marker_cell))),cy=static_cast<uint32_t>(static_cast<int32_t>(std::floor(y/marker_cell)));
                    const uint64_t cell=(static_cast<uint64_t>(cx)<<32)|cy;
                    if(!p.options.adaptive||chosen||marker_cells.insert(cell).second){const auto* style=forge_plot_point_style(s,sample.index);if(style){const double size=style->size==0?s.spec.point_size:style->size;if(std::isfinite(size)&&size>0&&size<=16384){std::string mark;forge_plot_symbol(mark,s.spec.scatter_style,x,y,size*.5);const uint32_t c=style->color>>24?style->color:color;forge_plot_add_path(p,mark,c,s.spec.scatter_style==2?c:0,1,ax->spec.panel,s.spec.layer+1);}}else forge_plot_symbol(symbols,s.spec.scatter_style,x,y,s.spec.point_size*.5);}
                }
                if(original_sample&&chosen&&inside)forge_plot_symbol(selected,1,x,y,std::max(5.,s.spec.point_size*.5+3));
                if(original_sample&&s.spec.selectable&&inside)forge_plot_add_hit(p,s,sample.index,x,y,x,y,false,false,std::max(4.,s.spec.point_size*.5));
                if(s.spec.selectable&&s.spec.line_style==5)forge_plot_add_hit(p,s,sample.index,x,baseline,x,y,true,false);
                if(s.spec.selectable&&active&&s.spec.line_style!=0&&s.spec.line_style!=5){
                    if(s.spec.line_style==2){forge_plot_add_hit(p,s,sample.index,previous.x,previous.y,x,previous.y,true,false);forge_plot_add_hit(p,s,sample.index,x,previous.y,x,y,true,false);}
                    else if(s.spec.line_style==3){forge_plot_add_hit(p,s,sample.index,previous.x,previous.y,previous.x,y,true,false);forge_plot_add_hit(p,s,sample.index,previous.x,y,x,y,true,false);}
                    else if(s.spec.line_style==4){const double mid=(previous.x+x)*.5;forge_plot_add_hit(p,s,sample.index,previous.x,previous.y,mid,previous.y,true,false);forge_plot_add_hit(p,s,sample.index,mid,previous.y,mid,y,true,false);forge_plot_add_hit(p,s,sample.index,mid,y,x,y,true,false);}
                    else forge_plot_add_hit(p,s,sample.index,previous.x,previous.y,x,y,true,false);
                }
                segment.push_back({sample.index,x,y});previous={sample.index,x,y};active=true;
            } else if(s.spec.kind==7) {
                const auto* style=forge_plot_point_style(s,sample.index);const double size=style&&style->size!=0?style->size:s.spec.point_size;
                if(!std::isfinite(size)||size<=0||size>16384)continue;
                const uint32_t point_color=style&&(style->color>>24)?style->color:color;std::string bubble;forge_plot_ellipse(bubble,x,y,size*.5,size*.5);
                forge_plot_add_path(p,bubble,chosen?p.options.selection_color:point_color,point_color,chosen?3:1,ax->spec.panel,s.spec.layer);
                if(s.spec.selectable)forge_plot_add_hit(p,s,sample.index,x,y,x,y,false,false,size*.5);
            } else if(s.spec.kind==2) {
                double lo=s.spec.baseline,hi=lo+pt.y;double shift=0,width=s.spec.bar_width>0?s.spec.bar_width:.8;
                if(s.spec.stack_group>=0){auto& stack=stacks[{s.spec.x_axis,s.spec.y_axis,s.spec.stack_group,pt.x}];double& sum=pt.y>=0?stack.first:stack.second;lo+=sum;sum+=pt.y;hi=s.spec.baseline+sum;}
                else{size_t group=0,pos=0;for(size_t k=0;k<p.series.size();++k)if(!p.series[k].spec.hidden&&p.series[k].spec.kind==2&&p.series[k].spec.stack_group<0&&p.series[k].spec.x_axis==s.spec.x_axis&&p.series[k].spec.y_axis==s.spec.y_axis){if(k==si)pos=group;++group;}width/=std::max<size_t>(1,group);shift=(static_cast<double>(pos)-static_cast<double>(group-1)*.5)*width;}
                const double x1=forge_plot_axis_pixel(p,*ax,pt.x+shift-width*.45),x2=forge_plot_axis_pixel(p,*ax,pt.x+shift+width*.45),y2=forge_plot_axis_pixel(p,*ay,hi);double y1=forge_plot_axis_pixel(p,*ay,lo);
                if(!std::isfinite(y1)&&ay->spec.scale)y1=forge_plot_axis_pixel(p,*ay,ay->minimum>0?ay->minimum:ay->maximum);
                if(!std::isfinite(x1+x2+y1+y2))continue;
                const auto* style=forge_plot_point_style(s,sample.index);if(style&&(style->color>>24)){std::string bar;forge_plot_rectangle(bar,std::min(x1,x2),std::min(y1,y2),std::abs(x2-x1),std::abs(y2-y1));forge_plot_add_path(p,bar,style->color,style->color,s.spec.line_width,ax->spec.panel,s.spec.layer);}else forge_plot_rectangle(path,std::min(x1,x2),std::min(y1,y2),std::abs(x2-x1),std::abs(y2-y1));if(s.spec.selectable)forge_plot_add_hit(p,s,sample.index,std::min(x1,x2),std::min(y1,y2),std::max(x1,x2),std::max(y1,y2),false,true);
                if(chosen)forge_plot_rectangle(selected,std::min(x1,x2),std::min(y1,y2),std::abs(x2-x1),std::abs(y2-y1));
            } else {
                const double w=std::abs(forge_plot_axis_pixel(p,*ax,pt.x+(s.spec.bar_width>0?s.spec.bar_width:.6)*.5)-x);
                const double low=forge_plot_axis_pixel(p,*ay,pt.low),high=forge_plot_axis_pixel(p,*ay,pt.high);if(!std::isfinite(low+high+w))continue;
                forge_plot_line(errors,x,low,x,high);
                if(s.spec.kind==3){const double q1=forge_plot_axis_pixel(p,*ay,pt.q1),q3=forge_plot_axis_pixel(p,*ay,pt.q3);if(!std::isfinite(q1+q3))continue;forge_plot_rectangle(path,x-w,std::min(q1,q3),2*w,std::abs(q3-q1));forge_plot_line(errors,x-w,y,x+w,y);forge_plot_line(errors,x-w*.5,low,x+w*.5,low);forge_plot_line(errors,x-w*.5,high,x+w*.5,high);const auto outliers=s.outliers.find(sample.index);if(outliers!=s.outliers.end())for(double outlier:outliers->second){const double py=forge_plot_axis_pixel(p,*ay,outlier);if(std::isfinite(py)){forge_plot_symbol(symbols,1,x,py,3);if(chosen)forge_plot_symbol(selected,1,x,py,6);if(s.spec.selectable){forge_plot_add_hit(p,s,sample.index,x,py,x,py,false,false);p.hits.back().hit.y=outlier;p.hits.back().hit.value=outlier;}}}}
                else{const double open=forge_plot_axis_pixel(p,*ay,pt.open),close=forge_plot_axis_pixel(p,*ay,pt.close);if(!std::isfinite(open+close))continue;if(s.spec.kind==4){std::string candle;forge_plot_rectangle(candle,x-w,std::min(open,close),2*w,std::max(1.,std::abs(close-open)));forge_plot_add_path(p,candle,color,pt.close>=pt.open?0:color,s.spec.line_width,ax->spec.panel,s.spec.layer);}else{forge_plot_line(errors,x-w,open,x,open);forge_plot_line(errors,x,close,x+w,close);}}
                if(s.spec.selectable)forge_plot_add_hit(p,s,sample.index,x-w,std::min(low,high),x+w,std::max(low,high),false,true);
                if(chosen)forge_plot_rectangle(selected,x-w,std::min(low,high),2*w,std::max(1.,std::abs(low-high)));
            }
            ++p.rendered_points;
            if(original_sample&&inside&&s.spec.kind!=6){const double xl=forge_plot_axis_pixel(p,*ax,pt.x-std::max(0.,pt.error_x_minus)),xr=forge_plot_axis_pixel(p,*ax,pt.x+std::max(0.,pt.error_x_plus));if((pt.error_x_minus>0||pt.error_x_plus>0)&&std::isfinite(xl+xr)){forge_plot_line(errors,xl,y,xr,y);forge_plot_line(errors,xl,y-4,xl,y+4);forge_plot_line(errors,xr,y-4,xr,y+4);}const double yl=forge_plot_axis_pixel(p,*ay,pt.y-std::max(0.,pt.error_y_minus)),yr=forge_plot_axis_pixel(p,*ay,pt.y+std::max(0.,pt.error_y_plus));if((pt.error_y_minus>0||pt.error_y_plus>0)&&std::isfinite(yl+yr)){forge_plot_line(errors,x,yl,x,yr);forge_plot_line(errors,x-4,yl,x+4,yl);forge_plot_line(errors,x-4,yr,x+4,yr);}}
        }
        fill_segment();forge_plot_add_path(p,filled,0,s.spec.fill_color,0,ax->spec.panel,s.spec.layer-1);
        forge_plot_add_path(p,path,color,s.spec.kind==2?color:s.spec.kind==3?s.spec.fill_color:0,s.spec.line_width,ax->spec.panel,s.spec.layer);
        forge_plot_add_path(p,errors,color,0,s.spec.line_width,ax->spec.panel,s.spec.layer+1);
        forge_plot_add_path(p,symbols,color,s.spec.scatter_style==2?color:0,s.spec.line_width,ax->spec.panel,s.spec.layer+2);
        forge_plot_add_path(p,selected,p.options.selection_color>>24?p.options.selection_color:0xffffb020,0,s.spec.line_width+2,ax->spec.panel,s.spec.layer+3);
    }
    return true;
}

static uint32_t forge_plot_crc(const uint8_t* data,size_t n) {
    uint32_t crc=0xffffffffu;for(size_t i=0;i<n;++i){crc^=data[i];for(int bit=0;bit<8;++bit)crc=(crc>>1)^((crc&1)?0xedb88320u:0);}return ~crc;
}
static void forge_plot_u32(std::vector<uint8_t>& out,uint32_t v) {out.push_back(static_cast<uint8_t>(v>>24));out.push_back(static_cast<uint8_t>(v>>16));out.push_back(static_cast<uint8_t>(v>>8));out.push_back(static_cast<uint8_t>(v));}
static void forge_plot_png_chunk(std::vector<uint8_t>& png,const char* type,const std::vector<uint8_t>& data) {
    forge_plot_u32(png,static_cast<uint32_t>(data.size()));const size_t start=png.size();png.insert(png.end(),type,type+4);png.insert(png.end(),data.begin(),data.end());forge_plot_u32(png,forge_plot_crc(png.data()+start,png.size()-start));
}
static std::vector<uint8_t> forge_plot_png(int width,int height,const std::vector<uint8_t>& rgba) {
    std::vector<uint8_t> png{137,80,78,71,13,10,26,10},header;forge_plot_u32(header,width);forge_plot_u32(header,height);header.insert(header.end(),{8,6,0,0,0});forge_plot_png_chunk(png,"IHDR",header);
    std::vector<uint8_t> raw;raw.reserve(static_cast<size_t>(height)*(1+width*4));for(int y=0;y<height;++y){raw.push_back(0);raw.insert(raw.end(),rgba.begin()+static_cast<size_t>(y)*width*4,rgba.begin()+static_cast<size_t>(y+1)*width*4);}
    std::vector<uint8_t> compressed{0x78,0x01};for(size_t start=0;start<raw.size();){const size_t count=std::min<size_t>(65535,raw.size()-start);compressed.push_back(start+count==raw.size()?1:0);compressed.push_back(static_cast<uint8_t>(count));compressed.push_back(static_cast<uint8_t>(count>>8));const uint16_t inverse=static_cast<uint16_t>(~count);compressed.push_back(static_cast<uint8_t>(inverse));compressed.push_back(static_cast<uint8_t>(inverse>>8));compressed.insert(compressed.end(),raw.begin()+start,raw.begin()+start+count);start+=count;}
    uint32_t a=1,b=0;for(uint8_t v:raw){a=(a+v)%65521;b=(b+a)%65521;}forge_plot_u32(compressed,(b<<16)|a);forge_plot_png_chunk(png,"IDAT",compressed);forge_plot_png_chunk(png,"IEND",{});return png;
}
static std::string forge_plot_base64(const std::vector<uint8_t>& data) {
    static const char chars[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";std::string out;out.reserve((data.size()+2)/3*4);
    for(size_t i=0;i<data.size();i+=3){const uint32_t value=(static_cast<uint32_t>(data[i])<<16)|(i+1<data.size()?static_cast<uint32_t>(data[i+1])<<8:0)|(i+2<data.size()?data[i+2]:0);out+=chars[(value>>18)&63];out+=chars[(value>>12)&63];out+=i+1<data.size()?chars[(value>>6)&63]:'=';out+=i+2<data.size()?chars[value&63]:'=';}return out;
}
static bool forge_plot_decode_image(ForgePlotCoreItemState& image) {
    if(image.spec.image_pixels&&image.spec.image_width>0&&image.spec.image_height>0&&image.spec.image_width<=8192&&image.spec.image_height<=8192&&static_cast<size_t>(image.spec.image_width)*image.spec.image_height*4==image.spec.image_byte_count){image.image_width=image.spec.image_width;image.image_height=image.spec.image_height;image.pixels.assign(image.spec.image_pixels,image.spec.image_pixels+image.spec.image_byte_count);image.spec.image_pixels=nullptr;return true;}
    const auto comma=image.image.find(',');if(image.image.rfind("data:image/",0)!=0||comma==std::string::npos||image.image.substr(0,comma).find(";base64")==std::string::npos)return false;
#ifdef _WIN32
    std::vector<BYTE> data;uint32_t value=0;int bits=0;const std::string alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    if(image.image.size()>96*1024*1024)return false;
    for(size_t i=comma+1;i<image.image.size();++i){const char c=image.image[i];if(c=='=')break;if(c=='\r'||c=='\n'||c==' ')continue;const auto index=alphabet.find(c);if(index==std::string::npos)return false;value=(value<<6)|static_cast<uint32_t>(index);bits+=6;if(bits>=8){bits-=8;data.push_back(static_cast<BYTE>(value>>bits));}}
    if(data.empty())return false;const HRESULT initialized=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);if(FAILED(initialized)&&initialized!=RPC_E_CHANGED_MODE)return false;
    bool valid=false;
    {Microsoft::WRL::ComPtr<IWICImagingFactory> factory;Microsoft::WRL::ComPtr<IWICStream> stream;Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
        HRESULT result=CoCreateInstance(CLSID_WICImagingFactory,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(factory.GetAddressOf()));if(SUCCEEDED(result))result=factory->CreateStream(stream.GetAddressOf());if(SUCCEEDED(result))result=stream->InitializeFromMemory(data.data(),static_cast<DWORD>(data.size()));if(SUCCEEDED(result))result=factory->CreateDecoderFromStream(stream.Get(),nullptr,WICDecodeMetadataCacheOnLoad,decoder.GetAddressOf());if(SUCCEEDED(result))result=decoder->GetFrame(0,frame.GetAddressOf());if(SUCCEEDED(result))result=factory->CreateFormatConverter(converter.GetAddressOf());if(SUCCEEDED(result))result=converter->Initialize(frame.Get(),GUID_WICPixelFormat32bppRGBA,WICBitmapDitherTypeNone,nullptr,0,WICBitmapPaletteTypeCustom);
        UINT width=0,height=0;if(SUCCEEDED(result))result=converter->GetSize(&width,&height);if(SUCCEEDED(result)&&width&&height&&width<=8192&&height<=8192){image.pixels.resize(static_cast<size_t>(width)*height*4);result=converter->CopyPixels(nullptr,width*4,static_cast<UINT>(image.pixels.size()),image.pixels.data());if(SUCCEEDED(result)){image.image_width=static_cast<int>(width);image.image_height=static_cast<int>(height);valid=true;}}
    }
    if(SUCCEEDED(initialized))CoUninitialize();return valid;
#else
    return false;
#endif
}
extern "C" size_t forge_plot_core_image_data(const char* uri,uint8_t* data,size_t capacity) {
    if(!uri)return 0;try{const std::string text(uri);const auto comma=text.find(',');if(text.size()>96*1024*1024||text.rfind("data:image/",0)!=0||comma==std::string::npos||text.substr(0,comma).find(";base64")==std::string::npos)return 0;
        uint32_t value=0;int bits=0;size_t count=0;const std::string alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        for(size_t i=comma+1;i<text.size();++i){const char c=text[i];if(c=='=')break;if(c=='\r'||c=='\n'||c==' ')continue;const auto index=alphabet.find(c);if(index==std::string::npos)return 0;value=(value<<6)|static_cast<uint32_t>(index);bits+=6;if(bits>=8){bits-=8;if(data&&count<capacity)data[count]=static_cast<uint8_t>(value>>bits);++count;}}return count;
    }catch(...){return 0;}
}
static uint32_t forge_plot_gradient(double t,int palette) {
    t=std::clamp(t,0.,1.);std::array<std::array<double,3>,5> colors{};
    if(palette==1)colors={{{0,0,0},{.6,0,.2},{1,.2,0},{1,.8,0},{1,1,1}}};
    else if(palette==2)colors={{{0,0,0},{.25,.25,.25},{.5,.5,.5},{.75,.75,.75},{1,1,1}}};
    else if(palette==3)colors={{{.1,.25,.8},{.45,.65,1},{1,1,1},{1,.55,.45},{.8,.1,.1}}};
    else colors={{{.267,.005,.329},{.23,.322,.545},{.128,.567,.551},{.369,.789,.383},{.993,.906,.144}}};
    const size_t i=std::min<size_t>(3,static_cast<size_t>(t*4));const double f=t*4-i;uint32_t c=0xff000000u;for(int k=0;k<3;++k)c|=static_cast<uint32_t>(std::round((colors[i][k]*(1-f)+colors[i+1][k]*f)*255))<<(16-8*k);return c;
}
static bool forge_plot_map_range(const ForgePlotCoreMapState& m,double& low,double& high) {
    low=m.spec.z_min;high=m.spec.z_max;
    if(m.spec.auto_range){low=std::numeric_limits<double>::infinity();high=-low;for(double v:m.values)if(std::isfinite(v)&&(!m.spec.logarithmic||v>0)){low=std::min(low,v);high=std::max(high,v);}if(!std::isfinite(low)){low=0;high=1;return false;}if(low==high){if(m.spec.logarithmic){low/=2;high*=2;}else{low-=.5;high+=.5;}}}
    return std::isfinite(low)&&std::isfinite(high)&&low<high&&(!m.spec.logarithmic||low>0);
}
static bool forge_plot_draw_maps(ForgePlotCoreState& p) {
    for(const auto& m:p.maps)if(!m.spec.hidden) {
        const auto* ax=forge_plot_axis_find(p,m.spec.x_axis);const auto* ay=forge_plot_axis_find(p,m.spec.y_axis);const auto* panel=forge_plot_panel_find(p,m.spec.panel);
        if(!ax||!ay||!panel||ax->spec.panel!=m.spec.panel||ay->spec.panel!=m.spec.panel||ax->spec.side>=2||ay->spec.side<2||m.spec.columns<=0||m.spec.rows<=0||static_cast<size_t>(m.spec.columns)*m.spec.rows!=m.values.size()||!std::isfinite(m.spec.x_min+m.spec.x_max+m.spec.y_min+m.spec.y_max)||m.spec.x_min>=m.spec.x_max||m.spec.y_min>=m.spec.y_max){p.error="色图维度、数据长度、坐标或绘图区无效";return false;}
        double lo,hi;if(!forge_plot_map_range(m,lo,hi)){if(!m.spec.auto_range){p.error="色范围须有限且有序，对数色范围须为正";return false;}continue;}
        const auto& r=panel->rect;int width=std::clamp(static_cast<int>(std::ceil(r.width)),1,2048),height=std::clamp(static_cast<int>(std::ceil(r.height)),1,2048);
        ForgePlotCoreDraw d;d.panel=m.spec.panel;d.layer=m.spec.layer;d.clip=true;d.x=r.x;d.y=r.y;d.image_width=r.width;d.image_height=r.height;
        if(!m.spec.interpolate&&ax->spec.scale==0&&ay->spec.scale==0&&m.spec.columns<=width&&m.spec.rows<=height){
            // 线性小色图直接复用源单元大小，避免为每个屏幕像素重复编码同一颜色。
            const double x1=forge_plot_axis_pixel(p,*ax,m.spec.x_min),x2=forge_plot_axis_pixel(p,*ax,m.spec.x_max),y1=forge_plot_axis_pixel(p,*ay,m.spec.y_min),y2=forge_plot_axis_pixel(p,*ay,m.spec.y_max);
            d.x=std::min(x1,x2);d.y=std::min(y1,y2);d.image_width=std::abs(x2-x1);d.image_height=std::abs(y2-y1);d.nearest=true;
            width=m.spec.columns;height=m.spec.rows;
        }
        d.raster_width=width;d.raster_height=height;d.raster.resize(static_cast<size_t>(width)*height*4);
        for(int y=0;y<height;++y)for(int x=0;x<width;++x){const double vx=forge_plot_axis_value(p,*ax,d.x+(x+.5)*d.image_width/width),vy=forge_plot_axis_value(p,*ay,d.y+(y+.5)*d.image_height/height);if(vx<m.spec.x_min||vx>m.spec.x_max||vy<m.spec.y_min||vy>m.spec.y_max)continue;
            const double fx=(vx-m.spec.x_min)/(m.spec.x_max-m.spec.x_min)*m.spec.columns-.5,fy=(vy-m.spec.y_min)/(m.spec.y_max-m.spec.y_min)*m.spec.rows-.5;
            const int ix=std::clamp(static_cast<int>(std::floor(fx)),0,m.spec.columns-1),iy=std::clamp(static_cast<int>(std::floor(fy)),0,m.spec.rows-1);double v=m.values[static_cast<size_t>(iy)*m.spec.columns+ix];
            if(m.spec.interpolate){const int jx=std::min(ix+1,m.spec.columns-1),jy=std::min(iy+1,m.spec.rows-1);const double tx=std::clamp(fx-ix,0.,1.),ty=std::clamp(fy-iy,0.,1.);const double values[]{v,m.values[static_cast<size_t>(iy)*m.spec.columns+jx],m.values[static_cast<size_t>(jy)*m.spec.columns+ix],m.values[static_cast<size_t>(jy)*m.spec.columns+jx]};const double weights[]{(1-tx)*(1-ty),tx*(1-ty),(1-tx)*ty,tx*ty};v=0;for(int k=0;k<4;++k)if(weights[k]>1e-12){if(!std::isfinite(values[k])){v=std::numeric_limits<double>::quiet_NaN();break;}v+=values[k]*weights[k];}}
            else{const int cx=std::clamp(static_cast<int>(std::floor(fx+.5)),0,m.spec.columns-1),cy=std::clamp(static_cast<int>(std::floor(fy+.5)),0,m.spec.rows-1);v=m.values[static_cast<size_t>(cy)*m.spec.columns+cx];}
            if(!std::isfinite(v)||(m.spec.logarithmic&&v<=0))continue;const double t=m.spec.logarithmic?(std::log(v)-std::log(lo))/(std::log(hi)-std::log(lo)):(v-lo)/(hi-lo);const uint32_t color=forge_plot_gradient(t,m.spec.palette);const size_t k=(static_cast<size_t>(y)*width+x)*4;d.raster[k]=static_cast<uint8_t>(color>>16);d.raster[k+1]=static_cast<uint8_t>(color>>8);d.raster[k+2]=static_cast<uint8_t>(color);d.raster[k+3]=255;
        }
        d.image="data:image/png;base64,"+forge_plot_base64(forge_plot_png(width,height,d.raster));p.draw.push_back(std::move(d));p.rendered_points+=static_cast<uint64_t>(std::count_if(m.values.begin(),m.values.end(),[&](double value){return std::isfinite(value)&&(!m.spec.logarithmic||value>0);}));
        if(m.spec.color_scale){for(int i=0;i<64;++i){std::string stripe;forge_plot_rectangle(stripe,r.x+r.width+12,r.y+r.height*(1-(i+1)/64.),16,r.height/64.+.2);forge_plot_add_path(p,stripe,0,forge_plot_gradient(i/63.,m.spec.palette),0,m.spec.panel,50,false);}forge_plot_add_text(p,r.x+r.width+32,r.y+10,forge_plot_number(hi,2),p.options.foreground,p.options.font_size);forge_plot_add_text(p,r.x+r.width+32,r.y+r.height,forge_plot_number(lo,2),p.options.foreground,p.options.font_size);}
    }return true;
}

static std::pair<double,double> forge_plot_item_point(const ForgePlotCoreState& p,const ForgePlotItemCore& s,double x,double y) {
    const auto* panel=forge_plot_panel_find(p,s.panel);if(!panel)return {NAN,NAN};
    if(s.coordinates==2)return {x,y};if(s.coordinates==1)return {panel->rect.x+x*panel->rect.width,panel->rect.y+y*panel->rect.height};
    const auto* ax=forge_plot_axis_find(p,s.x_axis);const auto* ay=forge_plot_axis_find(p,s.y_axis);if(!ax||!ay||ax->spec.side>=2||ay->spec.side<2||ax->spec.panel!=s.panel||ay->spec.panel!=s.panel)return {NAN,NAN};if(panel->polar)return forge_plot_polar_point(*panel,*ax,*ay,x,y);return {forge_plot_axis_pixel(p,*ax,x),forge_plot_axis_pixel(p,*ay,y)};
}
static bool forge_plot_draw_items(ForgePlotCoreState& p) {
    std::map<int,std::pair<double,double>> anchors;std::map<int,int> ids;
    for(const auto& item:p.items)if(ids[item.spec.id]++){p.error="图元 ID 重复";return false;}
    std::vector<bool> done(p.items.size(),false);size_t remaining=p.items.size();
    for(size_t round=0;round<=p.items.size()&&remaining;++round)for(size_t i=0;i<p.items.size();++i)if(!done[i]) {
        const auto& item=p.items[i];const auto& s=item.spec;if(s.anchor>=0&&!anchors.count(s.anchor))continue;
        if(s.coordinates<0||s.coordinates>2||!std::isfinite(s.width)||!std::isfinite(s.size)){p.error="图元坐标类型、尺寸无效";return false;}
        auto a=forge_plot_item_point(p,s,s.x,s.y),b=forge_plot_item_point(p,s,s.x2,s.y2),c=forge_plot_item_point(p,s,s.x3,s.y3),d=forge_plot_item_point(p,s,s.x4,s.y4);
        if(s.kind==0||s.kind==7)b=a;
        if(s.anchor>=0){const auto parent=anchors[s.anchor];const double dx=parent.first-a.first,dy=parent.second-a.second;a=parent;b.first+=dx;b.second+=dy;c.first+=dx;c.second+=dy;d.first+=dx;d.second+=dy;}
        if(!std::isfinite(a.first+a.second+b.first+b.second)){p.error="图元坐标、坐标轴或对数域无效";return false;}
        std::string path;double left=std::min(a.first,b.first),top=std::min(a.second,b.second),w=std::abs(a.first-b.first),h=std::abs(a.second-b.second);
        if(s.kind==0)forge_plot_add_text(p,a.first,a.second,item.text,s.color,s.size,s.layer,s.panel,s.clip!=0);
        else if(s.kind==1||s.kind==2){forge_plot_line(path,a.first,a.second,b.first,b.second);if(s.kind==2){const double angle=std::atan2(b.second-a.second,b.first-a.first),size=s.size>0?s.size:10;forge_plot_line(path,b.first,b.second,b.first-size*std::cos(angle-.45),b.second-size*std::sin(angle-.45));forge_plot_line(path,b.first,b.second,b.first-size*std::cos(angle+.45),b.second-size*std::sin(angle+.45));}}
        else if(s.kind==3)forge_plot_rectangle(path,left,top,w,h);
        else if(s.kind==4)forge_plot_ellipse(path,left+w/2,top+h/2,w/2,h/2);
        else if(s.kind==5){if(!std::isfinite(c.first+c.second+d.first+d.second)){p.error="曲线控制点无效";return false;}forge_plot_path_point(path,'M',a.first,a.second);path+='C';path+=forge_plot_number(c.first)+' '+forge_plot_number(c.second)+' '+forge_plot_number(d.first)+' '+forge_plot_number(d.second)+' '+forge_plot_number(b.first)+' '+forge_plot_number(b.second)+' ';}
        else if(s.kind==6){forge_plot_line(path,a.first,a.second,a.first,a.second+8);forge_plot_line(path,a.first,a.second+8,b.first,b.second+8);forge_plot_line(path,b.first,b.second+8,b.first,b.second);}
        else if(s.kind==7)forge_plot_symbol(path,8,a.first,a.second,s.size>0?s.size:6);
        else if(s.kind==8){if(item.pixels.empty()){p.error="图片图元须使用可解码的 PNG/JPEG/BMP base64 URI";return false;}ForgePlotCoreDraw image;image.x=left;image.y=top;image.image_width=w;image.image_height=h;image.image=item.image;image.raster_width=item.image_width;image.raster_height=item.image_height;image.raster=item.pixels;image.panel=s.panel;image.layer=s.layer;image.clip=s.clip!=0;p.draw.push_back(std::move(image));}
        else{p.error="图元类型无效";return false;}
        forge_plot_add_path(p,path,s.color,s.fill_color,s.width,s.panel,s.layer,s.clip!=0);
        const int part=std::clamp(s.anchor_part,0,8);anchors[s.id]={left+w*(part%3)/2.,top+h*(part/3)/2.};
        if(s.selectable){ForgePlotCoreHitState hit;hit.hit.kind=1;hit.hit.id=s.id;hit.hit.index=-1;hit.hit.panel=s.panel;hit.hit.pixel_x=left;hit.hit.pixel_y=top;hit.hit.x=s.x;hit.hit.y=s.y;hit.hit.label=item.text.c_str();hit.x2=left+w;hit.y2=top+h;hit.rect=true;hit.clip=s.clip!=0;hit.radius=std::max(4.,s.size*.5);
            if(s.kind==0){hit.hit.pixel_x=a.first;hit.hit.pixel_y=a.second-s.size;hit.x2=a.first+item.text.size()*s.size*.6;hit.y2=a.second+s.size*.2;}
            if(s.kind==1||s.kind==2){hit.hit.pixel_x=a.first;hit.hit.pixel_y=a.second;hit.x2=b.first;hit.y2=b.second;hit.rect=false;hit.segment=true;}
            if(s.kind==7){hit.hit.pixel_x=a.first;hit.hit.pixel_y=a.second;hit.x2=a.first;hit.y2=a.second;hit.rect=false;}
            p.hits.push_back(hit);}
        done[i]=true;--remaining;
    }
    if(remaining){p.error="图元锚点缺失或存在循环";return false;}return true;
}

static void forge_plot_draw_legend(ForgePlotCoreState& p) {
    auto layout=forge_plot_legend_layout(p);if(layout.entries.empty())return;
    layout.x=p.legend_area.x;layout.y=p.legend_area.y;
    const bool clip=p.options.legend_placement!=1;const auto& rect=clip?layout.rect:p.legend_area;
    if(p.options.legend_style==1){std::string box;forge_plot_rectangle(box,layout.x,layout.y,layout.width,layout.height);forge_plot_add_path(p,box,p.options.grid_color,0,1,rect.id,99,clip);}
    for(const auto& entry:layout.entries){const auto& s=p.series[entry.index];const double x=layout.x+entry.x,y=layout.y+entry.y;
        const auto* style=entry.point>=0?forge_plot_point_style(s,entry.point):nullptr;const bool hidden=s.spec.hidden||(style&&style->hidden);
        const uint32_t color=hidden?((p.options.foreground&0xffffff)|0x66000000):entry.point>=0?(style&&(style->color>>24)?style->color:forge_plot_palette(entry.point)):s.spec.color>>24?s.spec.color:forge_plot_palette(entry.index);
        std::string mark;forge_plot_line(mark,x,y-4,x+22,y-4);forge_plot_add_path(p,mark,color,0,3,rect.id,100,clip);forge_plot_add_text(p,x+28,y,entry.text,hidden?color:p.options.foreground,p.options.font_size,100,rect.id,clip);
        ForgePlotCoreHitState hit;hit.clip=clip;hit.hit.kind=2;hit.hit.id=s.spec.id;hit.hit.index=entry.point;hit.hit.panel=rect.id;hit.hit.pixel_x=std::max(x,rect.x);hit.hit.pixel_y=std::max(y-p.options.font_size-4,rect.y);hit.hit.label=s.name.c_str();hit.x2=std::min(x+entry.width,rect.x+rect.width);hit.y2=std::min(y+4,rect.y+rect.height);hit.rect=true;if(hit.x2>hit.hit.pixel_x&&hit.y2>hit.hit.pixel_y)p.hits.push_back(hit);
    }
}
static std::string forge_plot_serialize_svg(const ForgePlotCoreState& p) {
    std::unique_ptr<ForgePlotLegendMetrics> font_metrics;
    std::string out="<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" width=\""+forge_plot_number(p.options.width)+"\" height=\""+forge_plot_number(p.options.height)+"\" viewBox=\"0 0 "+forge_plot_number(p.options.width)+' '+forge_plot_number(p.options.height)+"\"><defs>";
    for(const auto& panel:p.panels){const auto& r=panel.rect;out+="<clipPath id=\"p"+std::to_string(panel.spec.id)+"\">";if(panel.polar)out+="<circle cx=\""+forge_plot_number(r.x+r.width*.5)+"\" cy=\""+forge_plot_number(r.y+r.height*.5)+"\" r=\""+forge_plot_number(std::min(r.width,r.height)*.5)+"\"/>";else out+="<rect x=\""+forge_plot_number(r.x)+"\" y=\""+forge_plot_number(r.y)+"\" width=\""+forge_plot_number(r.width)+"\" height=\""+forge_plot_number(r.height)+"\"/>";out+="</clipPath>";}
    out+="</defs><g shape-rendering=\""+std::string(p.options.antialias?"geometricPrecision":"crispEdges")+"\"><rect width=\"100%\" height=\"100%\" fill=\""+forge_plot_color(p.options.background)+"\" fill-opacity=\""+forge_plot_number((p.options.background>>24)/255.)+"\"/>";
    for(const auto& d:p.draw){if(d.clip)out+="<g clip-path=\"url(#p"+std::to_string(d.panel)+")\">";
        if(!d.path.empty())out+="<path d=\""+d.path+"\" stroke=\""+(d.stroke>>24?forge_plot_color(d.stroke):"none")+"\" stroke-opacity=\""+forge_plot_number((d.stroke>>24)/255.)+"\" stroke-width=\""+forge_plot_number(d.width)+"\" stroke-linejoin=\"round\" stroke-linecap=\"round\" fill=\""+(d.fill>>24?forge_plot_color(d.fill):"none")+"\" fill-opacity=\""+forge_plot_number((d.fill>>24)/255.)+"\"/>";
        if(!d.text.empty()){
            auto family=p.font;
            // Slint 的 SVG 回退仅查询通用族；缺字时显式选择系统中文字体。
            if(std::any_of(d.text.begin(),d.text.end(),[](unsigned char c){return c>=128;})){if(!font_metrics)font_metrics=std::make_unique<ForgePlotLegendMetrics>(p.font);if(!font_metrics->supports(d.text)||((p.font.rfind("Segoe UI",0)==0||p.font.rfind("Consolas",0)==0)&&forge_plot_has_cjk(d.text)))family="Microsoft YaHei UI, Microsoft YaHei, Noto Sans CJK SC, sans-serif";}
            out+="<text x=\""+forge_plot_number(d.x)+"\" y=\""+forge_plot_number(d.y)+"\" transform=\"rotate("+forge_plot_number(d.rotation)+' '+forge_plot_number(d.x)+' '+forge_plot_number(d.y)+")\" fill=\""+forge_plot_color(d.fill)+"\" fill-opacity=\""+forge_plot_number((d.fill>>24)/255.)+"\" font-family=\""+forge_plot_xml(family)+"\" font-size=\""+forge_plot_number(d.size)+"\">"+forge_plot_xml(d.text)+"</text>";
        }
        if(!d.image.empty())out+="<image x=\""+forge_plot_number(d.x)+"\" y=\""+forge_plot_number(d.y)+"\" width=\""+forge_plot_number(d.image_width)+"\" height=\""+forge_plot_number(d.image_height)+"\" preserveAspectRatio=\"none\" image-rendering=\""+(d.nearest?std::string("optimizeSpeed"):std::string("optimizeQuality"))+"\" xlink:href=\""+forge_plot_xml(d.image)+"\"/>";
        if(d.clip)out+="</g>";
    }return out+"</g></svg>";
}

static int64_t forge_plot_cell(int x,int y) {return static_cast<int64_t>((static_cast<uint64_t>(static_cast<uint32_t>(x))<<32)|static_cast<uint32_t>(y));}
static void forge_plot_index_hits(ForgePlotCoreState& p) {
    for(size_t i=0;i<p.hits.size();++i){const auto& h=p.hits[i];const double r=h.radius;const int x1=static_cast<int>(std::floor(std::clamp(std::min(h.hit.pixel_x,h.x2)-r,-64.,p.options.width+64.)/32)),x2=static_cast<int>(std::floor(std::clamp(std::max(h.hit.pixel_x,h.x2)+r,-64.,p.options.width+64.)/32)),y1=static_cast<int>(std::floor(std::clamp(std::min(h.hit.pixel_y,h.y2)-r,-64.,p.options.height+64.)/32)),y2=static_cast<int>(std::floor(std::clamp(std::max(h.hit.pixel_y,h.y2)+r,-64.,p.options.height+64.)/32));
        for(int y=y1;y<=y2;++y)for(int x=x1;x<=x2;++x)p.hit_cells[forge_plot_cell(x,y)].push_back(i);
    }
}
static double forge_plot_hit_distance(const ForgePlotCoreHitState& h,double x,double y) {
    if(h.sector){const double r=std::hypot(x-h.cx,y-h.cy);if(r<h.inner||r>h.outer)return INFINITY;double a=std::fmod(std::atan2(y-h.cy,x-h.cx)-h.start+4*forge_plot_pi,2*forge_plot_pi);return a<=h.sweep+1e-9?0:INFINITY;}
    if(h.rect){const double dx=std::max({h.hit.pixel_x-x,0.,x-h.x2}),dy=std::max({h.hit.pixel_y-y,0.,y-h.y2});return std::hypot(dx,dy);}
    if(h.segment){const double vx=h.x2-h.hit.pixel_x,vy=h.y2-h.hit.pixel_y,den=vx*vx+vy*vy,t=den>0?std::clamp(((x-h.hit.pixel_x)*vx+(y-h.hit.pixel_y)*vy)/den,0.,1.):0;return std::hypot(x-h.hit.pixel_x-t*vx,y-h.hit.pixel_y-t*vy);}
    return std::max(0.,std::hypot(x-h.hit.pixel_x,y-h.hit.pixel_y)-h.radius);
}

static std::string forge_plot_pdf_utf16(const std::string& s) {
    std::string out;for(size_t i=0;i<s.size();){uint32_t c=static_cast<unsigned char>(s[i++]);int n=0;if(c>=0xf0){c&=7;n=3;}else if(c>=0xe0){c&=15;n=2;}else if(c>=0xc0){c&=31;n=1;}for(int j=0;j<n&&i<s.size();++j)c=(c<<6)|(static_cast<unsigned char>(s[i++])&63);char buf[12];if(c<=0xffff){std::snprintf(buf,sizeof(buf),"%04x",c);out+=buf;}else{c-=0x10000;std::snprintf(buf,sizeof(buf),"%04x%04x",0xd800+(c>>10),0xdc00+(c&1023));out+=buf;}}return out;
}
static std::string forge_plot_pdf_path(const std::string& path) {
    std::istringstream stream(path);stream.imbue(std::locale::classic());std::string out;char cmd;double x,y;
    while(stream>>cmd){if(cmd=='Z'){out+="h\n";continue;}if(cmd=='M'||cmd=='L'){if(!(stream>>x>>y))break;out+=forge_plot_number(x)+' '+forge_plot_number(y)+(cmd=='M'?" m\n":" l\n");}
        else if(cmd=='C'){for(int i=0;i<3;++i){if(!(stream>>x>>y))return out;out+=forge_plot_number(x)+' '+forge_plot_number(y)+' ';}out+="c\n";}else break;
    }
    return out;
}
static std::string forge_plot_pdf_rgb(uint32_t color) {return forge_plot_number(((color>>16)&255)/255.,4)+' '+forge_plot_number(((color>>8)&255)/255.,4)+' '+forge_plot_number((color&255)/255.,4);}
#ifdef _WIN32
static std::wstring forge_plot_wide(const std::string& text) {
    const int count=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,text.data(),static_cast<int>(text.size()),nullptr,0);if(!count)return {};
    std::wstring out(count,L'\0');MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,text.data(),static_cast<int>(text.size()),out.data(),count);return out;
}
struct ForgePlotPdfGlyph { std::string path; double advance=0; bool valid=false; };
class ForgePlotPdfFont {
    HDC dc_=nullptr;HFONT font_=nullptr;HFONT fallback_=nullptr;HGDIOBJ previous_=nullptr;std::map<wchar_t,ForgePlotPdfGlyph> glyphs_;
public:
    explicit ForgePlotPdfFont(std::string family){const auto comma=family.find(',');family=family.substr(0,comma);family.erase(std::remove(family.begin(),family.end(),'\''),family.end());family.erase(std::remove(family.begin(),family.end(),'"'),family.end());if(family.empty()||family=="sans-serif")family="Microsoft YaHei";
        dc_=CreateCompatibleDC(nullptr);if(dc_){font_=CreateFontW(-128,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_TT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH,forge_plot_wide(family).c_str());if(font_)previous_=SelectObject(dc_,font_);}}
    ~ForgePlotPdfFont(){if(dc_&&previous_)SelectObject(dc_,previous_);if(font_)DeleteObject(font_);if(fallback_)DeleteObject(fallback_);if(dc_)DeleteDC(dc_);}
    const ForgePlotPdfGlyph& glyph(wchar_t character){const auto found=glyphs_.find(character);if(found!=glyphs_.end())return found->second;ForgePlotPdfGlyph glyph;if(!dc_||!font_)return glyphs_.emplace(character,std::move(glyph)).first->second;
        SelectObject(dc_,font_);WORD index=0xffff;
        if(GetGlyphIndicesW(dc_,&character,1,&index,GGI_MARK_NONEXISTING_GLYPHS)==GDI_ERROR||index==0xffff){
            if(!fallback_)fallback_=CreateFontW(-128,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_TT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH,L"Microsoft YaHei UI");
            if(fallback_)SelectObject(dc_,fallback_);
            if(GetGlyphIndicesW(dc_,&character,1,&index,GGI_MARK_NONEXISTING_GLYPHS)==GDI_ERROR||index==0xffff)return glyphs_.emplace(character,std::move(glyph)).first->second;
        }
        MAT2 matrix{};matrix.eM11.value=1;matrix.eM22.value=1;GLYPHMETRICS metrics{};const DWORD bytes=GetGlyphOutlineW(dc_,character,GGO_BEZIER,&metrics,0,nullptr,&matrix);glyph.advance=metrics.gmCellIncX;
        if(bytes!=GDI_ERROR&&bytes<16*1024*1024){std::vector<uint8_t> data(bytes);if(!bytes||GetGlyphOutlineW(dc_,character,GGO_BEZIER,&metrics,bytes,data.data(),&matrix)!=GDI_ERROR){glyph.valid=true;const auto point=[](const POINTFX& value){return std::make_pair(value.x.value+value.x.fract/65536.,-(value.y.value+value.y.fract/65536.));};
            size_t offset=0;while(offset+sizeof(TTPOLYGONHEADER)<=data.size()){const auto* header=reinterpret_cast<const TTPOLYGONHEADER*>(data.data()+offset);if(header->cb<sizeof(TTPOLYGONHEADER)||offset+header->cb>data.size()){glyph.valid=false;break;}const auto start=point(header->pfxStart);forge_plot_path_point(glyph.path,'M',start.first,start.second);size_t curve_offset=offset+sizeof(TTPOLYGONHEADER);
                while(curve_offset+offsetof(TTPOLYCURVE,apfx)<=offset+header->cb){const auto* curve=reinterpret_cast<const TTPOLYCURVE*>(data.data()+curve_offset);const size_t curve_bytes=offsetof(TTPOLYCURVE,apfx)+sizeof(POINTFX)*curve->cpfx;if(curve_offset+curve_bytes>offset+header->cb||!curve->cpfx){glyph.valid=false;break;}
                    if(curve->wType==TT_PRIM_LINE){for(WORD i=0;i<curve->cpfx;++i){const auto v=point(curve->apfx[i]);forge_plot_path_point(glyph.path,'L',v.first,v.second);}}
                    else if(curve->wType==TT_PRIM_CSPLINE&&curve->cpfx%3==0){for(WORD i=0;i<curve->cpfx;i+=3){glyph.path+='C';for(WORD k=0;k<3;++k){const auto v=point(curve->apfx[i+k]);glyph.path+=forge_plot_number(v.first)+' '+forge_plot_number(v.second)+' ';}}}
                    else{glyph.valid=false;break;}curve_offset+=curve_bytes;
                }glyph.path+="Z ";offset+=header->cb;
            }
        }}return glyphs_.emplace(character,std::move(glyph)).first->second;
    }
    std::string outline(const ForgePlotCoreDraw& draw){const auto text=forge_plot_wide(draw.text);if(text.empty())return {};std::string content;double x=draw.x;const double scale=draw.size/128.;
        for(wchar_t c:text){const auto& g=glyph(c);if(!g.valid)return {};if(!g.path.empty())content+="q "+forge_plot_number(scale,6)+" 0 0 "+forge_plot_number(scale,6)+' '+forge_plot_number(x)+' '+forge_plot_number(draw.y)+" cm\n"+forge_plot_pdf_path(g.path)+"f Q\n";x+=g.advance*scale;}return content;
    }
};
#endif
static std::string forge_plot_serialize_pdf(const ForgePlotCoreState& p) {
    std::vector<std::string> objects(7);std::string content="q\n1 0 0 -1 0 "+forge_plot_number(p.options.height)+" cm\n";
    content+="/GSBackground gs\n"+forge_plot_pdf_rgb(p.options.background)+" rg\n0 0 "+forge_plot_number(p.options.width)+' '+forge_plot_number(p.options.height)+" re f\n/GSReset gs\n";
    std::map<int,int> alpha_ids;std::string resources="/Font << /F1 4 0 R /F2 << /Type /Font /Subtype /Type1 /BaseFont /Helvetica >> >> /XObject << ";int image_index=0;
#ifdef _WIN32
    ForgePlotPdfFont outline_font(p.font);
#endif
    for(const auto& d:p.draw) {
        content+="q\n";
        if(d.clip){const auto* panel=forge_plot_panel_find(p,d.panel);if(panel){const auto& r=panel->rect;if(panel->polar){std::string circle;forge_plot_ellipse(circle,r.x+r.width*.5,r.y+r.height*.5,std::min(r.width,r.height)*.5,std::min(r.width,r.height)*.5);content+=forge_plot_pdf_path(circle)+" W n\n";}else content+=forge_plot_number(r.x)+' '+forge_plot_number(r.y)+' '+forge_plot_number(r.width)+' '+forge_plot_number(r.height)+" re W n\n";}}
        const int alpha=static_cast<int>(((d.stroke>>24)<<8)|(!d.image.empty()?255:d.fill>>24));if(!alpha_ids.count(alpha))alpha_ids[alpha]=static_cast<int>(alpha_ids.size());content+="/GS"+std::to_string(alpha_ids[alpha])+" gs\n";
        if(!d.path.empty()){content+=forge_plot_pdf_rgb(d.stroke)+" RG\n"+forge_plot_pdf_rgb(d.fill)+" rg\n"+forge_plot_number(d.width)+" w\n1 J 1 j\n"+forge_plot_pdf_path(d.path);content+=(d.stroke>>24)?(d.fill>>24?"B\n":"S\n"):"f\n";}
        if(!d.text.empty()){
            if(d.rotation!=0){const double angle=d.rotation*forge_plot_pi/180,c=std::cos(angle),s=std::sin(angle);content+=forge_plot_number(c)+' '+forge_plot_number(s)+' '+forge_plot_number(-s)+' '+forge_plot_number(c)+' '+forge_plot_number(d.x-c*d.x+s*d.y)+' '+forge_plot_number(d.y-s*d.x-c*d.y)+" cm\n";}
            std::string outline;
#ifdef _WIN32
            outline=outline_font.outline(d);
#endif
            if(!outline.empty())content+=forge_plot_pdf_rgb(d.fill)+" rg\n"+outline;
            else{const bool ascii=std::all_of(d.text.begin(),d.text.end(),[](unsigned char c){return c<128;});std::string encoded;if(ascii){for(unsigned char c:d.text){char byte[3];std::snprintf(byte,sizeof(byte),"%02x",c);encoded+=byte;}}else encoded=forge_plot_pdf_utf16(d.text);content+=forge_plot_pdf_rgb(d.fill)+" rg\nBT "+std::string(ascii?"/F2 ":"/F1 ")+forge_plot_number(d.size)+" Tf 1 0 0 -1 "+forge_plot_number(d.x)+' '+forge_plot_number(d.y)+" Tm <"+encoded+"> Tj ET\n";}
        }
        if(!d.raster.empty()) {
            const int image_object=static_cast<int>(objects.size())+1;const int mask_object=image_object+1;std::string rgb,mask;rgb.reserve(d.raster.size()/4*3);mask.reserve(d.raster.size()/4);
            for(size_t i=0;i<d.raster.size();i+=4){rgb.append(reinterpret_cast<const char*>(d.raster.data()+i),3);mask.push_back(static_cast<char>(d.raster[i+3]));}
            objects.push_back("<< /Type /XObject /Subtype /Image /Width "+std::to_string(d.raster_width)+" /Height "+std::to_string(d.raster_height)+" /ColorSpace /DeviceRGB /BitsPerComponent 8 /SMask "+std::to_string(mask_object)+" 0 R /Length "+std::to_string(rgb.size())+" >>\nstream\n"+rgb+"\nendstream");
            objects.push_back("<< /Type /XObject /Subtype /Image /Width "+std::to_string(d.raster_width)+" /Height "+std::to_string(d.raster_height)+" /ColorSpace /DeviceGray /BitsPerComponent 8 /Length "+std::to_string(mask.size())+" >>\nstream\n"+mask+"\nendstream");
            resources+="/Im"+std::to_string(image_index)+' '+std::to_string(image_object)+" 0 R ";content+=forge_plot_number(d.image_width)+" 0 0 -"+forge_plot_number(d.image_height)+' '+forge_plot_number(d.x)+' '+forge_plot_number(d.y+d.image_height)+" cm /Im"+std::to_string(image_index)+" Do\n";++image_index;
        }
        content+="Q\n";
    }
    content+="Q\n";resources+=">> /ExtGState << /GSBackground << /CA 1 /ca "+forge_plot_number((p.options.background>>24)/255.)+" >> /GSReset << /CA 1 /ca 1 >> ";for(const auto& alpha:alpha_ids)resources+="/GS"+std::to_string(alpha.second)+" << /CA "+forge_plot_number((alpha.first>>8)/255.)+" /ca "+forge_plot_number((alpha.first&255)/255.)+" >> ";resources+=">>";
    objects[0]="<< /Type /Catalog /Pages 2 0 R >>";objects[1]="<< /Type /Pages /Kids [3 0 R] /Count 1 >>";
    objects[2]="<< /Type /Page /Parent 2 0 R /MediaBox [0 0 "+forge_plot_number(p.options.width)+' '+forge_plot_number(p.options.height)+"] /Resources << "+resources+" >> /Contents 7 0 R >>";
    objects[3]="<< /Type /Font /Subtype /Type0 /BaseFont /STSong-Light /Encoding /UniGB-UCS2-H /DescendantFonts [5 0 R] >>";
    objects[4]="<< /Type /Font /Subtype /CIDFontType0 /BaseFont /STSong-Light /CIDSystemInfo << /Registry (Adobe) /Ordering (GB1) /Supplement 4 >> /FontDescriptor 6 0 R /DW 1000 /W [1 [500 500 500 500 500 500 500 500 500 500]] >>";
    objects[5]="<< /Type /FontDescriptor /FontName /STSong-Light /Flags 6 /FontBBox [0 -200 1000 900] /ItalicAngle 0 /Ascent 880 /Descent -120 /CapHeight 700 /StemV 80 >>";
    objects[6]="<< /Length "+std::to_string(content.size())+" >>\nstream\n"+content+"endstream";
    std::string out="%PDF-1.4\n%\xe2\xe3\xcf\xd3\n";std::vector<size_t> offsets{0};for(size_t i=0;i<objects.size();++i){offsets.push_back(out.size());out+=std::to_string(i+1)+" 0 obj\n"+objects[i]+"\nendobj\n";}
    const size_t xref=out.size();out+="xref\n0 "+std::to_string(offsets.size())+"\n0000000000 65535 f \n";for(size_t i=1;i<offsets.size();++i){char line[32];std::snprintf(line,sizeof(line),"%010llu 00000 n \n",static_cast<unsigned long long>(offsets[i]));out+=line;}out+="trailer\n<< /Size "+std::to_string(offsets.size())+" /Root 1 0 R >>\nstartxref\n"+std::to_string(xref)+"\n%%EOF\n";return out;
}

// C ABI 不向调用端传播 C++ 异常。输入错误保存在场景状态中。
static void forge_plot_error(ForgePlotCoreState* p,const char* message) {if(p){p->status=2;p->error=message;}}
extern "C" void* forge_plot_core_new(const ForgePlotOptionsCore* options) {
    if(!options)return nullptr;try{auto p=std::make_unique<ForgePlotCoreState>();p->options=*options;p->title=forge_plot_string(options->title);p->font=forge_plot_font_families(forge_plot_string(options->font_family));return p.release();}catch(...){return nullptr;}
}
extern "C" void forge_plot_core_free(void* plot) {delete static_cast<ForgePlotCoreState*>(plot);}
extern "C" void forge_plot_core_begin_view(void* plot,const ForgePlotOptionsCore* options) {
    auto* p=static_cast<ForgePlotCoreState*>(plot);if(!p||!options)return;try{p->options=*options;p->title=forge_plot_string(options->title);p->font=forge_plot_font_families(forge_plot_string(options->font_family));p->axes.clear();p->status=0;p->error.clear();}catch(...){forge_plot_error(p,"视图更新失败");}
}
extern "C" void forge_plot_core_panel(void* plot,const ForgePlotPanelCore* panel) {
    auto* p=static_cast<ForgePlotCoreState*>(plot);if(!p||!panel)return;try{ForgePlotCorePanelState s;s.spec=*panel;s.title=forge_plot_string(panel->title);if(panel->categories)for(size_t i=0;i<panel->category_count;++i)s.categories.push_back(forge_plot_string(panel->categories[i].label));s.spec.categories=nullptr;p->panels.push_back(std::move(s));}catch(...){forge_plot_error(p,"绘图区分配失败");}
}
extern "C" void forge_plot_core_axis(void* plot,const ForgePlotAxisCore* axis) {
    auto* p=static_cast<ForgePlotCoreState*>(plot);if(!p||!axis)return;try{ForgePlotCoreAxisState s;s.spec=*axis;s.label=forge_plot_string(axis->label);s.format=forge_plot_string(axis->format);if(axis->ticks)for(size_t i=0;i<axis->tick_count;++i)s.ticks.emplace_back(axis->ticks[i].value,forge_plot_string(axis->ticks[i].label));s.spec.ticks=nullptr;p->axes.push_back(std::move(s));}catch(...){forge_plot_error(p,"坐标轴分配失败");}
}
extern "C" void forge_plot_core_series(void* plot,const ForgePlotSeriesCore* series,const ForgePlotPointCore* points,size_t count) {
    auto* p=static_cast<ForgePlotCoreState*>(plot);if(!p||!series||(!points&&count))return;try{ForgePlotCoreSeriesState s;s.spec=*series;s.name=forge_plot_string(series->name);forge_plot_copy_styles(s);if(count)s.points.assign(points,points+count);double previous=-std::numeric_limits<double>::infinity();
        for(size_t i=0;i<count;++i){auto& pt=s.points[i];if(pt.label&&*pt.label)s.labels[i]=pt.label;if(pt.outliers&&pt.outlier_count)s.outliers[i].assign(pt.outliers,pt.outliers+pt.outlier_count);pt.label=nullptr;pt.outliers=nullptr;if(!std::isfinite(pt.x)){s.binary_searchable=false;previous=-std::numeric_limits<double>::infinity();}else{if(pt.x<previous)s.monotone=false;previous=pt.x;}}
        // 折线按键值稳定排序；参数曲线保留顺序。断点隔开的各段独立排序，原始索引不变。
        forge_plot_series_order(s);
        for(size_t i=0;i<count;++i)forge_plot_series_bounds(s,i);
        p->input_points+=count;p->series.push_back(std::move(s));
    }catch(...){forge_plot_error(p,"系列数据分配失败");}
}
extern "C" int forge_plot_core_append(void* plot,int series_id,const ForgePlotPointCore* points,size_t count) {
    auto* p=static_cast<ForgePlotCoreState*>(plot);if(!p||(!points&&count))return 0;
    const auto found=std::find_if(p->series.begin(),p->series.end(),[&](const auto& s){return s.spec.id==series_id;});if(found==p->series.end())return 0;
    try{
        auto& s=*found;const size_t first=s.points.size();if(!count)return 1;
        s.points.insert(s.points.end(),points,points+count);
        double previous=first?s.points[first-1].x:-INFINITY;
        for(size_t i=first;i<s.points.size();++i){auto& pt=s.points[i];if(pt.label&&*pt.label)s.labels[i]=pt.label;if(pt.outliers&&pt.outlier_count)s.outliers[i].assign(pt.outliers,pt.outliers+pt.outlier_count);pt.label=nullptr;pt.outliers=nullptr;
            if(!std::isfinite(pt.x)){s.binary_searchable=false;previous=-INFINITY;}else{if(pt.x<previous)s.monotone=false;previous=pt.x;}forge_plot_series_bounds(s,i);
        }
        // 有序采集只处理新点；乱序图形保持稳定排序和原始索引语义。
        forge_plot_series_order(s);
        p->input_points+=count;return 1;
    }catch(...){forge_plot_error(p,"追加系列数据分配失败");return 0;}
}
extern "C" int forge_plot_core_update_series(void* plot,const ForgePlotSeriesCore* series) {
    auto* p=static_cast<ForgePlotCoreState*>(plot);if(!p||!series)return 0;
    try{for(auto& s:p->series)if(s.spec.id==series->id){const bool kind_changed=s.spec.kind!=series->kind;const bool bounds_changed=kind_changed||s.spec.bar_width!=series->bar_width||s.spec.baseline!=series->baseline;
        s.spec=*series;s.name=forge_plot_string(series->name);forge_plot_copy_styles(s);if(kind_changed)forge_plot_series_order(s);if(bounds_changed){s.x_bounds={};s.y_bounds={};s.sample_blocks={};s.simple_samples=true;s.latest_valid=std::numeric_limits<size_t>::max();for(size_t i=0;i<s.points.size();++i)forge_plot_series_bounds(s,i);}return 1;}
    }catch(...){forge_plot_error(p,"更新系列属性失败");}return 0;
}
extern "C" void forge_plot_core_map(void* plot,const ForgePlotMapCore* map,const double* values,size_t count) {
    auto* p=static_cast<ForgePlotCoreState*>(plot);if(!p||!map||(!values&&count))return;try{ForgePlotCoreMapState s;s.spec=*map;s.name=forge_plot_string(map->name);if(count)s.values.assign(values,values+count);p->input_points+=count;p->maps.push_back(std::move(s));}catch(...){forge_plot_error(p,"色图数据分配失败");}
}
extern "C" void forge_plot_core_item(void* plot,const ForgePlotItemCore* item) {
    auto* p=static_cast<ForgePlotCoreState*>(plot);if(!p||!item)return;try{ForgePlotCoreItemState s;s.spec=*item;s.text=forge_plot_string(item->text);s.image=forge_plot_string(item->image);if(item->kind==8&&!forge_plot_decode_image(s)){forge_plot_error(p,"图片图元解码失败，须使用 PNG/JPEG/BMP base64 URI");return;}p->items.push_back(std::move(s));}catch(...){forge_plot_error(p,"图元分配失败");}
}
extern "C" void forge_plot_core_selection(void* plot,const ForgePlotSelectionCore* selection,size_t count) {
    auto* p=static_cast<ForgePlotCoreState*>(plot);if(!p||(!selection&&count))return;try{p->selection.clear();if(count)p->selection.assign(selection,selection+count);std::erase_if(p->selection,[](const auto& s){return s.first<0||s.last<s.first;});std::sort(p->selection.begin(),p->selection.end(),[](const auto& a,const auto& b){return std::tie(a.id,a.first)<std::tie(b.id,b.first);});std::vector<ForgePlotSelectionCore> normalized;for(const auto& s:p->selection){if(!normalized.empty()&&normalized.back().id==s.id&&static_cast<int64_t>(normalized.back().last)+1>=s.first)normalized.back().last=std::max(normalized.back().last,s.last);else normalized.push_back(s);}p->selection=std::move(normalized);}catch(...){forge_plot_error(p,"选择区间分配失败");}
}
extern "C" const char* forge_plot_core_render(void* plot) {
    auto* p=static_cast<ForgePlotCoreState*>(plot);if(!p)return "";try{
        if(p->status==2)return "";p->draw.clear();p->hits.clear();p->hit_cells.clear();p->rendered_points=0;
        std::map<int,int> ids;for(const auto& s:p->series)if(ids[s.spec.id]++){forge_plot_error(p,"系列 ID 重复");return "";}
        if(!forge_plot_layout(*p)||!forge_plot_ranges(*p)||!forge_plot_draw_maps(*p)||!forge_plot_draw_series(*p)||!forge_plot_draw_items(*p)){p->status=2;return "";}
        forge_plot_draw_axes(*p);forge_plot_draw_legend(*p);std::stable_sort(p->draw.begin(),p->draw.end(),[](const auto& a,const auto& b){return a.layer<b.layer;});forge_plot_index_hits(*p);p->svg=forge_plot_serialize_svg(*p);p->status=p->rendered_points||!p->items.empty()?0:1;return p->svg.c_str();
    }catch(...){forge_plot_error(p,"场景计算或绘制分配失败");return "";}
}
extern "C" ForgePlotInfoCore forge_plot_core_info(const void* plot) {
    const auto* p=static_cast<const ForgePlotCoreState*>(plot);if(!p)return {2,0,0,0,0,"场景创建失败"};return {p->status,p->input_points,p->rendered_points,p->draw.size(),p->svg.size(),p->error.c_str()};
}
extern "C" size_t forge_plot_core_ranges(const void* plot,ForgePlotRangeCore* ranges,size_t capacity) {
    const auto* p=static_cast<const ForgePlotCoreState*>(plot);if(!p)return 0;if(ranges)for(size_t i=0;i<std::min(capacity,p->axes.size());++i){const auto& a=p->axes[i];ranges[i]={a.spec.id,a.spec.scale,a.spec.reversed,a.minimum,a.maximum};}return p->axes.size();
}
extern "C" size_t forge_plot_core_rects(const void* plot,ForgePlotRectCore* rects,size_t capacity) {
    const auto* p=static_cast<const ForgePlotCoreState*>(plot);if(!p)return 0;if(rects)for(size_t i=0;i<std::min(capacity,p->panels.size());++i)rects[i]=p->panels[i].rect;return p->panels.size();
}
extern "C" ForgePlotRectCore forge_plot_core_annotation_rect(const void* plot,int kind) {
    const auto* p=static_cast<const ForgePlotCoreState*>(plot);if(!p||kind<0||kind>1)return {-1,0,0,0,0};return kind==0?p->legend_area:p->measurement_area;
}
extern "C" size_t forge_plot_core_domains(const void* plot,ForgePlotRangeCore* ranges,size_t capacity) {
    const auto* p=static_cast<const ForgePlotCoreState*>(plot);if(!p)return 0;if(ranges)for(size_t i=0;i<std::min(capacity,p->axes.size());++i){const auto& a=p->axes[i];ranges[i]={a.spec.id,a.spec.scale,a.spec.reversed,a.domain_minimum,a.domain_maximum};}return p->axes.size();
}
extern "C" ForgePlotHitCore forge_plot_core_measure(const void* plot,int series_id,double x) {
    ForgePlotHitCore out{};out.kind=-1;out.id=-1;out.index=-1;const auto* p=static_cast<const ForgePlotCoreState*>(plot);if(!p||!std::isfinite(x))return out;
    for(const auto& s:p->series)if(s.spec.id==series_id&&!s.points.empty()&&s.spec.kind<8&&s.spec.kind!=6){
        size_t index=0;if(s.monotone&&s.binary_searchable){const auto it=std::lower_bound(s.points.begin(),s.points.end(),x,[](const auto& pt,double v){return pt.x<v;});index=static_cast<size_t>(it-s.points.begin());if(index==s.points.size())--index;else if(index&&std::abs(s.points[index-1].x-x)<=std::abs(s.points[index].x-x))--index;}
        else{double distance=INFINITY;for(size_t i=0;i<s.points.size();++i)if(forge_plot_point_valid(s,s.points[i])&&std::abs(s.points[i].x-x)<distance){index=i;distance=std::abs(s.points[i].x-x);}}
        const auto& pt=s.points[index];if(forge_plot_point_hidden(s,index)||!forge_plot_point_valid(s,pt))return out;const auto* ax=forge_plot_axis_find(*p,s.spec.x_axis);const auto* ay=forge_plot_axis_find(*p,s.spec.y_axis);if(!ax||!ay)return out;
        out.kind=0;out.id=series_id;out.index=static_cast<int>(index);out.panel=ax->spec.panel;out.x=pt.x;out.y=s.spec.kind==4||s.spec.kind==5?pt.close:pt.y;out.value=out.y;out.pixel_x=forge_plot_axis_pixel(*p,*ax,out.x);out.pixel_y=forge_plot_axis_pixel(*p,*ay,out.y);out.label=s.name.c_str();return out;
    }return out;
}
extern "C" double forge_plot_core_coordinate(const void* plot,int axis,double pixel,int inverse) {
    const auto* p=static_cast<const ForgePlotCoreState*>(plot);if(!p||!std::isfinite(pixel))return NAN;const auto* a=forge_plot_axis_find(*p,axis);return !a?NAN:inverse?forge_plot_axis_value(*p,*a,pixel):forge_plot_axis_pixel(*p,*a,pixel);
}
extern "C" ForgePlotRangeCore forge_plot_core_view(const void* plot,const ForgePlotRangeCore* baseline,int mode,double position,double amount,double endpoint) {
    if(!baseline)return {};auto result=*baseline;const auto* p=static_cast<const ForgePlotCoreState*>(plot);if(!p||!std::isfinite(position+amount+endpoint))return result;
    const auto* axis=forge_plot_axis_find(*p,baseline->id);if(!axis)return result;const auto* panel=forge_plot_panel_find(*p,axis->spec.panel);if(!panel||panel->system!=0)return result;const auto& r=panel->rect;
    const bool horizontal=axis->spec.side<2;const double span=horizontal?r.width:-r.height,origin=horizontal?r.x:r.y+r.height;
    const double lo=forge_plot_transform(baseline->minimum,baseline->scale),hi=forge_plot_transform(baseline->maximum,baseline->scale);double low=lo,high=hi;
    const auto transformed=[&](double pixel){double t=(pixel-origin)/span;if(baseline->reversed)t=1-t;return lo+t*(hi-lo);};
    if(mode==0){const double shift=-amount/span*(hi-lo)*(baseline->reversed?-1:1);low+=shift;high+=shift;}
    else if(mode==1){const double center=transformed(position),factor=std::clamp(amount,.01,100.);low=center+(lo-center)*factor;high=center+(hi-center)*factor;}
    else if(mode==2){low=std::min(transformed(position),transformed(endpoint));high=std::max(transformed(position),transformed(endpoint));}
    const double minimum=forge_plot_inverse(low,baseline->scale,baseline->minimum),maximum=forge_plot_inverse(high,baseline->scale,baseline->minimum);
    if(std::isfinite(minimum)&&std::isfinite(maximum)&&minimum<maximum&&(!baseline->scale||((minimum>0&&maximum>0)||(minimum<0&&maximum<0)))){result.minimum=minimum;result.maximum=maximum;}return result;
}
extern "C" ForgePlotHitCore forge_plot_core_hit(const void* plot,double x,double y,double tolerance) {
    ForgePlotHitCore result{-1,-1,-1,-1,0,0,0,0,0,0,""};const auto* p=static_cast<const ForgePlotCoreState*>(plot);if(!p||p->status==2||!std::isfinite(x+y+tolerance))return result;
    for(const auto& panel:p->panels){const auto& r=panel.rect;if(forge_plot_inside(panel,x,y)){result.panel=panel.spec.id;result.pixel_x=x;result.pixel_y=y;bool got_x=false,got_y=false;for(const auto& a:p->axes)if(a.spec.panel==panel.spec.id){
        double t;if(panel.polar){if(a.spec.side<2){t=std::atan2(r.y+r.height*.5-y,x-r.x-r.width*.5)/(2*forge_plot_pi);if(t<0)t+=1;}else t=std::hypot(x-r.x-r.width*.5,y-r.y-r.height*.5)/(std::min(r.width,r.height)*.5);if(a.spec.reversed)t=1-t;const double lo=forge_plot_transform(a.minimum,a.spec.scale),hi=forge_plot_transform(a.maximum,a.spec.scale);t=forge_plot_inverse(lo+t*(hi-lo),a.spec.scale,a.minimum);}
        else t=forge_plot_axis_value(*p,a,a.spec.side<2?x:y);
        if(a.spec.side<2&&!got_x){result.x=t;got_x=true;}else if(a.spec.side>=2&&!got_y){result.y=t;got_y=true;}
    }break;}}
    if(tolerance<0)return result;
    tolerance=std::clamp(tolerance,0.,64.);double nearest=tolerance+1e-6,nearest_center=std::numeric_limits<double>::infinity();int nearest_rank=-1;const int radius=static_cast<int>(std::ceil(tolerance/32));const int cx=static_cast<int>(std::floor(x/32)),cy=static_cast<int>(std::floor(y/32));
    for(int iy=cy-radius;iy<=cy+radius;++iy)for(int ix=cx-radius;ix<=cx+radius;++ix){const auto cell=p->hit_cells.find(forge_plot_cell(ix,iy));if(cell==p->hit_cells.end())continue;for(size_t index:cell->second){const auto& hit=p->hits[index];if(hit.clip&&hit.hit.panel>=0){const auto* panel=forge_plot_panel_find(*p,hit.hit.panel);if(!panel||!forge_plot_inside(*panel,x,y))continue;}const double distance=forge_plot_hit_distance(hit,x,y),center=std::hypot(x-hit.hit.pixel_x,y-hit.hit.pixel_y);const int rank=hit.hit.kind==2?6:hit.hit.kind==1?5:hit.hit.kind==4?0:hit.segment?1:hit.rect?2:3;const bool tie=std::abs(distance-nearest)<1e-6;if(distance<=tolerance&&(nearest_rank<0||distance<nearest-1e-6||(tie&&(rank>nearest_rank||(rank==nearest_rank&&center<nearest_center))))){result=hit.hit;result.distance=distance;nearest=distance;nearest_center=center;nearest_rank=rank;}}}
    if(result.kind>=0)return result;
    for(const auto& m:p->maps)if(!m.spec.hidden){const auto* ax=forge_plot_axis_find(*p,m.spec.x_axis);const auto* ay=forge_plot_axis_find(*p,m.spec.y_axis);const auto* panel=forge_plot_panel_find(*p,m.spec.panel);if(!ax||!ay||!panel||!forge_plot_inside(*panel,x,y))continue;const double vx=forge_plot_axis_value(*p,*ax,x),vy=forge_plot_axis_value(*p,*ay,y);if(vx<m.spec.x_min||vx>=m.spec.x_max||vy<m.spec.y_min||vy>=m.spec.y_max)continue;const int ix=static_cast<int>((vx-m.spec.x_min)/(m.spec.x_max-m.spec.x_min)*m.spec.columns),iy=static_cast<int>((vy-m.spec.y_min)/(m.spec.y_max-m.spec.y_min)*m.spec.rows);if(ix<0||ix>=m.spec.columns||iy<0||iy>=m.spec.rows)continue;const size_t index=static_cast<size_t>(iy)*m.spec.columns+ix;if(index>=m.values.size()||!std::isfinite(m.values[index])||(m.spec.logarithmic&&m.values[index]<=0))continue;return {3,m.spec.id,static_cast<int>(index),m.spec.panel,vx,vy,m.values[index],x,y,0,m.name.c_str()};}
    return result;
}
extern "C" size_t forge_plot_core_select(const void* plot,double x,double y,double width,double height,ForgePlotHitCore* hits,size_t capacity) {
    const auto* p=static_cast<const ForgePlotCoreState*>(plot);if(!p||p->status==2||!std::isfinite(x+y+width+height))return 0;
    try {
    const double left=std::min(x,x+width),right=std::max(x,x+width),top=std::min(y,y+height),bottom=std::max(y,y+height);size_t count=0;
    const auto key=[](int id,size_t index){return(static_cast<uint64_t>(static_cast<uint32_t>(id))<<32)|static_cast<uint32_t>(index);};
    std::unordered_map<uint64_t,std::pair<double,double>> shape_hits;
    std::unordered_set<int> shape_series;for(const auto& s:p->series)if((s.spec.kind>=2&&s.spec.kind<=5)||s.spec.kind>=7)shape_series.insert(s.spec.id);
    // 柱、统计箱和金融图以实际几何框选，金融点未使用的 y 不参与选择判断。
    for(const auto& h:p->hits)if(h.hit.kind==0&&shape_series.contains(h.hit.id)) {
        const auto* panel=forge_plot_panel_find(*p,h.hit.panel);if(!panel)continue;
        const double l=std::max(left,panel->rect.x),r=std::min(right,panel->rect.x+panel->rect.width),t=std::max(top,panel->rect.y),b=std::min(bottom,panel->rect.y+panel->rect.height);
        if(l>r||t>b)continue;
        if(h.sector){bool intersect=false;for(int j=0;j<=32&&!intersect;++j){const double a=h.start+h.sweep*j/32;for(double rr:{h.inner,(h.inner+h.outer)*.5,h.outer}){const double xx=h.cx+rr*std::cos(a),yy=h.cy+rr*std::sin(a);if(xx>=l&&xx<=r&&yy>=t&&yy<=b){intersect=true;break;}}}for(double xx:{l,r})for(double yy:{t,b})if(forge_plot_hit_distance(h,xx,yy)==0)intersect=true;if(intersect)shape_hits[key(h.hit.id,h.hit.index)]={h.hit.pixel_x,h.hit.pixel_y};}
        else if(h.rect&&h.hit.pixel_x<=r&&h.x2>=l&&h.hit.pixel_y<=b&&h.y2>=t)shape_hits[key(h.hit.id,h.hit.index)]={(h.hit.pixel_x+h.x2)*.5,(h.hit.pixel_y+h.y2)*.5};
        else if(!h.segment&&!h.rect&&h.hit.pixel_x>=l&&h.hit.pixel_x<=r&&h.hit.pixel_y>=t&&h.hit.pixel_y<=b)shape_hits[key(h.hit.id,h.hit.index)]={h.hit.pixel_x,h.hit.pixel_y};
    }
    for(const auto& s:p->series)if(!s.spec.hidden&&s.spec.selectable){
        const auto* ax=forge_plot_axis_find(*p,s.spec.x_axis);const auto* ay=forge_plot_axis_find(*p,s.spec.y_axis);if(!ax||!ay)continue;
        const auto* panel=forge_plot_panel_find(*p,ax->spec.panel);if(!panel)continue;
        for(size_t i=0;i<s.points.size();++i){
            const auto& pt=s.points[i];if(forge_plot_point_hidden(s,i)||!forge_plot_point_valid(s,pt))continue;
            const double value=s.spec.kind==4||s.spec.kind==5?pt.close:pt.y;
            double px=forge_plot_axis_pixel(*p,*ax,pt.x),py=forge_plot_axis_pixel(*p,*ay,value);bool selected=false;
            if((s.spec.kind>=2&&s.spec.kind<=5)||s.spec.kind>=7){const auto geometry=shape_hits.find(key(s.spec.id,i));if(geometry!=shape_hits.end()){selected=true;px=geometry->second.first;py=geometry->second.second;}}
            else{if(s.spec.kind==6){const auto point=forge_plot_polar_point(*panel,*ax,*ay,pt.x,pt.y);px=point.first;py=point.second;}selected=forge_plot_inside(*panel,px,py)&&px>=std::max(left,panel->rect.x)&&px<=std::min(right,panel->rect.x+panel->rect.width)&&py>=std::max(top,panel->rect.y)&&py<=std::min(bottom,panel->rect.y+panel->rect.height);}
            if(selected){if(hits&&count<capacity){const auto label=s.labels.find(i);hits[count]={0,s.spec.id,static_cast<int>(i),ax->spec.panel,pt.x,value,value,px,py,0,label==s.labels.end()?s.name.c_str():label->second.c_str()};}++count;}
        }
    }
    return count;
    }catch(...){return 0;}
}
extern "C" int forge_plot_core_export(const void* plot,const char* path,int format) {
    const auto* p=static_cast<const ForgePlotCoreState*>(plot);if(!p||p->status==2||!path||(format!=0&&format!=1))return 0;try{const auto content=format==0?p->svg:forge_plot_serialize_pdf(*p);const std::string_view utf8(path);std::ofstream out(std::filesystem::path(std::u8string(reinterpret_cast<const char8_t*>(utf8.data()),utf8.size())),std::ios::binary);out.write(content.data(),static_cast<std::streamsize>(content.size()));return out.good()?1:0;}catch(...){return 0;}
}
