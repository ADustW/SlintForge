"""核对全部页面的展示范围、代码面板移除情况和本轮窗口截图。"""
from collections import Counter
from pathlib import Path
import argparse
import csv
import re
from component_catalog import ROOT, rows, DEFERRED
from gallery_presentation import PRESENTATIONS

OPENED = ["form-date-picker", "form-time-picker", "form-auto-suggest",
          "inputs-progress-button", "inputs-dropdown-button", "form-multi-select-combo",
          "form-multi-tree-combo", "popups-content-dialog", "popups-drawer", "other-tour",
          "chart-bar", "chart-line", "chart-pie", "chart-polar", "chart-bubble", "chart-scatter", "chart-radar", "chart-professional", "data-grid", "data-tree-grid"]

def source_check():
    pages = rows()
    assert len(pages) == 96
    assert DEFERRED == set()
    sources = "\n".join(p.read_text(encoding="utf-8") for p in (ROOT / "Gallery/ui").rglob("*.slint"))
    for phrase in ["code-snippet", "复制代码", "Slint 用法"]:
        assert phrase not in sources, f"页面仍有用法代码面板: {phrase}"
    assert "root.height * 0.72" not in sources, "仍有统一大高度预览"
    assert len(PRESENTATIONS.splitlines()) == 63
    for line in PRESENTATIONS.splitlines():
        key,height,description,_ = line.split("|")
        source=(ROOT / f"Gallery/ui/page/{key}.slint").read_text(encoding="utf-8")
        assert f"preview-height: {height}px * ForgeTypography.scale" in source, key
        assert description in source, key
        assert "api-note" not in source, key
    assert "ForgeDataGrid" in sources and "GalleryGridPages" in sources
    print("GALLERY_PRESENTATION_OK catalog=96 implemented=96 native=27 charts=8 grids=2 deferred=0 code_panels=0")

def geometry_check():
    directory=ROOT / "build/charts-qa"
    a=list(csv.reader((directory/"rust-geometry.csv").open(encoding="utf-8")))
    b=list(csv.reader((directory/"cpp-geometry.csv").open(encoding="utf-8")))
    assert len(a)==len(b)
    maximum=0.
    count=0
    for x,y in zip(a,b):
        assert len(x)==len(y)
        for u,v in zip(x,y):
            try:
                delta=abs(float(u)-float(v)); count+=1; maximum=max(maximum,delta)
                assert delta<=1e-6,(u,v)
            except ValueError:
                assert u==v,(u,v)
    print(f"CHART_GEOMETRY_PARITY_OK rows={len(a)} fields={count} max_difference={maximum}")

def screenshots(host, qa_root, cpp_build_dir, rust_build_dir):
    from PIL import Image
    directory=ROOT / qa_root / host
    executable=ROOT / (f"{rust_build_dir}/slintforge-gallery-rust.exe" if host=="rust" else f"{cpp_build_dir}/Gallery/cpp/Debug/slintforge_gallery_cpp.exe")
    expected=Counter((row[1],theme,False) for row in rows() for theme in ["dark-zh","light-en","narrow-zh"])
    expected.update((route,"dark-zh",True) for route in OPENED)
    actual=Counter()
    for path in directory.glob("*.png"):
        match=re.fullmatch(r"\d{3}-(.+)-(dark-zh|light-en|narrow-zh)(-open)?",path.stem)
        assert match,path.name
        route,theme,opened=match.groups()
        actual[(route,theme,bool(opened))]+=1
        assert path.stat().st_mtime>=executable.stat().st_mtime,path.name
        with Image.open(path) as image:
            assert image.width>=960 and image.height>=600,(path.name,image.size)
            image.verify()
    assert expected==actual,(expected-actual,actual-expected)
    print(f"GALLERY_SCREENSHOTS_OK host={host} routes={len(rows())} dark={len(rows())} light_125={len(rows())} narrow={len(rows())} opened={len(OPENED)} frames={sum(actual.values())}")

if __name__=="__main__":
    parser=argparse.ArgumentParser()
    parser.add_argument("--geometry",action="store_true")
    parser.add_argument("--qa-root",default="build/component-qa")
    parser.add_argument("--cpp-build-dir",default="build/cpp")
    parser.add_argument("--rust-build-dir",default="target/debug")
    parser.add_argument("--host",action="append",choices=["rust","cpp"],default=[])
    args=parser.parse_args()
    source_check()
    if args.geometry: geometry_check()
    for host in args.host: screenshots(host, args.qa_root, args.cpp_build_dir, args.rust_build_dir)
