#!/usr/bin/env python3
"""Generates graphics/joker_gfx89..N.png (A8 batch 6 Jokers, ID 124+, 32x32 each, 15 colors + transparent).

Run from the repository root: python3 ru-tools/sprites/gen_a8_jokers_6.py
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
PURPLE = (0x7A, 0x5A, 0xB0)
SKIN, SKIN_D = (0xF0, 0xC8, 0xA0), (0xC0, 0x90, 0x6A)
LEG_BG = (0x3A, 0x1E, 0x5C)  # legendary background: deep purple


def stars(d):
    for x, y in ((5, 6), (26, 8), (6, 25), (27, 24)):
        d.line((x - 1, y, x + 1, y), fill=GOLD)
        d.line((x, y - 1, x, y + 1), fill=GOLD)


def canio():
    im, d = card(LEG_BG)
    stars(d)
    d.ellipse((9, 7, 23, 22), fill=SKIN, outline=INK)
    d.polygon([(8, 12), (16, 3), (24, 12), (22, 9), (16, 7), (10, 9)], fill=RED, outline=INK)
    d.rectangle((12, 12, 14, 14), fill=INK)
    d.rectangle((18, 12, 20, 14), fill=INK)
    d.arc((12, 14, 20, 21), 0, 180, fill=INK)
    d.polygon([(9, 24), (23, 24), (21, 29), (11, 29)], fill=GOLD, outline=INK)
    d.line((12, 26, 20, 26), fill=GOLD_D)
    return im


def triboulet():
    im, d = card(LEG_BG)
    stars(d)
    d.ellipse((10, 10, 22, 22), fill=SKIN, outline=INK)
    d.polygon([(7, 14), (5, 4), (11, 9), (16, 3), (21, 9), (27, 4), (25, 14), (16, 8)], fill=GOLD, outline=INK)
    d.rectangle((13, 14, 14, 15), fill=INK)
    d.rectangle((18, 14, 19, 15), fill=INK)
    d.arc((13, 16, 19, 21), 0, 180, fill=RED)
    for x in (5, 16, 27):
        d.ellipse((x - 2, 3, x + 1, 6), fill=RED, outline=INK) if x != 16 else d.ellipse((14, 2, 18, 6), fill=BLUE, outline=INK)
    d.polygon([(9, 24), (23, 24), (21, 29), (11, 29)], fill=PURPLE, outline=INK)
    return im


def yorick():
    im, d = card(LEG_BG)
    stars(d)
    d.ellipse((8, 5, 24, 21), fill=WHITE, outline=INK)
    d.rectangle((12, 20, 20, 25), fill=WHITE, outline=INK)
    d.ellipse((11, 10, 15, 15), fill=INK)
    d.ellipse((17, 10, 21, 15), fill=INK)
    d.polygon([(16, 15), (14, 18), (18, 18)], fill=INK)
    for x in (14, 16, 18):
        d.line((x, 21, x, 24), fill=INK)
    d.line((8, 27, 24, 27), fill=GREY)
    return im


def chicot():
    im, d = card(LEG_BG)
    stars(d)
    d.ellipse((9, 9, 23, 23), fill=SKIN, outline=INK)
    d.polygon([(8, 13), (10, 3), (14, 9), (16, 2), (18, 9), (22, 3), (24, 13)], fill=RED, outline=INK)
    d.rectangle((12, 14, 14, 16), fill=INK)
    d.rectangle((18, 14, 20, 16), fill=INK)
    d.line((12, 20, 20, 20), fill=INK)
    # a broken-off boss mark
    d.line((4, 26, 28, 26), fill=GOLD_D)
    d.line((22, 22, 28, 30), fill=WHITE, width=2)
    d.line((28, 22, 22, 30), fill=WHITE, width=2)
    return im


def perkeo():
    im, d = card(LEG_BG)
    stars(d)
    d.ellipse((10, 8, 22, 20), fill=SKIN, outline=INK)
    d.polygon([(9, 12), (6, 2), (12, 7), (16, 1), (20, 7), (26, 2), (23, 12)], fill=GREEN, outline=INK)
    d.rectangle((13, 12, 14, 13), fill=INK)
    d.rectangle((18, 12, 19, 13), fill=INK)
    d.arc((13, 14, 19, 19), 0, 180, fill=INK)
    d.rectangle((8, 22, 24, 29), fill=PURPLE, outline=INK)
    d.rectangle((13, 24, 19, 27), fill=GOLD, outline=INK)
    d.point((16, 26), fill=INK)
    return im


JOKERS = [canio, triboulet, yorick, chicot, perkeo]
FIRST_SHEET = 89


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
