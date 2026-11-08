# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

"""核对真实窗口截图中的卡片重绘、下拉弹层和主题切换。"""
import json
import sys
from pathlib import Path

from PIL import Image, ImageChops


folder = Path(sys.argv[1])
initial = Image.open(folder / 'cache-initial.png').convert('RGB')
scale = initial.width / 1100


def region(name, box):
    image = Image.open(folder / f'{name}.png').convert('RGB')
    return image.crop(tuple(round(x * scale) for x in box))


def changed(first, second, box):
    return ImageChops.difference(region(first, box), region(second, box)).getbbox() is not None


checks = {
    'form_data_repainted': changed('cache-initial', 'cache-data-updated', (112, 246, 308, 283)),
    'device_state_repainted': changed('cache-initial', 'cache-data-updated', (402, 246, 598, 320)),
    'typed_text_repainted': changed('cache-data-updated', 'cache-typing', (112, 246, 308, 283)),
    'device_button_repainted': changed('cache-data-updated', 'cache-submitted', (402, 246, 598, 320)),
    'popup_outside_card': changed('cache-submitted', 'cache-popup', (160, 333, 320, 395)),
    'theme_repainted': changed('cache-submitted', 'cache-dark', (402, 246, 598, 320)),
}
report = {'passed': all(checks.values()), 'checks': checks, 'scale_factor': scale}
(folder / 'render-cache-result.json').write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding='utf-8')
print(json.dumps(report, ensure_ascii=False))
sys.exit(0 if report['passed'] else 1)
