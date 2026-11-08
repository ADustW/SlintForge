// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

// 跨平台截图编码：使用 PNG RGBA8 与无压缩 DEFLATE，避免依赖 Windows WIC。
static void forge_png_u32(std::vector<uint8_t> &bytes, uint32_t value) {
    for (int shift = 24; shift >= 0; shift -= 8)
        bytes.push_back(static_cast<uint8_t>(value >> shift));
}
static uint32_t forge_png_crc(const uint8_t *bytes, size_t size) {
    uint32_t crc = 0xffffffff;
    for (size_t i = 0; i < size; ++i) {
        crc ^= bytes[i];
        for (int bit = 0; bit < 8; ++bit)
            crc = (crc & 1) ? (crc >> 1) ^ 0xedb88320u : crc >> 1;
    }
    return crc ^ 0xffffffff;
}
static void forge_png_chunk(std::vector<uint8_t> &png, const char *type, const std::vector<uint8_t> &data) {
    forge_png_u32(png, static_cast<uint32_t>(data.size()));
    const size_t start = png.size();
    png.insert(png.end(), type, type + 4);
    png.insert(png.end(), data.begin(), data.end());
    forge_png_u32(png, forge_png_crc(png.data() + start, png.size() - start));
}
static std::string forge_png_write_rgba(const uint8_t *pixels, uint32_t width, uint32_t height,
                                        const std::filesystem::path &path) {
    if (!pixels || !width || !height || width > 8192 || height > 8192)
        return "PNG 图像尺寸无效或超过 8192 像素";
    std::vector<uint8_t> scanlines;
    scanlines.reserve(static_cast<size_t>(height) * (1 + static_cast<size_t>(width) * 4));
    for (uint32_t y = 0; y < height; ++y) {
        scanlines.push_back(0);
        const auto *row = pixels + static_cast<size_t>(y) * width * 4;
        scanlines.insert(scanlines.end(), row, row + static_cast<size_t>(width) * 4);
    }
    std::vector<uint8_t> deflate{0x78, 0x01};
    for (size_t start = 0; start < scanlines.size();) {
        const auto n = static_cast<uint16_t>(std::min<size_t>(65535, scanlines.size() - start));
        deflate.push_back(start + n == scanlines.size() ? 1 : 0);
        deflate.push_back(static_cast<uint8_t>(n));
        deflate.push_back(static_cast<uint8_t>(n >> 8));
        const auto inverse = static_cast<uint16_t>(~n);
        deflate.push_back(static_cast<uint8_t>(inverse));
        deflate.push_back(static_cast<uint8_t>(inverse >> 8));
        deflate.insert(deflate.end(), scanlines.begin() + start, scanlines.begin() + start + n);
        start += n;
    }
    uint32_t a = 1, b = 0;
    for (uint8_t value : scanlines) {
        a = (a + value) % 65521;
        b = (b + a) % 65521;
    }
    forge_png_u32(deflate, (b << 16) | a);
    std::vector<uint8_t> png{137, 80, 78, 71, 13, 10, 26, 10}, header;
    forge_png_u32(header, width);
    forge_png_u32(header, height);
    header.insert(header.end(), {8, 6, 0, 0, 0});
    forge_png_chunk(png, "IHDR", header);
    forge_png_chunk(png, "IDAT", deflate);
    forge_png_chunk(png, "IEND", {});
    std::ofstream output(path, std::ios::binary);
    output.write(reinterpret_cast<const char *>(png.data()), static_cast<std::streamsize>(png.size()));
    output.close();
    return output ? std::string{} : "PNG 文件写入失败";
}
