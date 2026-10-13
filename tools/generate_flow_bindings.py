"""由共享 Slint 数据契约生成双端模型转换，保留运行中的节点模型实例。"""
from pathlib import Path
import re
import argparse

ROOT = Path(__file__).resolve().parents[1]

def generate():
    source = (ROOT / 'ui/types/flow.slint').read_text(encoding='utf-8')
    types = {name: [(key.replace('-', '_'), kind) for key, kind in re.findall(r'([\w-]+):\s*(\[\w+\]|\w+)', body)] for name, body in re.findall(r'export struct (\w+)\s*\{([^}]+)\}', source)}
    names = list(types)
    cpp = ['// 由 tools/generate_flow_bindings.py 生成。', '#pragma once', '#include "ForgeFlow.hpp"', '#include <slint.h>', '#include <nlohmann/json.hpp>', '#include <map>', '#include <fstream>', '#include <filesystem>', '#include <algorithm>', '']
    params = ', '.join('typename '+name for name in names)
    for name, fields in types.items():
        if name == 'ForgeFlowState': continue
        cpp += [f'template<{params}>', f'{name} forge_flow_decode_{name}(const nlohmann::json& value) {{', f'    {name} result{{}};']
        for key, kind in fields:
            if kind.startswith('['):
                item = kind[1:-1]
                cpp += [f'    std::vector<{item}> {key};', f'    for (const auto& item : value.at("{key}")) {key}.push_back(forge_flow_decode_{item}<{", ".join(names)}>(item));', f'    result.{key} = std::make_shared<slint::VectorModel<{item}>>(std::move({key}));']
            else:
                ctype = {'string': 'std::string', 'float': 'float', 'int': 'int', 'bool': 'bool'}[kind]
                suffix = '.c_str()' if kind == 'string' else ''
                cpp += [f'    result.{key} = value.at("{key}").get<{ctype}>(){suffix};']
        cpp += ['    return result;', '}', '']
    cpp += [f'template<{params}, typename Service, typename Window>', 'void forge_bind_flow_services(const slint::ComponentHandle<Window>& ui) {', '    struct Document { ForgeFlowDocument core; std::shared_ptr<slint::VectorModel<ForgeFlowNodeData>> nodes = std::make_shared<slint::VectorModel<ForgeFlowNodeData>>(); std::shared_ptr<slint::VectorModel<ForgeFlowEdgeData>> edges = std::make_shared<slint::VectorModel<ForgeFlowEdgeData>>(); explicit Document(const std::string& mode) : core(mode) {} };', '    auto documents = std::make_shared<std::map<int, std::shared_ptr<Document>>>();', '    auto next = std::make_shared<int>(0);', '    auto update = [documents](int token, const std::string& text) {', '        ForgeFlowState state{}; state.token = token;', '        auto found = documents->find(token); if (found == documents->end()) { state.error = "图文档已释放"; return state; }', '        auto d = found->second;', '        const auto value = nlohmann::json::parse(text);']
    for model, item in [('nodes', 'ForgeFlowNodeData'), ('edges', 'ForgeFlowEdgeData')]:
        cpp += [f'        for (std::size_t i = d->{model}->row_count(); i-- > 0;) {{', f'            const std::string id(d->{model}->row_data(i)->id); bool remove = false;', f'            for (const auto& gone : value.at("removed_{model}")) if (gone.get<std::string>() == id) remove = true;', f'            if (remove) d->{model}->erase(i);', '        }', f'        for (const auto& item : value.at("{model}")) {{', f'            auto row = forge_flow_decode_{item}<{", ".join(names)}>(item);', f'            std::size_t i = 0; for (; i < d->{model}->row_count(); ++i) if (d->{model}->row_data(i)->id == row.id) break;', f'            if (i == d->{model}->row_count()) d->{model}->push_back(row); else d->{model}->set_row_data(i, row);', '        }']
    for key, kind in types['ForgeFlowState']:
        if key == 'token': continue
        if kind.startswith('['): cpp += [f'        state.{key} = d->{key};']
        elif kind in names: cpp += [f'        state.{key} = value.at("{key}").empty() ? {kind}{{}} : forge_flow_decode_{kind}<{", ".join(names)}>(value.at("{key}"));']
        else:
            ctype = {'string': 'std::string', 'float': 'float', 'int': 'int', 'bool': 'bool'}[kind]
            cpp += [f'        state.{key} = value.at("{key}").get<{ctype}>()'+('.c_str()' if kind=='string' else '')+';']
    cpp += ['        return state;', '    };', '    auto& service = ui->template global<Service>();', '    service.on_create([documents, next, update](slint::SharedString mode) { int id = ++*next; auto doc = std::make_shared<Document>(std::string(mode)); (*documents)[id] = doc; return update(id, doc->core.snapshot()); });', '    service.on_release([documents](int token) { documents->erase(token); });', '    service.on_execute([documents, update](int token, const ForgeFlowAction& action) {', '        auto found = documents->find(token); if (found == documents->end()) { ForgeFlowState state{}; state.error = "图文档已释放"; return state; }', '        nlohmann::json value = nlohmann::json::object();']
    for key, kind in types['ForgeFlowAction']:
        cpp += [f'        value["{key}"] = '+(f'std::string(action.{key})' if kind == 'string' else f'action.{key}')+';']
    cpp += ['        return update(token, found->second->core.execute(value.dump()));', '    });', '    service.on_export_json([documents](int token) { auto found = documents->find(token); return slint::SharedString(found == documents->end() ? "" : found->second->core.export_json().c_str()); });', '    service.on_field([](slint::SharedString data, slint::SharedString key) { auto value = nlohmann::json::parse(std::string(data), nullptr, false); if (!value.is_object() || !value.contains(std::string(key))) return slint::SharedString(); const auto& field = value[std::string(key)]; return slint::SharedString((field.is_string() ? field.get<std::string>() : field.dump()).c_str()); });', '    service.on_save_file([documents](int token, slint::SharedString name) { try { auto found = documents->find(token); if (found == documents->end()) return slint::SharedString("图文档已释放"); std::filesystem::path path = std::filesystem::u8path(std::string(name)); if (!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path()); std::ofstream file(path, std::ios::binary | std::ios::trunc); file << found->second->core.export_json(); file.close(); if (!file) return slint::SharedString("保存失败，请检查文件路径"); return slint::SharedString(); } catch (const std::exception& e) { return slint::SharedString(e.what()); } });', '    service.on_load_file([documents, update](int token, slint::SharedString name) { auto found = documents->find(token); if (found == documents->end()) return ForgeFlowState{}; try { std::ifstream file(std::filesystem::u8path(std::string(name)), std::ios::binary); if (!file) throw std::runtime_error("无法打开图文档"); std::string text((std::istreambuf_iterator<char>(file)), {}); return update(token, found->second->core.execute(nlohmann::json{{"op","load"},{"text",text}}.dump())); } catch (const std::exception& e) { auto state = update(token, found->second->core.snapshot()); state.ok = false; state.error = e.what(); return state; } });', '    service.set_connected(true);', '}', '']

    # 连接预检与写入分离，让业务获得已解析的节点及端口 ID。
    insert = cpp.index('    service.set_connected(true);')
    body = ['    service.on_resolve([documents](int token, const ForgeFlowAction& action) {', '        auto it = documents->find(token); if (it == documents->end()) return ForgeFlowAction{};', '        nlohmann::json value = nlohmann::json::object();']
    for key, kind in types['ForgeFlowAction']:
        body += [f'        value["{key}"] = '+(f'std::string(action.{key})' if kind == 'string' else f'action.{key}')+';']
    body += ['        value["op"] = "resolve";', f'        return forge_flow_decode_ForgeFlowAction<{", ".join(names)}>(nlohmann::json::parse(it->second->core.execute(value.dump())));', '    });']
    cpp[insert:insert] = body

    rust = ['// 由 tools/generate_flow_bindings.py 生成。', '#[macro_export]', 'macro_rules! bind_flow_services {', '    ($ui:expr, $service:ty, '+', '.join('$'+name+':ty' for name in names)+') => {{', '        use slint::{ComponentHandle, Model};', '        use $crate::flow::json::{self, Value};']
    for name, fields in types.items():
        if name == 'ForgeFlowState': continue
        rust += [f'        let decode_{name.lower()} = move |value: &Value| -> ${name} {{', f'            let mut result = <${name}>::default();']
        for key,kind in fields:
            if kind.startswith('['): expr = f'slint::ModelRc::new(slint::VecModel::from(value["{key}"].as_array().unwrap().iter().map(|v| decode_{kind[1:-1].lower()}(v)).collect::<Vec<_>>()))'
            else: expr = f'value["{key}"].'+{'string':'as_str().unwrap_or_default().into()', 'float':'as_f64().unwrap_or_default() as f32','int':'as_i64().unwrap_or_default() as i32','bool':'as_bool().unwrap_or(false)'}[kind]
            rust += [f'            result.{key} = {expr};']
        rust += ['            result', '        };']
    rust += ['        type Documents<N,E> = std::collections::HashMap<i32, ($crate::flow::FlowDocument, std::rc::Rc<slint::VecModel<N>>, std::rc::Rc<slint::VecModel<E>>)>;', '        let docs = std::rc::Rc::new(std::cell::RefCell::new(Documents::<$ForgeFlowNodeData, $ForgeFlowEdgeData>::new()));', '        let next = std::rc::Rc::new(std::cell::Cell::new(0));', '        let update = std::rc::Rc::new(move |token: i32, text: &str, nodes: &std::rc::Rc<slint::VecModel<$ForgeFlowNodeData>>, edges: &std::rc::Rc<slint::VecModel<$ForgeFlowEdgeData>>| -> $ForgeFlowState {', '            let value: Value = json::from_str(text).expect("无效的核心响应");']
    for model,item in [('nodes','ForgeFlowNodeData'),('edges','ForgeFlowEdgeData')]:
        rust += [f'            for i in (0..{model}.row_count()).rev() {{ let id = {model}.row_data(i).unwrap().id; if value["removed_{model}"].as_array().unwrap().iter().any(|v| v.as_str() == Some(id.as_str())) {{ {model}.remove(i); }} }}', f'            for item in value["{model}"].as_array().unwrap() {{ let row = decode_{item.lower()}(item); let index = (0..{model}.row_count()).find(|&i| {model}.row_data(i).unwrap().id == row.id); if let Some(i) = index {{ {model}.set_row_data(i,row); }} else {{ {model}.push(row); }} }}']
    rust += ['            let mut result = <$ForgeFlowState>::default(); result.token = token;']
    for key, kind in types['ForgeFlowState']:
        if key=='token':continue
        if kind.startswith('['): expr=f'slint::ModelRc::from({key}.clone())'
        elif kind in names: expr=f'decode_{kind.lower()}(&value["{key}"])'
        else: expr = f'value["{key}"].'+{'string':'as_str().unwrap_or_default().into()', 'float':'as_f64().unwrap_or_default() as f32','int':'as_i64().unwrap_or_default() as i32','bool':'as_bool().unwrap_or(false)'}[kind]
        rust += [f'            result.{key} = {expr};']
    rust += ['            result', '        });', '        let service = $ui.global::<$service>();', '        let d = docs.clone(); let u = update.clone();', '        service.on_create(move |mode| { let id = next.get() + 1; next.set(id); let mut doc = $crate::flow::FlowDocument::new(mode.as_str()).expect("图运行库初始化失败"); let n = std::rc::Rc::new(slint::VecModel::default()); let e = std::rc::Rc::new(slint::VecModel::default()); let state = u(id, &doc.snapshot().unwrap(), &n, &e); d.borrow_mut().insert(id,(doc,n,e)); state });', '        let d = docs.clone(); service.on_release(move |token| { d.borrow_mut().remove(&token); });', '        let d = docs.clone(); let u = update.clone();', '        service.on_execute(move |token, action| { let mut docs = d.borrow_mut(); let Some((doc,n,e)) = docs.get_mut(&token) else { return <$ForgeFlowState>::default(); };', '            let value = json::json!({']
    for key, kind in types['ForgeFlowAction']:
        rust += [f'                "{key}": action.{key}'+('.as_str()' if kind=='string' else '')+',']
    rust += ['            });', '            u(token, &doc.execute(&value.to_string()).unwrap(), n, e)', '        });', '        let d = docs.clone(); service.on_export_json(move |token| d.borrow_mut().get_mut(&token).map(|(doc,_,_)| doc.export_json().unwrap().into()).unwrap_or_default());', '        service.on_field(|data, key| { let value: Value = json::from_str(data.as_str()).unwrap_or(Value::Null); match &value[key.as_str()] { Value::Null => "".into(), Value::String(s) => s.as_str().into(), other => other.to_string().into() } });', '        let d = docs.clone(); service.on_save_file(move |token, name| { let result = (|| -> Result<(), String> { let mut docs = d.borrow_mut(); let (doc,_,_) = docs.get_mut(&token).ok_or("图文档已释放")?; let path = std::path::Path::new(name.as_str()); if let Some(parent) = path.parent().filter(|p| !p.as_os_str().is_empty()) { std::fs::create_dir_all(parent).map_err(|e| e.to_string())?; } std::fs::write(path,doc.export_json().map_err(|e| e.to_string())?).map_err(|e| e.to_string()) })(); result.err().unwrap_or_default().into() });', '        let d = docs.clone(); let u = update.clone(); service.on_load_file(move |token, name| { let mut docs = d.borrow_mut(); let Some((doc,n,e)) = docs.get_mut(&token) else { return <$ForgeFlowState>::default(); }; match std::fs::read_to_string(name.as_str()) { Ok(text) => u(token,&doc.execute(&json::json!({"op":"load","text":text}).to_string()).unwrap(),n,e), Err(error) => { let mut state = u(token,&doc.snapshot().unwrap(),n,e); state.ok = false; state.error = error.to_string().into(); state } } });', '        service.set_connected(true);', '    }};', '}', '']
    insert = rust.index('        service.set_connected(true);')
    body = ['        let d = docs.clone(); service.on_resolve(move |token, action| { let mut docs = d.borrow_mut(); let Some((doc,_,_)) = docs.get_mut(&token) else { return <$ForgeFlowAction>::default(); }; let value = json::json!({']
    for key, kind in types['ForgeFlowAction']:
        body += [f'            "{key}": '+('"resolve"' if key == 'op' else f'action.{key}'+('.as_str()' if kind == 'string' else ''))+',']
    body += ['        }); decode_forgeflowaction(&json::from_str(&doc.execute(&value.to_string()).unwrap()).unwrap()) });']
    rust[insert:insert] = body
    return {'sdk/cpp/include/ForgeFlowBindings.hpp':'\n'.join(cpp), 'sdk/rust/src/flow_bindings.rs':'\n'.join(rust)}

if __name__ == '__main__':
    parser = argparse.ArgumentParser(); parser.add_argument('--check', action='store_true'); args=parser.parse_args()
    for name, text in generate().items():
        path=ROOT/name
        if args.check:
            assert path.read_text(encoding='utf-8') == text, name
        else: path.write_text(text,encoding='utf-8')
    print('FLOW_BINDINGS_OK')
