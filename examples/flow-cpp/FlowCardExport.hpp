// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <nlohmann/json.hpp>

// 业务卡片在导出时提供静态图元；输入、按钮保留当前值，图片卡片保留矢量示意内容。
static std::string forge_flow_demo_export_card(const std::string& input) {
    const auto node=nlohmann::json::parse(input);const auto kind=node.value("kind",std::string());
    auto result=nlohmann::json::array();
    if(kind=="form"||kind=="auto-form") {
        result={{{"type","rect"},{"width",180},{"height",28},{"fill","#f8fafc"},{"stroke","#d9e2ee"},{"radius",4}},
            {{"type","text"},{"x",8},{"y",6},{"text",node["data"].value("text",std::string())},{"fill","#334155"}},
            {{"type","text"},{"x",8},{"y",39},{"text",std::string("提交 / ")+node["data"].value("mode",std::string("Manual"))}}};
    } else if(kind=="device") {
        const bool stopped=node["data"].value("status",std::string())=="stopped";
        result={{{"type","ellipse"},{"x",2},{"y",7},{"width",7},{"height",7},{"fill",stopped?"#94a3b8":"#11998a"}},
            {{"type","text"},{"x",17},{"y",3},{"text",stopped?"设备已停止":"设备在线 · 运行中"}},
            {{"type","text"},{"x",17},{"y",38},{"text",stopped?"启动设备":"停止设备"}}};
    } else if(kind=="image") {
        result={{{"type","rect"},{"width",192},{"height",70},{"radius",5},{"fill","#e6edf4"}},
            {{"type","rect"},{"x",30},{"y",6},{"width",132},{"height",58},{"radius",7},{"fill","#405a72"}},
            {{"type","rect"},{"x",42},{"y",16},{"width",63},{"height",35},{"radius",3},{"fill","#b6e4e3"}},
            {{"type","path"},{"points",nlohmann::json::array({{{"x",47},{"y",36}},{{"x",54},{"y",36}},{{"x",61},{"y",23}},{{"x",70},{"y",46}},{{"x",78},{"y",28}},{{"x",85},{"y",36}},{{"x",100},{"y",36}}})},{"stroke","#267c83"},{"stroke_width",2.5}},
            {{"type","ellipse"},{"x",118},{"y",20},{"width",9},{"height",9},{"fill","#67c69c"}},
            {{"type","ellipse"},{"x",138},{"y",20},{"width",9},{"height",9},{"fill","#f6cc6b"}},
            {{"type","rect"},{"x",118},{"y",39},{"width",33},{"height",3},{"fill","#a1b6c8"}},
            {{"type","rect"},{"x",118},{"y",46},{"width",33},{"height",3},{"fill","#a1b6c8"}}};
    } else return {};
    return result.dump();
}
