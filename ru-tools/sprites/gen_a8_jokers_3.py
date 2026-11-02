#!/usr/bin/env python3
"""Generates graphics/joker_gfx44..58.png (A8 batch 3 Jokers, 32x32 each, 15 colors + transparent).

Run from the repository root: python3 ru-tools/sprites/gen_a8_jokers_3.py
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


def steel():
    im, d = card((0x4A, 0x6A, 0x8C))
    d.rectangle((6, 9, 25, 24), fill=GREY, outline=INK)
    d.rectangle((7, 10, 24, 12), fill=GREY_L)
    d.rectangle((7, 21, 24, 23), fill=GREY_D)
    d.line((9, 16, 22, 16), fill=GREY_D)
    d.line((9, 17, 22, 17), fill=GREY_L)
    for x, y in ((9, 13), (22, 13), (9, 20), (22, 20)):
        d.point((x, y), fill=INK)
    d.line((8, 10, 12, 10), fill=WHITE)
    return im


def glass():
    im, d = card((0xA8, 0xE0, 0xF0))
    cy = (0x70, 0xC8, 0xE8)
    d.polygon([(16, 4), (26, 12), (22, 27), (10, 27), (6, 12)], fill=(0xDC, 0xF6, 0xFF), outline=INK)
    d.polygon([(16, 4), (26, 12), (16, 14)], fill=cy)
    d.polygon([(6, 12), (16, 14), (10, 27)], fill=cy)
    d.line((16, 14, 22, 27), fill=INK)
    d.line((16, 14, 20, 18), fill=INK)
    d.line((9, 10, 13, 7), fill=WHITE)
    d.point((9, 13), fill=WHITE)
    return im


def lucky_cat():
    im, d = card((0xD8, 0x48, 0x58))
    d.polygon([(7, 14), (7, 6), (13, 10)], fill=WHITE, outline=INK)
    d.polygon([(25, 14), (25, 6), (19, 10)], fill=WHITE, outline=INK)
    d.ellipse((6, 9, 25, 26), fill=WHITE, outline=INK)
    for x in (11, 19):
        d.ellipse((x - 1, 15, x + 1, 18), fill=INK)
    d.polygon([(15, 20), (17, 20), (16, 21)], fill=RED)
    d.line((16, 21, 16, 22), fill=INK)
    d.arc((12, 20, 16, 24), 0, 180, fill=INK)
    d.arc((16, 20, 20, 24), 0, 180, fill=INK)
    d.line((3, 18, 8, 19), fill=INK)
    d.line((29, 18, 24, 19), fill=INK)
    d.ellipse((12, 25, 19, 29), fill=GOLD, outline=INK)
    return im


def vampire():
    im, d = card((0x5A, 0x12, 0x24))
    d.polygon([(3, 8), (9, 14), (9, 28), (3, 24)], fill=INK)
    d.polygon([(29, 8), (23, 14), (23, 28), (29, 24)], fill=INK)
    d.ellipse((9, 6, 23, 24), fill=(0xF0, 0xE4, 0xE0), outline=INK)
    d.polygon([(9, 12), (16, 8), (23, 12), (16, 15)], fill=INK)
    for x in (12, 18):
        d.rectangle((x, 14, x + 2, 15), fill=RED)
    d.rectangle((12, 19, 19, 20), fill=INK)
    for x in (12, 18):
        d.polygon([(x, 20), (x + 2, 20), (x + 1, 24)], fill=WHITE, outline=INK)
    d.line((14, 20, 17, 20), fill=RED)
    return im


def midas_mask():
    im, d = card((0x8C, 0x2E, 0x3C))
    d.polygon([(8, 6), (24, 6), (25, 18), (21, 27), (11, 27), (7, 18)], fill=GOLD, outline=INK)
    d.polygon([(10, 8), (22, 8), (23, 12), (9, 12)], fill=(0xFF, 0xE8, 0x7A))
    for x in (10, 18):
        d.polygon([(x, 14), (x + 4, 14), (x + 3, 17), (x + 1, 17)], fill=INK)
    d.line((16, 17, 16, 22), fill=GOLD_D)
    d.line((13, 24, 19, 24), fill=GOLD_D)
    d.line((9, 7, 12, 7), fill=WHITE)
    return im


def tarot_card(d, x, y, w, h, fill, outline=INK):
    d.rectangle((x, y, x + w, y + h), fill=fill, outline=outline)


def cartomancer():
    im, d = card((0x3A, 0x2A, 0x6A))
    d.polygon([(6, 24), (6, 12), (14, 10), (14, 25)], fill=(0xC8, 0xB4, 0xE8), outline=INK)
    d.polygon([(26, 24), (26, 12), (18, 10), (18, 25)], fill=(0xC8, 0xB4, 0xE8), outline=INK)
    tarot_card(d, 11, 6, 9, 20, (0xFF, 0xF3, 0xD6))
    cx, cy = 15, 15
    pts = []
    for i in range(10):
        r = 5 if i % 2 == 0 else 2
        a = -math.pi / 2 + i * math.pi / 5
        pts.append((cx + r * math.cos(a), cy + r * math.sin(a)))
    d.polygon(pts, fill=GOLD, outline=GOLD_D)
    d.line((13, 22, 18, 22), fill=GOLD_D)
    return im


def constellation():
    im, d = card((0x14, 0x1C, 0x4C))
    pts = [(6, 22), (11, 12), (18, 16), (22, 7), (26, 20)]
    d.line(pts, fill=(0x6A, 0x84, 0xC8))
    for i, (x, y) in enumerate(pts):
        d.rectangle((x - 1, y - 1, x + 1, y + 1), fill=GOLD if i % 2 == 0 else WHITE)
        d.point((x, y - 3), fill=WHITE)
    for x, y in ((5, 6), (14, 5), (28, 12), (9, 28), (20, 27)):
        d.point((x, y), fill=(0xC0, 0xC8, 0xE8))
    return im


def fortune_teller():
    im, d = card((0x50, 0x3A, 0x8A))
    d.rectangle((3, 24, 28, 29), fill=(0x3A, 0x2A, 0x6A))
    d.polygon([(10, 28), (22, 28), (20, 23), (12, 23)], fill=GOLD, outline=INK)
    d.ellipse((7, 4, 25, 23), fill=(0x9A, 0xD8, 0xF0), outline=INK)
    d.ellipse((10, 7, 16, 13), fill=WHITE)
    d.arc((9, 9, 23, 21), 20, 160, fill=BLUE)
    d.polygon([(19, 12), (20, 14), (22, 14), (20, 16), (21, 18), (19, 16), (17, 18), (18, 16), (16, 14), (18, 14)], fill=GOLD)
    return im


def hallucination():
    im, d = card((0xE8, 0x6A, 0xB4))
    d.ellipse((4, 8, 27, 24), fill=WHITE, outline=INK)
    d.ellipse((10, 9, 21, 23), fill=GREEN, outline=INK)
    d.ellipse((13, 12, 18, 20), fill=INK)
    d.point((14, 13), fill=WHITE)
    d.arc((9, 4, 23, 16), 200, 340, fill=INK)
    for x in (7, 11, 16, 21, 25):
        d.line((x, 6, x + (x - 16) // 4, 3), fill=INK)
    d.arc((8, 22, 24, 30), 30, 150, fill=PURPLE)
    d.arc((11, 24, 21, 30), 30, 150, fill=GOLD)
    return im


def stone_joker():
    im, d = card((0x6A, 0x72, 0x7C))
    d.polygon([(6, 24), (4, 15), (10, 7), (20, 5), (27, 11), (28, 22), (22, 27), (11, 27)], fill=GREY_L, outline=INK)
    d.polygon([(10, 7), (20, 5), (27, 11), (16, 12)], fill=(0xE0, 0xE4, 0xEA))
    d.line((12, 14, 15, 19), fill=GREY_D)
    d.line((15, 19, 13, 25), fill=GREY_D)
    d.line((20, 16, 24, 21), fill=GREY_D)
    for x in (11, 19):
        d.rectangle((x, 12, x + 1, 13), fill=INK)
    d.point((8, 10), fill=WHITE)
    return im


def marble():
    im, d = card((0x2C, 0x6A, 0x78))
    d.ellipse((6, 6, 26, 26), fill=(0xF4, 0xF6, 0xF8), outline=INK)
    d.arc((9, 9, 23, 23), 20, 200, fill=(0x70, 0x90, 0xC8))
    d.arc((11, 12, 25, 26), 180, 330, fill=GREY)
    d.line((10, 16, 14, 14), fill=GREY)
    d.line((14, 14, 16, 18), fill=(0x70, 0x90, 0xC8))
    d.ellipse((9, 9, 13, 12), fill=WHITE)
    d.arc((7, 7, 25, 25), 300, 60, fill=GREY_L)
    return im


def certificate():
    im, d = card((0xD8, 0xC0, 0x90))
    d.rectangle((5, 6, 26, 25), fill=(0xFF, 0xF6, 0xDC), outline=INK)
    for y in (10, 13, 16):
        d.line((8, y, 23, y), fill=GREY)
    d.ellipse((17, 17, 25, 25), fill=RED, outline=INK)
    d.ellipse((19, 19, 23, 23), outline=GOLD)
    d.polygon([(18, 24), (17, 29), (20, 27), (21, 29), (22, 24)], fill=RED, outline=INK)
    d.line((8, 21, 14, 21), fill=GOLD_D)
    return im


def space():
    im, d = card((0x10, 0x14, 0x3C))
    d.ellipse((9, 9, 23, 23), fill=(0xE8, 0x8A, 0x3C), outline=INK)
    d.arc((11, 11, 23, 19), 180, 330, fill=(0xF8, 0xC8, 0x70))
    d.polygon([(5, 18), (16, 22), (27, 14), (27, 17), (16, 26), (5, 21)], fill=(0xC8, 0xD0, 0xF0), outline=INK)
    d.ellipse((14, 14, 18, 17), fill=(0xE8, 0x8A, 0x3C))
    for x, y in ((5, 5), (26, 5), (5, 28), (27, 27), (14, 4)):
        d.point((x, y), fill=WHITE)
    return im


def hiker():
    im, d = card((0x8C, 0xCC, 0xF0))
    d.polygon([(2, 28), (12, 9), (19, 20), (23, 14), (30, 28)], fill=(0x6A, 0x7A, 0x70), outline=INK)
    d.polygon([(12, 9), (9, 15), (12, 14), (14, 16), (16, 14)], fill=WHITE)
    d.rectangle((3, 25, 28, 29), fill=GREEN)
    d.line((17, 8, 17, 18), fill=INK)
    d.polygon([(17, 8), (23, 10), (17, 12)], fill=RED, outline=INK)
    d.ellipse((24, 4, 28, 8), fill=GOLD)
    return im


def burglar():
    im, d = card((0x34, 0x3C, 0x58))
    d.ellipse((8, 5, 24, 25), fill=(0xF0, 0xC8, 0xA0), outline=INK)
    d.rectangle((7, 12, 25, 18), fill=INK)
    for x in (11, 18):
        d.rectangle((x, 14, x + 2, 15), fill=WHITE)
    d.line((12, 22, 20, 22), fill=INK)
    d.rectangle((13, 26, 24, 28), fill=GOLD, outline=INK)
    d.ellipse((10, 25, 14, 29), outline=GOLD_D, fill=GOLD)
    return im


JOKERS = [steel, glass, lucky_cat, vampire, midas_mask, cartomancer, constellation, fortune_teller,
          hallucination, stone_joker, marble, certificate, space, hiker, burglar]
FIRST_SHEET = 44


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
