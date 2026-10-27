#pragma once
#include "ForgeCapture.hpp"
#include <fstream>
#include <algorithm>
#include <stdexcept>

// SVG 由共用文档核心生成；PNG 使用 Slint 同一 SVG 解码器，不依赖当前窗口视口。
inline std::string forge_flow_save_image(const std::string& svg,const std::filesystem::path& path) {
    try {
        auto extension=path.extension().string();std::transform(extension.begin(),extension.end(),extension.begin(),[](unsigned char c){return c>='A'&&c<='Z'?static_cast<char>(c+32):static_cast<char>(c);});
        if(extension!=".svg"&&extension!=".png")throw std::runtime_error("图像格式必须为 .svg 或 .png");
        if(!path.parent_path().empty())std::filesystem::create_directories(path.parent_path());
        if(extension==".svg") {
            std::ofstream file(path,std::ios::binary|std::ios::trunc);file.write(svg.data(),static_cast<std::streamsize>(svg.size()));file.close();
            if(!file)throw std::runtime_error("SVG 保存失败，请检查文件路径");
        } else {
            const auto image=slint::Image::load_from_data(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(svg.data()),svg.size()),"svg");
            const auto size=image.size();
            if(size.width==0||size.height==0||double(size.width)*size.height>64000000)throw std::runtime_error("SVG 解码失败或超过 6400 万像素");
            const auto pixels=image.to_rgba8();if(!pixels)throw std::runtime_error("SVG 像素转换失败");
            return forge_save_capture_png(*pixels,path);
        }
        return {};
    } catch(const std::exception& error){return error.what();}
}
