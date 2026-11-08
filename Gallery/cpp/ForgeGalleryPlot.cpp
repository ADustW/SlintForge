// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#include "ForgeGalleryPlot.hpp"
#include "ForgePlot.hpp"
#include <cmath>

void forge_gallery_connect_plot(const slint::ComponentHandle<GalleryWindow>& ui) {
    forge_bind_plot_services<ForgePlotServices, ForgePlotScene, ForgePlotAxis, ForgePlotRange,
        ForgePlotRect, ForgePlotHit, ForgePlotSelection, ForgePlotExportResult>(ui);
    const slint::ComponentWeakHandle<GalleryWindow> weak(ui);
    auto sampler = std::make_shared<slint::Timer>();
    auto clock=std::make_shared<double>(0);
    ui->global<GalleryPlotDemoModels>().on_sample_running([weak, sampler, clock](bool running) {
        if (!running) { sampler->stop(); return; }
        const auto started=std::chrono::steady_clock::now();const double base=*clock;
        sampler->start(slint::TimerMode::Repeated, std::chrono::milliseconds(33), [weak, clock, started, base]() {
            if (auto ui = weak.lock()) {
                const double target=base+std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count();std::vector<ForgePlotPoint> measured,reference;
                while(*clock+.01<=target){*clock+=.01;ForgePlotPoint pt{};pt.x=static_cast<float>(*clock);pt.y=static_cast<float>(std::sin(*clock*3)*2+5);measured.push_back(pt);pt.y=static_cast<float>(std::sin(*clock*1.5)+2);reference.push_back(pt);}
                if(!measured.empty()){(*ui)->invoke_append_plot_points(0,std::make_shared<slint::VectorModel<ForgePlotPoint>>(std::move(measured)));(*ui)->invoke_append_plot_points(1,std::make_shared<slint::VectorModel<ForgePlotPoint>>(std::move(reference)));}
            }
        });
    });
}
