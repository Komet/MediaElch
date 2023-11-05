#!/usr/bin/env python3

# This script can be used to update light.css, which is based on
# base.css and palette_light.json.
#
# Usage:
#  ./scripts/create_themes_from_palettes.py
#

import os
import json
from dataclasses import dataclass
from typing import Any

script_dir = os.path.dirname(os.path.realpath(__file__))
os.chdir(script_dir)

@dataclass
class Palette:
    palette: str
    json: Any
    output: str

palettes = [
    Palette('../src/ui/palette_light.json', None, '../src/ui/light.css'),
    Palette('../src/ui/palette_dark.json', None, '../src/ui/dark.css'),
]

for palette in palettes:
    with open(palette.palette, 'r') as f:
        palette.json = json.load(f)

base_css = ''
with open('../src/ui/base.css', 'r') as reader:
    base_css = reader.read()

for p in palettes:
    palette_css = base_css
    for key in p.json:
        palette_css = palette_css.replace('$' + key, p.json[key])
    with open(p.output, 'w') as f:
        f.write(palette_css)
