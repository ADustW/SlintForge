#pragma once
#include <slint.h>
#include <string>
#include <string_view>
#include <vector>

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
