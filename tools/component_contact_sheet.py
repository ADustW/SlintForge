# Copyright © 2026 陕西禾又科技有限公司
# SPDX-License-Identifier: GPL-3.0-only

"""汇总实际窗口 PNG，供组件布局复核；不替代原始分辨率检查。"""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
import argparse

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('host', nargs='?', choices=['rust', 'cpp'], default='cpp')
parser.add_argument('--qa-root', default='build/component-qa')
parser.add_argument('--route', action='append', default=[])
args = parser.parse_args()
host = args.host
directory = root / args.qa_root / host
status = {line.split('|')[0]: line.split('|')[1] for line in (root / 'Gallery/data/component-status.txt').read_text(encoding='utf-8').splitlines() if not line.startswith('#')}
font = ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf', 15)
groups = {name: [] for name in ['dark', 'light', 'narrow', 'opened']}
for path in sorted(directory.glob('*.png')):
    route = path.stem[4:].split('-dark-zh')[0].split('-light-en')[0].split('-narrow-zh')[0]
    if args.route and route not in args.route:
        continue
    if path.stem.endswith('-open'):
        groups['opened'].append(path)
    elif '-light-en' in path.stem:
        groups['light'].append(path)
    elif '-narrow-zh' in path.stem:
        groups['narrow'].append(path)
    else:
        groups['dark'].append(path)
output = root / args.qa_root / 'contact-sheets'
output.mkdir(parents=True, exist_ok=True)
for group, files in groups.items():
    for part in range(0, len(files), 8):
        selected = files[part:part + 8]
        sheet = Image.new('RGB', (1680, 680), '#e8e8ea')
        draw = ImageDraw.Draw(sheet)
        for index, path in enumerate(selected):
            image = Image.open(path).convert('RGB')
            image = image.crop((320, 48, image.width - 8, image.height - 20))
            image.thumbnail((408, 302))
            x, y = index % 4 * 420 + 6, index // 4 * 340 + 28
            sheet.paste(image, (x, y))
            draw.text((x, y - 23), path.stem[4:], font=font, fill='#191b24')
        path = output / f'{host}-{group}-{part // 8 + 1}.png'
        sheet.save(path)
        print(path)
