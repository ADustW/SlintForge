"""从实际 Gallery 截图检查基础图表色块、复选框和说明文字的垂直对齐。"""
import argparse
from collections import Counter
from pathlib import Path
import re
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
COLORS = {(54, 162, 235), (255, 99, 132), (75, 192, 192), (255, 205, 86), (120, 134, 148)}
ROUTES = {'bar', 'line', 'pie', 'polar', 'bubble', 'scatter', 'radar'}


def boxes(points):
    remaining = set(points)
    result = []
    while remaining:
        start = remaining.pop()
        pending = [start]
        component = [start]
        while pending:
            x, y = pending.pop()
            for point in ((x-1, y), (x+1, y), (x, y-1), (x, y+1)):
                if point in remaining:
                    remaining.remove(point)
                    pending.append(point)
                    component.append(point)
        xs, ys = zip(*component)
        result.append((min(xs), min(ys), max(xs)+1, max(ys)+1, len(component)))
    return result


def verify(path):
    image = Image.open(path).convert('RGB')
    pixels = image.load()
    width, height = image.size
    candidates = []
    # 只寻找预览区的实心方形色块，排除曲线圆点和柱形数据。
    for color in COLORS:
        points = ((x, y) for y in range(int(height*.45), height-80)
                  for x in range(width//4, width-20) if pixels[x, y] == color)
        for box in boxes(points):
            left, top, right, bottom, count = box
            w, h = right-left, bottom-top
            if 8 <= w <= 18 and 8 <= h <= 18 and abs(w-h) <= 1 and count >= w*h*.95:
                candidates.append(box)
    assert candidates, f'{path.name}: 未找到图例色块'
    row = Counter(round((box[1]+box[3])/2) for box in candidates).most_common(1)[0][0]
    markers = sorted((box for box in candidates if abs((box[1]+box[3])/2-row) <= 1), key=lambda b: b[0])
    assert len(markers) in (2, 5), f'{path.name}: 图例数量错误 {len(markers)}'
    deltas = []
    for index, (left, top, right, bottom, _) in enumerate(markers):
        center = (top+bottom)/2
        # Fluent 主题的选中复选框为蓝色；只取当前色块右侧的控件区域。
        controls = boxes((x, y) for y in range(max(0, int(center-24)), min(height, int(center+25)))
                         for x in range(right+1, min(width, right+40))
                         if pixels[x, y][2] > pixels[x, y][0]+30 and pixels[x, y][2] > pixels[x, y][1])
        controls = [box for box in controls if 12 <= box[2]-box[0] <= 25 and 12 <= box[3]-box[1] <= 25]
        assert controls, f'{path.name}: 未找到对应复选框 {index}'
        control = min(controls, key=lambda box: box[0])
        indicator_delta = abs(center-(control[1]+control[3])/2)
        assert indicator_delta <= 1, f'{path.name}: 色块与复选框偏移 {indicator_delta:.1f}px'
        end = markers[index+1][0]-4 if index+1 < len(markers) else min(width-20, control[2]+180)
        dark = sum(pixels[min(width-1, left), max(0, top-5)]) < 400
        text = [(x, y) for y in range(max(0, int(center-15)), min(height, int(center+16)))
                for x in range(control[2]+3, end)
                if max(pixels[x, y])-min(pixels[x, y]) < 25
                and (min(pixels[x, y]) > 160 if dark else max(pixels[x, y]) < 120)]
        assert text, f'{path.name}: 未找到图例说明 {index}'
        text_delta = abs(center-(min(y for _, y in text)+max(y for _, y in text)+1)/2)
        # 字形的升部和降部不对称，允许可见文字包围框有少量偏差。
        assert text_delta <= 3.5, f'{path.name}: 色块与说明偏移 {text_delta:.1f}px'
        deltas.append({'marker_checkbox_px': indicator_delta, 'marker_text_px': text_delta})
    return deltas


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--qa-root', default='build/gallery-followup-qa/gallery')
    parser.add_argument('--host', action='append', choices=['rust', 'cpp'])
    args = parser.parse_args()
    checked = 0
    markers = 0
    for host in args.host or ['rust', 'cpp']:
        paths = []
        for path in sorted((ROOT/args.qa_root/host).glob('*.png')):
            match = re.search(r'-chart-([a-z]+)-(dark-zh|light-en|narrow-zh)(-open)?\.png$', path.name)
            if match and match[1] in ROUTES:
                paths.append(path)
        assert len(paths) == 28, f'{host}: 需要七类图表的三种布局与操作状态，共 28 帧，实际 {len(paths)}'
        for path in paths:
            result = verify(path)
            markers += len(result)
            checked += 1
    print(f'CHART_LEGENDS_ALIGNED_OK frames={checked} markers={markers}')


if __name__ == '__main__':
    main()
