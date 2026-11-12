# Copyright © 2026 陕西禾又科技有限公司
# SPDX-License-Identifier: GPL-3.0-only

"""组装二进制 SDK，并以白名单核对发布目录；验收证据留在 build。"""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
API_DOCS = ('API.md', 'CAPTURE.en.md', 'COMPONENTS.md', 'CHARTS.md', 'GRIDS.md', 'PLOT.md', 'FLOW.md', 'INTEGRATION.md', 'WINUI3_THEME.md', 'RENDERING.md', 'PICKERS.md', 'RELEASE.md', 'RUST_DLL.md')


INTERFACE_CRATES = ('rust', 'runtime-api', 'rust-build')


def interface_files():
    for name in INTERFACE_CRATES:
        source = ROOT / 'sdk' / name
        for path in sorted(source.rglob('*')):
            relative = path.relative_to(source)
            if path.is_file() and (relative.as_posix() in ('Cargo.toml', 'build.rs')
                                   or (relative.parts[0] == 'src' and path.suffix == '.rs')):
                yield path, Path('sdk') / name / relative


def interface_text(path):
    text = path.read_text(encoding='utf-8')
    if path.suffix == '.rs':
        text = text.split('#[cfg(test)]', 1)[0].rstrip() + '\n'
    if path.name == 'Cargo.toml':
        text = text.split('[dev-dependencies]', 1)[0].rstrip() + '\n'
    return text


def assemble(payload, runtime, compiler, slint_sdk):
    rust = payload / 'sdk/rust'
    for folder in ('runtime', 'bin'):
        (rust / folder).mkdir(parents=True, exist_ok=True)
    for source, relative in interface_files():
        target = payload / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(interface_text(source), encoding='utf-8')
    shutil.copy2(runtime, rust / 'runtime/slintforge_runtime.dll')
    shutil.copy2(runtime, payload / 'rust/slintforge_runtime.dll')
    shutil.copy2(compiler, rust / 'bin/slintforge-ui-compiler.exe')
    shutil.copy2(ROOT / 'tools/windows/Compile-Rust.ps1', rust / 'Compile-Rust.ps1')
    metadata = dict(version='0.1.2', runtime_build_rustc=subprocess.check_output(['rustc', '--version'], text=True).strip(), minimum_client_rust='1.98.0', abi_version=2, target='x86_64-pc-windows-msvc', runtime='runtime/slintforge_runtime.dll', ui='../cpp/share/slintforge/ui/slintforge.slint', dependency_acquisition='cargo-first-online')
    (rust / 'sdk.json').write_text(json.dumps(metadata, ensure_ascii=False, indent=2), encoding='utf-8')
    # 官方 Slint SDK 的头文件、库和编译器是接入依赖，不复制它的安装器或源码。
    for folder in ('include', 'lib', 'bin', 'licenses'):
        source = slint_sdk / folder
        shutil.copytree(source, payload / 'sdk/slint' / folder)
    docs = payload / 'docs'
    docs.mkdir(exist_ok=True)
    for name in API_DOCS:
        shutil.copy2(ROOT / 'docs' / name, docs / name)
    site = ROOT / 'build/api-docs-site'
    if not (site / 'index.html').is_file() or not (site / 'en/index.html').is_file():
        raise RuntimeError('缺少已构建的中英文 API 文档站点')
    shutil.copytree(site, docs / 'site')
    (payload / 'README.md').write_text((ROOT / 'docs/RELEASE.md').read_text(encoding='utf-8').replace('(INTEGRATION.md)', '(docs/INTEGRATION.md)').replace('(PICKERS.md)', '(docs/PICKERS.md)'), encoding='utf-8')


def validate(payload):
    forbidden = []
    allowed_interfaces = {relative.as_posix(): source for source, relative in interface_files()}
    for path in payload.rglob('*'):
        relative = path.relative_to(payload)
        interface = relative.as_posix() in allowed_interfaces
        allowed_src = relative.parts[:3] in (('sdk','rust','src'),('sdk','runtime-api','src'),('sdk','rust-build','src'))
        if any(part.lower() in ('source', '.git', 'target', 'build', 'tests', 'examples', 'tools', '.cache') or part.endswith('-source') for part in relative.parts) or ('src' in relative.parts and not allowed_src):
            forbidden.append(relative.as_posix())
        if path.is_file() and path.suffix.lower() in ('.cpp', '.c', '.cc', '.py', '.qml', '.obj', '.pdb', '.zip', '.7z', '.tar', '.crate', '.rlib', '.rmeta'):
            forbidden.append(relative.as_posix())
        if path.is_file() and path.suffix == '.rs' and not interface:
            forbidden.append(relative.as_posix())
        if path.name in ('CMakeLists.txt', 'Cargo.lock', 'source-sha256.json') or (path.name == 'Cargo.toml' and not interface):
            forbidden.append(relative.as_posix())
        if interface:
            assert path.read_text(encoding='utf-8') == interface_text(allowed_interfaces[relative.as_posix()]), f'接口源码清单不一致: {relative}'
    assert not forbidden, f'发布目录混入项目源码或额外归档: {forbidden}'
    for name in ('rust/slintforge-gallery-rust.exe', 'rust/slintforge_runtime.dll', 'cpp/slintforge_gallery_cpp.exe', 'sdk/rust/sdk.json', 'sdk/rust/runtime/slintforge_runtime.dll', 'sdk/cpp/share/slintforge/ui/slintforge.slint', 'docs/site/index.html', 'docs/site/en/index.html'):
        assert (payload / name).is_file(), name
    for name in allowed_interfaces:
        assert (payload / name).is_file(), name
    stats = content_statistics(payload)
    assert stats['rust_sdk_bytes'] <= 100 * 1024 * 1024, stats
    # 安装器和未压缩目录使用同一份已核对的载荷。
    return {p.relative_to(payload).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(payload.rglob('*')) if p.is_file()}


def content_statistics(payload):
    roots = [payload / 'sdk' / name for name in INTERFACE_CRATES]
    roots.append(payload / 'sdk/cpp/share/slintforge/ui')
    sdk_files = [p for root in roots for p in root.rglob('*') if p.is_file()]
    return dict(interface_source_files=sum(p.suffix == '.rs' for p in sdk_files), core_source_files=0,
                rust_sdk_bytes=sum(p.stat().st_size for p in sdk_files),
                rust_sdk_mib=round(sum(p.stat().st_size for p in sdk_files) / 1024**2, 3),
                precompiled_rust_dependency_files=sum(p.suffix in ('.rlib','.rmeta') for p in sdk_files))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--payload', required=True, type=Path)
    parser.add_argument('--runtime', type=Path)
    parser.add_argument('--compiler', type=Path)
    parser.add_argument('--slint-sdk', type=Path)
    parser.add_argument('--receipt', type=Path)
    parser.add_argument('--final', action='store_true')
    args = parser.parse_args()
    if args.runtime:
        assemble(args.payload, args.runtime, args.compiler, args.slint_sdk)
    hashes = validate(args.payload)
    if args.final:
        assert (args.payload / 'SlintForge-Docs.exe').is_file(), 'SlintForge-Docs.exe'
    if args.receipt:
        args.receipt.write_text(json.dumps({'passed': True, 'files': hashes, **content_statistics(args.payload)}, ensure_ascii=False, indent=2), encoding='utf-8')
    print(f'RELEASE_CONTENT_OK core_source_files=0 archives=0 files={len(hashes)} rust_sdk_mib={content_statistics(args.payload)["rust_sdk_mib"]}')
