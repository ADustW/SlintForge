# Copyright © 2026 陕西禾又科技有限公司
# SPDX-License-Identifier: GPL-3.0-only

"""验证专业图表的双端矢量、位图、PDF 导出和网格性能样本一致性。"""
from pathlib import Path
import argparse
import base64
import csv
import io
import json
import subprocess
import xml.etree.ElementTree as ET

from PIL import Image, ImageChops, ImageStat
from pypdf import PdfReader

ROOT = Path(__file__).resolve().parents[1]
SVG = '{http://www.w3.org/2000/svg}'
XLINK = '{http://www.w3.org/1999/xlink}href'


def svg_check(path):
    tree = ET.fromstring(path.read_bytes())
    assert tree.tag == SVG + 'svg', path
    size = tuple(round(float(tree.attrib[name])) for name in ['width', 'height'])
    assert min(size) >= 260, (path, size)
    assert list(tree.iter(SVG + 'path')), f'没有矢量路径: {path}'
    images = []
    for element in tree.iter(SVG + 'image'):
        uri = element.attrib[XLINK]
        assert uri.startswith('data:image/'), path
        image = Image.open(io.BytesIO(base64.b64decode(uri.split(',', 1)[1])))
        image.load()
        assert image.format in ['PNG', 'JPEG', 'BMP'], path
        images.append(image.size)
    return size, images


def pdf_check(path, size, image_expected):
    document = PdfReader(path, strict=True)
    assert len(document.pages) == 1, path
    page = document.pages[0]
    assert tuple(round(float(page.mediabox[index])) for index in [2, 3]) == size, path
    content = page.get_contents().get_data()
    assert len(content) > 1000 and b' m\n' in content and b' c\n' in content, f'缺少矢量几何: {path}'
    images = page['/Resources'].get('/XObject', {})
    if image_expected:
        assert images, f'PDF 图片缺失: {path}'
    for reference in images.values():
        image = reference.get_object()
        assert image['/Width'] > 0 and image['/Height'] > 0, path
        assert '/SMask' in image, f'PDF 图片透明掩码缺失: {path}'
        assert len(image.get_data()) == image['/Width'] * image['/Height'] * 3, path
    assert '/ExtGState' in page['/Resources'], path


def render_pdf(poppler, path, destination):
    result = subprocess.run([poppler, '-png', '-singlefile', '-r', '96', str(path), str(destination)], capture_output=True)
    assert result.returncode == 0, result.stderr.decode('utf-8', errors='replace')
    with Image.open(destination.with_suffix('.png')) as image:
        image.load()
        assert min(ImageStat.Stat(image.convert('RGB')).stddev) > 10, f'PDF 渲染为空: {path}'


def consumers(directory, poppler):
    counts = {'scenes': 0, 'svg': 0, 'pdf': 0, 'raster_formats': 0, 'pdf_rendered': 0}
    for host in ['rust', 'cpp']:
        output = directory / f'{host}-consumer'
        for mode in range(10):
            size, images = svg_check(output / f'mode-{mode}.svg')
            pdf_check(output / f'mode-{mode}.pdf', size, bool(images))
            with Image.open(output / f'mode-{mode}.png') as image:
                image.load()
                assert image.size == (1080, 760), (host, mode, image.size)
                assert min(ImageStat.Stat(image.convert('RGB')).stddev) > 10, (host, mode)
            if mode == 0:
                assert not images, '默认曲线示例不应显示红色图片图元'
            if mode == 6:
                assert images == [(5, 5)], f'小色图源单元尺寸错误: {images}'
            if mode == 8:
                assert b'<circle' in (output / f'mode-{mode}.svg').read_bytes(), '极坐标裁剪缺失'
            if poppler:
                destination = directory / 'pdf-render' / host / f'mode-{mode}'
                destination.parent.mkdir(parents=True, exist_ok=True)
                render_pdf(poppler, output / f'mode-{mode}.pdf', destination)
                counts['pdf_rendered'] += 1
            counts['scenes'] += 1
            counts['svg'] += 1
            counts['pdf'] += 1
        size, images = svg_check(output / '专业图表.svg')
        pdf_check(output / '专业图表.pdf', size, bool(images))
        counts['svg'] += 1
        counts['pdf'] += 1
        for extension, expected in [('png', 'PNG'), ('jpeg', 'JPEG'), ('bmp', 'BMP')]:
            with Image.open(output / f'专业图表.{extension}') as image:
                image.load()
                assert image.format == expected and image.size == size, (host, extension, image.size, size)
                assert min(ImageStat.Stat(image.convert('RGB')).stddev) > 10, (host, extension)
            counts['raster_formats'] += 1
    for mode in range(10):
        a = (directory / 'rust-consumer' / f'mode-{mode}.svg').read_bytes()
        b = (directory / 'cpp-consumer' / f'mode-{mode}.svg').read_bytes()
        assert a == b, f'两端数值或矢量不一致: mode={mode}'
    for extension in ['png', 'bmp']:
        with Image.open(directory / 'rust-consumer' / f'专业图表.{extension}') as a, Image.open(directory / 'cpp-consumer' / f'专业图表.{extension}') as b:
            assert a.size == b.size, extension
            delta = ImageChops.difference(a.convert('RGB'), b.convert('RGB'))
            assert delta.getbbox() is None, f'两端导出像素不一致: {extension}'
    print('PLOT_EXPORT_PARITY_OK', json.dumps(counts, ensure_ascii=False))
    return counts


def grid_samples(directory):
    samples = []
    for host in ['rust', 'cpp']:
        with (directory / 'grid-review' / f'{host}-after.csv').open(encoding='utf-8') as file:
            rows = list(csv.DictReader(file))
        assert len(rows) == 16, host
        samples.append([(r['rows'], r['shape'], r['operation'], r['visible']) for r in rows])
    assert samples[0] == samples[1], '两端网格排序/筛选数量不一致'
    print('GRID_REVIEW_PARITY_OK samples=16 rows=10000,100000 shapes=flat,tree')


def installed(directory):
    for name in ['ForgePlot.hpp', 'ForgePlotBindings.hpp', 'ForgePlotCore.h', 'ForgeGrids.hpp']:
        source = ROOT / ('sdk/common' if name == 'ForgePlotCore.h' else 'sdk/cpp/include') / name
        assert source.read_bytes() == (directory / 'install/include' / name).read_bytes(), f'安装头文件过期: {name}'
    for relative in ['charts/plot.slint', 'types/plot.slint', 'data/data-grid.slint', 'types/grids.slint', 'slintforge.slint']:
        assert (ROOT / 'ui' / relative).read_bytes() == (directory / 'install/share/slintforge/ui' / relative).read_bytes(), f'安装契约过期: {relative}'
    print('PLOT_INSTALL_CONTENT_OK headers=4 ui_contracts=5')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--qa-root', default='build/plot-qa')
    parser.add_argument('--pdftoppm', help='可选 Poppler 渲染器，验证每个 PDF 的实际图像')
    args = parser.parse_args()
    directory = ROOT / args.qa_root
    result = consumers(directory, args.pdftoppm)
    grid_samples(directory)
    installed(directory)
    (directory / 'verification-results.json').write_text(json.dumps(result, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print('PLOT_VERIFICATION_OK')


if __name__ == '__main__':
    main()
