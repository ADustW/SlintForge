// 由 tools/generate_flow_bindings.py 生成。
#[macro_export]
macro_rules! bind_flow_services {
    ($ui:expr, $service:ty, $ForgeFlowPortData:ty, $ForgeFlowNodeData:ty, $ForgeFlowEdgeData:ty, $ForgeFlowViewport:ty, $ForgeFlowRect:ty, $ForgeFlowAction:ty, $ForgeFlowState:ty) => {{
        use slint::{ComponentHandle, Model};
        use $crate::flow::json::{self, Value};
        let decode_forgeflowportdata = move |value: &Value| -> $ForgeFlowPortData {
            let mut result = <$ForgeFlowPortData>::default();
            result.id = value["id"].as_str().unwrap_or_default().into();
            result.direction = value["direction"].as_i64().unwrap_or_default() as i32;
            result.side = value["side"].as_i64().unwrap_or_default() as i32;
            result.offset = value["offset"].as_f64().unwrap_or_default() as f32;
            result
        };
        let decode_forgeflownodedata = move |value: &Value| -> $ForgeFlowNodeData {
            let mut result = <$ForgeFlowNodeData>::default();
            result.id = value["id"].as_str().unwrap_or_default().into();
            result.kind = value["kind"].as_str().unwrap_or_default().into();
            result.title = value["title"].as_str().unwrap_or_default().into();
            result.data_json = value["data_json"].as_str().unwrap_or_default().into();
            result.x = value["x"].as_f64().unwrap_or_default() as f32;
            result.y = value["y"].as_f64().unwrap_or_default() as f32;
            result.width = value["width"].as_f64().unwrap_or_default() as f32;
            result.height = value["height"].as_f64().unwrap_or_default() as f32;
            result.ports = slint::ModelRc::new(slint::VecModel::from(value["ports"].as_array().unwrap().iter().map(|v| decode_forgeflowportdata(v)).collect::<Vec<_>>()));
            result.parent = value["parent"].as_str().unwrap_or_default().into();
            result.side = value["side"].as_i64().unwrap_or_default() as i32;
            result.order = value["order"].as_i64().unwrap_or_default() as i32;
            result.collapsed = value["collapsed"].as_bool().unwrap_or(false);
            result.selected = value["selected"].as_bool().unwrap_or(false);
            result.hidden = value["hidden"].as_bool().unwrap_or(false);
            result
        };
        let decode_forgeflowedgedata = move |value: &Value| -> $ForgeFlowEdgeData {
            let mut result = <$ForgeFlowEdgeData>::default();
            result.id = value["id"].as_str().unwrap_or_default().into();
            result.path = value["path"].as_str().unwrap_or_default().into();
            result.arrow_path = value["arrow_path"].as_str().unwrap_or_default().into();
            result.label = value["label"].as_str().unwrap_or_default().into();
            result.style = value["style"].as_i64().unwrap_or_default() as i32;
            result.arrow = value["arrow"].as_bool().unwrap_or(false);
            result.stroke_width = value["stroke_width"].as_f64().unwrap_or_default() as f32;
            result.label_x = value["label_x"].as_f64().unwrap_or_default() as f32;
            result.label_y = value["label_y"].as_f64().unwrap_or_default() as f32;
            result.source_x = value["source_x"].as_f64().unwrap_or_default() as f32;
            result.source_y = value["source_y"].as_f64().unwrap_or_default() as f32;
            result.target_x = value["target_x"].as_f64().unwrap_or_default() as f32;
            result.target_y = value["target_y"].as_f64().unwrap_or_default() as f32;
            result.min_x = value["min_x"].as_f64().unwrap_or_default() as f32;
            result.min_y = value["min_y"].as_f64().unwrap_or_default() as f32;
            result.max_x = value["max_x"].as_f64().unwrap_or_default() as f32;
            result.max_y = value["max_y"].as_f64().unwrap_or_default() as f32;
            result.selected = value["selected"].as_bool().unwrap_or(false);
            result.hidden = value["hidden"].as_bool().unwrap_or(false);
            result
        };
        let decode_forgeflowviewport = move |value: &Value| -> $ForgeFlowViewport {
            let mut result = <$ForgeFlowViewport>::default();
            result.x = value["x"].as_f64().unwrap_or_default() as f32;
            result.y = value["y"].as_f64().unwrap_or_default() as f32;
            result.zoom = value["zoom"].as_f64().unwrap_or_default() as f32;
            result
        };
        let decode_forgeflowrect = move |value: &Value| -> $ForgeFlowRect {
            let mut result = <$ForgeFlowRect>::default();
            result.x = value["x"].as_f64().unwrap_or_default() as f32;
            result.y = value["y"].as_f64().unwrap_or_default() as f32;
            result.width = value["width"].as_f64().unwrap_or_default() as f32;
            result.height = value["height"].as_f64().unwrap_or_default() as f32;
            result
        };
        let decode_forgeflowaction = move |value: &Value| -> $ForgeFlowAction {
            let mut result = <$ForgeFlowAction>::default();
            result.op = value["op"].as_str().unwrap_or_default().into();
            result.id = value["id"].as_str().unwrap_or_default().into();
            result.target = value["target"].as_str().unwrap_or_default().into();
            result.port = value["port"].as_str().unwrap_or_default().into();
            result.target_port = value["target_port"].as_str().unwrap_or_default().into();
            result.text = value["text"].as_str().unwrap_or_default().into();
            result.x = value["x"].as_f64().unwrap_or_default() as f32;
            result.y = value["y"].as_f64().unwrap_or_default() as f32;
            result.width = value["width"].as_f64().unwrap_or_default() as f32;
            result.height = value["height"].as_f64().unwrap_or_default() as f32;
            result.zoom = value["zoom"].as_f64().unwrap_or_default() as f32;
            result.number = value["number"].as_i64().unwrap_or_default() as i32;
            result.count = value["count"].as_i64().unwrap_or_default() as i32;
            result.flag = value["flag"].as_bool().unwrap_or(false);
            result.additive = value["additive"].as_bool().unwrap_or(false);
            result
        };
        type Documents<N,E> = std::collections::HashMap<i32, ($crate::flow::FlowDocument, std::rc::Rc<slint::VecModel<N>>, std::rc::Rc<slint::VecModel<E>>)>;
        let docs = std::rc::Rc::new(std::cell::RefCell::new(Documents::<$ForgeFlowNodeData, $ForgeFlowEdgeData>::new()));
        let next = std::rc::Rc::new(std::cell::Cell::new(0));
        let update = std::rc::Rc::new(move |token: i32, text: &str, nodes: &std::rc::Rc<slint::VecModel<$ForgeFlowNodeData>>, edges: &std::rc::Rc<slint::VecModel<$ForgeFlowEdgeData>>| -> $ForgeFlowState {
            let value: Value = json::from_str(text).expect("无效的核心响应");
            for i in (0..nodes.row_count()).rev() { let id = nodes.row_data(i).unwrap().id; if value["removed_nodes"].as_array().unwrap().iter().any(|v| v.as_str() == Some(id.as_str())) { nodes.remove(i); } }
            for item in value["nodes"].as_array().unwrap() { let row = decode_forgeflownodedata(item); let index = (0..nodes.row_count()).find(|&i| nodes.row_data(i).unwrap().id == row.id); if let Some(i) = index { nodes.set_row_data(i,row); } else { nodes.push(row); } }
            for i in (0..edges.row_count()).rev() { let id = edges.row_data(i).unwrap().id; if value["removed_edges"].as_array().unwrap().iter().any(|v| v.as_str() == Some(id.as_str())) { edges.remove(i); } }
            for item in value["edges"].as_array().unwrap() { let row = decode_forgeflowedgedata(item); let index = (0..edges.row_count()).find(|&i| edges.row_data(i).unwrap().id == row.id); if let Some(i) = index { edges.set_row_data(i,row); } else { edges.push(row); } }
            let mut result = <$ForgeFlowState>::default(); result.token = token;
            result.revision = value["revision"].as_i64().unwrap_or_default() as i32;
            result.ok = value["ok"].as_bool().unwrap_or(false);
            result.error = value["error"].as_str().unwrap_or_default().into();
            result.mode = value["mode"].as_str().unwrap_or_default().into();
            result.nodes = slint::ModelRc::from(nodes.clone());
            result.edges = slint::ModelRc::from(edges.clone());
            result.viewport = decode_forgeflowviewport(&value["viewport"]);
            result.bounds = decode_forgeflowrect(&value["bounds"]);
            result.can_undo = value["can_undo"].as_bool().unwrap_or(false);
            result.can_redo = value["can_redo"].as_bool().unwrap_or(false);
            result.selected_id = value["selected_id"].as_str().unwrap_or_default().into();
            result.selected_title = value["selected_title"].as_str().unwrap_or_default().into();
            result.selected_edge = decode_forgeflowedgedata(&value["selected_edge"]);
            result.selected_edge_count = value["selected_edge_count"].as_i64().unwrap_or_default() as i32;
            result.node_count = value["node_count"].as_i64().unwrap_or_default() as i32;
            result.edge_count = value["edge_count"].as_i64().unwrap_or_default() as i32;
            result
        });
        let service = $ui.global::<$service>();
        let d = docs.clone(); let u = update.clone();
        service.on_create(move |mode| { let id = next.get() + 1; next.set(id); let mut doc = $crate::flow::FlowDocument::new(mode.as_str()).expect("图运行库初始化失败"); let n = std::rc::Rc::new(slint::VecModel::default()); let e = std::rc::Rc::new(slint::VecModel::default()); let state = u(id, &doc.snapshot().unwrap(), &n, &e); d.borrow_mut().insert(id,(doc,n,e)); state });
        let d = docs.clone(); service.on_release(move |token| { d.borrow_mut().remove(&token); });
        let d = docs.clone(); let u = update.clone();
        service.on_execute(move |token, action| { let mut docs = d.borrow_mut(); let Some((doc,n,e)) = docs.get_mut(&token) else { return <$ForgeFlowState>::default(); };
            let value = json::json!({
                "op": action.op.as_str(),
                "id": action.id.as_str(),
                "target": action.target.as_str(),
                "port": action.port.as_str(),
                "target_port": action.target_port.as_str(),
                "text": action.text.as_str(),
                "x": action.x,
                "y": action.y,
                "width": action.width,
                "height": action.height,
                "zoom": action.zoom,
                "number": action.number,
                "count": action.count,
                "flag": action.flag,
                "additive": action.additive,
            });
            u(token, &doc.execute(&value.to_string()).unwrap(), n, e)
        });
        let d = docs.clone(); service.on_export_json(move |token| d.borrow_mut().get_mut(&token).map(|(doc,_,_)| doc.export_json().unwrap().into()).unwrap_or_default());
        service.on_field(|data, key| { let value: Value = json::from_str(data.as_str()).unwrap_or(Value::Null); match &value[key.as_str()] { Value::Null => "".into(), Value::String(s) => s.as_str().into(), other => other.to_string().into() } });
        let d = docs.clone(); service.on_save_file(move |token, name| { let result = (|| -> Result<(), String> { let mut docs = d.borrow_mut(); let (doc,_,_) = docs.get_mut(&token).ok_or("图文档已释放")?; let path = std::path::Path::new(name.as_str()); if let Some(parent) = path.parent().filter(|p| !p.as_os_str().is_empty()) { std::fs::create_dir_all(parent).map_err(|e| e.to_string())?; } std::fs::write(path,doc.export_json().map_err(|e| e.to_string())?).map_err(|e| e.to_string()) })(); result.err().unwrap_or_default().into() });
        let d = docs.clone(); let u = update.clone(); service.on_load_file(move |token, name| { let mut docs = d.borrow_mut(); let Some((doc,n,e)) = docs.get_mut(&token) else { return <$ForgeFlowState>::default(); }; match std::fs::read_to_string(name.as_str()) { Ok(text) => u(token,&doc.execute(&json::json!({"op":"load","text":text}).to_string()).unwrap(),n,e), Err(error) => { let mut state = u(token,&doc.snapshot().unwrap(),n,e); state.ok = false; state.error = error.to_string().into(); state } } });
        let d = docs.clone(); service.on_resolve(move |token, action| { let mut docs = d.borrow_mut(); let Some((doc,_,_)) = docs.get_mut(&token) else { return <$ForgeFlowAction>::default(); }; let value = json::json!({
            "op": "resolve",
            "id": action.id.as_str(),
            "target": action.target.as_str(),
            "port": action.port.as_str(),
            "target_port": action.target_port.as_str(),
            "text": action.text.as_str(),
            "x": action.x,
            "y": action.y,
            "width": action.width,
            "height": action.height,
            "zoom": action.zoom,
            "number": action.number,
            "count": action.count,
            "flag": action.flag,
            "additive": action.additive,
        }); decode_forgeflowaction(&json::from_str(&doc.execute(&value.to_string()).unwrap()).unwrap()) });
        service.set_connected(true);
    }};
}
