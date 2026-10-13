//! 图文档的 DLL 安全封装。图算法和历史只在共用核心内执行。
use crate::runtime::{self, RuntimeError};
use slintforge_runtime_api::{models::*, Arena, Wire};

pub struct FlowDocument {
    handle: u64,
}
impl FlowDocument {
    pub fn new(mode: &str) -> Result<Self, RuntimeError> {
        let mut arena = Arena::default();
        let mode = mode.to_owned();
        let args = ArgsFlowNew {
            mode: mode.view(&mut arena),
        };
        Ok(Self {
            handle: runtime::try_value(OP_FLOWNEW, &args)?,
        })
    }
    pub fn execute(&mut self, command: &str) -> Result<String, RuntimeError> {
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
    /// 修改线型、箭头、标签和线宽，保留端点及业务扩展数据。
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
    pub fn import_json(&mut self, text: &str) -> Result<String, RuntimeError> {
        self.execute(&serde_json::json!({"op":"load","text":text}).to_string())
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
    fn dll_edge_appearance_roundtrip_and_history() {
        let mut doc = FlowDocument::new("graph").unwrap();
        doc.execute(r#"{"op":"demo","number":1}"#).unwrap();
        let original = doc.export_json().unwrap();
        let result: Value = serde_json::from_str(
            &doc.update_edge_appearance(
                "edge-3",
                &json!({"style":1,"stroke_width":4,"arrow":false,"label":"跨行连接"}),
            )
            .unwrap(),
        )
        .unwrap();
        assert_eq!(result["ok"], true);
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
