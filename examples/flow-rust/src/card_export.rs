// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

use slintforge::flow::json::{self, Value};

// 业务控件导出为当前状态的静态图元；与 C++ 示例使用相同内容。
pub fn content(input: &str) -> String {
    let Ok(node)=json::from_str::<Value>(input) else {return String::new();};
    let data=&node["data"];
    let items=match node["kind"].as_str().unwrap_or_default() {
        "form" | "auto-form" => json::json!([
            {"type":"rect","width":180,"height":28,"fill":"#f8fafc","stroke":"#d9e2ee","radius":4},
            {"type":"text","x":8,"y":6,"text":data["text"].as_str().unwrap_or_default(),"fill":"#334155"},
            {"type":"text","x":8,"y":39,"text":format!("提交 / {}",data["mode"].as_str().unwrap_or("Manual"))}
        ]),
        "device" => {let stopped=data["status"]=="stopped";json::json!([
            {"type":"ellipse","x":2,"y":7,"width":7,"height":7,"fill":if stopped {"#94a3b8"}else{"#11998a"}},
            {"type":"text","x":17,"y":3,"text":if stopped {"设备已停止"}else{"设备在线 · 运行中"}},
            {"type":"text","x":17,"y":38,"text":if stopped {"启动设备"}else{"停止设备"}}
        ])},
        "image" => json::json!([
            {"type":"rect","width":192,"height":70,"radius":5,"fill":"#e6edf4"},
            {"type":"rect","x":30,"y":6,"width":132,"height":58,"radius":7,"fill":"#405a72"},
            {"type":"rect","x":42,"y":16,"width":63,"height":35,"radius":3,"fill":"#b6e4e3"},
            {"type":"path","points":[{"x":47,"y":36},{"x":54,"y":36},{"x":61,"y":23},{"x":70,"y":46},{"x":78,"y":28},{"x":85,"y":36},{"x":100,"y":36}],"stroke":"#267c83","stroke_width":2.5},
            {"type":"ellipse","x":118,"y":20,"width":9,"height":9,"fill":"#67c69c"},
            {"type":"ellipse","x":138,"y":20,"width":9,"height":9,"fill":"#f6cc6b"},
            {"type":"rect","x":118,"y":39,"width":33,"height":3,"fill":"#a1b6c8"},
            {"type":"rect","x":118,"y":46,"width":33,"height":3,"fill":"#a1b6c8"}
        ]),
        _ => return String::new(),
    };
    items.to_string()
}
