// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "ForgePlotCore.h"
#include <slint.h>
#include <algorithm>
#include <array>
#include <deque>
#include <filesystem>
#include <map>
#include <memory>
#include <limits>
#include <string>
#include <tuple>
#include <vector>
#include <charconv>
#include <bit>

// 引擎拥有输入副本；add_* 的字符串和数组只需在本次调用期间有效。
class ForgePlot {
public:
    explicit ForgePlot(const ForgePlotOptionsCore& options):value_(forge_plot_core_new(&options)){}
    ~ForgePlot(){forge_plot_core_free(value_);}
    ForgePlot(const ForgePlot&)=delete;
    ForgePlot& operator=(const ForgePlot&)=delete;
    void add_axis(const ForgePlotAxisCore& v){forge_plot_core_axis(value_,&v);}
    void begin_view(const ForgePlotOptionsCore& v){forge_plot_core_begin_view(value_,&v);}
    void add_panel(const ForgePlotPanelCore& v){forge_plot_core_panel(value_,&v);}
    void add_series(const ForgePlotSeriesCore& v,const std::vector<ForgePlotPointCore>& points){forge_plot_core_series(value_,&v,points.data(),points.size());}
    bool append(int id,const std::vector<ForgePlotPointCore>& points){return forge_plot_core_append(value_,id,points.data(),points.size())!=0;}
    bool append_xy(int id,const std::vector<std::pair<double,double>>& points){std::vector<ForgePlotPointCore> data;data.reserve(points.size());for(const auto& [x,y]:points){ForgePlotPointCore pt{};pt.x=x;pt.y=y;data.push_back(pt);}return append(id,data);}
    bool update_series(const ForgePlotSeriesCore& v){return forge_plot_core_update_series(value_,&v)!=0;}
    void add_map(const ForgePlotMapCore& v,const std::vector<double>& values){forge_plot_core_map(value_,&v,values.data(),values.size());}
    void add_item(const ForgePlotItemCore& v);
    void set_selection(const std::vector<ForgePlotSelectionCore>& v){forge_plot_core_selection(value_,v.data(),v.size());}
    std::string render(){std::string svg=forge_plot_core_render(value_);if(svg!=svg_){svg_=std::move(svg);display_=slint::Image();display_scale_=0;}return info().status==2?std::string(info().error):"";}
    const std::string& svg() const{return svg_;}
    ForgePlotInfoCore info() const{return forge_plot_core_info(value_);}
    slint::Image image() const{return slint::Image::load_from_data(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(svg_.data()),svg_.size()),"svg");}
    slint::Image display_image(float scale) const;
    std::vector<ForgePlotRangeCore> ranges() const {std::vector<ForgePlotRangeCore> out(forge_plot_core_ranges(value_,nullptr,0));forge_plot_core_ranges(value_,out.data(),out.size());return out;}
    std::vector<ForgePlotRangeCore> domains() const {std::vector<ForgePlotRangeCore> out(forge_plot_core_domains(value_,nullptr,0));forge_plot_core_domains(value_,out.data(),out.size());return out;}
    ForgePlotHitCore measure(int series,double x) const{return forge_plot_core_measure(value_,series,x);}
    ForgePlotRectCore annotation_rect(int kind) const{return forge_plot_core_annotation_rect(value_,kind);}
    std::vector<ForgePlotRectCore> rects() const {std::vector<ForgePlotRectCore> out(forge_plot_core_rects(value_,nullptr,0));forge_plot_core_rects(value_,out.data(),out.size());return out;}
    ForgePlotHitCore hit(double x,double y,double tolerance) const{return forge_plot_core_hit(value_,x,y,tolerance);}
    double coordinate(int axis,double value,bool inverse) const{return forge_plot_core_coordinate(value_,axis,value,inverse?1:0);}
    ForgePlotRangeCore view(const ForgePlotRangeCore& range,int mode,double position,double amount) const{return forge_plot_core_view(value_,&range,mode,position,amount,amount);}
    std::vector<std::tuple<int,int,int>> select_rect(double x,double y,double width,double height) const {
        std::vector<ForgePlotHitCore> hits(forge_plot_core_select(value_,x,y,width,height,nullptr,0));forge_plot_core_select(value_,x,y,width,height,hits.data(),hits.size());std::vector<std::tuple<int,int,int>> out;
        for(const auto& h:hits){if(!out.empty()&&std::get<0>(out.back())==h.id&&std::get<2>(out.back())+1==h.index){std::get<2>(out.back())=h.index;}else out.emplace_back(h.id,h.index,h.index);}return out;
    }
    std::string export_file(const std::filesystem::path& path,std::string format) const;
private:
    void* value_=nullptr;
    std::string svg_;
    mutable slint::Image display_;
    mutable float display_scale_=0;
};
struct ForgePlotServiceState {
    int next=0;
    std::map<int,std::shared_ptr<ForgePlot>> plots;
    std::map<int,std::pair<std::array<std::shared_ptr<void>,4>,int>> inputs;
    std::map<int,std::vector<std::pair<int,std::shared_ptr<void>>>> point_inputs;
    int allocate(){return ++next;}
};
inline uint32_t forge_plot_ui_color(slint::Color color){return(static_cast<uint32_t>(color.alpha())<<24)|(static_cast<uint32_t>(color.red())<<16)|(static_cast<uint32_t>(color.green())<<8)|color.blue();}
inline double forge_plot_ui_number(const slint::SharedString& text){if(text.empty())return 0;const std::string_view value(text);double number=0;const auto result=std::from_chars(value.data(),value.data()+value.size(),number);return result.ec==std::errc{}&&result.ptr==value.data()+value.size()?number:std::numeric_limits<double>::quiet_NaN();}
std::vector<std::tuple<int,int,int>> forge_plot_merge_selection(const std::vector<std::tuple<int,int,int>>& current,const std::vector<std::tuple<int,int,int>>& next,int mode);

class ForgePlotBuffer {
public:
    explicit ForgePlotBuffer(size_t capacity):capacity_(std::max<size_t>(1,capacity)){}
    void push(double x,double y){if(points_.size()==capacity_)points_.pop_front();points_.emplace_back(x,y);}
    std::vector<std::pair<double,double>> snapshot()const{return {points_.begin(),points_.end()};}
    size_t size()const{return points_.size();}
private:
    size_t capacity_;
    std::deque<std::pair<double,double>> points_;
};

#include "ForgePlotBindings.hpp"
