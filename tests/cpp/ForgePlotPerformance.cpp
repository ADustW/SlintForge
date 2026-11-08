// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#include "ForgePlotCore.h"
#include <chrono>
#include <cmath>
#include <iostream>
#include <vector>

// 固定尺寸、字体、数据与操作序列，迁移前后的原生内核用同一编译器独立运行。
int main(){
    const auto milliseconds=[](auto start){return std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();};
    std::cout<<"points,prepare_ms,interaction_ms\n";
    for(size_t count:{100000,1000000}){
        std::vector<ForgePlotPointCore> points(count);for(size_t i=0;i<count;++i){points[i].x=static_cast<double>(i);points[i].y=std::sin(i*.007)+(i%4096==0?4.:0.);}
        ForgePlotOptionsCore options{};options.width=960;options.height=540;options.font_size=14;options.font_family="Segoe UI";options.background=0xfff9f9f9;options.foreground=0xff202020;options.grid_color=0x20000000;options.selection_color=0xff005fb8;options.adaptive=1;options.antialias=1;options.max_points_per_pixel=4;
        auto started=std::chrono::steady_clock::now();void* plot=forge_plot_core_new(&options);ForgePlotSeriesCore series{};series.id=7;series.y_axis=1;series.line_style=1;series.line_width=2;series.fill_target=-1;series.stack_group=-1;series.selectable=1;series.color=0xff005fb8;forge_plot_core_series(plot,&series,points.data(),points.size());forge_plot_core_render(plot);
        const double prepare=milliseconds(started);if(forge_plot_core_info(plot).status!=0)return 2;
        started=std::chrono::steady_clock::now();
        for(int frame=0;frame<32;++frame){forge_plot_core_begin_view(plot,&options);ForgePlotAxisCore x{};x.id=0;x.visible=1;x.grid=1;x.minimum=count*.1+frame*100;x.maximum=x.minimum+count*.6;ForgePlotAxisCore y{};y.id=1;y.side=2;y.auto_range=1;y.visible=1;y.grid=1;forge_plot_core_axis(plot,&x);forge_plot_core_axis(plot,&y);
            ForgePlotPointCore extra{};extra.x=count+frame;extra.y=std::sin(frame*.3);forge_plot_core_append(plot,7,&extra,1);forge_plot_core_render(plot);forge_plot_core_hit(plot,100+frame*20,240,8);if(forge_plot_core_info(plot).status!=0)return 3;
        }
        std::cout<<count<<','<<prepare<<','<<milliseconds(started)/32<<'\n';forge_plot_core_free(plot);
    }
}
