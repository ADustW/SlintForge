# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

"""核对全部页面的展示范围、代码面板移除情况和本轮窗口截图。"""
from collections import Counter
from pathlib import Path
import argparse
import csv
import re
from component_catalog import ROOT, rows, DEFERRED, NATIVE

# 非组件页面（首页、关于、设置、生命周期、待实现占位）不使用组件页模板。
NON_COMPONENT_PAGES = {"home", "about", "settings", "lifecycle", "pending"}
# 由共享容器页实现模板的路由。
SHARED_TEMPLATE_PAGES = {"data-grid": "GalleryGridPage", "data-tree-grid": "GalleryGridPage"}
RESPONSIVE_SHOWCASES = {"inputs-standard-button", "form-textbox", "form-color-picker", "form-auto-suggest", "feedback-result", "feedback-skeleton", "chart-bar", "chart-line", "chart-professional", "data-grid", "data-tree-grid", "popups-content-dialog"}
SHOWCASES = [line.split('|') for line in (ROOT / 'Gallery/data/showcase-routes.inc').read_text(encoding='utf-8').splitlines() if '|' in line]

OPENED = ["form-date-picker", "form-time-picker", "form-auto-suggest",
          "inputs-progress-button", "inputs-dropdown-button", "form-multi-select-combo",
          "form-multi-tree-combo", "popups-content-dialog", "popups-drawer", "other-tour",
          "chart-bar", "chart-line", "chart-pie", "chart-polar", "chart-bubble", "chart-scatter", "chart-radar", "chart-professional", "data-grid", "data-tree-grid"]

def source_check():
    pages = rows()
    assert pages and len({p[1] for p in pages}) == len(pages)
    assert DEFERRED == set()
    sources = "\n".join(p.read_text(encoding="utf-8") for p in (ROOT / "Gallery/ui").rglob("*.slint"))
    for phrase in ["code-snippet", "复制代码", "Slint 用法"]:
        assert phrase not in sources, f"页面仍有用法代码面板: {phrase}"
    assert "root.height * 0.72" not in sources, "仍有统一大高度预览"
    for phrase in ["GallerySamplePage", "GalleryShowcaseControl", "GalleryDemoCard", "GalleryPickerExample"]:
        assert phrase not in sources, f"仍在使用旧展示组件: {phrase}"
    # 每个组件页都使用统一模板，并提供 API 摘要。
    templated = 0
    for path in sorted((ROOT / "Gallery/ui/page").glob("*.slint")):
        key = path.stem
        if key in NON_COMPONENT_PAGES:
            continue
        source = path.read_text(encoding="utf-8")
        if key in SHARED_TEMPLATE_PAGES:
            assert SHARED_TEMPLATE_PAGES[key] in source, key
            continue
        assert "GalleryComponentPage" in source, f"未使用组件页模板: {key}"
        assert re.search(r"\bapi:\s*\[", source), f"缺少 API 摘要: {key}"
        templated += 1
    for shared in ["grid-page.slint", "chart-page.slint"]:
        source = (ROOT / "Gallery/ui/component" / shared).read_text(encoding="utf-8")
        assert "GalleryComponentPage" in source and re.search(r"\bapi:\s*\[", source), shared
    assert "ForgeDataGrid" in sources and "GalleryGridPages" in sources
    # 截图验收的附加示例数与页面锚点数一致。
    assert len(SHOWCASES) == 50
    for route, count in SHOWCASES:
        if route == "chart-professional":
            # ForgePlot 页使用自身的标签页切换，由 GalleryShowcaseState.tab 直接选择标签。
            assert "GalleryShowcaseState" in (ROOT / "Gallery/ui/component/plot-page.slint").read_text(encoding="utf-8")
            continue
        if route.startswith(('chart-', 'data-grid', 'data-tree-grid')):
            source = (ROOT / "Gallery/ui/component" / ("chart-page.slint" if route.startswith("chart-") else "grid-page.slint")).read_text(encoding="utf-8")
        else:
            source = (ROOT / f'Gallery/ui/page/{route}.slint').read_text(encoding='utf-8')
            assert f'showcase-route: "{route}"' in source, route
        anchors = re.search(r"qa-anchors:\s*\[([^\]]*)\]", source)
        assert anchors and len([a for a in anchors.group(1).split(",") if a.strip()]) == int(count), route
    print(f"GALLERY_PRESENTATION_OK catalog={len(pages)} implemented={len(pages)} native={len(NATIVE)} templated={templated} charts=8 grids=2 deferred=0 code_panels=0")

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

def screenshots(host, qa_root, cpp_build_dir, rust_build_dir, selected_routes):
    from PIL import Image
    directory=ROOT / qa_root / host
    executable=ROOT / (f"{rust_build_dir}/slintforge-gallery-rust.exe" if host=="rust" else f"{cpp_build_dir}/Gallery/cpp/Debug/slintforge_gallery_cpp.exe")
    expected=Counter((row[1],theme,False,-1) for row in rows() for theme in ["dark-zh","light-en","narrow-zh"])
    expected.update((route,"dark-zh",True,-1) for route in OPENED)
    expected[("form-calendar-picker", "dark-zh", True, -1)] += 1
    expected.update((route, theme, True, -1) for route in ["form-date-picker", "form-calendar-picker", "form-time-picker"] for theme in ["light-en", "narrow-zh"])
    # 截图序号区分 12 小时时钟、24 小时数字和 12 小时数字三种时间模式。
    expected.update(("form-time-picker", theme, True, -1) for _ in range(3) for theme in ["dark-zh", "light-en", "narrow-zh"])
    expected.update(("form-calendar-picker", theme, True, -1) for theme in ["dark-zh", "light-en", "narrow-zh"])
    expected.update((route, theme, True, -1) for route in ["form-color-picker", "form-shortcut-picker", "inputs-delay-button"] for theme in ["dark-zh", "light-en", "narrow-zh"])
    for route, count in SHOWCASES:
        for tab in range(int(count)):
            for theme in ["dark-zh", "light-en", "narrow-zh"] if route in RESPONSIVE_SHOWCASES else ["dark-zh"]:
                expected[(route,theme,route == 'popups-content-dialog',tab)] += 1
    if selected_routes:
        expected = Counter({key: value for key, value in expected.items() if key[0] in selected_routes})
    actual=Counter()
    for path in directory.glob("*.png"):
        match=re.fullmatch(r"\d{3}-(.+)-(dark-zh|light-en|narrow-zh)(-open)?(?:-showcase(\d+))?",path.stem)
        assert match,path.name
        route,theme,opened,showcase=match.groups()
        actual[(route,theme,bool(opened),int(showcase) if showcase is not None else -1)]+=1
        assert path.stat().st_mtime>=executable.stat().st_mtime,path.name
        with Image.open(path) as image:
            assert image.width>=960 and image.height>=600,(path.name,image.size)
            image.verify()
    assert expected==actual,(expected-actual,actual-expected)
    print(f"GALLERY_SCREENSHOTS_OK host={host} routes={len({key[0] for key in expected})} showcase_pages={len({key[0] for key in expected if key[3] >= 0})} frames={sum(actual.values())}")

if __name__=="__main__":
    parser=argparse.ArgumentParser()
    parser.add_argument("--geometry",action="store_true")
    parser.add_argument("--qa-root",default="build/component-qa")
    parser.add_argument("--cpp-build-dir",default="build/cpp")
    parser.add_argument("--rust-build-dir",default="target/debug")
    parser.add_argument("--host",action="append",choices=["rust","cpp"],default=[])
    parser.add_argument("--route",action="append",default=[])
    args=parser.parse_args()
    source_check()
    if args.geometry: geometry_check()
    for host in args.host: screenshots(host, args.qa_root, args.cpp_build_dir, args.rust_build_dir, args.route)
