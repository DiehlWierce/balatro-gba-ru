#!/usr/bin/env python3
"""Generates graphics/joker_gfx59..73.png (A8 batch 4 Jokers, ID 94-108, 32x32 each, 15 colors + transparent).

Run from the repository root: python3 ru-tools/sprites/gen_a8_jokers_4.py
Pass --sheet <file> to also write a contact sheet of the 15 sprites.
"""
import math
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from gen_a8_jokers_1 import card, INK, WHITE, GRIT  # noqa: E402
from PIL import Image  # noqa: E402

GOLD, GOLD_D = (0xF2, 0xC2, 0x30), (0xB8, 0x86, 0x1B)
RED, BLUE, GREEN = (0xE0, 0x3C, 0x4C), (0x3E, 0x60, 0xD4), (0x2F, 0x8F, 0x6F)
GREY, GREY_D, GREY_L = (0x8A, 0x92, 0x9C), (0x5C, 0x64, 0x70), (0xC4, 0xCA, 0xD2)
PURPLE = (0x7A, 0x5A, 0xB0)


def heart(d, cx, cy, r, fill, outline=INK):
    d.ellipse((cx - r, cy - r, cx, cy), fill=fill)
    d.ellipse((cx, cy - r, cx + r, cy), fill=fill)
    d.polygon([(cx - r, cy - r // 2), (cx + r, cy - r // 2), (cx, cy + r)], fill=fill)


def bloodstone():
    im, d = card((0x6A, 0x14, 0x26))
    d.polygon([(16, 5), (25, 11), (25, 20), (16, 28), (7, 20), (7, 11)], fill=(0x2C, 0x6A, 0x4C), outline=INK)
    d.polygon([(16, 5), (25, 11), (16, 14)], fill=(0x58, 0xA0, 0x7A))
    for x, y in ((12, 17), (19, 15), (15, 22), (21, 20)):
        d.rectangle((x, y, x + 1, y + 1), fill=RED)
    d.line((10, 9, 13, 7), fill=WHITE)
    return im


def flower_pot():
    im, d = card((0xA8, 0xDC, 0xF0))
    d.polygon([(9, 20), (23, 20), (21, 28), (11, 28)], fill=(0xC8, 0x6A, 0x38), outline=INK)
    d.rectangle((8, 18, 24, 21), fill=(0xE0, 0x84, 0x4C), outline=INK)
    d.line((16, 18, 16, 11), fill=GREEN)
    for x, y, c in ((16, 6, RED), (11, 10, GOLD), (21, 10, BLUE), (16, 11, WHITE)):
        d.ellipse((x - 3, y - 3, x + 3, y + 3), fill=c, outline=INK)
    d.ellipse((14, 9, 18, 13), fill=GOLD, outline=INK)
    return im


def seeing_double():
    im, d = card((0x3A, 0x5A, 0x4A))
    for x, c in ((6, (0x1E, 0x7A, 0x4A)), (17, (0xE0, 0x3C, 0x4C))):
        d.rectangle((x, 7, x + 9, 25), fill=WHITE, outline=INK)
    for cx, cy in ((11, 13), (9, 17), (13, 17)):
        d.ellipse((cx - 2, cy - 2, cx + 2, cy + 2), fill=(0x1E, 0x7A, 0x4A))
    d.line((11, 17, 11, 21), fill=(0x1E, 0x7A, 0x4A))
    heart(d, 22, 15, 3, RED)
    d.point((21, 12), fill=WHITE)
    return im


def idol():
    im, d = card((0x7A, 0x5A, 0x2A))
    d.rectangle((8, 25, 24, 28), fill=GREY_D, outline=INK)
    d.polygon([(10, 25), (13, 12), (19, 12), (22, 25)], fill=GOLD, outline=INK)
    d.ellipse((12, 4, 20, 13), fill=GOLD, outline=INK)
    for x in (14, 17):
        d.point((x, 8), fill=INK)
    d.line((13, 17, 12, 21), fill=GOLD_D)
    d.line((19, 17, 20, 21), fill=GOLD_D)
    d.line((13, 6, 15, 5), fill=WHITE)
    return im


def ancient():
    im, d = card((0x56, 0x44, 0x2C))
    d.ellipse((7, 6, 25, 24), fill=(0xE8, 0xD8, 0xA8), outline=INK)
    d.ellipse((10, 9, 22, 21), outline=GOLD_D)
    heart(d, 16, 14, 3, RED)
    for a in range(0, 360, 60):
        x = 16 + 11 * math.cos(math.radians(a))
        y = 15 + 11 * math.sin(math.radians(a))
        d.point((x, y), fill=GOLD)
    d.line((9, 27, 23, 27), fill=GOLD_D)
    d.line((8, 25, 24, 25), fill=GREY)
    return im


def baseball_card():
    im, d = card((0x2E, 0x6E, 0x4A))
    d.rectangle((9, 5, 23, 27), fill=(0xFF, 0xF3, 0xD6), outline=INK)
    d.rectangle((11, 7, 21, 19), fill=(0x9A, 0xD4, 0xF0), outline=INK)
    d.ellipse((13, 10, 19, 16), fill=WHITE, outline=INK)
    d.arc((13, 10, 19, 16), 200, 340, fill=RED)
    d.arc((13, 10, 19, 16), 20, 160, fill=RED)
    d.line((12, 22, 20, 22), fill=GREY)
    d.line((12, 24, 18, 24), fill=GREY)
    return im


def erosion():
    im, d = card((0x8C, 0x7A, 0x5A))
    d.rectangle((6, 6, 25, 26), fill=(0xB4, 0x9C, 0x70), outline=INK)
    d.polygon([(6, 18), (11, 15), (13, 20), (18, 16), (20, 22), (25, 19), (25, 26), (6, 26)], fill=(0x6A, 0x58, 0x3C))
    for x, y in ((9, 10), (15, 8), (21, 12), (12, 13), (18, 18)):
        d.rectangle((x, y, x + 1, y + 1), fill=(0x6A, 0x58, 0x3C))
    d.polygon([(16, 6), (20, 6), (18, 11)], fill=(0x8C, 0x7A, 0x5A))
    return im


def drivers_license():
    im, d = card((0x3E, 0x60, 0xD4))
    d.rectangle((5, 8, 26, 24), fill=(0xF4, 0xF6, 0xF8), outline=INK)
    d.rectangle((6, 9, 25, 12), fill=GOLD, outline=INK)
    d.rectangle((7, 14, 13, 21), fill=(0x9A, 0xD4, 0xF0), outline=INK)
    d.ellipse((8, 15, 12, 19), fill=(0xF0, 0xC8, 0xA0))
    for y in (15, 18, 21):
        d.line((15, y, 24, y), fill=GREY)
    d.rectangle((20, 20, 24, 22), fill=RED)
    return im


def card_sharp():
    im, d = card((0x3A, 0x2A, 0x4A))
    d.polygon([(7, 24), (7, 12), (14, 8), (14, 22)], fill=WHITE, outline=INK)
    d.polygon([(25, 24), (25, 12), (18, 8), (18, 22)], fill=WHITE, outline=INK)
    d.rectangle((12, 6, 20, 25), fill=(0xFF, 0xF3, 0xD6), outline=INK)
    heart(d, 16, 13, 3, RED)
    d.polygon([(12, 25), (16, 29), (20, 25)], fill=RED, outline=INK)
    d.line((13, 20, 19, 20), fill=GOLD)
    return im


def superposition():
    im, d = card((0x1C, 0x2A, 0x5A))
    d.rectangle((5, 8, 15, 24), fill=WHITE, outline=INK)
    heart(d, 10, 15, 3, RED)
    d.rectangle((11, 8, 21, 24), fill=(0xE4, 0xE8, 0xF8), outline=INK)
    d.point((13, 11), fill=INK)
    d.polygon([(16, 14), (14, 20), (18, 20)], fill=INK)
    for i, c in enumerate((GOLD, WHITE, GREY_L)):
        d.line((22, 10 + i * 6, 27, 10 + i * 6 - 2), fill=c)
    return im


def eight_ball():
    im, d = card((0xC8, 0xD8, 0xE8))
    d.ellipse((5, 5, 26, 26), fill=INK, outline=INK)
    d.ellipse((10, 6, 14, 10), fill=GREY_L)
    d.ellipse((12, 11, 20, 19), fill=WHITE)
    d.ellipse((13, 12, 15, 14), fill=INK)
    d.ellipse((17, 12, 19, 14), fill=INK)
    d.rectangle((14, 15, 18, 17), outline=INK)
    d.point((16, 16), fill=INK)
    return im


def to_do_list():
    im, d = card((0xE8, 0xD8, 0x9A))
    d.rectangle((7, 5, 24, 27), fill=WHITE, outline=INK)
    for i, y in enumerate((10, 16, 22)):
        d.rectangle((9, y, 11, y + 2), outline=INK)
        d.line((13, y + 1, 22, y + 1), fill=GREY)
    d.line((9, 17, 10, 18), fill=GREEN)
    d.line((10, 18, 12, 14), fill=GREEN)
    d.line((9, 11, 11, 11), fill=GREEN)
    return im


def green_joker():
    im, d = card((0x2F, 0x8F, 0x4A))
    d.polygon([(6, 12), (10, 6), (13, 12), (16, 5), (19, 12), (22, 6), (26, 12), (16, 24)], fill=(0xB8, 0xEC, 0x6A), outline=INK)
    d.ellipse((8, 14, 24, 28), fill=(0xB8, 0xEC, 0x6A), outline=INK)
    for x in (12, 18):
        d.ellipse((x, 18, x + 2, 20), fill=INK)
    d.arc((11, 20, 21, 26), 20, 160, fill=INK)
    for x, y in ((6, 12), (26, 12), (16, 5)):
        d.ellipse((x - 1, y - 1, x + 1, y + 1), fill=GOLD, outline=INK)
    return im


def faceless():
    im, d = card((0x6A, 0x5A, 0x7A))
    d.ellipse((8, 5, 24, 26), fill=(0xE8, 0xE0, 0xD8), outline=INK)
    d.polygon([(6, 12), (10, 6), (13, 11), (16, 5), (19, 11), (22, 6), (26, 12)], fill=PURPLE, outline=INK)
    d.line((12, 17, 20, 17), fill=GREY_L)
    d.line((13, 21, 19, 21), fill=GREY_L)
    d.point((13, 15), fill=GREY)
    d.point((19, 15), fill=GREY)
    d.rectangle((10, 26, 22, 28), fill=PURPLE)
    return im


def hit_the_road():
    im, d = card((0xE8, 0xA8, 0x4C))
    d.rectangle((3, 20, 28, 29), fill=GREY_D)
    d.polygon([(13, 20), (19, 20), (22, 29), (10, 29)], fill=GREY)
    for y in (22, 25, 28):
        d.line((15, y, 17, y), fill=WHITE)
    d.rectangle((16, 6, 17, 18), fill=INK)
    d.rectangle((9, 7, 24, 12), fill=WHITE, outline=INK)
    d.polygon([(20, 8), (23, 9), (20, 11)], fill=RED)
    d.line((11, 9, 17, 9), fill=GREY)
    return im


JOKERS = [bloodstone, flower_pot, seeing_double, idol, ancient, baseball_card, erosion,
          drivers_license, card_sharp, superposition, eight_ball, to_do_list, green_joker,
          faceless, hit_the_road]
FIRST_SHEET = 59


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
