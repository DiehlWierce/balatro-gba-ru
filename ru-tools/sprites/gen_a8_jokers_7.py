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


def riff_raff():
    im, d = card((0x6A, 0x7A, 0x8A))
    d.rectangle((8, 14, 24, 25), fill=GREY_L, outline=INK)
    d.polygon([(8, 14), (16, 8), (24, 14)], fill=RED, outline=INK)
    d.rectangle((14, 19, 18, 25), fill=BROWN, outline=INK)
    d.rectangle((10, 16, 12, 18), fill=BLUE)
    d.rectangle((20, 16, 22, 18), fill=BLUE)
    for x, y in ((5, 8), (26, 8), (4, 24), (28, 24)):
        d.rectangle((x - 1, y - 1, x + 1, y + 1), fill=GOLD, outline=INK)
    return im


def satellite():
    im, d = card((0x14, 0x1E, 0x4E))
    d.rectangle((13, 12, 19, 20), fill=GREY_L, outline=INK)
    d.rectangle((3, 14, 11, 18), fill=BLUE, outline=INK)
    d.rectangle((21, 14, 29, 18), fill=BLUE, outline=INK)
    d.line((7, 14, 7, 18), fill=WHITE)
    d.line((25, 14, 25, 18), fill=WHITE)
    d.line((16, 12, 16, 7), fill=INK)
    d.ellipse((14, 4, 18, 8), fill=GOLD, outline=INK)
    d.ellipse((14, 14, 18, 18), fill=GOLD_D)
    for x, y in ((6, 6), (26, 7), (7, 26), (25, 25)):
        d.line((x - 1, y, x + 1, y), fill=WHITE)
        d.line((x, y - 1, x, y + 1), fill=WHITE)
    return im


def hologram():
    im, d = card((0x1C, 0x3A, 0x5A))
    d.rectangle((9, 7, 23, 25), fill=(0x6A, 0xD8, 0xE8), outline=INK)
    d.rectangle((11, 9, 21, 23), outline=WHITE)
    d.ellipse((13, 11, 19, 17), fill=WHITE, outline=INK)
    d.polygon([(11, 23), (16, 17), (21, 23)], fill=WHITE, outline=INK)
    d.line((5, 12, 8, 12), fill=GREEN)
    d.line((24, 18, 27, 18), fill=GREEN)
    d.line((5, 20, 8, 20), fill=GREEN)
    d.line((24, 10, 27, 10), fill=GREEN)
    return im


def burnt():
    im, d = card((0x2A, 0x1A, 0x1A))
    d.rectangle((9, 8, 23, 25), fill=(0xE8, 0xE0, 0xC8), outline=INK)
    d.polygon([(16, 5), (22, 14), (20, 22), (12, 22), (10, 14)], fill=ORANGE, outline=INK)
    d.polygon([(16, 11), (19, 17), (18, 21), (14, 21), (13, 17)], fill=GOLD)
    d.polygon([(9, 25), (12, 22), (15, 25)], fill=INK)
    d.polygon([(17, 25), (21, 21), (23, 25)], fill=INK)
    return im


def invisible():
    im, d = card((0x3A, 0x44, 0x5A))
    d.ellipse((9, 6, 23, 20), outline=GREY_L)
    d.rectangle((10, 13, 22, 26), outline=GREY_L)
    d.line((12, 9, 20, 17), fill=GREY_L)
    d.line((12, 17, 20, 9), fill=GREY_L)
    d.ellipse((13, 11, 14, 12), fill=WHITE)
    d.ellipse((18, 11, 19, 12), fill=WHITE)
    for x, y in ((5, 6), (27, 26), (5, 26)):
        d.line((x - 1, y, x + 1, y), fill=GREY_L)
        d.line((x, y - 1, x, y + 1), fill=GREY_L)
    return im


def showman():
    im, d = card((0x8A, 0x1E, 0x3A))
    d.rectangle((10, 4, 22, 9), fill=INK)
    d.rectangle((8, 9, 24, 11), fill=INK)
    d.ellipse((10, 11, 22, 23), fill=(0xF0, 0xC8, 0xA0), outline=INK)
    d.rectangle((13, 15, 14, 16), fill=INK)
    d.rectangle((18, 15, 19, 16), fill=INK)
    d.arc((13, 16, 19, 21), 0, 180, fill=INK)
    d.polygon([(16, 23), (11, 28), (21, 28)], fill=RED, outline=INK)
    d.line((22, 24, 28, 19), fill=GOLD, width=2)
    d.ellipse((26, 16, 29, 19), fill=GOLD_D)
    return im


def diet_cola():
    im, d = card((0x3A, 0x7A, 0xB0))
    d.rectangle((11, 7, 21, 27), fill=RED, outline=INK)
    d.rectangle((11, 12, 21, 21), fill=WHITE)
    d.rectangle((13, 5, 19, 7), fill=GREY_L, outline=INK)
    d.line((13, 15, 19, 15), fill=BLUE, width=2)
    d.line((13, 18, 17, 18), fill=BLUE)
    d.line((22, 8, 26, 4), fill=INK)
    d.ellipse((5, 22, 8, 25), fill=WHITE)
    d.ellipse((25, 18, 28, 21), fill=WHITE)
    return im


JOKERS = [throwback, madness, campfire, riff_raff, satellite, hologram, burnt, invisible, showman, diet_cola]
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
