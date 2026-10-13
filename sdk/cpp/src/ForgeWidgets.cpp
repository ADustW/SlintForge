#include "ForgeWidgets.hpp"
#include "qrcodegen.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <sstream>
#include <iomanip>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

std::string forge_color_to_hex(slint::Color color) {
    std::ostringstream text;
    text << '#' << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(color.red())
         << std::setw(2) << static_cast<int>(color.green()) << std::setw(2) << static_cast<int>(color.blue());
    if (color.alpha() != 255) text << std::setw(2) << static_cast<int>(color.alpha());
    return text.str();
}

std::optional<slint::Color> forge_color_from_hex(std::string_view text) {
    if (!text.empty() && text.front() == '#') text.remove_prefix(1);
    if (text.size() != 6 && text.size() != 8) return std::nullopt;
    std::uint32_t packed = 0;
    for (const unsigned char ch : text) {
        const int digit = ch >= '0' && ch <= '9' ? ch - '0' : ch >= 'a' && ch <= 'f' ? ch - 'a' + 10 : ch >= 'A' && ch <= 'F' ? ch - 'A' + 10 : -1;
        if (digit < 0) return std::nullopt;
        packed = packed * 16 + static_cast<unsigned>(digit);
    }
    if (text.size() == 6) packed = (packed << 8) | 255;
    return slint::Color::from_argb_uint8(packed & 255, (packed >> 24) & 255, (packed >> 16) & 255, (packed >> 8) & 255);
}

slint::Color forge_screen_color() {
#ifdef _WIN32
    POINT point{};
    if (GetCursorPos(&point)) {
        const HDC screen = GetDC(nullptr);
        if (screen) {
            const COLORREF pixel = GetPixel(screen, point.x, point.y); ReleaseDC(nullptr, screen);
            if (pixel != CLR_INVALID) return slint::Color::from_rgb_uint8(GetRValue(pixel), GetGValue(pixel), GetBValue(pixel));
        }
    }
#endif
    return {};
}

int forge_screen_color_input() {
#ifdef _WIN32
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) return 2;
    if (GetAsyncKeyState(VK_RETURN) & 0x8000) return 3;
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) return 1;
#endif
    return 0;
}

ForgeQrImage forge_qr_code_image(std::string_view text) {
    if (text.empty() || text.find('\0') != std::string_view::npos) return {false, {}, 0, "二维码内容不能为空或包含空字符"};
    try {
        const bool ascii = std::all_of(text.begin(), text.end(), [](unsigned char value) { return value < 128; });
        const std::string owned(text);
        const auto qr = ascii ? qrcodegen::QrCode::encodeText(owned.c_str(), qrcodegen::QrCode::Ecc::MEDIUM)
            : qrcodegen::QrCode::encodeSegments({qrcodegen::QrSegment::makeEci(26), qrcodegen::QrSegment::makeBytes(std::vector<std::uint8_t>(text.begin(), text.end()))}, qrcodegen::QrCode::Ecc::MEDIUM);
        const int modules = qr.getSize();
        const std::uint32_t size = static_cast<std::uint32_t>(modules + 8) * 4;
        slint::SharedPixelBuffer<slint::Rgba8Pixel> pixels(size, size);
        for (std::uint32_t y = 0; y < size; ++y) for (std::uint32_t x = 0; x < size; ++x) {
            const std::uint8_t value = qr.getModule(static_cast<int>(x / 4) - 4, static_cast<int>(y / 4) - 4) ? 0 : 255;
            pixels.begin()[y * size + x] = {value, value, value, 255};
        }
        return {true, slint::Image(pixels), modules, {}};
    } catch (const std::exception&) { return {false, {}, 0, "二维码内容超过编码容量"}; }
}

static std::string forge_syntax_escape(std::string_view text) {
    std::string result;
    for (const char value : text) {
        switch (value) {
        case '&': result += "&amp;"; break;
        case '<': result += "&lt;"; break;
        case '>': result += "&gt;"; break;
        case ' ': result += "\xc2\xa0"; break;
        case '\t': result += "\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0"; break;
        case '\\': case '*': case '_': case '[': case ']': case '#': case '`': case '~': result += '\\'; result += value; break;
        default: result += value; break;
        }
    }
    return result;
}

static bool forge_syntax_letter(unsigned char value) { return std::isalpha(value) != 0 || value >= 128; }
static bool forge_syntax_word(unsigned char value) { return std::isalnum(value) != 0 || value >= 128 || value == '_'; }

std::vector<ForgeSyntaxSourceLine> forge_highlight_syntax(std::string_view code, std::string_view language) {
    const std::string_view keywords = language == "rust" ? "fn let mut pub use mod struct enum impl trait match if else for while loop return const static true false self Self move async await where type as in crate super"
        : language == "cpp" || language == "c++" || language == "c" ? "auto const static class struct enum public private protected return if else for while switch case break void int float double bool true false char include define template typename using new delete nullptr virtual override"
        : language == "python" ? "def class import from as if elif else for while return True False None pass try except finally with lambda async await in and or not is yield"
        : language == "json" ? "true false null"
        : language == "slint" ? "import export component inherits property callback in out in-out private public function struct enum if for return true false changed init animate states transitions" : "";
    std::vector<ForgeSyntaxSourceLine> lines;
    bool block_comment = false;
    std::size_t offset = 0;
    for (;;) {
        auto end = code.find('\n', offset);
        auto raw = code.substr(offset, end == std::string_view::npos ? code.size() - offset : end - offset);
        if (!raw.empty() && raw.back() == '\r') raw.remove_suffix(1);
        std::string markdown;
        std::size_t index = 0;
        while (index < raw.size()) {
            const auto start = index;
            std::string_view color = "#dce3f0";
            const auto matches = [&](std::string_view value) { return raw.substr(index).starts_with(value); };
            if (block_comment) {
                const auto closing = raw.find("*/", index);
                index = closing == std::string_view::npos ? raw.size() : closing + 2;
                block_comment = closing == std::string_view::npos; color = "#8bb58a";
            } else if ((language == "python" && raw[index] == '#') || (language != "python" && language != "json" && matches("//"))) {
                index = raw.size(); color = "#8bb58a";
            } else if (language != "python" && language != "json" && matches("/*")) {
                const auto closing = raw.find("*/", index + 2);
                index = closing == std::string_view::npos ? raw.size() : closing + 2;
                block_comment = closing == std::string_view::npos; color = "#8bb58a";
            } else if (raw[index] == '"' || raw[index] == '\'') {
                const char quote = raw[index++];
                while (index < raw.size()) {
                    if (raw[index] == '\\') index = std::min(index + 2, raw.size());
                    else if (raw[index++] == quote) break;
                }
                color = "#efb285";
            } else if (std::isdigit(static_cast<unsigned char>(raw[index]))) {
                ++index;
                while (index < raw.size() && (std::isalnum(static_cast<unsigned char>(raw[index])) || raw[index] == '.' || raw[index] == '_')) ++index;
                color = "#aad4a7";
            } else if (forge_syntax_letter(static_cast<unsigned char>(raw[index])) || raw[index] == '_') {
                ++index;
                while (index < raw.size() && (forge_syntax_word(static_cast<unsigned char>(raw[index])) || (language == "slint" && raw[index] == '-'))) ++index;
                const auto word = raw.substr(start, index - start);
                std::istringstream stream{std::string(keywords)}; std::string keyword;
                while (stream >> keyword) if (keyword == word) { color = "#c39eee"; break; }
            } else { ++index; }
            markdown += "<font color='" + std::string(color) + "'>" + forge_syntax_escape(raw.substr(start, index - start)) + "</font>";
        }
        lines.push_back({static_cast<int>(lines.size() + 1), std::string(raw), std::move(markdown)});
        if (end == std::string_view::npos) break;
        offset = end + 1;
    }
    return lines;
}

static slint::SharedPixelBuffer<slint::Rgba8Pixel> forge_blur_pass(const slint::SharedPixelBuffer<slint::Rgba8Pixel>& source, int radius, bool horizontal) {
    const int width = static_cast<int>(source.width()), height = static_cast<int>(source.height());
    slint::SharedPixelBuffer<slint::Rgba8Pixel> output(width, height);
    const int lines = horizontal ? height : width, length = horizontal ? width : height;
    for (int line = 0; line < lines; ++line) {
        const auto pixel_at = [&](int position) { position = std::clamp(position, 0, length - 1); return source.begin()[horizontal ? line * width + position : position * width + line]; };
        std::array<std::int64_t, 4> sums{};
        const auto add = [&](slint::Rgba8Pixel pixel, int sign) {
            sums[0] += static_cast<std::int64_t>(pixel.r) * pixel.a * sign;
            sums[1] += static_cast<std::int64_t>(pixel.g) * pixel.a * sign;
            sums[2] += static_cast<std::int64_t>(pixel.b) * pixel.a * sign;
            sums[3] += static_cast<std::int64_t>(pixel.a) * sign;
        };
        for (int offset = -radius; offset <= radius; ++offset) add(pixel_at(offset), 1);
        for (int position = 0; position < length; ++position) {
            if (position > 0) { add(pixel_at(position - radius - 1), -1); add(pixel_at(position + radius), 1); }
            const auto color = [&](int channel) -> std::uint8_t { return sums[3] == 0 ? 0 : static_cast<std::uint8_t>(sums[channel] / sums[3]); };
            output.begin()[horizontal ? line * width + position : position * width + line] = {color(0), color(1), color(2), static_cast<std::uint8_t>(sums[3] / (radius * 2 + 1))};
        }
    }
    return output;
}

slint::SharedPixelBuffer<slint::Rgba8Pixel> forge_process_backdrop(const slint::SharedPixelBuffer<slint::Rgba8Pixel>& source, int radius, bool glass) {
    if (source.width() == 0 || source.height() == 0) return source;
    radius = std::clamp(radius, 0, 32);
    auto output = source;
    if (radius > 0) for (int pass = 0; pass < 2; ++pass) output = forge_blur_pass(forge_blur_pass(output, radius, true), radius, false);
    if (glass) {
        const auto before = output;
        const auto width = output.width(), height = output.height();
        for (std::uint32_t y = 0; y < height; ++y) for (std::uint32_t x = 0; x < width; ++x) {
            const double nx = (2.0 * x + 1.0) / width - 1.0, ny = (2.0 * y + 1.0) / height - 1.0;
            const double scale = 1.0 + 0.12 * std::pow(std::max(std::abs(nx), std::abs(ny)), 4);
            const auto sx = static_cast<std::uint32_t>(std::clamp(std::round(((nx / scale + 1.0) * width - 1.0) / 2.0), 0.0, static_cast<double>(width - 1)));
            const auto sy = static_cast<std::uint32_t>(std::clamp(std::round(((ny / scale + 1.0) * height - 1.0) / 2.0), 0.0, static_cast<double>(height - 1)));
            output.begin()[y * width + x] = before.begin()[sy * width + sx];
        }
    }
    return output;
}
