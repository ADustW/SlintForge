"""验证 Gallery 用法示例、公共目录、双端二维码输出及第一方 C++ 命名规则。"""
from pathlib import Path
import argparse
import hashlib
import json
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]

def snippets(compiler):
    output = ROOT / 'build/component-qa/snippets'
    output.mkdir(parents=True, exist_ok=True)
    count, failures = 0, []
    for page in sorted((ROOT / 'examples/component-usage').glob('*.slint')):
        source = page.read_text(encoding='utf-8')
        path = output / page.name
        path.write_text(source, encoding='utf-8')
        result = subprocess.run([compiler, str(path), '-f', 'cpp', '-o', str(path.with_suffix('.h')), '--style', 'fluent', '-L', f'slintforge={ROOT / "ui/slintforge.slint"}'], capture_output=True, text=True, encoding='utf-8')
        count += 1
        if result.returncode:
            failures.append(f'{page.stem}:\n{result.stderr}')
    (output / 'results.txt').write_text('\n'.join(failures) or f'SNIPPETS_OK count={count}\n', encoding='utf-8')
    if failures:
        raise SystemExit('\n'.join(failures))
    assert count == 66, count
    print(f'SNIPPETS_OK count={count}')

def qr_codes():
    sys.path.insert(0, str(ROOT / 'build/qa-python'))
    from PIL import Image
    import zxingcpp
    output = ROOT / 'build/component-qa/qr'
    for name, payload in [('ascii', 'SlintForge|Rust|C++'), ('unicode', '中文组件🙂|SlintForge'), ('url', 'https://slint.dev')]:
        rust = Image.open(output / f'rust-{name}.png').convert('RGBA')
        cpp = Image.open(output / f'cpp-{name}.png').convert('RGBA')
        assert rust.size == cpp.size and rust.tobytes() == cpp.tobytes(), f'两端二维码像素不同: {name}'
        for host, image in [('rust', rust), ('cpp', cpp)]:
            decoded = zxingcpp.read_barcode(image.convert('RGB'))
            assert decoded and decoded.text == payload, f'{host}/{name} 二维码解码失败: {decoded}'
        print(f'QR_DECODE_OK name={name} size={rust.size} sha256={hashlib.sha256(rust.tobytes()).hexdigest()}')

def namespace_policy():
    violations = []
    pattern = re.compile(r'\b(?:using\s+namespace\s+\w|namespace\s+(?:\w+\s*)?\{)')
    for directory in ['sdk/cpp', 'sdk/common', 'Gallery/cpp', 'examples/cpp', 'examples/plot-cpp', 'tests/cpp']:
        for path in (ROOT / directory).rglob('*'):
            if path.suffix not in ['.cpp', '.hpp', '.h']:
                continue
            source = path.read_text(encoding='utf-8')
            source = re.sub(r'/\*.*?\*/|//[^\n]*', '', source, flags=re.S)
            if pattern.search(source): violations.append(str(path.relative_to(ROOT)))
    assert not violations, violations
    print('NAMESPACE_POLICY_OK first_party_only')

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--compiler')
    parser.add_argument('--qr', action='store_true')
    args = parser.parse_args()
    subprocess.run([sys.executable, str(ROOT / 'tools/component_catalog.py'), '--check'], check=True)
    subprocess.run([sys.executable, str(ROOT / 'Gallery/i18n/sync.py'), '--check'], check=True)
    namespace_policy()
    if args.compiler: snippets(args.compiler)
    if args.qr: qr_codes()

if __name__ == '__main__': main()
