# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

"""第一方 C/C++ namespace 规则检查；排除第三方、生成与构建目录。"""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
FOLDERS = ('sdk/common', 'sdk/cpp', 'Gallery/cpp', 'examples/cpp', 'examples/plot-cpp', 'examples/flow-cpp', 'tests/cpp')
SUFFIXES = {'.c','.cc','.cpp','.h','.hpp'}

def check():
    failures=[]
    count=0
    for directory in FOLDERS:
        for path in (ROOT/directory).rglob('*'):
            if not path.is_file() or path.suffix not in SUFFIXES:
                continue
            if any(part in ('third_party','vendor','CMakeFiles','build','target') or part.startswith(('build-','cmake-build-')) for part in path.relative_to(ROOT).parts):
                continue
            count+=1
            text=path.read_text(encoding='utf-8-sig')
            text=re.sub(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',lambda m:'\n'*m.group().count('\n'),text,flags=re.S)
            for match in re.finditer(r'\b(?:using\s+namespace\b|namespace\s*(?:[A-Za-z_]\w*(?:\s*::\s*\w+)*\s*)?\{)',text):
                failures.append(f'{path.relative_to(ROOT)}:{text[:match.start()].count(chr(10))+1}')
    if failures:
        raise SystemExit('FIRST_PARTY_NAMESPACE_VIOLATION '+', '.join(failures))
    print(f'FIRST_PARTY_NAMESPACE_OK files={count}')

if __name__=='__main__':
    check()
