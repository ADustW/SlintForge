#pragma once

// 简单通道无法连通时，在障碍物边界形成的稀疏网格上寻路。
// 方向进入状态，使拐弯成本与路径长度一起参与确定性选择。
static std::vector<ForgeFlowPoint> forge_flow_search_route(const ForgeFlowRouteContext& context, ForgeFlowPoint start, ForgeFlowPoint end) {
    std::vector<double> xs{start.x,end.x},ys{start.y,end.y};
    for(const auto& obstacle:context.flow.obstacles) {
        // 使用本条线实际需要的净空，保留窄通道中的可行坐标。
        const auto box=forge_flow_route_box(context,obstacle);
        xs.push_back(box.left);xs.push_back(box.right);
        ys.push_back(box.top);ys.push_back(box.bottom);
    }
    const auto unique=[](auto& values){std::sort(values.begin(),values.end());values.erase(std::unique(values.begin(),values.end()),values.end());};
    unique(xs);unique(ys);
    if(xs.size()*ys.size()>262144) return {};
    const int width=static_cast<int>(xs.size()),height=static_cast<int>(ys.size());
    const auto index=[&](ForgeFlowPoint p){return static_cast<int>(std::lower_bound(ys.begin(),ys.end(),p.y)-ys.begin())*width+static_cast<int>(std::lower_bound(xs.begin(),xs.end(),p.x)-xs.begin());};
    const auto point=[&](int index){return ForgeFlowPoint{xs[index%width],ys[index/width]};};
    const int from=index(start),to=index(end),states=width*height*2;
    std::vector<double> costs(states,std::numeric_limits<double>::infinity());
    std::vector<int> previous(states,-1);
    std::priority_queue<std::tuple<double,double,int>,std::vector<std::tuple<double,double,int>>,std::greater<>> pending;
    for(int direction=0;direction<2;++direction){costs[from*2+direction]=0;pending.emplace(0.,0.,from*2+direction);}
    std::unordered_map<uint64_t,bool> clear;
    int goal=-1;
    while(!pending.empty()) {
        const auto [estimate,cost,state]=pending.top();pending.pop();
        if(cost!=costs[state])continue;
        const int cell=state/2;
        if(cell==to){goal=state;break;}
        const auto a=point(cell);
        const int neighbors[4]={cell%width>0?cell-1:-1,cell%width+1<width?cell+1:-1,cell>=width?cell-width:-1,cell+width<width*height?cell+width:-1};
        for(int direction=0;direction<4;++direction) {
            const int next=neighbors[direction];if(next<0)continue;
            const auto b=point(next);
            const uint64_t key=static_cast<uint64_t>(std::min(cell,next))*static_cast<uint64_t>(width*height)+std::max(cell,next);
            auto found=clear.find(key);
            if(found==clear.end())found=clear.emplace(key,forge_flow_clear_route(context,{a,b})).first;
            if(!found->second)continue;
            const int axis=direction/2,target=next*2+axis;
            const double candidate=cost+std::abs(b.x-a.x)+std::abs(b.y-a.y)+(axis==state%2?0:16);
            if(candidate>=costs[target])continue;
            costs[target]=candidate;previous[target]=state;
            pending.emplace(candidate+std::abs(end.x-b.x)+std::abs(end.y-b.y),candidate,target);
        }
    }
    if(goal<0)return {};
    std::vector<ForgeFlowPoint> result;
    for(int state=goal;state>=0;state=previous[state])result.push_back(point(state/2));
    std::reverse(result.begin(),result.end());
    return forge_flow_simplify(std::move(result));
}
