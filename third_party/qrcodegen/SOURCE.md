# QR-Code-generator

Project Nayuki，MIT 许可，原始版本 `v1.8.0`。
[官方仓库](https://github.com/nayuki/QR-Code-generator/tree/v1.8.0)
源码保持原样；每个源文件顶部包含完整许可证。
`rust/Cargo.toml` 是本项目添加的构建包装，未改动上游源码；上游含绘图伪代码的文档示例不作为 Rust 工作区 doctest，二维码实际输出由两端测试和独立解码器验证。

SHA-256：
- cpp/qrcodegen.cpp: `1f3b3fcdac6954c32cf583ccd02ec9b5901f756a38c461acedc70be4a77d3757`
- cpp/qrcodegen.hpp: `b779c3b156cf7a57ce789d6fee4fc991ccc2913774d26c909d22bb8f26b2a793`
- rust/lib.rs: `a4546464799faaea27393dd60a24be08349618fb98333decbaF686d4c98ca410`

静态规则和格式化检查排除此目录，第三方的 namespace 和原文注释不属于第一方代码。
