# Copyright © 2026 陕西禾又科技有限公司
# SPDX-License-Identifier: GPL-3.0-only

from pathlib import Path
import json
import re
import sys

root = Path(__file__).resolve().parents[2]
catalog = Path(__file__).with_name("en.tsv")
translations = {}
for number, line in enumerate(catalog.read_text(encoding="utf-8").splitlines(), 1):
    if not line or line.startswith("#"):
        continue
    if "|" not in line:
        raise SystemExit(f"en.tsv 第 {number} 行缺少分隔符")
    key, value = line.split("|", 1)
    if not key or not value or key in translations:
        raise SystemExit(f"en.tsv 第 {number} 行无效或重复")
    translations[key] = value

used = set()
pattern = re.compile(r'@tr\("((?:\\.|[^"\\])*)"')
for directory in (root / "ui", root / "Gallery" / "ui", root / "examples" / "ui"):
    for path in directory.rglob("*.slint"):
        used.update(pattern.findall(path.read_text(encoding="utf-8")))
missing = sorted(used - translations.keys())
if missing:
    raise SystemExit("缺少英文翻译：" + "、".join(missing))

header = 'msgid ""\nmsgstr ""\n"Content-Type: text/plain; charset=UTF-8\\n"\n"Language: en\\n"\n\n'
def decode_literal(value):
    # Slint 字面量中的转义要与 PO 中的实际换行一致。
    escapes = {"n": "\n", "r": "\r", "t": "\t", "\\": "\\", '"': '"'}
    return re.sub(r'\\([nrt\\"])', lambda match: escapes[match[1]], value)

body = "".join(
    f"msgid {json.dumps(decode_literal(key), ensure_ascii=False)}\nmsgstr {json.dumps(decode_literal(translations[key]), ensure_ascii=False)}\n\n"
    for key in sorted(used)
)
content = header + body
output_dir = Path(__file__).parent / "en" / "LC_MESSAGES"
check = "--check" in sys.argv
for domain in ("slintforge-gallery-rust", "slintforge_gallery_cpp"):
    output = output_dir / f"{domain}.po"
    if check:
        if not output.exists() or output.read_text(encoding="utf-8") != content:
            raise SystemExit(f"翻译目录未同步：{output.name}")
    else:
        output.write_text(content, encoding="utf-8")
