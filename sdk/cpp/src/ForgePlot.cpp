// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#include "ForgePlot.hpp"
#include "ForgeCapture.hpp"
#include <cctype>
#include <fstream>
#include <cmath>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <wincodec.h>
#include <wrl/client.h>
#endif

// 每个场景保留一幅物理像素图像；DPI 变化时重建，矢量导出保持原始尺寸。
slint::Image ForgePlot::display_image(float scale) const {
    if(!std::isfinite(scale)||scale<=0)scale=1;
    if(display_scale_==scale&&display_.size().width>0)return display_;
    std::string svg=svg_;
    for(const std::string attribute:{" width=\""," height=\""}){
        const auto found=svg.find(attribute);if(found==std::string::npos)return {};
        const auto start=found+attribute.size(),end=svg.find('"',start);if(end==std::string::npos)return {};
        double logical=0;const auto parsed=std::from_chars(svg.data()+start,svg.data()+end,logical);
        if(parsed.ec!=std::errc{})return {};
        svg.replace(start,end-start,std::to_string(static_cast<uint32_t>(std::max(1.,std::ceil(logical*scale)))));
    }
    const auto vector=slint::Image::load_from_data(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(svg.data()),svg.size()),"svg");
    const auto pixels=vector.to_rgba8();if(!pixels)return {};
    display_=slint::Image(*pixels);display_scale_=scale;return display_;
}

std::vector<std::tuple<int,int,int>> forge_plot_merge_selection(const std::vector<std::tuple<int,int,int>>& current,const std::vector<std::tuple<int,int,int>>& next,int mode) {
    if(mode==0)return forge_plot_merge_selection({},next,1);
    std::map<std::pair<int,int64_t>,std::array<int,2>> events;const std::array<const std::vector<std::tuple<int,int,int>>*,2> lists{&current,&next};
    for(size_t source=0;source<2;++source)for(const auto& [id,first,last]:*lists[source]){if(first<0||last<first)continue;++events[{id,first}][source];--events[{id,static_cast<int64_t>(last)+1}][source];}
    std::vector<std::tuple<int,int,int>> result;std::array<int,2> active{};int old_id=0;int64_t start=0;bool initial=true;
    const auto selected=[mode](const auto& count){return mode==2?(count[0]>0)!=(count[1]>0):count[0]>0||count[1]>0;};
    for(const auto& [key,delta]:events){const auto [id,index]=key;if(initial||id!=old_id){active={0,0};old_id=id;initial=false;}const bool before=selected(active);active[0]+=delta[0];active[1]+=delta[1];const bool after=selected(active);if(!before&&after)start=index;if(before&&!after)result.emplace_back(id,static_cast<int>(start),static_cast<int>(index-1));}return result;
}
#ifdef _WIN32
class ForgePlotComScope {
public:
    HRESULT status=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
    ~ForgePlotComScope(){if(SUCCEEDED(status))CoUninitialize();}
};
static std::string forge_plot_save_raster(const slint::SharedPixelBuffer<slint::Rgba8Pixel>& pixels,const std::filesystem::path& path,bool jpeg) {
    ForgePlotComScope scope;if(FAILED(scope.status)&&scope.status!=RPC_E_CHANGED_MODE)return "WIC 初始化失败";
    Microsoft::WRL::ComPtr<IWICImagingFactory> factory;Microsoft::WRL::ComPtr<IWICStream> stream;Microsoft::WRL::ComPtr<IWICBitmapEncoder> encoder;Microsoft::WRL::ComPtr<IWICBitmapFrameEncode> frame;
    auto status=CoCreateInstance(CLSID_WICImagingFactory,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(factory.GetAddressOf()));
    if(SUCCEEDED(status))status=factory->CreateStream(stream.GetAddressOf());
    if(SUCCEEDED(status))status=stream->InitializeFromFilename(path.c_str(),GENERIC_WRITE);
    if(SUCCEEDED(status))status=factory->CreateEncoder(jpeg?GUID_ContainerFormatJpeg:GUID_ContainerFormatBmp,nullptr,encoder.GetAddressOf());
    if(SUCCEEDED(status))status=encoder->Initialize(stream.Get(),WICBitmapEncoderNoCache);
    if(SUCCEEDED(status))status=encoder->CreateNewFrame(frame.GetAddressOf(),nullptr);
    if(SUCCEEDED(status))status=frame->Initialize(nullptr);
    if(SUCCEEDED(status))status=frame->SetSize(pixels.width(),pixels.height());
    WICPixelFormatGUID format=GUID_WICPixelFormat24bppBGR;if(SUCCEEDED(status))status=frame->SetPixelFormat(&format);
    if(FAILED(status)||format!=GUID_WICPixelFormat24bppBGR)return "图像编码器不支持 RGB 像素";
    std::vector<BYTE> bytes(static_cast<size_t>(pixels.width())*pixels.height()*3);
    for(size_t i=0;i<bytes.size()/3;++i){const auto pixel=pixels.cbegin()[i];const unsigned alpha=pixel.a;bytes[i*3]=static_cast<BYTE>((pixel.b*alpha+255*(255-alpha)+127)/255);bytes[i*3+1]=static_cast<BYTE>((pixel.g*alpha+255*(255-alpha)+127)/255);bytes[i*3+2]=static_cast<BYTE>((pixel.r*alpha+255*(255-alpha)+127)/255);}
    status=frame->WritePixels(pixels.height(),pixels.width()*3,static_cast<UINT>(bytes.size()),bytes.data());if(SUCCEEDED(status))status=frame->Commit();if(SUCCEEDED(status))status=encoder->Commit();if(SUCCEEDED(status))status=stream->Commit(STGC_DEFAULT);return FAILED(status)?"图像文件写入失败":"";
}
#endif
void ForgePlot::add_item(const ForgePlotItemCore& value) {
    if(value.kind==8&&!value.image_pixels&&value.image){const size_t count=forge_plot_core_image_data(value.image,nullptr,0);if(count){std::vector<uint8_t> bytes(count);forge_plot_core_image_data(value.image,bytes.data(),count);const std::string uri(value.image);const auto end=uri.find(';');const std::string format=uri.substr(11,end-11);const auto image=slint::Image::load_from_data(bytes,format.c_str());const auto pixels=image.to_rgba8();if(pixels){auto core=value;core.image_width=static_cast<int>(pixels->width());core.image_height=static_cast<int>(pixels->height());core.image_byte_count=static_cast<size_t>(pixels->width())*pixels->height()*4;core.image_pixels=reinterpret_cast<const uint8_t*>(pixels->cbegin());forge_plot_core_item(value_,&core);return;}}}
    forge_plot_core_item(value_,&value);
}
std::string ForgePlot::export_file(const std::filesystem::path& path,std::string format)const {
    std::transform(format.begin(),format.end(),format.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});
    if(info().status==2)return info().error;
    if(format=="svg"){std::ofstream out(path,std::ios::binary);out.write(svg_.data(),static_cast<std::streamsize>(svg_.size()));return out.good()?"":"SVG 写入失败";}
    if(format=="pdf"){const auto encoded=path.u8string();const std::string utf8(encoded.begin(),encoded.end());return forge_plot_core_export(value_,utf8.c_str(),1)?"":"PDF 写入失败";}
    const auto pixels=image().to_rgba8();if(!pixels)return "图表图像像素读取失败";
    if(format=="png")return forge_save_capture_png(*pixels,path);
#ifdef _WIN32
    if(format=="jpg"||format=="jpeg"||format=="bmp")return forge_plot_save_raster(*pixels,path,format!="bmp");
#endif
    return "支持 PNG、JPEG、BMP、SVG、PDF";
}
