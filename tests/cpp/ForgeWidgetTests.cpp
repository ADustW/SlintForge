#include "ForgeWidgets.hpp"
#include "ForgeCapture.hpp"
#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iostream>

int main() {
    const auto directory = std::filesystem::path("build/component-qa/qr");
    std::filesystem::create_directories(directory);
    for (const auto& entry : std::vector<std::pair<std::string, std::string>>{{"ascii", "SlintForge|Rust|C++"}, {"unicode", "中文组件🙂|SlintForge"}, {"url", "https://slint.dev"}}) {
        const auto qr = forge_qr_code_image(entry.second); assert(qr.ok && qr.modules >= 21);
        const auto pixels = qr.image.to_rgba8(); assert(pixels);
        const auto size = static_cast<std::uint32_t>((qr.modules + 8) * 4); assert(pixels->width() == size && pixels->height() == size);
        for (std::uint32_t y = 0; y < size; ++y) for (std::uint32_t x = 0; x < size; ++x) {
            const auto p = pixels->begin()[y * size + x]; assert(p.a == 255 && p.r == p.g && p.g == p.b && (p.r == 0 || p.r == 255));
            if (x < 16 || y < 16 || x >= size - 16 || y >= size - 16) assert(p.r == 255);
        }
        assert(forge_save_capture_png(*pixels, directory / ("cpp-" + entry.first + ".png")).empty());
    }
    assert(!forge_qr_code_image("").ok); assert(!forge_qr_code_image(std::string_view("a\0b", 3)).ok); assert(!forge_qr_code_image(std::string(5000, 'x')).ok);
    const auto lines = forge_highlight_syntax("fn main() {\r\n\tlet x = \"<script>&_*中文\"; // 42\n/* multi\nend */ return 3;\n}", "rust");
    assert(lines.size() == 5 && lines[1].number == 2 && lines[0].plain == "fn main() {");
    assert(lines[1].markdown.find("&lt;script&gt;&amp;") != std::string::npos); assert(lines[1].markdown.find("<script>") == std::string::npos);
    assert(lines[2].markdown.find("#8bb58a") != std::string::npos && lines[3].markdown.find("#c39eee") != std::string::npos);
    for (const auto& line : lines) assert(slint::StyledText::from_markdown(line.markdown).has_value());
    for (const auto& [language, source] : std::vector<std::pair<std::string, std::string>>{{"python", "def run(): # ready"}, {"json", "{\"ok\":true}"}, {"cpp", "const auto x = 4;"}, {"slint", "in-out property<int> value: 2;"}}) assert(forge_highlight_syntax(source, language)[0].markdown.find("#c39eee") != std::string::npos);
    assert(forge_highlight_syntax("a\n", "unknown").size() == 2);
    slint::SharedPixelBuffer<slint::Rgba8Pixel> solid(16, 12);
    for (auto& p : solid) p = {37, 64, 128, 128};
    const auto blurred = forge_process_backdrop(solid, 32, true);
    assert(std::equal(solid.begin(), solid.end(), blurred.begin(), [](auto a, auto b) { return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a; }));
    slint::SharedPixelBuffer<slint::Rgba8Pixel> alpha(3, 1);
    alpha.begin()[0] = {255, 0, 0, 0}; alpha.begin()[1] = {0, 0, 255, 255}; alpha.begin()[2] = {255, 0, 0, 0};
    for (auto p : forge_process_backdrop(alpha, 1, false)) assert(p.r == 0 && p.b == 255 && p.a > 0);
    slint::SharedPixelBuffer<slint::Rgba8Pixel> large(96, 64);
    for (std::uint32_t y = 0; y < 64; ++y) for (std::uint32_t x = 0; x < 96; ++x) large.begin()[y * 96 + x] = {static_cast<std::uint8_t>(x * 2), static_cast<std::uint8_t>(y * 3), 80, 255};
    const auto glass = forge_process_backdrop(large, 0, true);
    assert(!std::equal(large.begin(), large.end(), glass.begin(), [](auto a, auto b) { return a.r == b.r && a.g == b.g; }));
    const auto clamped = forge_process_backdrop(large, 99, false), limit = forge_process_backdrop(large, 32, false);
    assert(std::equal(clamped.begin(), clamped.end(), limit.begin(), [](auto a, auto b) { return a.r == b.r && a.g == b.g; }));
    std::cout << "WIDGET_SDK_OK qr=3 syntax=5_languages backdrop=alpha_and_refraction\n";
}
