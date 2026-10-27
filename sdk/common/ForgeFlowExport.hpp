#pragma once

static std::string forge_flow_xml(const std::string& text) {
    std::string result;
    for(const unsigned char c:text) {
        if(c=='&')result+="&amp;";else if(c=='<')result+="&lt;";else if(c=='>')result+="&gt;";
        else if(c=='\"')result+="&quot;";else if(c=='\'')result+="&apos;";
        else if(c>=32||c=='\n'||c=='\t'||c=='\r')result+=static_cast<char>(c);
    }
    return result;
}
static std::string forge_flow_svg_color(const ForgeFlowJson& value,const char* key,const std::string& fallback) {
    const auto color=value.value(key,fallback);
    forge_flow_require(color=="none"||(color.size()==7&&color[0]=='#'&&color.find_first_not_of("0123456789abcdefABCDEF",1)==std::string::npos),"导出颜色必须为 #RRGGBB 或 none");
    return color;
}
static std::set<std::string> forge_flow_export_ids(ForgeFlowCore& f,bool selected) {
    auto ids=selected?forge_flow_descendants(f,f.selected_nodes):std::set<std::string>();
    if(!selected)for(const auto& n:f.document["nodes"])ids.insert(forge_flow_id(n));
    return ids;
}
static ForgeFlowJson forge_flow_export_nodes(ForgeFlowCore& f,bool selected) {
    auto result=ForgeFlowJson::array();const auto ids=forge_flow_export_ids(f,selected);
    for(const auto& n:f.document["nodes"])if(ids.contains(forge_flow_id(n))&&!forge_flow_hidden(f,n))result.push_back(forge_flow_scene_node(f,n));
    return result;
}
// 卡片适配器提交受限图元，文本统一转义，图片只接受内嵌 PNG/JPEG。
static void forge_flow_svg_content(std::ostringstream& out,const ForgeFlowJson& content,const std::string& text_color) {
    forge_flow_require(content.is_array()&&content.size()<=1024,"卡片导出内容必须为不超过 1024 项的图元数组");
    for(const auto& item:content) {
        forge_flow_require(item.is_object(),"导出图元必须为对象");
        const auto kind=item.value("type",std::string());const double x=forge_flow_number(item,"x"),y=forge_flow_number(item,"y");
        const auto fill=forge_flow_svg_color(item,"fill",kind=="text"?text_color:"none"),stroke=forge_flow_svg_color(item,"stroke","none");
        const double line=forge_flow_number(item,"stroke_width",1.);
        forge_flow_require(line>=0&&line<=64,"导出图元线宽无效");
        if(kind=="text") {
            const double size=forge_flow_number(item,"font_size",12.);forge_flow_require(size>=6&&size<=128,"导出字体尺寸无效");
            const auto align=item.value("align",std::string("start"));forge_flow_require(align=="start"||align=="middle"||align=="end","导出文本对齐无效");
            out<<"<text x=\""<<x<<"\" y=\""<<y+size<<"\" font-size=\""<<size<<"\" fill=\""<<fill<<"\" text-anchor=\""<<align<<"\">"<<forge_flow_xml(item.value("text",std::string()))<<"</text>";
        } else if(kind=="rect"||kind=="ellipse"||kind=="image") {
            const double w=forge_flow_number(item,"width"),h=forge_flow_number(item,"height");forge_flow_require(w>=0&&h>=0,"导出图元尺寸无效");
            if(kind=="image") {
                const auto uri=item.value("source",std::string());const auto comma=uri.find(',');
                forge_flow_require((uri.starts_with("data:image/png;base64,")||uri.starts_with("data:image/jpeg;base64,"))&&uri.size()<=16*1024*1024&&comma!=std::string::npos&&uri.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=",comma+1)==std::string::npos,"导出图片只接受内嵌 PNG/JPEG 的 Base64 数据");
                out<<"<image x=\""<<x<<"\" y=\""<<y<<"\" width=\""<<w<<"\" height=\""<<h<<"\" href=\""<<uri<<"\" xlink:href=\""<<uri<<"\"/>";
            } else if(kind=="rect") {
                const double radius=forge_flow_number(item,"radius");forge_flow_require(radius>=0,"导出圆角不能为负数");
                out<<"<rect x=\""<<x<<"\" y=\""<<y<<"\" width=\""<<w<<"\" height=\""<<h<<"\" rx=\""<<radius<<"\" fill=\""<<fill<<"\" stroke=\""<<stroke<<"\" stroke-width=\""<<line<<"\"/>";
            } else out<<"<ellipse cx=\""<<x+w/2<<"\" cy=\""<<y+h/2<<"\" rx=\""<<w/2<<"\" ry=\""<<h/2<<"\" fill=\""<<fill<<"\" stroke=\""<<stroke<<"\" stroke-width=\""<<line<<"\"/>";
        } else if(kind=="path") {
            const auto points=item.value("points",ForgeFlowJson::array());forge_flow_require(points.is_array()&&points.size()>=2&&points.size()<=512,"导出路径需要 2 至 512 个点");
            out<<"<path d=\"";for(std::size_t i=0;i<points.size();++i)out<<(i?" L ":"M ")<<forge_flow_number(points[i],"x")<<' '<<forge_flow_number(points[i],"y");
            if(item.value("closed",false))out<<" Z";
            out<<"\" fill=\""<<fill<<"\" stroke=\""<<stroke<<"\" stroke-width=\""<<line<<"\"/>";
        } else throw std::runtime_error("不支持的卡片导出图元");
    }
}
static double forge_flow_svg_text_width(const std::string& text) {
    double width=0;for(const unsigned char c:text)if((c&0xc0)!=0x80)width+=c<128?6.:11.;return width;
}
static std::string forge_flow_export_svg(ForgeFlowCore& f,const ForgeFlowJson& a) {
    const bool selected=a.value("flag",false);const auto ids=forge_flow_export_ids(f,selected);
    const auto options=a.contains("options")?a["options"]:ForgeFlowJson::parse(a.value("text",std::string("{}")));
    forge_flow_require(options.is_object(),"图片导出选项必须为对象");
    const auto theme=options.value("theme",std::string("light"));forge_flow_require(theme=="light"||theme=="dark","导出主题无效");
    const bool dark=theme=="dark";
    const auto background=forge_flow_svg_color(options,"background",dark?"#141b27":"#f8fafc");
    const auto surface=dark?"#202b3b":"#ffffff",border=dark?"#41516b":"#d9e2ee",text=dark?"#e9eff8":"#1e293b",muted=dark?"#9cacbf":"#64748b",accent=dark?"#76a9ff":"#2563eb";
    const double padding=forge_flow_number(options,"padding",24.),scale=forge_flow_number(options,"scale",1.);
    forge_flow_require(padding>=0&&padding<=512&&scale>=.01&&scale<=8,"导出边距或倍率超出范围");
    const auto cards=options.value("node_content",ForgeFlowJson::object());forge_flow_require(cards.is_object(),"卡片导出适配内容必须为对象");
    struct ForgeFlowExportLayer {int layer;std::size_t index;bool node;};
    std::vector<ForgeFlowExportLayer> layers;std::vector<const ForgeFlowJson*> nodes,edges;
    bool first=true;double left=0,top=0,right=1,bottom=1;
    const auto include=[&](double x,double y,double r,double b){if(first){left=x;top=y;right=r;bottom=b;first=false;}else{left=std::min(left,x);top=std::min(top,y);right=std::max(right,r);bottom=std::max(bottom,b);}};
    forge_flow_sync_obstacles(f);
    for(const auto& n:f.document["nodes"])if(ids.contains(forge_flow_id(n))&&!forge_flow_hidden(f,n)) {
        const auto& scene=forge_flow_scene_node(f,n);const double x=scene["x"],y=scene["y"];
        include(x-6,y-6,x+scene["width"].get<double>()+6,y+scene["height"].get<double>()+6);
        layers.push_back({(n.value("is_group",false)?-1000:1)+forge_flow_depth(f,n),nodes.size(),true});nodes.push_back(&n);
    }
    for(const auto& e:f.document["edges"])if((!selected||f.selected_edges.contains(forge_flow_id(e))||(ids.contains(forge_flow_id(e,"source_node"))&&ids.contains(forge_flow_id(e,"target_node"))))&&!e.value("hidden",false)&&!forge_flow_hidden(f,forge_flow_node(f,forge_flow_id(e,"source_node")))&&!forge_flow_hidden(f,forge_flow_node(f,forge_flow_id(e,"target_node")))) {
        const auto& g=forge_flow_edge_geometry(f,e).view;include(g["min_x"],g["min_y"],g["max_x"],g["max_y"]);
        const auto label=e.value("label",std::string());if(!label.empty()){const double w=forge_flow_svg_text_width(label)/2+8,x=g["label_x"],y=g["label_y"];include(x-w,y-12,x+w,y+12);}
        layers.push_back({e.value("layer",0),edges.size(),false});edges.push_back(&e);
    }
    forge_flow_require(!selected||!layers.empty(),"请先选择需要导出的节点或连线");
    const double width=std::max(1.,right-left)+padding*2,height=std::max(1.,bottom-top)+padding*2;
    const double pixel_width=std::ceil(width*scale),pixel_height=std::ceil(height*scale);
    forge_flow_require(pixel_width<=32768&&pixel_height<=32768&&pixel_width*pixel_height<=64000000,"导出超过 6400 万像素，请减小倍率或导出选区");
    std::stable_sort(layers.begin(),layers.end(),[](const auto& a,const auto& b){return a.layer<b.layer;});
    std::ostringstream out;out.imbue(std::locale::classic());out<<std::fixed<<std::setprecision(3);
    out<<"<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" width=\""<<pixel_width<<"\" height=\""<<pixel_height<<"\" viewBox=\""<<left-padding<<' '<<top-padding<<' '<<width<<' '<<height<<"\" font-family=\"Microsoft YaHei, Segoe UI, sans-serif\">";
    if(background!="none")out<<"<rect x=\""<<left-padding<<"\" y=\""<<top-padding<<"\" width=\""<<width<<"\" height=\""<<height<<"\" fill=\""<<background<<"\"/>";
    for(const auto& item:layers) {
        if(!item.node) {
            const auto& e=*edges[item.index];const auto& v=forge_flow_edge_geometry(f,e).view;auto color=e.value("stroke_color",std::string());if(color.empty())color=dark?"#8d9db7":"#8191ab";
            out<<"<g data-edge-id=\""<<forge_flow_xml(forge_flow_id(e))<<"\"><path d=\""<<v["path"].get<std::string>()<<"\" fill=\"none\" stroke=\""<<color<<"\" stroke-width=\""<<e.value("stroke_width",2.)<<"\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>";
            out<<"<path d=\""<<v["arrow_path"].get<std::string>()<<"\" fill=\""<<color<<"\"/><path d=\""<<v["marker_stroke_path"].get<std::string>()<<"\" fill=\"none\" stroke=\""<<color<<"\" stroke-width=\""<<e.value("stroke_width",2.)<<"\"/>";
            const auto label=e.value("label",std::string());if(!label.empty()) {
                const double w=forge_flow_svg_text_width(label)+16,x=v["label_x"],y=v["label_y"];
                out<<"<rect x=\""<<x-w/2<<"\" y=\""<<y-11<<"\" width=\""<<w<<"\" height=\"22\" rx=\"6\" fill=\""<<surface<<"\" stroke=\""<<border<<"\"/><text x=\""<<x<<"\" y=\""<<y+4<<"\" text-anchor=\"middle\" font-size=\"11\" fill=\""<<muted<<"\">"<<forge_flow_xml(label)<<"</text>";
            }
            out<<"</g>";continue;
        }
        const auto& n=forge_flow_scene_node(f,*nodes[item.index]);const auto id=forge_flow_id(n);const double x=n["x"],y=n["y"],w=n["width"],h=n["height"],header=n.value("header_visible",true)?n.value("header_height",36.):0.;
        out<<"<g data-node-id=\""<<forge_flow_xml(id)<<"\"><rect x=\""<<x<<"\" y=\""<<y<<"\" width=\""<<w<<"\" height=\""<<h<<"\" rx=\"8\" fill=\""<<surface<<"\" stroke=\""<<border<<"\"/>";
        const auto clip="card-"+std::to_string(item.index);
        out<<"<defs><clipPath id=\""<<clip<<"\"><rect x=\"0\" y=\"0\" width=\""<<std::max(0.,w-20)<<"\" height=\""<<std::max(0.,h-header-20)<<"\"/></clipPath><clipPath id=\""<<clip<<"-title\"><rect x=\""<<x+10<<"\" y=\""<<y<<"\" width=\""<<w-20<<"\" height=\""<<header<<"\"/></clipPath></defs>";
        if(header>0)out<<"<text x=\""<<x+12<<"\" y=\""<<y+header/2+5<<"\" clip-path=\"url(#"<<clip<<"-title)\" font-size=\"13\" font-weight=\"600\" fill=\""<<text<<"\">"<<forge_flow_xml(n.value("title",std::string()))<<"</text><path d=\"M "<<x+10<<' '<<y+header<<" H "<<x+w-10<<"\" stroke=\""<<border<<"\"/>";
        out<<"<g transform=\"translate("<<x+10<<' '<<y+header+10<<")\" clip-path=\"url(#"<<clip<<")\">";
        if(!n.value("collapsed",false)) {
            if(cards.contains(id))forge_flow_svg_content(out,cards[id],text);
            else {
                const auto& data=n["data"];const auto kind=n.value("kind",std::string("text"));
                const auto description=data.contains("text")&&data["text"].is_string()?data["text"].get<std::string>():data.contains("status")&&data["status"].is_string()?data["status"].get<std::string>():kind=="group"?std::string():kind=="text"?std::string():"["+kind+"]";
                forge_flow_svg_content(out,ForgeFlowJson::array({{{"type","text"},{"text",description}}}),text);
            }
        }
        out<<"</g>";
        for(const auto& port:n["ports"]){const auto p=forge_flow_anchor(n,port);out<<"<circle cx=\""<<p.x<<"\" cy=\""<<p.y<<"\" r=\"5\" fill=\""<<surface<<"\" stroke=\""<<accent<<"\" stroke-width=\"2\"/>";}
        out<<"</g>";
    }
    out<<"</svg>";return out.str();
}
