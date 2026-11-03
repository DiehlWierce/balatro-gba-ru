#!/usr/bin/env python3
"""Generates graphics/joker_gfx112..113.png (A8 batch 8 Jokers: Matador 147, Mime 148, 32x32 each).

Run from the repository root: python3 ru-tools/sprites/gen_a8_jokers_8.py
Pass --sheet <file> to also write a contact sheet of the sprites.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from gen_a8_jokers_1 import card, INK, WHITE  # noqa: E402
from PIL import Image  # noqa: E402

GRIT = "-gB4 -Mw4 -Mh4 -m! -pn 16"
GOLD, GOLD_D = (0xF2, 0xC2, 0x30), (0xB8, 0x86, 0x1B)
RED, RED_D = (0xE0, 0x3C, 0x4C), (0x9A, 0x22, 0x30)
GREY_L = (0xC4, 0xCA, 0xD2)
SKIN = (0xF0, 0xC8, 0xA0)


def matador():
    im, d = card((0x5A, 0x1E, 0x2C))
    # Muleta: a red cape held in front, with a sword over it
    d.polygon([(7, 8), (25, 8), (27, 25), (5, 25)], fill=RED, outline=INK)
    d.polygon([(7, 8), (13, 8), (12, 25), (5, 25)], fill=RED_D)
    d.line((16, 9, 16, 24), fill=RED_D)
    d.line((6, 12, 26, 12), fill=GOLD)
    d.line((6, 22, 27, 22), fill=GOLD)
    d.line((24, 5, 12, 27), fill=GREY_L, width=2)
    d.line((24, 5, 12, 27), fill=INK)
    d.rectangle((22, 3, 26, 6), fill=GOLD, outline=INK)
    return im


def mime():
    im, d = card((0x2A, 0x30, 0x4A))
    d.ellipse((8, 7, 24, 24), fill=WHITE, outline=INK)
    # beret
    d.pieslice((7, 3, 25, 15), 180, 360, fill=INK)
    d.rectangle((14, 2, 17, 3), fill=INK)
    # eyes with raised brows, painted tear and a thin smile
    d.line((10, 12, 14, 11), fill=INK)
    d.line((18, 11, 22, 12), fill=INK)
    d.ellipse((11, 13, 14, 16), fill=INK)
    d.ellipse((18, 13, 21, 16), fill=INK)
    d.line((12, 18, 12, 20), fill=(0x3E, 0x60, 0xD4))
    d.line((12, 21, 15, 22), fill=RED)
    d.line((15, 22, 19, 22), fill=RED)
    d.line((19, 22, 21, 21), fill=RED)
    # striped shirt
    for y in (26, 29):
        d.line((8, y, 24, y), fill=INK)
    for x, y in ((5, 6), (27, 8)):
        d.line((x - 1, y, x + 1, y), fill=GOLD)
        d.line((x, y - 1, x, y + 1), fill=GOLD)
    return im


JOKERS = [matador, mime]
FIRST_SHEET = 112


def main():
    sheet = None
    if "--sheet" in sys.argv:
        sheet = sys.argv[sys.argv.index("--sheet") + 1]
    ims = []
    for i, fn in enumerate(JOKERS):
        im = fn()
        n = len(set(p for p in im.getdata() if p[3]))
        assert n <= 15, (fn.__name__, n)
        idx = FIRST_SHEET + i
        im.save(f"graphics/joker_gfx{idx}.png")
        with open(f"graphics/joker_gfx{idx}.grit", "w") as f:
            f.write(GRIT)
        ims.append(im)
    if sheet:
        out = Image.new("RGBA", (len(ims) * 34, 34), (60, 120, 60, 255))
        for i, im in enumerate(ims):
            out.alpha_composite(im, (i * 34 + 1, 1))
        out.resize((out.width * 4, out.height * 4), Image.NEAREST).save(sheet)
    print("ok", len(JOKERS))


if __name__ == "__main__":
    sys.exit(main())
