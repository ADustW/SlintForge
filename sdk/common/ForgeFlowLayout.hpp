// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

struct ForgeFlowLayoutOptions {
    std::string algorithm="layered",direction="LR",parent;
    double node_gap=32,rank_gap=80;
    bool recursive=true;
};

// 普通图的布局以同一父级的直接子节点为单位；跨层连线映射到该层的容器。
static std::string forge_flow_layout_representative(ForgeFlowCore& f,std::string id,const std::string& parent) {
    while(!id.empty()) {
        const auto& node=forge_flow_node(f,id);const auto ancestor=forge_flow_id(node,"parent");
        if(ancestor==parent)return id;
        id=ancestor;
    }
    return {};
}

static void forge_flow_layout_scope(ForgeFlowCore& f,const ForgeFlowLayoutOptions& options,const std::string& parent) {
    std::vector<std::string> ids;
    for(const auto& node:f.document["nodes"])if(forge_flow_id(node,"parent")==parent&&!node.value("hidden",false))ids.push_back(forge_flow_id(node));
    if(ids.empty())return;
    // 稳定排序不依赖输入数组顺序，也不依赖上一次布局后的坐标。
    std::sort(ids.begin(),ids.end());
    if(options.recursive)for(const auto& id:ids) {
        const auto node=forge_flow_node(f,id);
        if(node.value("is_group",false)&&!node.value("collapsed",false))forge_flow_layout_scope(f,options,id);
    }
    std::map<std::string,int> index;for(int i=0;i<static_cast<int>(ids.size());++i)index[ids[i]]=i;
    const int size=static_cast<int>(ids.size());std::vector<std::set<int>> next(size),previous(size);
    for(const auto& edge:f.document["edges"]) {
        const auto a=forge_flow_layout_representative(f,forge_flow_id(edge,"source_node"),parent),b=forge_flow_layout_representative(f,forge_flow_id(edge,"target_node"),parent);
        if(a!=b&&index.contains(a)&&index.contains(b)){next[index[a]].insert(index[b]);previous[index[b]].insert(index[a]);}
        else if(options.algorithm=="tree"&&a==b&&index.contains(a)&&forge_flow_id(edge,"source_node")==forge_flow_id(edge,"target_node"))throw std::runtime_error("树布局不支持自连接");
    }
    const bool horizontal=options.direction=="LR"||options.direction=="RL",reverse=options.direction=="RL"||options.direction=="BT";
    std::vector<double> along(size),across(size),u(size),v(size);
    double anchor_x=1e100,anchor_y=1e100;
    for(int i=0;i<size;++i) {
        const auto& node=forge_flow_node(f,ids[i]);const auto& scene=forge_flow_scene_node(f,node);
        along[i]=scene[horizontal?"width":"height"];across[i]=scene[horizontal?"height":"width"];
        anchor_x=std::min(anchor_x,node["x"].get<double>());anchor_y=std::min(anchor_y,node["y"].get<double>());
    }
    if(!parent.empty()){anchor_x=12;const auto& p=forge_flow_node(f,parent);anchor_y=p.value("header_visible",true)?p.value("header_height",36.)+12.:12.;}
    if(options.algorithm=="tree") {
        std::vector<int> roots;std::vector<double> spans(size);std::vector<int> seen(size);int visited=0;
        for(int i=0;i<size;++i){forge_flow_require(previous[i].size()<=1,"树布局要求每个节点最多一个父节点");if(previous[i].empty())roots.push_back(i);}
        std::function<double(int)> measure=[&](int i){forge_flow_require(seen[i]==0,"树布局不能包含环");seen[i]=1;++visited;double sum=0;for(const auto child:next[i])sum+=measure(child)+options.node_gap;return spans[i]=std::max(across[i],std::max(0.,sum-options.node_gap));};
        for(const auto root:roots)measure(root);
        forge_flow_require(visited==size,"树布局不能包含环");
        std::function<void(int,double,double)> place=[&](int i,double x,double y){u[i]=x;v[i]=y+(spans[i]-across[i])/2;double sum=0;for(const auto child:next[i])sum+=spans[child]+options.node_gap;double cursor=y+(spans[i]-std::max(0.,sum-options.node_gap))/2;for(const auto child:next[i]){place(child,x+along[i]+options.rank_gap,cursor);cursor+=spans[child]+options.node_gap;}};
        double cursor=0;for(const auto root:roots){place(root,0,cursor);cursor+=spans[root]+options.node_gap;}
    }else {
        // Tarjan 强连通分量把环收缩为同一层；普通流程和拓扑图都可以布局。
        int serial=0,component_count=0;std::vector<int> number(size,-1),low(size),stack,component(size,-1);std::vector<bool> stacked(size,false);
        std::function<void(int)> visit=[&](int node){number[node]=low[node]=serial++;stack.push_back(node);stacked[node]=true;for(const auto child:next[node]){if(number[child]<0){visit(child);low[node]=std::min(low[node],low[child]);}else if(stacked[child])low[node]=std::min(low[node],number[child]);}if(low[node]==number[node]){for(;;){const int last=stack.back();stack.pop_back();stacked[last]=false;component[last]=component_count;if(last==node)break;}++component_count;}};
        for(int i=0;i<size;++i)if(number[i]<0)visit(i);
        std::vector<std::set<int>> links(component_count);std::vector<int> degrees(component_count),ranks(component_count);
        for(int i=0;i<size;++i)for(const auto child:next[i])if(component[i]!=component[child]&&links[component[i]].insert(component[child]).second)++degrees[component[child]];
        std::set<int> ready;for(int i=0;i<component_count;++i)if(!degrees[i])ready.insert(i);
        while(!ready.empty()){const int current=*ready.begin();ready.erase(ready.begin());for(const auto child:links[current]){ranks[child]=std::max(ranks[child],ranks[current]+1);if(--degrees[child]==0)ready.insert(child);}}
        const int layers=*std::max_element(ranks.begin(),ranks.end())+1;
        std::vector<std::vector<int>> rows(layers);for(int i=0;i<size;++i)rows[ranks[component[i]]].push_back(i);
        std::vector<double> widths(layers),heights(layers),offset(layers),position(size);
        for(auto& row:rows)for(int i=0;i<static_cast<int>(row.size());++i)position[row[i]]=i;
        // 两次重心扫描减少跨层交叉；相同重心保持 ID 顺序，保证布局可重复。
        for(int pass=0;pass<2;++pass)for(int layer=1;layer<layers;++layer) {
            auto score=[&](int node){double total=0;int count=0;for(const auto parent_node:previous[node])if(ranks[component[parent_node]]<layer){total+=position[parent_node];++count;}return count?total/count:position[node];};
            std::stable_sort(rows[layer].begin(),rows[layer].end(),[&](int a,int b){return score(a)<score(b);});
            for(int i=0;i<static_cast<int>(rows[layer].size());++i)position[rows[layer][i]]=i;
        }
        double maximum_height=0;
        for(int layer=0;layer<layers;++layer){for(const auto node:rows[layer]){widths[layer]=std::max(widths[layer],along[node]);heights[layer]+=across[node]+options.node_gap;}heights[layer]-=options.node_gap;maximum_height=std::max(maximum_height,heights[layer]);if(layer)offset[layer]=offset[layer-1]+widths[layer-1]+options.rank_gap;}
        for(int layer=0;layer<layers;++layer){double cursor=(maximum_height-heights[layer])/2;for(const auto node:rows[layer]){u[node]=offset[layer];v[node]=cursor;cursor+=across[node]+options.node_gap;}}
    }
    double extent=0;for(int i=0;i<size;++i)extent=std::max(extent,u[i]+along[i]);
    std::vector<ForgeFlowBox> occupied;
    for(const auto& id:ids){const auto& node=forge_flow_node(f,id);if(node.value("layout_fixed",false)||!node.value("draggable",true))occupied.push_back(forge_flow_box(node));}
    for(int i=0;i<size;++i) {
        auto& node=forge_flow_node(f,ids[i]);if(node.value("layout_fixed",false)||!node.value("draggable",true))continue;
        const double forward=reverse?extent-u[i]-along[i]:u[i];
        double x=anchor_x+(horizontal?forward:v[i]),y=anchor_y+(horizontal?v[i]:forward);
        // 固定节点保留原位，移动节点沿横向排列轴避开它们以及已放置节点。
        const double width=node["width"],height=node["height"];
        for(std::size_t attempt=0;attempt<=occupied.size();++attempt) {
            bool shifted=false;const ForgeFlowBox box{x,y,x+width,y+height};
            for(const auto& obstacle:occupied)if(forge_flow_boxes_intersect(box,obstacle)) {
                if(horizontal)y=obstacle.bottom+std::max(1.,options.node_gap);else x=obstacle.right+std::max(1.,options.node_gap);
                shifted=true;break;
            }
            if(!shifted)break;
        }
        node["x"]=x;node["y"]=y;occupied.push_back({x,y,x+width,y+height});
    }
    f.scene_nodes.clear();
    if(!parent.empty()) {
        auto& group=forge_flow_node(f,parent);
        if(group.value("resizable",true)) {
            double width=group.value("min_width",64.),height=group.value("min_height",48.);
            for(const auto& id:ids){const auto& child=forge_flow_node(f,id);width=std::max(width,child["x"].get<double>()+child["width"].get<double>()+12);height=std::max(height,child["y"].get<double>()+child["height"].get<double>()+12);}
            const double ratio=group.value("aspect_ratio",0.);if(ratio>0){width=std::max(width,height*ratio);height=width/ratio;}
            forge_flow_require(width<=group.value("max_width",1e6)&&height<=group.value("max_height",1e6),"布局所需空间超过分组最大尺寸");group["width"]=width;group["height"]=height;
        }
    }
}

static void forge_flow_graph_layout(ForgeFlowCore& f,const ForgeFlowJson& action) {
    if(f.document["mode"]=="mindmap"){forge_flow_layout(f);return;}
    auto value=action.contains("options")?action["options"]:action.value("text",std::string()).empty()?ForgeFlowJson::object():ForgeFlowJson::parse(action["text"].get<std::string>());
    forge_flow_require(value.is_object(),"布局选项必须为对象");ForgeFlowLayoutOptions options;
    options.algorithm=value.value("algorithm",action.value("target",std::string("layered")));if(options.algorithm.empty())options.algorithm="layered";
    const int direction=forge_flow_integer(action,"number",0);forge_flow_require(direction>=0&&direction<4,"布局方向无效");
    options.direction=value.value("direction",std::vector<std::string>{"LR","RL","TB","BT"}[direction]);
    options.node_gap=forge_flow_number(value,"node_gap",action.value("height",0.)>0?forge_flow_number(action,"height"):32);
    options.rank_gap=forge_flow_number(value,"rank_gap",action.value("width",0.)>0?forge_flow_number(action,"width"):80);
    options.parent=value.value("parent",forge_flow_id(action));options.recursive=value.value("recursive",true);
    forge_flow_require(options.algorithm=="layered"||options.algorithm=="tree","布局算法无效");
    forge_flow_require(options.direction=="LR"||options.direction=="RL"||options.direction=="TB"||options.direction=="BT","布局方向无效");
    forge_flow_require(options.node_gap>=0&&options.node_gap<=2000&&options.rank_gap>=0&&options.rank_gap<=2000,"布局间距必须位于 0 至 2000");
    if(!options.parent.empty())forge_flow_require(forge_flow_node(f,options.parent).value("is_group",false),"布局范围必须是分组");
    std::map<std::string,ForgeFlowPoint> before;
    for(const auto& node:f.document["nodes"]){const auto& world=forge_flow_scene_node(f,node);before[forge_flow_id(node)]={world["x"],world["y"]};}
    forge_flow_layout_scope(f,options,options.parent);f.scene_nodes.clear();
    // 内部端点整体平移时同步手动路径；相对位置变化时保留用户控制点，避免破坏自定义路径。
    for(auto& edge:f.document["edges"])if(edge.contains("control_points")) {
        const auto source=forge_flow_id(edge,"source_node"),target=forge_flow_id(edge,"target_node");
        const auto& a=forge_flow_scene_node(f,forge_flow_node(f,source));const auto& b=forge_flow_scene_node(f,forge_flow_node(f,target));
        const double dx=a["x"].get<double>()-before[source].x,dy=a["y"].get<double>()-before[source].y;
        if(std::abs(dx-(b["x"].get<double>()-before[target].x))<.001&&std::abs(dy-(b["y"].get<double>()-before[target].y))<.001)for(auto& point:edge["control_points"]){point["x"]=point["x"].get<double>()+dx;point["y"]=point["y"].get<double>()+dy;}
    }
    forge_flow_validate(f);
}
