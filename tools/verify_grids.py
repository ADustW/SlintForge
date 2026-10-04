"""核对表格双宿主契约、独立消费者状态截图及安装文件。"""
import argparse
import csv
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def contract():
    directory = ROOT / "build/grids-qa"
    records = [list(csv.reader((directory / f"{host}-contract.csv").open(encoding="utf-8")))
               for host in ["rust", "cpp"]]
    assert records[0] == records[1], "Rust/C++ 表格契约不一致"
    scenes = [record for record in records[0] if record[0] == "scene"]
    assert len(scenes) == 10 and {int(row[1]) for row in scenes} == set(range(10))
    print(f"GRID_CONTRACT_PARITY_OK cases=10 records={len(records[0])}")


def consumer(host, cpp_build_dir, rust_build_dir):
    from PIL import Image
    executable = ROOT / (f"{rust_build_dir}/slintforge-example-rust.exe" if host == "rust"
                         else f"{cpp_build_dir}/examples/cpp/Debug/slintforge_example_cpp.exe")
    files = sorted((ROOT / "build/grids-qa").glob(f"consumer-{host}-*.png"))
    assert {path.name for path in files} == {f"consumer-{host}-{i}.png" for i in range(7)}
    for path in files:
        assert path.stat().st_mtime >= executable.stat().st_mtime, f"截图早于当前程序: {path.name}"
        with Image.open(path) as image:
            assert image.width >= 800 and image.height >= 540, (path.name, image.size)
            image.verify()
    print(f"GRID_CONSUMER_SCREENSHOTS_OK host={host} states=7")


def installed(prefix, source_root):
    prefix = ROOT / prefix
    source_root = ROOT / source_root
    sources = list((source_root / "ui").rglob("*.slint"))
    for path in sources:
        destination = prefix / "share/slintforge/ui" / path.relative_to(source_root / "ui")
        assert destination.read_bytes() == path.read_bytes(), f"安装 UI 不一致: {path}"
    header = source_root / "sdk/cpp/include/ForgeGrids.hpp"
    assert header.read_bytes() == (prefix / "include/ForgeGrids.hpp").read_bytes()
    assert (prefix / "lib/slintforge_sdk.lib").is_file()
    print(f"GRID_INSTALL_OK shared_slint={len(sources)} grid_header=identical library=present")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", action="append", choices=["rust", "cpp"], default=[])
    parser.add_argument("--cpp-build-dir", default="build/cpp-grids")
    parser.add_argument("--rust-build-dir", default="target/debug")
    parser.add_argument("--source-root", default=".")
    parser.add_argument("--install-prefix")
    args = parser.parse_args()
    contract()
    for host in args.host:
        consumer(host, args.cpp_build_dir, args.rust_build_dir)
    if args.install_prefix:
        installed(args.install_prefix, args.source_root)
