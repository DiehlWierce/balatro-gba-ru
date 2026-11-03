#!/usr/bin/env python3
"""Generates graphics/joker_gfx100..N.png (A8 batch 7 Jokers, ID 135+, 32x32 each, 15 colors + transparent).

Run from the repository root: python3 ru-tools/sprites/gen_a8_jokers_7.py
Pass --sheet <file> to also write a contact sheet of the sprites.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from gen_a8_jokers_1 import card, INK, WHITE  # noqa: E402
from PIL import Image  # noqa: E402

GRIT = "-gB4 -Mw4 -Mh4 -m! -pn 16"
GOLD, GOLD_D = (0xF2, 0xC2, 0x30), (0xB8, 0x86, 0x1B)
RED, BLUE, GREEN = (0xE0, 0x3C, 0x4C), (0x3E, 0x60, 0xD4), (0x2F, 0x8F, 0x6F)
GREY, GREY_L = (0x8A, 0x92, 0x9C), (0xC4, 0xCA, 0xD2)
ORANGE = (0xF0, 0x8A, 0x2A)
BROWN = (0x7A, 0x4A, 0x2A)


def throwback():
    im, d = card((0x8A, 0x6A, 0x3A))
    d.polygon([(6, 16), (14, 8), (14, 13), (26, 13), (26, 19), (14, 19), (14, 24)], fill=GOLD, outline=INK)
    d.line((8, 16, 13, 11), fill=GOLD_D)
    d.line((17, 27, 25, 27), fill=INK)
    return im


def madness():
    im, d = card((0x5A, 0x1E, 0x3C))
    d.ellipse((8, 7, 24, 23), fill=(0xF0, 0xC8, 0xA0), outline=INK)
    d.line((9, 11, 14, 14), fill=INK)
    d.line((23, 11, 18, 14), fill=INK)
    d.ellipse((11, 14, 15, 18), fill=WHITE, outline=INK)
    d.ellipse((17, 14, 21, 18), fill=WHITE, outline=INK)
    d.rectangle((13, 16, 13, 16), fill=RED)
    d.rectangle((19, 16, 19, 16), fill=RED)
    d.polygon([(11, 20), (16, 22), (21, 20), (20, 23), (12, 23)], fill=INK)
    for x, y in ((5, 6), (27, 6), (6, 26), (26, 26)):
        d.line((x - 1, y - 1, x + 1, y + 1), fill=RED)
        d.line((x - 1, y + 1, x + 1, y - 1), fill=RED)
    return im


def campfire():
    im, d = card((0x1E, 0x24, 0x44))
    d.line((6, 26, 26, 22), fill=BROWN, width=3)
    d.line((6, 22, 26, 26), fill=BROWN, width=3)
    d.polygon([(16, 5), (23, 17), (21, 23), (11, 23), (9, 17)], fill=ORANGE, outline=INK)
    d.polygon([(16, 12), (20, 19), (19, 22), (13, 22), (12, 19)], fill=GOLD)
    d.polygon([(16, 16), (18, 21), (14, 21)], fill=WHITE)
    for x, y in ((5, 6), (26, 8), (8, 12)):
        d.line((x - 1, y, x + 1, y), fill=GOLD)
        d.line((x, y - 1, x, y + 1), fill=GOLD)
    return im


JOKERS = [throwback, madness, campfire]
FIRST_SHEET = 100


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
        out = Image.new("RGBA", (8 * 34, 2 * 34), (60, 120, 60, 255))
        for i, im in enumerate(ims):
            out.alpha_composite(im, ((i % 8) * 34 + 1, (i // 8) * 34 + 1))
        out.resize((out.width * 4, out.height * 4), Image.NEAREST).save(sheet)
    print("ok", len(JOKERS))


if __name__ == "__main__":
    sys.exit(main())
