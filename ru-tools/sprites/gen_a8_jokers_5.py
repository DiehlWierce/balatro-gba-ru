#!/usr/bin/env python3
"""Generates graphics/joker_gfx74..88.png (A8 batch 5 Jokers, ID 109-123, 32x32 each, 15 colors + transparent).

Run from the repository root: python3 ru-tools/sprites/gen_a8_jokers_5.py
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
BANANA, BANANA_D = (0xF6, 0xDC, 0x3C), (0xB8, 0x8A, 0x1C)


DIGITS = {
    6: ["01110", "10000", "10000", "11110", "10001", "10001", "01110"],
    9: ["01110", "10001", "10001", "01111", "00001", "00001", "01110"],
}


def digit(d, n, x, y, scale, fill):
    for r, row in enumerate(DIGITS[n]):
        for c, v in enumerate(row):
            if v == "1":
                d.rectangle((x + c * scale, y + r * scale, x + (c + 1) * scale - 1, y + (r + 1) * scale - 1), fill=fill)


def seance():
    im, d = card((0x2E, 0x22, 0x4E))
    d.polygon([(8, 27), (11, 21), (21, 21), (24, 27)], fill=GOLD, outline=INK)
    d.line((10, 25, 22, 25), fill=GOLD_D)
    d.ellipse((7, 4, 25, 22), fill=(0x8A, 0xA8, 0xF0), outline=INK)
    d.ellipse((10, 7, 15, 12), fill=(0xC8, 0xDC, 0xFF))
    d.polygon([(16, 10), (17, 14), (21, 15), (17, 16), (16, 20), (15, 16), (11, 15), (15, 14)], fill=PURPLE)
    d.point((13, 9), fill=WHITE)
    return im


def sixth_sense():
    im, d = card((0x2A, 0x5A, 0x6A))
    d.rectangle((8, 4, 24, 27), fill=WHITE, outline=INK)
    digit(d, 6, 11, 8, 2, RED)
    d.line((11, 23, 21, 23), fill=GREY_L)
    for x, y in ((5, 8), (27, 12), (5, 23), (27, 24)):
        d.line((x - 1, y, x + 1, y), fill=GOLD)
        d.line((x, y - 1, x, y + 1), fill=GOLD)
    return im


def dna():
    im, d = card((0x1E, 0x5A, 0x48))
    pts_a, pts_b = [], []
    for y in range(4, 29):
        x = 16 + 8 * math.sin((y - 4) / 24 * 2 * math.pi)
        pts_a.append((x, y))
        pts_b.append((32 - x, y))
    for y in range(6, 28, 3):
        xa = 16 + 8 * math.sin((y - 4) / 24 * 2 * math.pi)
        if abs(32 - 2 * xa) > 4:
            d.line((xa, y, 32 - xa, y), fill=WHITE)
    d.line(pts_a, fill=GOLD, width=2)
    d.line(pts_b, fill=(0xF0, 0x6A, 0x8A), width=2)
    return im


def banana(d, flip=False, fill=BANANA, shade=BANANA_D):
    sign = -1 if flip else 1
    cx = 16
    outer = [(cx - sign * 9, 7), (cx - sign * 12, 15), (cx - sign * 7, 23), (cx + sign * 3, 26),
             (cx + sign * 10, 23)]
    inner = [(cx + sign * 8, 20), (cx + sign * 2, 21), (cx - sign * 3, 17), (cx - sign * 5, 11)]
    d.polygon(outer + inner, fill=fill, outline=INK)
    d.line(outer[1:4], fill=shade)
    d.rectangle((min(cx - sign * 10, cx - sign * 8), 4, max(cx - sign * 10, cx - sign * 8), 7), fill=(0x6A, 0x4A, 0x20), outline=INK)
    d.rectangle((min(cx + sign * 9, cx + sign * 11), 22, max(cx + sign * 9, cx + sign * 11), 24), fill=(0x6A, 0x4A, 0x20))


def gros_michel():
    im, d = card((0x5A, 0xA8, 0xD8))
    banana(d)
    d.point((10, 12), fill=WHITE)
    return im


def cavendish():
    im, d = card((0x8A, 0xCC, 0x6A))
    banana(d, flip=True, fill=(0xF8, 0xEA, 0x6A), shade=(0xC8, 0xA8, 0x2C))
    d.line((6, 6, 8, 4), fill=WHITE)
    d.line((24, 7, 26, 5), fill=WHITE)
    return im


def popcorn():
    im, d = card((0xC8, 0x3C, 0x3C))
    for x, y in ((9, 8), (14, 5), (19, 7), (23, 10), (12, 11), (17, 11)):
        d.ellipse((x - 3, y - 3, x + 3, y + 3), fill=(0xFF, 0xF3, 0xC6), outline=INK)
    d.polygon([(7, 14), (25, 14), (22, 28), (10, 28)], fill=WHITE, outline=INK)
    for x in (11, 16, 20):
        d.line((x, 15, x + (x - 16) // 4, 27), fill=RED, width=2)
    d.point((13, 8), fill=GOLD)
    return im


def ramen():
    im, d = card((0xE8, 0x9A, 0x4A))
    for x in (8, 12, 16):
        d.line((x, 10 - x // 8, x + 8, 3), fill=GOLD_D)
    d.line((14, 11, 18, 4), fill=GOLD_D)
    for y in (13, 16, 19):
        d.line((8, y, 24, y), fill=BANANA)
        d.arc((8, y - 1, 24, y + 2), 0, 180, fill=GOLD)
    d.ellipse((13, 12, 17, 16), fill=WHITE, outline=INK)
    d.ellipse((14, 13, 16, 15), fill=GOLD)
    d.chord((5, 10, 27, 30), 0, 180, fill=WHITE, outline=INK)
    d.arc((5, 10, 27, 14), 0, 360, fill=INK)
    d.line((8, 21, 24, 21), fill=RED)
    return im


def castle():
    im, d = card((0x6A, 0x9A, 0xD8))
    d.rectangle((5, 14, 26, 28), fill=GREY, outline=INK)
    for x in (5, 10, 15, 20, 25):
        d.rectangle((x, 11, x + 2, 14), fill=GREY, outline=INK)
    d.rectangle((12, 5, 19, 14), fill=GREY_L, outline=INK)
    for x in (12, 16, 18):
        d.rectangle((x, 3, x + 1, 5), fill=GREY_L, outline=INK)
    d.line((16, 1, 16, 5), fill=INK)
    d.polygon([(16, 1), (21, 3), (16, 4)], fill=RED)
    d.rectangle((13, 21, 18, 28), fill=GREY_D, outline=INK)
    d.pieslice((13, 19, 18, 24), 180, 360, fill=GREY_D)
    d.rectangle((7, 18, 8, 20), fill=INK)
    d.rectangle((23, 18, 24, 20), fill=INK)
    return im


def mail_in_rebate():
    im, d = card((0x3E, 0x68, 0xA8))
    d.rectangle((5, 9, 26, 24), fill=WHITE, outline=INK)
    d.line((5, 9, 16, 18), fill=GREY)
    d.line((26, 9, 16, 18), fill=GREY)
    d.ellipse((12, 14, 20, 22), fill=GOLD, outline=INK)
    d.line((16, 15, 16, 21), fill=INK)
    d.arc((13, 15, 19, 18), 90, 360, fill=INK)
    d.arc((13, 18, 19, 21), 270, 180, fill=INK)
    d.rectangle((20, 5, 25, 8), fill=RED, outline=INK)
    return im


def trading_card():
    im, d = card((0x8C, 0x3A, 0x6A))
    d.polygon([(6, 24), (4, 11), (14, 8), (17, 21)], fill=GREY_L, outline=INK)
    d.rectangle((13, 6, 26, 26), fill=WHITE, outline=INK)
    d.rectangle((15, 8, 24, 17), fill=(0x9A, 0xD4, 0xF0), outline=INK)
    d.polygon([(19, 9), (20, 12), (23, 12), (21, 14), (22, 16), (19, 15), (16, 16), (17, 14), (15, 12), (18, 12)],
              fill=GOLD)
    d.line((15, 20, 24, 20), fill=GREY)
    d.line((15, 23, 21, 23), fill=GREY)
    return im


def vagabond():
    im, d = card((0x9A, 0x7A, 0x4A))
    d.line((7, 27, 24, 5), fill=(0x5A, 0x3A, 0x1C), width=2)
    d.ellipse((17, 3, 28, 14), fill=RED, outline=INK)
    for x, y in ((21, 6), (24, 9), (20, 10)):
        d.point((x, y), fill=WHITE)
    d.line((21, 13, 22, 15), fill=INK)
    d.polygon([(8, 20), (13, 18), (15, 24), (10, 27)], fill=(0xD8, 0xC0, 0x8A), outline=INK)
    return im


def obelisk():
    im, d = card((0xE8, 0xB8, 0x6A))
    d.ellipse((19, 4, 27, 12), fill=(0xFF, 0xE8, 0x7A), outline=GOLD_D)
    d.polygon([(4, 28), (28, 28), (24, 24), (8, 24)], fill=(0xB8, 0x86, 0x4A), outline=INK)
    d.polygon([(11, 24), (21, 24), (19, 8), (16, 3), (13, 8)], fill=GREY_L, outline=INK)
    d.polygon([(16, 3), (19, 8), (16, 8)], fill=GOLD)
    d.line((16, 8, 16, 24), fill=GREY)
    for y in (12, 16, 20):
        d.line((14, y, 18, y), fill=GREY_D)
    return im


def turtle_bean():
    im, d = card((0x3A, 0x6A, 0x4A))
    d.ellipse((4, 14, 11, 21), fill=(0x8A, 0xC8, 0x6A), outline=INK)
    d.point((7, 16), fill=INK)
    for x in (10, 22):
        d.ellipse((x - 2, 22, x + 3, 28), fill=(0x8A, 0xC8, 0x6A), outline=INK)
    d.pieslice((8, 7, 28, 28), 180, 360, fill=(0x6A, 0x9A, 0x3A), outline=INK)
    d.chord((8, 10, 28, 30), 180, 360, fill=(0x6A, 0x9A, 0x3A), outline=INK)
    d.line((9, 21, 27, 21), fill=INK)
    for x, y in ((14, 15), (20, 15), (17, 11)):
        d.ellipse((x - 2, y - 2, x + 2, y + 2), outline=(0x3A, 0x5A, 0x2A))
    return im


def ceremonial_dagger():
    im, d = card((0x4A, 0x1A, 0x2E))
    d.polygon([(25, 4), (27, 8), (13, 21), (10, 18)], fill=GREY_L, outline=INK)
    d.line((25, 5, 12, 19), fill=WHITE)
    d.polygon([(7, 17), (14, 24), (12, 26), (5, 19)], fill=GOLD, outline=INK)
    d.line((10, 24, 6, 28), fill=PURPLE, width=2)
    d.ellipse((13, 19, 16, 22), fill=RED, outline=INK)
    d.rectangle((4, 27, 6, 29), fill=GOLD_D)
    return im


def cloud_nine():
    im, d = card((0x7A, 0xB8, 0xE8))
    blobs = ((9, 19, 6), (16, 13, 8), (23, 19, 6), (16, 20, 7))
    for x, y, r in blobs:
        d.ellipse((x - r - 1, y - r - 1, x + r + 1, y + r + 1), fill=INK)
    d.rectangle((8, 20, 24, 26), fill=INK)
    for x, y, r in blobs:
        d.ellipse((x - r, y - r, x + r, y + r), fill=WHITE)
    d.rectangle((9, 20, 23, 25), fill=WHITE)
    digit(d, 9, 12, 11, 2, BLUE)
    return im


JOKERS = [seance, sixth_sense, dna, gros_michel, cavendish, popcorn, ramen, castle,
          mail_in_rebate, trading_card, vagabond, obelisk, turtle_bean, ceremonial_dagger,
          cloud_nine]
FIRST_SHEET = 74


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
