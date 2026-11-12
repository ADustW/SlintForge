# Copyright © 2026 陕西禾又科技有限公司
# SPDX-License-Identifier: GPL-3.0-only

"""检查两端明暗主题图例的透明度、贴边间距及测量框配色。"""
import argparse
import csv
import hashlib
import json
import re
import statistics
import xml.etree.ElementTree as ET
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
SVG = '{http://www.w3.org/2000/svg}'


def near(a, b):
    assert abs(a - b) < 0.05, (a, b)


def verify_host(directory):
    rows = list(csv.DictReader((directory / 'appearance.csv').open(encoding='utf-8')))
    assert len(rows) == 48
    panel = None
    measurements = {}
    for row in rows:
        position, style = int(row['position']), int(row['style'])
        name = f"{row['theme']}-legend-{position}-{style}"
        tree = ET.parse(directory / f'{name}.svg').getroot()
        rect = tree.find(f'.//{SVG}clipPath/{SVG}rect')
        bounds = tuple(float(rect.attrib[key]) for key in ('x', 'y', 'width', 'height'))
        if panel is None:
            panel = bounds
        for actual, expected in zip(bounds, panel):
            near(actual, expected)
        background = tree.find(f'./{SVG}g/{SVG}rect').attrib['fill']
        paths = list(tree.iter(SVG + 'path'))
        assert not any(p.attrib.get('fill') == background for p in paths), (name, '图例存在不透明底色')
        if style == 1:
            frame = [p for p in paths if p.attrib['d'].rstrip().endswith('Z')][-1]
            assert frame.attrib['fill'] == 'none', name
            values = [float(x) for x in re.findall(r'-?\d+(?:\.\d+)?', frame.attrib['d'])]
            xs, ys = values[::2], values[1::2]
            left, top, right, bottom = min(xs), min(ys), max(xs), max(ys)
            x, y, width, height = bounds
            if position in (2, 6, 7):
                near(left - x, 6)
            elif position in (3, 4, 5):
                near(x + width - right, 6)
            else:
                near((left + right) / 2, x + width / 2)
            if position in (0, 5, 7):
                near(y + height - bottom, 6)
            elif position in (1, 4, 6):
                near(top - y, 6)
            else:
                near((top + bottom) / 2, y + height / 2)
        with Image.open(directory / f'{name}.png') as image:
            image.load()
            assert image.size == (1080, 760), (name, image.size)
        if position == 4 and style == 2:
            # 测量截图使用右上图例，测量框位于绘图区左下角。
            x = float(row['offset_x']) + bounds[0] + 8
            y = float(row['offset_y']) + bounds[1] + bounds[3] - 54 - 8
            with Image.open(directory / f"measure-{row['theme']}.png") as image:
                image = image.convert('RGB')
                tail = image.crop((round(x + 340), round(y + 8), round(x + 356), round(y + 46)))
                luminance = [sum(tail.getpixel((x, y))) / 3 for y in range(tail.height) for x in range(tail.width)]
                median = statistics.median(luminance)
                text = image.crop((round(x + 8), round(y + 8), round(x + 320), round(y + 46)))
                text_luminance = [sum(text.getpixel((x, y))) / 3 for y in range(text.height) for x in range(text.width)]
                if row['theme'] == 'dark':
                    assert median < 80 and max(text_luminance) > 150, ('深色测量框缺少可读对比', median)
                else:
                    assert median > 180 and min(text_luminance) < 100, ('浅色测量框缺少可读对比', median)
                measurements[row['theme']] = {'background_mean_rgb_median': median}
    assert len(list(directory.glob('*.png'))) == 54
    return {'legend_cases': 48, 'screenshots': 54, 'gap_logical_px': 6, 'measurement': measurements}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--directory', default='build/plot-appearance-qa')
    args = parser.parse_args()
    directory = ROOT / args.directory
    report = {host: verify_host(directory / f'{host}-window') for host in ('rust', 'cpp')}
    for host in ('rust', 'cpp'):
        frames = list(csv.DictReader((directory / f'{host}-performance' / 'incremental-frame-performance.csv').open(encoding='utf-8')))
        assert len(frames) == 64
        for index, frame in enumerate(frames):
            assert int(frame['history_points']) == 1000000 + (index + 1) * 32
            assert int(frame['rendered_points']) < 5000
        values = sorted(float(frame['total_ms']) for frame in frames)
        append = sorted(float(frame['append_ms']) for frame in frames)
        p95 = values[int((len(values) - 1) * .95)]
        append_p95 = append[int((len(append) - 1) * .95)]
        assert p95 < 50 and append_p95 < 5, (host, p95, append_p95)
        report[host]['performance'] = {'frames': 64, 'frame_median_ms': statistics.median(values), 'frame_p95_ms': p95, 'append_p95_ms': append_p95}
    for theme in ('dark', 'light'):
        for position in range(8):
            for style in range(3):
                name = f'{theme}-legend-{position}-{style}.svg'
                assert (directory / 'rust-window' / name).read_bytes() == (directory / 'cpp-window' / name).read_bytes(), ('两端图例场景不一致', name)
    files = ['sdk/common/ForgePlotCore.cpp', 'ui/charts/plot.slint', 'examples/ui/professional-plot.slint',
             'examples/plot-rust/src/main.rs', 'examples/plot-rust/src/appearance_qa.rs',
             'examples/plot-cpp/main.cpp', 'examples/plot-cpp/appearance_qa.hpp',
             'tests/cpp/ForgePlotTests.cpp', 'Gallery/rust/tests/plot.rs', 'tools/verify_plot_appearance.py',
             'docs/PLOT.md', 'docs/PLOT_APPEARANCE_REVIEW.md',
             'target/debug/slintforge-gallery-rust.exe', 'build/cpp/Gallery/cpp/Debug/slintforge_gallery_cpp.exe',
             'target/debug/slintforge-plot-example.exe', 'build/plot-appearance-qa/cpp-build/Release/slintforge_plot_example.exe']
    report['hashes'] = {name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest() for name in files}
    (directory / 'verification.json').write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding='utf-8')
    print(json.dumps({host: report[host] for host in ('rust', 'cpp')}, ensure_ascii=False))
    print('PLOT_APPEARANCE_VERIFICATION_OK')


if __name__ == '__main__':
    main()
