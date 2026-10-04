"""从共用 C 数据契约生成 Rust FFI，避免手工维护字段顺序。"""
import argparse
import pathlib
import re

ROOT = pathlib.Path(__file__).resolve().parents[1]
TYPES = {"int": "i32", "double": "f64", "size_t": "usize", "uint32_t": "u32", "uint64_t": "u64", "char": "std::ffi::c_char", "void": "std::ffi::c_void"}

def convert(value):
    value = value.strip()
    const = value.startswith("const ")
    value = value.removeprefix("const ").strip()
    if "*" in value:
        return ("*const " if const else "*mut ") + convert(value.replace("*", ""))
    return "u8" if value=="uint8_t" else TYPES.get(value, value)

def generate():
    text = (ROOT / "sdk/common/ForgePlotCore.h").read_text(encoding="utf-8")
    out = ["// 由 tools/generate_plot_ffi.py 生成；修改共用头文件后重新生成。", "#![allow(non_snake_case)]", ""]
    for name, body in re.findall(r"typedef struct (\w+)\s*\{(.*?)\}\s*\w+;", text, re.S):
        out += ["#[repr(C)]", "#[derive(Clone, Copy, Debug)]", "pub struct " + name + " {"]
        for decl in body.split(";"):
            decl = decl.strip()
            if not decl:
                continue
            match = re.fullmatch(r"(const\s+)?(\w+)\s*(\*?)\s*(.+)", decl)
            const, base, first_pointer, fields = match.groups()
            base = (const or "") + base
            for index, field in enumerate(v.strip() for v in fields.split(",")):
                pointer = field.startswith("*") or (index == 0 and first_pointer)
                out.append("    pub " + field.lstrip("*") + ": " + convert(base + ("*" if pointer else "")) + ",")
        out += ["}", "impl Default for " + name + " {", "    fn default() -> Self {", "        // 契约只包含数字和裸指针，零位模式均有效。", "        unsafe { std::mem::zeroed() }", "    }", "}", ""]
    out += ["extern \"C\" {"]
    for result, name, args in re.findall(r"([\w\s*]+?)\s+(forge_plot_core_\w+)\((.*?)\);", text):
        result = result.strip().split("\n")[-1]
        parsed = []
        for arg in args.split(","):
            match = re.fullmatch(r"(.+?)\s+(\w+)", arg.strip())
            typ, field = match.groups()
            parsed.append(field + ": " + convert(typ))
        suffix = "" if result == "void" else " -> " + convert(result)
        out.append("    pub fn " + name + "(" + ", ".join(parsed) + ")" + suffix + ";")
    out += ["}", ""]
    return "\n".join(out)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    path = ROOT / "sdk/rust/src/plot_ffi.rs"
    expected = generate()
    if args.check:
        if path.read_text(encoding="utf-8") != expected:
            raise SystemExit("PLOT_FFI_STALE")
    else:
        path.write_text(expected, encoding="utf-8")
    print("PLOT_FFI_OK")
