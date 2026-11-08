// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

use serde_json::json;
use slintforge::flow::FlowDocument;

/// 独立消费项目复用此入口，确保能力通过公开接口和运行库提供。
pub fn run() -> Result<(), Box<dyn std::error::Error>> {
    let mut document = FlowDocument::new("graph")?;
    document.apply_checked(&json!({"op":"batch","commands":[
        {"op":"insert","node":{"id":"device-a","kind":"external-card","data":{"name":"外部设备"}}},
        {"op":"insert","node":{"id":"device-b","x":400}},
        {"op":"connect","id":"device-a","port":"out","target":"device-b","target_port":"in"}
    ]}))?;
    if document
        .downstream("device-a", false)?
        .as_array()
        .unwrap()
        .len()
        != 1
        || document.ports("device-b")?[0]["connection_count"] != 1
    {
        return Err("图查询失败".into());
    }
    let saved = document.export_json()?;
    document.undo()?;
    if !document
        .query("nodes", "", json!({}))?
        .as_array()
        .unwrap()
        .is_empty()
    {
        return Err("事务历史失败".into());
    }
    document.redo()?;
    if document.export_json()? != saved {
        return Err("重做内容不一致".into());
    }
    document.set_before_edit(|a| {
        if a["op"] == "delete" {
            Err("设备受保护".into())
        } else {
            Ok(())
        }
    });
    document.select("device-a", false)?;
    if document.apply_checked(&json!({"op":"delete"})).is_ok() || document.export_json()? != saved {
        return Err("业务审核失败".into());
    }
    let mut other = FlowDocument::new("graph")?;
    other.import_json(&saved)?;
    if other.export_json()? != saved {
        return Err("序列化失败".into());
    }
    document.select("device-b", true)?;
    let state: serde_json::Value = serde_json::from_str(&document.group_selection("公共分组")?)?;
    let world = document.node_world("device-b")?;
    if state["ok"] != true || world["x"].as_f64() != Some(400.) {
        return Err(format!("分组接口或世界坐标错误: state={state}, world={world}").into());
    }
    let group = document.query("selection", "", json!({}))?["nodes"][0]
        .as_str()
        .unwrap()
        .to_owned();
    document.copy_selection()?;
    let fragment = document.selection_fragment()?;
    let mut pasted = FlowDocument::new("graph")?;
    let state: serde_json::Value = serde_json::from_str(&pasted.paste_clipboard(100., 200., "")?)?;
    if state["ok"] != true
        || pasted
            .query("nodes", "", json!({}))?
            .as_array()
            .unwrap()
            .len()
            != 3
        || pasted
            .query("edges", "", json!({}))?
            .as_array()
            .unwrap()
            .len()
            != 1
    {
        return Err("跨文档剪贴板失败".into());
    }
    let saved_paste = pasted.export_json()?;
    pasted.undo()?;
    if !pasted
        .query("nodes", "", json!({}))?
        .as_array()
        .unwrap()
        .is_empty()
    {
        return Err("粘贴撤销失败".into());
    }
    pasted.redo()?;
    if pasted.export_json()? != saved_paste {
        return Err("粘贴重做失败".into());
    }
    let collapsed: serde_json::Value = serde_json::from_str(&document.toggle_collapse(&group)?)?;
    let ungrouped: serde_json::Value = serde_json::from_str(&document.ungroup(&group)?)?;
    if fragment["nodes"].as_array().unwrap().len() != 3
        || collapsed["ok"] != true
        || ungrouped["ok"] != true
    {
        return Err("分组组合接口失败".into());
    }
    document.select("device-a", false)?;
    document.select("device-b", true)?;
    let aligned: serde_json::Value = serde_json::from_str(&document.align_selected("top")?)?;
    let laid_out: serde_json::Value = serde_json::from_str(
        &document.layout_graph(&json!({"algorithm":"layered","direction":"TB"}))?,
    )?;
    let revealed: serde_json::Value = serde_json::from_str(&document.reveal("device-a")?)?;
    if aligned["ok"] != true
        || laid_out["ok"] != true
        || revealed["ok"] != true
        || document.search("DEVICE-A")?.as_array().unwrap().len() != 1
        || document.bounds(false, "device-a")?["width"]
            .as_f64()
            .unwrap()
            <= 0.
    {
        return Err("布局对齐导航公共接口失败".into());
    }
    document.select("device-b", true)?;
    let before_properties = document.export_json()?;
    let patched: serde_json::Value =
        serde_json::from_str(&document.patch_selected_nodes(&json!({"layout_fixed":true}))?)?;
    if patched["ok"] != true
        || document.query("node", "device-b", json!({}))?["layout_fixed"] != true
    {
        return Err("批量属性公共接口失败".into());
    }
    document.undo()?;
    if document.export_json()? != before_properties {
        return Err("批量属性撤销失败".into());
    }
    let edge=document.query("edges","",json!({}))?[0]["id"].as_str().unwrap().to_owned();
    let before_style=document.export_json()?;
    let changed: serde_json::Value=serde_json::from_str(&document.update_edge_appearance(&edge,&json!({"marker_start":"circle","marker_end":"diamond","animated":true,"animation_speed":90.,"curvature":0.65,"layer":8}))?)?;
    let styled: serde_json::Value=serde_json::from_str(&document.snapshot()?)?;
    if changed["ok"]!=true || styled["edges"][0]["start_marker"]!="circle" || styled["edges"][0]["motion_points"].as_array().unwrap().len()!=65 || styled["edges"][0]["layer"]!=8 {return Err("高级连线公共接口或视图契约失败".into());}
    document.undo()?;
    if document.export_json()?!=before_style {return Err("高级连线撤销失败".into());}
    document.mark_saved()?;
    document.patch_node("device-a",&json!({"title":"导出 <卡片>"}))?;
    if document.document_status()?["dirty"]!=true {return Err("未保存状态接口失败".into());}
    document.undo()?;
    if document.document_status()?["dirty"]!=false {return Err("保存点撤销接口失败".into());}
    let exported=document.export_svg_with(false,json!({"theme":"dark"}),|node|json!([{"type":"text","text":format!("{} <SDK>",node["id"].as_str().unwrap())}]))?;
    if !exported.contains("&lt;SDK&gt;") || document.document_status()?["dirty"]!=false {return Err("图片导出公共接口失败".into());}
    document.select("device-a",false)?;let keyboard_before=document.export_json()?;
    document.nudge_selected(1.,0.)?;document.undo()?;
    if document.export_json()?!=keyboard_before {return Err("键盘移动公共撤销失败".into());}
    document.select_next(false,false)?;
    if document.query("selection","",json!({}))?["nodes"][0]!="device-b" {return Err("键盘导航公共接口失败".into());}
    other.configure_history(2,4096)?;
    for i in 0..4 { other.patch_node("device-a",&json!({"title":i.to_string()}))?; }
    let history=other.history_status()?;
    if history["undo_count"]!=2 || history["history_bytes"].as_u64().unwrap()>4096 {return Err("历史预算公共接口失败".into());}
    other.undo()?;other.undo()?;other.undo()?;
    if other.query("node","device-a",json!({}))?["title"]!="1" {return Err("预算裁剪后撤销越界".into());}
    let history_bytes=other.history_status()?["history_bytes"].clone();other.redo()?;other.redo()?;
    if other.query("node","device-a",json!({}))?["title"]!="3" || other.history_status()?["history_bytes"]!=history_bytes {return Err("重做未复用历史存储".into());}
    other.configure_history(100,1024)?;other.update_data("device-a",&json!({"blob":"x".repeat(8192)}))?;
    if other.history_status()?["history_barriers"]!=1 || other.history_status()?["history_bytes"]!=0 {return Err("超大编辑历史边界失败".into());}
    println!("FLOW_HISTORY_SDK_OK host=rust budget=public undo_redo=bounded oversized=barrier");
    println!("FLOW_SDK_CONTRACT_OK host=rust batch=atomic history=one_step queries=public callbacks=veto documents=isolated groups=world_coordinates clipboard=cross_document layout=layered alignment=top navigation=search_bounds_reveal properties=atomic edge_presentation=markers_motion_curvature_layer");
    Ok(())
}
