#pragma once
#include <slint.h>
#include <string>
#include <string_view>
#include <vector>
#include <optional>

std::string forge_color_to_hex(slint::Color color);
std::optional<slint::Color> forge_color_from_hex(std::string_view text);
slint::Color forge_screen_color();
int forge_screen_color_input();

struct ForgeQrImage {
    bool ok = false;
    slint::Image image;
    int modules = 0;
    std::string error;
};
struct ForgeSyntaxSourceLine { int number = 0; std::string plain; std::string markdown; };

ForgeQrImage forge_qr_code_image(std::string_view text);
std::vector<ForgeSyntaxSourceLine> forge_highlight_syntax(std::string_view code, std::string_view language);
slint::SharedPixelBuffer<slint::Rgba8Pixel> forge_process_backdrop(const slint::SharedPixelBuffer<slint::Rgba8Pixel>& source, int radius, bool glass);

// 传入公共 Slint 契约生成的类型，服务只绑定到当前组件所属窗口。
template<class Service, class QrResult, class SyntaxLine, class Handle>
void forge_bind_widget_services(const Handle& ui) {
    auto& service = ui->template global<Service>();
    service.on_color_to_hex([](slint::Color color) { return slint::SharedString(forge_color_to_hex(color)); });
    service.on_color_hex_valid([](slint::SharedString text) { return forge_color_from_hex(std::string_view(text)).has_value(); });
    service.on_color_from_hex([](slint::SharedString text) { return forge_color_from_hex(std::string_view(text)).value_or(slint::Color{}); });
    service.on_screen_color([] { return forge_screen_color(); });
    service.on_screen_color_input([] { return forge_screen_color_input(); });
#ifdef _WIN32
    service.set_screen_color_supported(true);
#endif
    service.on_suggestion_index([](const std::shared_ptr<slint::Model<slint::SharedString>>& suggestions, slint::SharedString query, int start, int direction) {
        if (!suggestions || direction == 0) return -1;
        const auto prefix = query.to_lowercase();
        const auto step = direction < 0 ? -1 : 1;
        for (std::int64_t index = start; index >= 0 && static_cast<std::size_t>(index) < suggestions->row_count(); index += step) {
            const auto value = suggestions->row_data(static_cast<std::size_t>(index));
            if (value && std::string_view(value->to_lowercase()).starts_with(std::string_view(prefix))) return static_cast<int>(index);
        }
        return -1;
    });
    service.on_qr_code([](slint::SharedString text) {
        const auto source = forge_qr_code_image(std::string_view(text));
        QrResult result{};
        result.ok = source.ok; result.image = source.image; result.modules = source.modules; result.error = source.error.c_str();
        return result;
    });
    service.on_syntax_lines([](slint::SharedString code, slint::SharedString language) -> std::shared_ptr<slint::Model<SyntaxLine>> {
        std::vector<SyntaxLine> lines;
        for (const auto& source : forge_highlight_syntax(std::string_view(code), std::string_view(language))) {
            SyntaxLine line{}; line.number = source.number;
            line.content = slint::StyledText::from_markdown(source.markdown).value_or(slint::StyledText::from_plain_text(source.plain));
            lines.push_back(std::move(line));
        }
        return std::make_shared<slint::VectorModel<SyntaxLine>>(std::move(lines));
    });
    service.on_backdrop([](slint::Image source, int radius, bool glass) {
        const auto pixels = source.to_rgba8();
        return pixels ? slint::Image(forge_process_backdrop(*pixels, radius, glass)) : source;
    });
    service.set_connected(true);
}
