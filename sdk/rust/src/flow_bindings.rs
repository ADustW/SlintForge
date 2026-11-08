// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

// 由 tools/generate_flow_bindings.py 生成。
#[macro_export]
macro_rules! bind_flow_services {
    ($ui:expr, $service:ty, $ForgeFlowPointData:ty, $ForgeFlowPortData:ty, $ForgeFlowNodeData:ty, $ForgeFlowEdgeData:ty, $ForgeFlowViewport:ty, $ForgeFlowRect:ty, $ForgeFlowAction:ty, $ForgeFlowState:ty) => {{
        use slint::{ComponentHandle, Model};
        use $crate::flow::json::{self, Value};
        let decode_forgeflowpointdata = move |value: &Value| -> $ForgeFlowPointData {
            let mut result = <$ForgeFlowPointData>::default();
            result.x = value["x"].as_f64().unwrap_or_default() as f32;
            result.y = value["y"].as_f64().unwrap_or_default() as f32;
            result
        };
        let decode_forgeflowportdata = move |value: &Value| -> $ForgeFlowPortData {
            let mut result = <$ForgeFlowPortData>::default();
            result.id = value["id"].as_str().unwrap_or_default().into();
            result.direction = value["direction"].as_i64().unwrap_or_default() as i32;
            result.side = value["side"].as_i64().unwrap_or_default() as i32;
            result.offset = value["offset"].as_f64().unwrap_or_default() as f32;
            result.connectable = value["connectable"].as_bool().unwrap_or(false);
            result.max_connections = value["max_connections"].as_i64().unwrap_or_default() as i32;
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
            result.local_x = value["local_x"].as_f64().unwrap_or_default() as f32;
            result.local_y = value["local_y"].as_f64().unwrap_or_default() as f32;
            result.depth = value["depth"].as_i64().unwrap_or_default() as i32;
            result.is_group = value["is_group"].as_bool().unwrap_or(false);
            result.constrain_parent = value["constrain_parent"].as_bool().unwrap_or(false);
            result.layout_fixed = value["layout_fixed"].as_bool().unwrap_or(false);
            result.ports = slint::ModelRc::new(slint::VecModel::from(value["ports"].as_array().map(|items| items.iter().map(|v| decode_forgeflowportdata(v)).collect::<Vec<_>>()).unwrap_or_default()));
            result.parent = value["parent"].as_str().unwrap_or_default().into();
            result.side = value["side"].as_i64().unwrap_or_default() as i32;
            result.order = value["order"].as_i64().unwrap_or_default() as i32;
            result.collapsed = value["collapsed"].as_bool().unwrap_or(false);
            result.selected = value["selected"].as_bool().unwrap_or(false);
            result.hidden = value["hidden"].as_bool().unwrap_or(false);
            result.draggable = value["draggable"].as_bool().unwrap_or(false);
            result.selectable = value["selectable"].as_bool().unwrap_or(false);
            result.connectable = value["connectable"].as_bool().unwrap_or(false);
            result.deletable = value["deletable"].as_bool().unwrap_or(false);
            result.resizable = value["resizable"].as_bool().unwrap_or(false);
            result.header_visible = value["header_visible"].as_bool().unwrap_or(false);
            result.header_height = value["header_height"].as_f64().unwrap_or_default() as f32;
            result.min_width = value["min_width"].as_f64().unwrap_or_default() as f32;
            result.min_height = value["min_height"].as_f64().unwrap_or_default() as f32;
            result.max_width = value["max_width"].as_f64().unwrap_or_default() as f32;
            result.max_height = value["max_height"].as_f64().unwrap_or_default() as f32;
            result.aspect_ratio = value["aspect_ratio"].as_f64().unwrap_or_default() as f32;
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
            result.stroke_color = value["stroke_color"].as_str().unwrap_or_default().into();
            result.color_red = value["color_red"].as_i64().unwrap_or_default() as i32;
            result.color_green = value["color_green"].as_i64().unwrap_or_default() as i32;
            result.color_blue = value["color_blue"].as_i64().unwrap_or_default() as i32;
            result.dashed = value["dashed"].as_bool().unwrap_or(false);
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
            result.source_node = value["source_node"].as_str().unwrap_or_default().into();
            result.source_port = value["source_port"].as_str().unwrap_or_default().into();
            result.target_node = value["target_node"].as_str().unwrap_or_default().into();
            result.target_port = value["target_port"].as_str().unwrap_or_default().into();
            result.kind = value["kind"].as_str().unwrap_or_default().into();
            result.data_json = value["data_json"].as_str().unwrap_or_default().into();
            result.path_kind = value["path_kind"].as_str().unwrap_or_default().into();
            result.control_points = slint::ModelRc::new(slint::VecModel::from(value["control_points"].as_array().map(|items| items.iter().map(|v| decode_forgeflowpointdata(v)).collect::<Vec<_>>()).unwrap_or_default()));
            result.selectable = value["selectable"].as_bool().unwrap_or(false);
            result.deletable = value["deletable"].as_bool().unwrap_or(false);
            result.reconnectable = value["reconnectable"].as_bool().unwrap_or(false);
            result.start_marker = value["start_marker"].as_str().unwrap_or_default().into();
            result.end_marker = value["end_marker"].as_str().unwrap_or_default().into();
            result.marker_stroke_path = value["marker_stroke_path"].as_str().unwrap_or_default().into();
            result.animated = value["animated"].as_bool().unwrap_or(false);
            result.animation_speed = value["animation_speed"].as_f64().unwrap_or_default() as f32;
            result.path_length = value["path_length"].as_f64().unwrap_or_default() as f32;
            result.motion_points = slint::ModelRc::new(slint::VecModel::from(value["motion_points"].as_array().map(|items| items.iter().map(|v| decode_forgeflowpointdata(v)).collect::<Vec<_>>()).unwrap_or_default()));
            result.curvature = value["curvature"].as_f64().unwrap_or_default() as f32;
            result.layer = value["layer"].as_i64().unwrap_or_default() as i32;
            result.parallel_offset = value["parallel_offset"].as_f64().unwrap_or_default() as f32;
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
        #[derive(Default)] struct RowIndex { nodes: std::collections::HashMap<String,usize>, edges: std::collections::HashMap<String,usize> }
        type Documents<N,E> = std::collections::HashMap<i32, ($crate::flow::FlowDocument, std::rc::Rc<slint::VecModel<N>>, std::rc::Rc<slint::VecModel<E>>, RowIndex)>;
        let docs = std::rc::Rc::new(std::cell::RefCell::new(Documents::<$ForgeFlowNodeData, $ForgeFlowEdgeData>::new()));
        let next = std::rc::Rc::new(std::cell::Cell::new(0));
        let update = std::rc::Rc::new(move |token: i32, text: &str, nodes: &std::rc::Rc<slint::VecModel<$ForgeFlowNodeData>>, edges: &std::rc::Rc<slint::VecModel<$ForgeFlowEdgeData>>, rows: &mut RowIndex| -> $ForgeFlowState {
            let value: Value = json::from_str(text).expect("无效的核心响应");
            if !value["removed_nodes"].as_array().unwrap().is_empty() {
                for i in (0..nodes.row_count()).rev() { let id = nodes.row_data(i).unwrap().id; if value["removed_nodes"].as_array().unwrap().iter().any(|v| v.as_str() == Some(id.as_str())) { nodes.remove(i); } }
                rows.nodes.clear(); for i in 0..nodes.row_count() { rows.nodes.insert(nodes.row_data(i).unwrap().id.to_string(),i); }
            }
            for item in value["nodes"].as_array().unwrap() { let mut row = decode_forgeflownodedata(item); let index = rows.nodes.get(row.id.as_str()).copied(); if let Some(i) = index {
                let previous = nodes.row_data(i).unwrap();
                if let Some(stable) = previous.ports.as_any().downcast_ref::<slint::VecModel<$ForgeFlowPortData>>() {
                    while stable.row_count() > row.ports.row_count() { stable.remove(stable.row_count()-1); }
                    for j in 0..row.ports.row_count() { let value=row.ports.row_data(j).unwrap(); if j==stable.row_count() { stable.push(value); } else if stable.row_data(j).as_ref()!=Some(&value) { stable.set_row_data(j,value); } }
                    row.ports=previous.ports.clone();
                }
                if previous!=row { nodes.set_row_data(i,row); }
            } else { rows.nodes.insert(row.id.to_string(),nodes.row_count()); nodes.push(row); } }
            if !value["removed_edges"].as_array().unwrap().is_empty() {
                for i in (0..edges.row_count()).rev() { let id = edges.row_data(i).unwrap().id; if value["removed_edges"].as_array().unwrap().iter().any(|v| v.as_str() == Some(id.as_str())) { edges.remove(i); } }
                rows.edges.clear(); for i in 0..edges.row_count() { rows.edges.insert(edges.row_data(i).unwrap().id.to_string(),i); }
            }
            for item in value["edges"].as_array().unwrap() { let mut row = decode_forgeflowedgedata(item); let index = rows.edges.get(row.id.as_str()).copied(); if let Some(i) = index {
                let previous = edges.row_data(i).unwrap();
                if let Some(stable) = previous.control_points.as_any().downcast_ref::<slint::VecModel<$ForgeFlowPointData>>() {
                    while stable.row_count() > row.control_points.row_count() { stable.remove(stable.row_count()-1); }
                    for j in 0..row.control_points.row_count() { let value=row.control_points.row_data(j).unwrap(); if j==stable.row_count() { stable.push(value); } else if stable.row_data(j).as_ref()!=Some(&value) { stable.set_row_data(j,value); } }
                    row.control_points=previous.control_points.clone();
                }
                if let Some(stable) = previous.motion_points.as_any().downcast_ref::<slint::VecModel<$ForgeFlowPointData>>() {
                    while stable.row_count() > row.motion_points.row_count() { stable.remove(stable.row_count()-1); }
                    for j in 0..row.motion_points.row_count() { let value=row.motion_points.row_data(j).unwrap(); if j==stable.row_count() { stable.push(value); } else if stable.row_data(j).as_ref()!=Some(&value) { stable.set_row_data(j,value); } }
                    row.motion_points=previous.motion_points.clone();
                }
                if previous!=row { edges.set_row_data(i,row); }
            } else { rows.edges.insert(row.id.to_string(),edges.row_count()); edges.push(row); } }
            let mut result = <$ForgeFlowState>::default(); result.token = token;
            result.revision = value["revision"].as_i64().unwrap_or_default() as i32;
            result.ok = value["ok"].as_bool().unwrap_or(false);
            result.error = value["error"].as_str().unwrap_or_default().into();
            result.mode = value["mode"].as_str().unwrap_or_default().into();
            result.dirty = value["dirty"].as_bool().unwrap_or(false);
            result.schema_version = value["schema_version"].as_i64().unwrap_or_default() as i32;
            result.loaded_version = value["loaded_version"].as_i64().unwrap_or_default() as i32;
            result.save_serial = value["save_serial"].as_i64().unwrap_or_default() as i32;
            result.nodes = slint::ModelRc::from(nodes.clone());
            result.edges = slint::ModelRc::from(edges.clone());
            result.viewport = decode_forgeflowviewport(&value["viewport"]);
            result.min_zoom = value["min_zoom"].as_f64().unwrap_or_default() as f32;
            result.max_zoom = value["max_zoom"].as_f64().unwrap_or_default() as f32;
            result.wheel_mode = value["wheel_mode"].as_str().unwrap_or_default().into();
            result.wheel_zoom_step = value["wheel_zoom_step"].as_f64().unwrap_or_default() as f32;
            result.wheel_zoom_requires_control = value["wheel_zoom_requires_control"].as_bool().unwrap_or(false);
            result.auto_pan = value["auto_pan"].as_bool().unwrap_or(false);
            result.auto_pan_margin = value["auto_pan_margin"].as_f64().unwrap_or_default() as f32;
            result.auto_pan_speed = value["auto_pan_speed"].as_f64().unwrap_or_default() as f32;
            result.touch_enabled = value["touch_enabled"].as_bool().unwrap_or(false);
            result.guides = slint::ModelRc::new(slint::VecModel::from(value["guides"].as_array().map(|items| items.iter().map(|v| decode_forgeflowrect(v)).collect::<Vec<_>>()).unwrap_or_default()));
            result.grid_size = value["grid_size"].as_f64().unwrap_or_default() as f32;
            result.selected_node_count = value["selected_node_count"].as_i64().unwrap_or_default() as i32;
            result.bounds = decode_forgeflowrect(&value["bounds"]);
            result.can_undo = value["can_undo"].as_bool().unwrap_or(false);
            result.can_redo = value["can_redo"].as_bool().unwrap_or(false);
            result.selected_id = value["selected_id"].as_str().unwrap_or_default().into();
            result.selected_title = value["selected_title"].as_str().unwrap_or_default().into();
            result.selected_edge = decode_forgeflowedgedata(&value["selected_edge"]);
            result.selected_edge_count = value["selected_edge_count"].as_i64().unwrap_or_default() as i32;
            result.node_count = value["node_count"].as_i64().unwrap_or_default() as i32;
            result.edge_count = value["edge_count"].as_i64().unwrap_or_default() as i32;
            result.selected_is_group = value["selected_is_group"].as_bool().unwrap_or(false);
            result.connection_mode = value["connection_mode"].as_str().unwrap_or_default().into();
            result.connection_radius = value["connection_radius"].as_f64().unwrap_or_default() as f32;
            result.allow_self_connections = value["allow_self_connections"].as_bool().unwrap_or(false);
            result.click_to_connect = value["click_to_connect"].as_bool().unwrap_or(false);
            result
        });
        let service = $ui.global::<$service>();
        let d = docs.clone(); let u = update.clone();
        service.on_create(move |mode| { let id = next.get() + 1; next.set(id); let mut doc = $crate::flow::FlowDocument::new(mode.as_str()).expect("图运行库初始化失败"); let n = std::rc::Rc::new(slint::VecModel::default()); let e = std::rc::Rc::new(slint::VecModel::default()); let mut rows=RowIndex::default(); let state = u(id, &doc.snapshot().unwrap(), &n, &e, &mut rows); d.borrow_mut().insert(id,(doc,n,e,rows)); state });
        let d = docs.clone(); service.on_release(move |token| { d.borrow_mut().remove(&token); });
        let d = docs.clone(); service.on_active_documents(move || d.borrow().len() as i32);
        let d = docs.clone(); let u = update.clone();
        service.on_execute(move |token, action| { let mut docs = d.borrow_mut(); let Some((doc,n,e,rows)) = docs.get_mut(&token) else { let mut state=<$ForgeFlowState>::default();state.error="图文档已释放".into();return state; };
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
            u(token, &doc.execute(&value.to_string()).unwrap(), n, e, rows)
        });
        let d = docs.clone(); service.on_export_json(move |token| d.borrow_mut().get_mut(&token).map(|(doc,_,_,_)| doc.export_json().unwrap().into()).unwrap_or_default());
        service.on_field(|data, key| { let value: Value = json::from_str(data.as_str()).unwrap_or(Value::Null); match &value[key.as_str()] { Value::Null => "".into(), Value::String(s) => s.as_str().into(), other => other.to_string().into() } });
        let d = docs.clone(); service.on_save_file(move |token, name| { let result = (|| -> Result<(), String> { let mut docs = d.borrow_mut(); let (doc,_,_,_) = docs.get_mut(&token).ok_or("图文档已释放")?; if doc.document_status().map_err(|e|e.to_string())?["editing"]==true {return Err("请先结束当前拖动再保存".into());} let path = std::path::Path::new(name.as_str()); if let Some(parent) = path.parent().filter(|p| !p.as_os_str().is_empty()) { std::fs::create_dir_all(parent).map_err(|e| e.to_string())?; } std::fs::write(path,doc.export_json().map_err(|e| e.to_string())?).map_err(|e| e.to_string())?; doc.mark_saved().map_err(|e|e.to_string())?; Ok(()) })(); result.err().unwrap_or_default().into() });
        let d = docs.clone(); let u = update.clone(); service.on_load_file(move |token, name| { let mut docs = d.borrow_mut(); let Some((doc,n,e,rows)) = docs.get_mut(&token) else { let mut state=<$ForgeFlowState>::default();state.error="图文档已释放".into();return state; }; match std::fs::read_to_string(name.as_str()) { Ok(text) => u(token,&doc.execute(&json::json!({"op":"load","text":text}).to_string()).unwrap(),n,e,rows), Err(error) => { let mut state = u(token,&doc.snapshot().unwrap(),n,e,rows); state.ok = false; state.error = error.to_string().into(); state } } });
        let d = docs.clone(); service.on_resolve(move |token, action| { let mut docs = d.borrow_mut(); let Some((doc,_,_,_)) = docs.get_mut(&token) else { return <$ForgeFlowAction>::default(); }; let value = json::json!({
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
        let d = docs.clone(); service.on_preview(move |token, action| { let mut docs = d.borrow_mut(); let Some((doc,_,_,_)) = docs.get_mut(&token) else { return <$ForgeFlowEdgeData>::default(); }; let value = json::json!({
            "op": "preview",
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
        }); let result: Value = doc.execute(&value.to_string()).ok().and_then(|text| json::from_str(&text).ok()).unwrap_or_default(); if result.get("path").is_some() { decode_forgeflowedgedata(&result) } else { <$ForgeFlowEdgeData>::default() } });
        let d = docs.clone(); service.on_query(move |token, request| { let mut docs = d.borrow_mut(); let Some((doc,_,_,_)) = docs.get_mut(&token) else { return "".into(); }; let Ok(mut value) = json::from_str::<Value>(request.as_str()) else { return "".into(); }; if !value.is_object() { return "".into(); } value["op"]="query".into(); doc.execute(&value.to_string()).unwrap_or_default().into() });
        let d=docs.clone(); service.on_bounds(move |token, selected, id| { let mut docs=d.borrow_mut(); let Some((doc,_,_,_))=docs.get_mut(&token) else {return <$ForgeFlowRect>::default();}; doc.bounds(selected,id.as_str()).map(|value|decode_forgeflowrect(&value)).unwrap_or_default() });
        let d=docs.clone(); service.on_search(move |token, text| { let mut docs=d.borrow_mut(); let result=docs.get_mut(&token).and_then(|(doc,_,_,_)|doc.search(text.as_str()).ok()).unwrap_or_default(); slint::ModelRc::new(slint::VecModel::from(result.as_array().map(|items|items.iter().filter_map(|id|id.as_str().map(slint::SharedString::from)).collect::<Vec<_>>()).unwrap_or_default())) });
        let d=docs.clone();let export_weak=$ui.as_weak();
        service.on_export_image(move |token,name,selected,text| {
            let result=(|| -> Result<(),String> {
                let mut options: Value=json::from_str(text.as_str()).map_err(|e|e.to_string())?;
                if !options.is_object()||options.get("node_content").is_some_and(|v|!v.is_object()){return Err("导出选项必须为对象".into());}
                let (before,nodes)={let mut table=d.borrow_mut();let (doc,_,_,_)=table.get_mut(&token).ok_or("图文档已释放")?;(doc.export_json().map_err(|e|e.to_string())?,doc.query("export-nodes","",json::json!({"flag":selected})).map_err(|e|e.to_string())?)};
                let mut content=options["node_content"].as_object().cloned().unwrap_or_default();
                for node in nodes.as_array().ok_or("导出节点不是数组")? {
                    let ui=export_weak.upgrade().ok_or("窗口已销毁")?;
                    let value=ui.global::<$service>().invoke_export_card(node.to_string().into());
                    if !value.is_empty(){content.insert(node["id"].as_str().ok_or("节点缺少标识")?.to_owned(),json::from_str(value.as_str()).map_err(|e|e.to_string())?);}
                }
                options["node_content"]=content.into();
                let mut table=d.borrow_mut();let (doc,_,_,_)=table.get_mut(&token).ok_or("图文档已释放")?;
                if doc.export_json().map_err(|e|e.to_string())?!=before {return Err("导出适配器修改了文档".into());}
                let svg=doc.export_svg(selected,&options).map_err(|e|e.to_string())?;
                $crate::save_flow_image(&svg,std::path::Path::new(name.as_str())).map_err(|e|e.to_string())
            })();result.err().unwrap_or_default().into()
        });
        service.set_connected(true);
    }};
}
