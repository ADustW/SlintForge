// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

static void forge_flow_edit_selected_nodes(ForgeFlowCore& f,const ForgeFlowJson& action) {
    forge_flow_require(!f.selected_nodes.empty(),"请先选择节点");
    auto patch=action.contains("patch")?action["patch"]:action.value("text",std::string()).empty()?ForgeFlowJson{{action.value("target",std::string()),action.value("flag",false)}}:ForgeFlowJson::parse(action.at("text").get<std::string>());
    forge_flow_require(patch.is_object()&&!patch.empty(),"节点属性补丁必须为非空对象");
    const std::set<std::string> allowed{"draggable","resizable","connectable","deletable","selectable","layout_fixed","header_visible","width","height","header_height","min_width","min_height","max_width","max_height","aspect_ratio"};
    for(const auto& [key,value]:patch.items())forge_flow_require(allowed.contains(key),"不支持的批量节点属性");
    // 先校验整个选择，再统一更新；任何节点的权限或父级约束失败均回滚整次修改。
    for(const auto& id:f.selected_nodes){const auto& n=forge_flow_node(f,id);forge_flow_require(!forge_flow_hidden(f,n),"不能批量修改隐藏节点");if(patch.contains("width")||patch.contains("height"))forge_flow_require(n.value("resizable",true),"选区包含不可调整尺寸的节点");}
    for(const auto& id:f.selected_nodes)forge_flow_node(f,id).update(patch);
    forge_flow_validate(f);
}
