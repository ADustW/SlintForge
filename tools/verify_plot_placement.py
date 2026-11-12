# Copyright © 2026 陕西禾又科技有限公司
# SPDX-License-Identifier: GPL-3.0-only

"""验证区域内外布局、真实窗口测量文字和双宿主一致性。"""
import argparse
import csv
import hashlib
import json
import statistics
from pathlib import Path
import xml.etree.ElementTree as ET
from PIL import Image


def overlap(a, b):
    return a[0] < b[0] + b[2] - .01 and a[0] + a[2] > b[0] + .01 and a[1] < b[1] + b[3] - .01 and a[1] + a[3] > b[1] + .01


def contains(a, b):
    return a[0] >= b[0] - .01 and a[1] >= b[1] - .01 and a[0] + a[2] <= b[0] + b[2] + .01 and a[1] + a[3] <= b[1] + b[3] + .01


def verify_overlay(output):
    with (output / 'overlay.csv').open(encoding='utf-8') as file:
        rows = list(csv.DictReader(file))
    assert len(rows) == 4
    results = []
    for row in rows:
        name = row['name']
        values = {key: float(value) for key, value in row.items() if key != 'name'}
        wave_x, wave_y = values['rect_x'], values['rect_y']
        for prefix, position in [('legend', values['legend_position']), ('measure', values['measure_position'])]:
            assert position in (6, 7), (name, prefix, position)
            assert abs(values[f'{prefix}_x'] - wave_x - 6) < .002, (name, prefix, '未贴左边界')
            expected_y = wave_y + 6 if position == 6 else wave_y + values['rect_height'] - values[f'{prefix}_height'] - 6
            assert abs(values[f'{prefix}_y'] - expected_y) < .002, (name, prefix, '左上/左下未生效')
        svg = ET.parse(output / f'{name}.svg').getroot()
        background = svg.find('{http://www.w3.org/2000/svg}g/{http://www.w3.org/2000/svg}rect').attrib['fill']
        assert not any(path.attrib.get('fill') == background for path in svg.iter('{http://www.w3.org/2000/svg}path')), (name, '图例出现底色')
        left = round(values['offset_x'] + values['measure_x'])
        top = round(values['offset_y'] + values['measure_y'])
        width, height = round(values['measure_width']), round(values['measure_height'])
        # 紧凑框的文字可占满宽度；排除文字/光标混合像素后比较整个框内原波形。
        bounds = (left + 3, top + 3, left + width - 3, top + height - 3)
        with Image.open(output / f'{name}-on.png') as on, Image.open(output / f'{name}-off.png') as off:
            a = list(on.convert('RGB').crop(bounds).get_flattened_data())
            b = list(off.convert('RGB').crop(bounds).get_flattened_data())
        colored = [(new, old) for new, old in zip(a, b) if max(old) - min(old) > 12]
        assert len(colored) >= 20, (name, '需要有颜色的波形作为透明对比样本', len(colored))
        foreground = (255, 255, 255) if name.startswith('dark') else (0, 0, 0)
        def overlay_pixel(new, old):
            for color in [foreground, (232, 155, 53), (170, 117, 237)]:
                vector = [f - p for f, p in zip(color, old)]
                norm = sum(v * v for v in vector)
                if not norm:
                    continue
                alpha = sum((n - p) * v for n, p, v in zip(new, old, vector)) / norm
                residual = max(abs(n - p - alpha * v) for n, p, v in zip(new, old, vector))
                if .02 < alpha <= 1.01 and residual < 3:
                    return True
            return False
        samples = [(new, old) for new, old in colored if new == old or not overlay_pixel(new, old)]
        assert len(samples) >= 20, (name, '透明对比剩余样本不足', len(samples))
        identical = sum(new == old for new, old in samples) / len(samples)
        assert identical > .95, (name, '测量框底色遮挡原波形', identical, len(colored))
        results.append({'name': name, 'colored_pixels': len(colored), 'compared_pixels':len(samples), 'unchanged_fraction': identical})
    return results


def verify(directory, require_overlay=False):
    results = {}
    report = {'scenes_per_host': 68, 'screenshots': 136, 'hosts': {}}
    for host in ['rust', 'cpp']:
        output = directory / f'{host}-placement'
        with (output / 'placement.csv').open(encoding='utf-8') as file:
            rows = list(csv.DictReader(file))
        assert len(rows) == 68, (host, len(rows))
        for row in rows:
            name = row['name']
            values = {key: float(value) for key, value in row.items() if key != 'name'}
            rect = lambda prefix: tuple(values[f'{prefix}_{field}'] for field in ['x', 'y', 'width', 'height'])
            wave, legend, measure = rect('rect'), rect('legend'), rect('measure')
            svg = ET.parse(output / f'{name}.svg').getroot()
            component = (0, 0, float(svg.attrib['width']), float(svg.attrib['height']))
            for box, area in [(legend, 'legend_area'), (measure, 'measure_area')]:
                assert contains(box, component), (host, name, '超出边界', box, component)
                assert not overlap(box, wave) if values[area] else contains(box, wave), (host, name, area)
            assert not overlap(legend, measure), (host, name, '信息框互相遮挡')
            if name.split('-')[1] == '0':
                assert not list(svg.iter('{http://www.w3.org/2000/svg}image')), '红色示例图片未移除'
            with Image.open(output / f'{name}.png') as image:
                assert image.size == (1080, 760), (host, name, image.size)
                x, y, width, height = measure
                left, top = round(x + values['offset_x']), round(y + values['offset_y'])
                crop = image.convert('RGB').crop((left + 3, top + 3, left + round(width) - 3, top + round(height) - 3))
                pixels = list(crop.get_flattened_data())
                luminance = [(r * .2126 + g * .7152 + b * .0722) for r, g, b in pixels]
                text = sum(v > 110 for v in luminance) if name.startswith('dark') else sum(v < 140 for v in luminance)
                assert text > 100, (host, name, '测量文字对比不足', text)
            results[(host, name)] = values
        print(f'PLOT_PLACEMENT_PIXELS_OK host={host} screenshots={len(rows)}')
        if (output / 'overlay.csv').exists():
            report.setdefault('transparent_corners', {})[host] = verify_overlay(output)
            report['screenshots'] += 8
            print(f'PLOT_TRANSPARENT_CORNERS_OK host={host} cases=4 screenshots=8')
        else:
            assert not require_overlay, (host, '缺少本轮透明叠加对比')
        with (output / 'incremental-frame-performance.csv').open(encoding='utf-8') as file:
            frames = list(csv.DictReader(file))
        assert len(frames) == 64
        for index, frame in enumerate(frames):
            assert int(frame['history_points']) == 1000000 + (index + 1) * 32
            assert int(frame['rendered_points']) < 5000
        totals = sorted(float(frame['total_ms']) for frame in frames)
        appends = sorted(float(frame['append_ms']) for frame in frames)
        p95, append_p95 = totals[59], appends[59]
        assert p95 < 50 and append_p95 < 5, (host, p95, append_p95)
        report['hosts'][host] = {'frames': len(frames), 'frame_median_ms': statistics.median(totals), 'frame_p95_ms': p95, 'append_p95_ms': append_p95}
        print(f'PLOT_PLACEMENT_PERF_OK host={host} median_ms={statistics.median(totals):.3f} p95_ms={p95:.3f} append_p95_ms={append_p95:.3f}')
    for (_, name), values in list(results.items()):
        a, b = results[('rust', name)], results[('cpp', name)]
        assert all(abs(a[key] - b[key]) < .002 for key in a), (name, a, b)
        assert (directory / 'rust-placement' / f'{name}.svg').read_bytes() == (directory / 'cpp-placement' / f'{name}.svg').read_bytes(), (name, '双端矢量不一致')
    print('PLOT_PLACEMENT_PARITY_OK scenes=68 screenshots=136')
    if (directory / 'rust-usability/usability.csv').exists():
        usability = {}
        for host in ['rust', 'cpp']:
            output = directory / f'{host}-usability'
            with (output / 'usability.csv').open(encoding='utf-8') as file:
                rows = [{key: float(value) for key, value in row.items()} for row in csv.DictReader(file)]
            assert len(rows) == 3
            for row, time in zip(rows, [130, 140, 170]):
                assert row['time'] == time and abs(row['minimum'] - time + 10) < .001
                assert abs(row['maximum'] - time) < .001 and row['input_points'] == (time * 100 + 1) * 2
                for cursor in ['a', 'b']:
                    assert row['minimum'] < row[f'cursor_{cursor}'] < row['maximum']
                    assert row['rect_x'] < row[f'pixel_{cursor}'] < row['rect_x'] + row['rect_width']
                assert row['measure_width'] <= 260 and row['measure_height'] <= 80
            for name in ['legend-menu-left-top', 'legend-menu-left-bottom', 'measure-menu-left-top', 'measure-menu-left-bottom',
                         'legend-left-top', 'legend-left-bottom', 'measure-left-top', 'measure-left-bottom', 'live-130', 'live-140', 'live-170']:
                with Image.open(output / f'{name}.png') as image:
                    assert image.size == (1080, 760)
            usability[host] = rows
        assert all(abs(a[key] - b[key]) < .002 for a, b in zip(usability['rust'], usability['cpp']) for key in a)
        report['usability'] = {'menu_clicks_per_host': 4, 'screenshots_per_host': 11, 'hosts': usability}
        print('PLOT_USABILITY_PARITY_OK menu_clicks=8 live_times=130,140,170 screenshots=22')
    root = Path(__file__).resolve().parents[1]
    sources = ['sdk/common/ForgePlotCore.h', 'sdk/common/ForgePlotCore.cpp', 'ui/types/plot.slint', 'ui/charts/plot.slint', 'ui/slintforge.slint',
               'sdk/cpp/include/ForgePlot.hpp', 'sdk/cpp/include/ForgePlotBindings.hpp', 'sdk/rust/src/plot.rs', 'sdk/rust/src/plot_ffi.rs', 'sdk/rust/src/plot_bindings.rs',
               'tools/generate_plot_bindings.py', 'examples/ui/professional-plot-demo.slint', 'examples/ui/professional-plot.slint',
               'examples/plot-rust/src/main.rs', 'examples/plot-rust/src/placement_qa.rs', 'examples/plot-rust/src/streaming_qa.rs',
               'examples/plot-cpp/main.cpp', 'examples/plot-cpp/placement_qa.hpp', 'tests/cpp/ForgePlotTests.cpp', 'Gallery/rust/tests/plot.rs',
               'Gallery/ui/component/plot-page.slint', 'Gallery/i18n/en.tsv', 'tools/verify_plot.py', 'tools/verify_plot_placement.py',
               'docs/PLOT.md', 'docs/PLOT_PLACEMENT_REVIEW.md', 'target/debug/slintforge-gallery-rust.exe',
               'build/cpp/Gallery/cpp/Debug/slintforge_gallery_cpp.exe', 'target/debug/slintforge-plot-example.exe',
               str((directory / 'cpp-build/Release/slintforge_plot_example.exe').resolve().relative_to(root))]
    if (root / 'docs/PLOT_OVERLAY_REVIEW.md').exists():
        sources.append('docs/PLOT_OVERLAY_REVIEW.md')
    if (directory / 'rust-usability/usability.csv').exists():
        sources += ['examples/plot-rust/src/usability_qa.rs', 'examples/plot-cpp/usability_qa.hpp', 'docs/PLOT_USABILITY_REVIEW.md']
    report['hashes'] = {name: hashlib.sha256((root / name).read_bytes()).hexdigest() for name in sources}
    (directory / 'verification.json').write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('directory', type=Path)
    parser.add_argument('--require-overlay', action='store_true')
    args = parser.parse_args()
    verify(args.directory, args.require_overlay)
