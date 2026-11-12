// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

//! 图文档的 DLL 安全封装。图算法和历史只在共用核心内执行。
use crate::runtime::{self, RuntimeError};
use slintforge_runtime_api::{models::*, Arena, Wire};

pub struct FlowDocument {
    handle: u64,
    before_edit: Option<Box<dyn FnMut(&serde_json::Value) -> Result<(), String>>>,
    after_edit: Option<Box<dyn FnMut(&serde_json::Value, &serde_json::Value)>>,
}
/// 区分运行库错误、业务拒绝和无效响应，避免遗漏 JSON 中的 ok/error。
#[derive(Debug)]
pub enum FlowError {
    Runtime(RuntimeError),
    Rejected(String),
    InvalidResponse(String),
}
impl std::fmt::Display for FlowError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::Runtime(error) => write!(f, "{error}"),
            Self::Rejected(error) | Self::InvalidResponse(error) => write!(f, "{error}"),
        }
    }
}
impl std::error::Error for FlowError {}
impl FlowDocument {
    pub fn new(mode: &str) -> Result<Self, RuntimeError> {
        let mut arena = Arena::default();
        let mode = mode.to_owned();
        let args = ArgsFlowNew {
            mode: mode.view(&mut arena),
        };
        Ok(Self {
            handle: runtime::try_value(OP_FLOWNEW, &args)?,
            before_edit: None,
            after_edit: None,
        })
    }
    pub fn execute(&mut self, command: &str) -> Result<String, RuntimeError> {
        if self.before_edit.is_none() && self.after_edit.is_none() {
            return self.execute_native(command);
        }
        let action: serde_json::Value = serde_json::from_str(command).unwrap_or_default();
        let operation = action["op"].as_str().unwrap_or_default();
        let notify = !matches!(operation, "resolve" | "preview" | "query" | "refresh");
        if notify && operation != "cancel" && operation != "mark-saved" {
            if let Some(callback) = self.before_edit.as_mut() {
                if let Err(error) = callback(&action) {
                    if matches!(operation, "drag-end" | "resize-end" | "bend-end") {
                        self.execute_native(r#"{"op":"cancel"}"#)?;
                    }
                    let mut state: serde_json::Value = serde_json::from_str(&self.snapshot()?)
                        .unwrap_or_else(|_| serde_json::json!({}));
                    state["ok"] = false.into();
                    state["error"] = error.into();
                    return Ok(state.to_string());
                }
            }
        }
        let result = self.execute_native(command)?;
        if notify {
            if let Some(callback) = self.after_edit.as_mut() {
                if let Ok(state) = serde_json::from_str::<serde_json::Value>(&result) {
                    if state["ok"] == true {
                        callback(&action, &state);
                    }
                }
            }
        }
        Ok(result)
    }
    fn execute_native(&mut self, command: &str) -> Result<String, RuntimeError> {
        let mut arena = Arena::default();
        let command = command.to_owned();
        let args = ArgsFlowCall {
            handle: self.handle,
            command: command.view(&mut arena),
        };
        runtime::try_value(OP_FLOWCALL, &args)
    }
    pub fn snapshot(&mut self) -> Result<String, RuntimeError> {
        runtime::try_value(
            OP_FLOWSNAPSHOT,
            &ArgsFlowSnapshot {
                handle: self.handle,
            },
        )
    }
    pub fn export_json(&mut self) -> Result<String, RuntimeError> {
        runtime::try_value(
            OP_FLOWEXPORT,
            &ArgsFlowExport {
                handle: self.handle,
            },
        )
    }
    pub fn undo(&mut self) -> Result<String, RuntimeError> {
        self.execute(r#"{"op":"undo"}"#)
    }
    /// 配置实例级历史预算；撤销与重做共用上限，0 关闭历史，不写入文档。
    pub fn configure_history(&mut self, max_steps: usize, max_bytes: usize) -> Result<serde_json::Value, FlowError> {
        self.apply_checked(&serde_json::json!({"op":"history-configure","options":{"max_steps":max_steps,"max_bytes":max_bytes}}))
    }
    /// 查询差量历史的计费字节、两端步数、预算裁剪和超大编辑边界。
    pub fn history_status(&mut self) -> Result<serde_json::Value, FlowError> {
        self.query("history", "", serde_json::json!({}))
    }
    pub fn redo(&mut self) -> Result<String, RuntimeError> {
        self.execute(r#"{"op":"redo"}"#)
    }
    pub fn add_node(&mut self, node: &serde_json::Value) -> Result<String, RuntimeError> {
        self.execute(&serde_json::json!({"op":"insert","node":node}).to_string())
    }
    pub fn update_node(
        &mut self,
        id: &str,
        node: &serde_json::Value,
    ) -> Result<String, RuntimeError> {
        self.execute(&serde_json::json!({"op":"node","id":id,"node":node}).to_string())
    }
    pub fn update_data(
        &mut self,
        id: &str,
        data: &serde_json::Value,
    ) -> Result<String, RuntimeError> {
        self.execute(&serde_json::json!({"op":"data","id":id,"text":data.to_string()}).to_string())
    }
    pub fn select(&mut self, id: &str, additive: bool) -> Result<String, RuntimeError> {
        self.execute(&serde_json::json!({"op":"select","id":id,"additive":additive}).to_string())
    }
    pub fn remove_selected(&mut self) -> Result<String, RuntimeError> {
        self.execute(r#"{"op":"delete"}"#)
    }
    pub fn select_next(&mut self, reverse: bool, edges: bool) -> Result<serde_json::Value, FlowError> {
        self.apply_checked(&serde_json::json!({"op":"select-next","flag":reverse,"target":if edges {"edge"}else{"node"}}))
    }
    pub fn nudge_selected(&mut self, dx: f64, dy: f64) -> Result<serde_json::Value, FlowError> {
        self.apply_checked(&serde_json::json!({"op":"nudge","x":dx,"y":dy}))
    }
    pub fn connect(
        &mut self,
        source: &str,
        port: &str,
        target: &str,
        target_port: &str,
    ) -> Result<String, RuntimeError> {
        self.execute(&serde_json::json!({"op":"connect","id":source,"port":port,"target":target,"target_port":target_port,"number":1,"flag":true}).to_string())
    }
    pub fn remove_edge(&mut self, id: &str) -> Result<String, RuntimeError> {
        self.execute(&serde_json::json!({"op":"remove-edge","id":id}).to_string())
    }
    /// 修改线型、双端标记、动画、曲率、层级、通道间距及基础外观，保留端点与业务数据。
    pub fn update_edge_appearance(
        &mut self,
        id: &str,
        appearance: &serde_json::Value,
    ) -> Result<String, RuntimeError> {
        self.execute(
            &serde_json::json!({"op":"edge-appearance","id":id,"appearance":appearance})
                .to_string(),
        )
    }
    pub fn set_viewport(&mut self, x: f64, y: f64, zoom: f64) -> Result<String, RuntimeError> {
        self.execute(&serde_json::json!({"op":"view","x":x,"y":y,"zoom":zoom}).to_string())
    }
    pub fn layout(&mut self) -> Result<String, RuntimeError> {
        self.execute(r#"{"op":"layout"}"#)
    }
    /// algorithm=layered/tree，direction=LR/RL/TB/BT，可指定间距、父级和递归范围。
    pub fn layout_graph(&mut self, options: &serde_json::Value) -> Result<String, RuntimeError> {
        self.execute(&serde_json::json!({"op":"layout","options":options}).to_string())
    }
    pub fn align_selected(&mut self, mode: &str) -> Result<String, RuntimeError> {
        self.execute(&serde_json::json!({"op":"align","target":mode}).to_string())
    }
    /// 普通图使用父级局部坐标；修改父级和解除分组保持画布中的位置。
    pub fn group_selection(&mut self, title: &str) -> Result<String, RuntimeError> {
        self.execute(&serde_json::json!({"op":"group","text":title}).to_string())
    }
    pub fn ungroup(&mut self, id: &str) -> Result<String, RuntimeError> {
        self.execute(&serde_json::json!({"op":"ungroup","id":id}).to_string())
    }
    pub fn reparent(&mut self, id: &str, parent: &str) -> Result<String, RuntimeError> {
        self.execute(&serde_json::json!({"op":"reparent","id":id,"target":parent}).to_string())
    }
    pub fn toggle_collapse(&mut self, id: &str) -> Result<String, RuntimeError> {
        self.execute(&serde_json::json!({"op":"collapse","id":id}).to_string())
    }
    pub fn node_world(&mut self, id: &str) -> Result<serde_json::Value, FlowError> {
        self.query("node-world", id, serde_json::json!({}))
    }
    /// 应用内复制可跨文档粘贴；片段接口可由宿主传递给系统剪贴板。
    pub fn copy_selection(&mut self) -> Result<String, RuntimeError> {
        self.execute(r#"{"op":"copy"}"#)
    }
    pub fn cut_selection(&mut self) -> Result<String, RuntimeError> {
        self.execute(r#"{"op":"cut"}"#)
    }
    pub fn selection_fragment(&mut self) -> Result<serde_json::Value, FlowError> {
        self.query("selection-fragment", "", serde_json::json!({}))
    }
    pub fn paste_fragment(
        &mut self,
        fragment: &serde_json::Value,
        x: f64,
        y: f64,
        parent: &str,
    ) -> Result<String, RuntimeError> {
        self.execute(&serde_json::json!({"op":"paste","fragment":fragment,"x":x,"y":y,"target":parent,"flag":true}).to_string())
    }
    pub fn paste_clipboard(
        &mut self,
        x: f64,
        y: f64,
        parent: &str,
    ) -> Result<String, RuntimeError> {
        self.execute(
            &serde_json::json!({"op":"paste","x":x,"y":y,"target":parent,"flag":true}).to_string(),
        )
    }
    pub fn import_json(&mut self, text: &str) -> Result<String, RuntimeError> {
        self.execute(&serde_json::json!({"op":"load","text":text}).to_string())
    }
    /// 回调中不能递归编辑本文档；批量操作审核一次，失败不产生修改通知。
    pub fn set_before_edit(
        &mut self,
        callback: impl FnMut(&serde_json::Value) -> Result<(), String> + 'static,
    ) {
        self.before_edit = Some(Box::new(callback));
    }
    pub fn set_after_edit(
        &mut self,
        callback: impl FnMut(&serde_json::Value, &serde_json::Value) + 'static,
    ) {
        self.after_edit = Some(Box::new(callback));
    }
    pub fn clear_edit_callbacks(&mut self) {
        self.before_edit = None;
        self.after_edit = None;
    }
    pub fn apply_checked(
        &mut self,
        action: &serde_json::Value,
    ) -> Result<serde_json::Value, FlowError> {
        let text = self
            .execute(&action.to_string())
            .map_err(FlowError::Runtime)?;
        let state: serde_json::Value = serde_json::from_str(&text)
            .map_err(|error| FlowError::InvalidResponse(error.to_string()))?;
        if state["ok"] != true {
            return Err(FlowError::Rejected(
                state["error"].as_str().unwrap_or("图操作失败").to_owned(),
            ));
        }
        Ok(state)
    }
    pub fn batch(&mut self, commands: &serde_json::Value) -> Result<String, RuntimeError> {
        self.execute(&serde_json::json!({"op":"batch","commands":commands}).to_string())
    }
    pub fn configure(&mut self, options: &serde_json::Value) -> Result<String, RuntimeError> {
        self.execute(&serde_json::json!({"op":"options","options":options}).to_string())
    }
    pub fn patch_node(
        &mut self,
        id: &str,
        patch: &serde_json::Value,
    ) -> Result<String, RuntimeError> {
        self.execute(&serde_json::json!({"op":"node-patch","id":id,"patch":patch}).to_string())
    }
    pub fn patch_edge(
        &mut self,
        id: &str,
        patch: &serde_json::Value,
    ) -> Result<String, RuntimeError> {
        self.execute(&serde_json::json!({"op":"edge-patch","id":id,"patch":patch}).to_string())
    }
    pub fn update_edge_data(
        &mut self,
        id: &str,
        data: &serde_json::Value,
    ) -> Result<String, RuntimeError> {
        self.execute(&serde_json::json!({"op":"edge-data","id":id,"data":data}).to_string())
    }
    pub fn connect_with_options(
        &mut self,
        source: &str,
        port: &str,
        target: &str,
        target_port: &str,
        style: i32,
        arrow: bool,
        label: &str,
    ) -> Result<String, RuntimeError> {
        self.execute(&serde_json::json!({"op":"connect","id":source,"port":port,"target":target,"target_port":target_port,"number":style,"flag":arrow,"text":label}).to_string())
    }
    pub fn query(
        &mut self,
        kind: &str,
        id: &str,
        mut options: serde_json::Value,
    ) -> Result<serde_json::Value, FlowError> {
        if !options.is_object() {
            return Err(FlowError::Rejected("查询参数必须为对象".into()));
        }
        options["op"] = "query".into();
        options["target"] = kind.into();
        options["id"] = id.into();
        Ok(self.apply_checked(&options)?["result"].clone())
    }
    pub fn document_status(&mut self) -> Result<serde_json::Value, FlowError> {
        self.query("document-status", "", serde_json::json!({}))
    }
    /// 持久化成功后确认保存；生成 JSON 或图片本身不清除未保存状态。
    pub fn mark_saved(&mut self) -> Result<serde_json::Value, FlowError> {
        self.apply_checked(&serde_json::json!({"op":"mark-saved"}))
    }
    pub fn export_svg(&mut self, selected: bool, options: &serde_json::Value) -> Result<String, FlowError> {
        let value=self.query("export-svg","",serde_json::json!({"flag":selected,"options":options}))?;
        value.as_str().map(str::to_owned).ok_or_else(||FlowError::InvalidResponse("导出结果不是 SVG 文本".into()))
    }
    /// 适配器返回 text/rect/ellipse/path/image 图元数组，坐标以卡片正文左上角为原点。
    pub fn export_svg_with(&mut self,selected: bool,mut options: serde_json::Value,mut adapter: impl FnMut(&serde_json::Value)->serde_json::Value) -> Result<String,FlowError> {
        if !options.is_object() || options.get("node_content").is_some_and(|v|!v.is_object()) {return Err(FlowError::Rejected("导出选项或卡片适配内容必须为对象".into()));}
        let before=self.export_json().map_err(FlowError::Runtime)?;
        let nodes=self.query("export-nodes","",serde_json::json!({"flag":selected}))?;
        let mut content=options["node_content"].as_object().cloned().unwrap_or_default();
        for node in nodes.as_array().ok_or_else(||FlowError::InvalidResponse("导出节点不是数组".into()))? {content.insert(node["id"].as_str().unwrap().to_owned(),adapter(node));}
        if self.export_json().map_err(FlowError::Runtime)?!=before {return Err(FlowError::Rejected("卡片导出适配器修改了文档，请使用只读适配器".into()));}
        options["node_content"]=content.into();self.export_svg(selected,&options)
    }
    /// 批量公共属性作为一个原子历史步骤提交。
    pub fn patch_selected_nodes(
        &mut self,
        patch: &serde_json::Value,
    ) -> Result<String, RuntimeError> {
        self.execute(&serde_json::json!({"op":"node-properties","patch":patch}).to_string())
    }
    /// 世界坐标边界；可限定为选区或一个节点。
    pub fn bounds(&mut self, selected: bool, id: &str) -> Result<serde_json::Value, FlowError> {
        self.query("bounds", id, serde_json::json!({"flag":selected}))
    }
    pub fn search(&mut self, text: &str) -> Result<serde_json::Value, FlowError> {
        self.query("search", "", serde_json::json!({"text":text}))
    }
    pub fn reveal(&mut self, id: &str) -> Result<String, RuntimeError> {
        self.execute(&serde_json::json!({"op":"reveal","id":id}).to_string())
    }
    pub fn upstream(&mut self, id: &str, recursive: bool) -> Result<serde_json::Value, FlowError> {
        self.query("upstream", id, serde_json::json!({"flag":recursive}))
    }
    pub fn downstream(
        &mut self,
        id: &str,
        recursive: bool,
    ) -> Result<serde_json::Value, FlowError> {
        self.query("downstream", id, serde_json::json!({"flag":recursive}))
    }
    pub fn connected_edges(
        &mut self,
        id: &str,
        port: &str,
    ) -> Result<serde_json::Value, FlowError> {
        self.query("connected-edges", id, serde_json::json!({"port":port}))
    }
    pub fn ports(&mut self, id: &str) -> Result<serde_json::Value, FlowError> {
        self.query("ports", id, serde_json::json!({}))
    }
    pub fn intersections(
        &mut self,
        x: f64,
        y: f64,
        width: f64,
        height: f64,
        partial: bool,
    ) -> Result<serde_json::Value, FlowError> {
        self.query(
            "intersections",
            "",
            serde_json::json!({"x":x,"y":y,"width":width,"height":height,"flag":partial}),
        )
    }
}
impl Drop for FlowDocument {
    fn drop(&mut self) {
        let _ = runtime::try_value::<bool, _>(
            OP_FLOWFREE,
            &ArgsFlowFree {
                handle: self.handle,
            },
        );
    }
}

#[doc(hidden)]
pub use serde_json as json;

#[cfg(test)]
mod tests {
    use super::*;
    use serde_json::{json, Value};
    #[test]
    fn dll_mindmap_edge_patch_preserves_tree_and_history() {
        let mut doc = FlowDocument::new("mindmap").unwrap();
        doc.apply_checked(&json!({"op":"demo","number":4})).unwrap();
        doc.apply_checked(&json!({"op":"edge-patch","id":"tree:node-2","patch":{"label":"保留属性"}})).unwrap();
        let decorated = doc.export_json().unwrap();
        doc.undo().unwrap();
        let before = doc.export_json().unwrap();
        let status = doc.document_status().unwrap();
        let rejected: Value = serde_json::from_str(
            &doc.patch_edge("tree:node-2", &json!({"source_node":"node-3"})).unwrap(),
        ).unwrap();
        assert_eq!(rejected["ok"], false);
        assert_eq!(doc.export_json().unwrap(), before);
        assert_eq!(doc.document_status().unwrap(), status);
        doc.redo().unwrap();
        assert_eq!(doc.export_json().unwrap(), decorated);
        let mut restored = FlowDocument::new("mindmap").unwrap();
        restored.apply_checked(&json!({"op":"load","text":decorated})).unwrap();
        assert_eq!(restored.export_json().unwrap(), decorated);
    }
    #[test]
    fn dll_incremental_history_budget_and_barriers() {
        let mut doc = FlowDocument::new("graph").unwrap();
        doc.add_node(&json!({"id":"n","data":{"value":"0"}})).unwrap();
        let initial = doc.export_json().unwrap();
        doc.execute(&json!({"op":"load","text":initial}).to_string()).unwrap();
        for value in ["1", "2", "3", "4", "5"] {
            doc.update_data("n", &json!({"value":value})).unwrap();
        }
        let bytes = doc.history_status().unwrap()["history_bytes"].as_u64().unwrap() as usize;
        assert!(bytes < 4096);
        doc.configure_history(2, bytes).unwrap();
        assert_eq!(doc.history_status().unwrap()["undo_count"], 2);
        doc.undo().unwrap(); doc.undo().unwrap(); doc.undo().unwrap();
        assert_eq!(doc.query("node", "n", json!({})).unwrap()["data"]["value"], "3");
        let retained = doc.history_status().unwrap()["history_bytes"].clone();
        doc.redo().unwrap(); doc.redo().unwrap();
        assert_eq!(doc.history_status().unwrap()["history_bytes"], retained);
        assert_eq!(doc.query("node", "n", json!({})).unwrap()["data"]["value"], "5");
        doc.configure_history(100, 1024).unwrap();
        doc.update_data("n", &json!({"blob":"a".repeat(8192)})).unwrap();
        assert_eq!(doc.history_status().unwrap()["history_bytes"], 0);
        assert_eq!(doc.history_status().unwrap()["history_barriers"], 1);
        let barrier = doc.export_json().unwrap();
        doc.undo().unwrap(); assert_eq!(doc.export_json().unwrap(), barrier);
        let before = doc.history_status().unwrap();
        assert!(doc.configure_history(10001, 1024).is_err());
        assert_eq!(doc.history_status().unwrap(), before);
        let mut other = FlowDocument::new("graph").unwrap();
        assert_eq!(other.history_status().unwrap()["history_budget_bytes"], 32 * 1024 * 1024);
    }
    #[test]
    fn dll_batch_policy_queries_and_callbacks() {
        let mut doc = FlowDocument::new("graph").unwrap();
        doc.batch(
            &json!([{"op":"insert","node":{"id":"a"}},{"op":"insert","node":{"id":"b","x":400}}]),
        )
        .unwrap();
        let before = doc.export_json().unwrap();
        let failed: Value=serde_json::from_str(&doc.batch(&json!([{"op":"title","id":"a","text":"不可保留"},{"op":"connect","id":"a","port":"out","target":"missing","target_port":"in"}])).unwrap()).unwrap();
        assert_eq!(failed["ok"], false);
        assert_eq!(doc.export_json().unwrap(), before);
        doc.undo().unwrap();
        assert_eq!(doc.query("nodes", "", json!({})).unwrap(), json!([]));
        doc.redo().unwrap();
        doc.connect("a", "out", "b", "in").unwrap();
        assert_eq!(doc.downstream("a", true).unwrap()[0]["id"], "b");
        assert_eq!(doc.ports("b").unwrap()[0]["connection_count"], 1);
        doc.patch_node("a", &json!({"draggable":false})).unwrap();
        assert!(matches!(
            doc.apply_checked(&json!({"op":"drag-begin","id":"a"})),
            Err(FlowError::Rejected(_))
        ));
        let calls = std::rc::Rc::new(std::cell::Cell::new(0));
        let observer = calls.clone();
        doc.set_after_edit(move |_, _| observer.set(observer.get() + 1));
        doc.set_before_edit(|a| {
            if a["op"] == "delete" {
                Err("业务禁止删除".into())
            } else {
                Ok(())
            }
        });
        doc.select("b", false).unwrap();
        let stable = doc.export_json().unwrap();
        assert!(doc.apply_checked(&json!({"op":"delete"})).is_err());
        assert_eq!(doc.export_json().unwrap(), stable);
        assert_eq!(calls.get(), 1);
    }
    #[test]
    fn dll_edge_appearance_roundtrip_and_history() {
        let mut doc = FlowDocument::new("graph").unwrap();
        doc.execute(r#"{"op":"demo","number":1}"#).unwrap();
        let original = doc.export_json().unwrap();
        let result: Value = serde_json::from_str(
            &doc.update_edge_appearance(
                "edge-3",
                &json!({"style":3,"stroke_width":4,"arrow":false,"label":"跨行连接","stroke_color":"#7c5ce7","dashed":true}),
            )
            .unwrap(),
        )
        .unwrap();
        assert_eq!(result["ok"], true);
        assert_eq!(result["edges"][0]["color_red"], 124);
        assert_eq!(result["edges"][0]["dashed"], true);
        let saved = doc.export_json().unwrap();
        doc.undo().unwrap();
        assert_eq!(doc.export_json().unwrap(), original);
        doc.redo().unwrap();
        assert_eq!(doc.export_json().unwrap(), saved);
        let mut other = FlowDocument::new("graph").unwrap();
        other.import_json(&saved).unwrap();
        assert_eq!(other.export_json().unwrap(), saved);
        let invalid: Value = serde_json::from_str(
            &doc.update_edge_appearance("edge-3", &json!({"stroke_width":0}))
                .unwrap(),
        )
        .unwrap();
        assert_eq!(invalid["ok"], false);
        assert_eq!(doc.export_json().unwrap(), saved);
    }
    #[test]
    fn dll_unicode_roundtrip_and_isolation() {
        let mut a = FlowDocument::new("graph").unwrap();
        let mut b = FlowDocument::new("graph").unwrap();
        let node = json!({"id":"中文节点🙂","kind":"external-card","title":"业务卡片","data":{"value":"未注册类型仍保留"}});
        let result: Value = serde_json::from_str(&a.add_node(&node).unwrap()).unwrap();
        assert_eq!(result["ok"], true);
        let saved = a.export_json().unwrap();
        b.import_json(&saved).unwrap();
        assert_eq!(b.export_json().unwrap(), saved);
        a.update_data("中文节点🙂", &json!({"nested":{"count":42}}))
            .unwrap();
        assert_eq!(b.export_json().unwrap(), saved);
        a.undo().unwrap();
        assert_eq!(a.export_json().unwrap(), saved);
        let bad = b.import_json("{bad json").unwrap();
        assert_eq!(serde_json::from_str::<Value>(&bad).unwrap()["ok"], false);
        assert_eq!(b.export_json().unwrap(), saved);
    }
    #[test]
    fn dll_stray_motion_keeps_unsaved_state() {
        let mut doc = FlowDocument::new("graph").unwrap();
        doc.add_node(&json!({"id":"a","x":0,"y":0})).unwrap();
        doc.mark_saved().unwrap();
        for (op, x) in [("drag-begin", 10.), ("drag-move", 60.), ("drag-end", 60.)] {
            doc.apply_checked(&json!({"op":op,"id":"a","x":x,"y":10})).unwrap();
        }
        assert_eq!(doc.document_status().unwrap()["dirty"], true);
        // 手势结束后误发的 drag-move/resize-move 既不移动节点，也不能把文档改回“已保存”。
        for op in ["drag-move", "resize-move"] {
            let state = doc.apply_checked(&json!({"op":op,"id":"a","x":200,"y":200})).unwrap();
            assert_eq!(state["dirty"], true, "{op}");
            assert_eq!(doc.document_status().unwrap()["dirty"], true, "{op}");
        }
        let world = doc.node_world("a").unwrap();
        assert_eq!(world["x"], 50.);
    }
    #[test]
    fn dll_direction_validation_and_history_limit() {
        let mut doc = FlowDocument::new("graph").unwrap();
        for id in ["a", "b"] {
            doc.add_node(&json!({"id":id})).unwrap();
        }
        let good: Value =
            serde_json::from_str(&doc.connect("a", "out", "b", "in").unwrap()).unwrap();
        assert_eq!(good["ok"], true);
        let bad: Value =
            serde_json::from_str(&doc.connect("b", "in", "a", "out").unwrap()).unwrap();
        assert_eq!(bad["ok"], false);
        for value in 0..105 {
            doc.update_data("a", &json!({"value":value})).unwrap();
        }
        for _ in 0..100 {
            doc.undo().unwrap();
        }
        let state: Value = serde_json::from_str(&doc.snapshot().unwrap()).unwrap();
        assert_eq!(state["can_undo"], false);
        assert_eq!(
            serde_json::from_str::<Value>(&doc.export_json().unwrap()).unwrap()["nodes"][0]["data"]
                ["value"],
            4
        );
    }
}
