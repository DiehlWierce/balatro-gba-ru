#!/usr/bin/env python3
"""Generates graphics/joker_gfx29..43.png (A8 batch 2 Jokers, 32x32 each, 15 colors + transparent).

Run from the repository root: python3 ru-tools/sprites/gen_a8_jokers_2.py
"""
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from gen_a8_jokers_1 import card, INK, WHITE, GRIT  # noqa: E402

GOLD, GOLD_D = (0xF2, 0xC2, 0x30), (0xB8, 0x86, 0x1B)
RED, BLUE = (0xE0, 0x3C, 0x4C), (0x3E, 0x60, 0xD4)
GREEN = (0x2F, 0x8F, 0x6F)


def coin(d, cx, cy, r=5):
    d.ellipse((cx - r, cy - r, cx + r, cy + r), fill=GOLD, outline=INK)
    d.ellipse((cx - r + 2, cy - r + 2, cx + r - 2, cy + r - 2), outline=GOLD_D)
    d.point((cx - 1, cy - 2), fill=WHITE)


def golden():
    im, d = card((0xF6, 0xE0, 0x8C))
    d.rectangle((3, 22, 28, 29), fill=(0xE6, 0xC8, 0x60))
    coin(d, 16, 14, 8)
    d.line((13, 17, 16, 10), fill=GOLD_D)
    d.line((16, 10, 19, 17), fill=GOLD_D)
    d.line((14, 15, 18, 15), fill=GOLD_D)
    coin(d, 8, 24, 3)
    coin(d, 24, 24, 3)
    return im


def delayed():
    im, d = card((0x7A, 0x5A, 0xB0))
    d.ellipse((7, 6, 24, 23), fill=WHITE, outline=INK)
    d.line((15, 14, 15, 9), fill=INK)
    d.line((15, 14, 20, 17), fill=INK)
    d.rectangle((13, 4, 18, 5), fill=GOLD, outline=INK)
    d.rectangle((9, 25, 22, 28), fill=GOLD, outline=INK)
    d.line((12, 26, 19, 26), fill=GOLD_D)
    return im


def to_the_moon():
    im, d = card((0x14, 0x1A, 0x44))
    d.ellipse((6, 5, 24, 23), fill=(0xF0, 0xF0, 0xDC), outline=INK)
    d.ellipse((12, 5, 28, 21), fill=(0x14, 0x1A, 0x44))
    for x, y in ((8, 10), (10, 17), (13, 21)):
        d.rectangle((x, y, x + 1, y + 1), fill=(0xC0, 0xC0, 0xA8))
    for x, y in ((5, 26), (26, 26), (27, 8), (4, 4)):
        d.point((x, y), fill=WHITE)
    d.polygon([(20, 28), (23, 24), (27, 28)], fill=GREEN, outline=INK)
    d.rectangle((22, 28, 24, 29), fill=GOLD)
    return im


def rocket():
    im, d = card((0x20, 0x2C, 0x58))
    d.polygon([(16, 3), (22, 11), (22, 23), (10, 23), (10, 11)], fill=WHITE, outline=INK)
    d.polygon([(10, 17), (5, 25), (10, 23)], fill=RED, outline=INK)
    d.polygon([(22, 17), (27, 25), (22, 23)], fill=RED, outline=INK)
    d.ellipse((13, 10, 19, 16), fill=(0x4C, 0xA8, 0xF0), outline=INK)
    d.polygon([(12, 24), (20, 24), (16, 30)], fill=(0xF8, 0x9A, 0x28), outline=INK)
    d.polygon([(14, 24), (18, 24), (16, 27)], fill=(0xFF, 0xE8, 0x7A))
    return im


def egg():
    im, d = card((0xA8, 0xD8, 0x98))
    d.ellipse((8, 5, 23, 26), fill=(0xFF, 0xF3, 0xD6), outline=INK)
    d.ellipse((11, 9, 14, 14), fill=WHITE)
    d.polygon([(8, 18), (11, 15), (14, 19), (17, 14), (20, 19), (23, 16), (23, 22), (8, 22)], fill=(0xF2, 0xC2, 0x30), outline=INK)
    d.arc((8, 5, 23, 26), 40, 120, fill=INK)
    return im


def credit_card():
    im, d = card((0x3A, 0x4A, 0x7A))
    d.rounded_rectangle((4, 8, 27, 24), 2, fill=(0xE8, 0xE8, 0xF0), outline=INK)
    d.rectangle((5, 11, 26, 14), fill=INK)
    d.rectangle((7, 17, 11, 20), fill=GOLD, outline=GOLD_D)
    for x in (14, 18, 22):
        d.rectangle((x, 19, x + 2, 20), fill=(0x80, 0x88, 0xA0))
    d.rectangle((20, 5, 25, 7), fill=RED)
    return im


def juggler():
    im, d = card((0xE8, 0x7A, 0x3C))
    for cx, cy, c in ((9, 9, RED), (16, 5, (0xF2, 0xC2, 0x30)), (23, 9, (0x4C, 0xA8, 0xF0))):
        d.ellipse((cx - 3, cy - 3, cx + 3, cy + 3), fill=c, outline=INK)
    d.ellipse((12, 14, 20, 22), fill=(0xFF, 0xE0, 0xC0), outline=INK)
    d.rectangle((11, 22, 21, 28), fill=BLUE, outline=INK)
    d.line((11, 23, 6, 14), fill=(0xFF, 0xE0, 0xC0))
    d.line((21, 23, 26, 14), fill=(0xFF, 0xE0, 0xC0))
    d.point((14, 17), fill=INK)
    d.point((18, 17), fill=INK)
    return im


def drunkard():
    im, d = card((0x9A, 0x4A, 0x58))
    d.polygon([(9, 6), (23, 6), (21, 26), (11, 26)], fill=(0xD8, 0xF0, 0xF8), outline=INK)
    d.polygon([(10, 14), (22, 14), (21, 26), (11, 26)], fill=(0xF2, 0xA8, 0x30))
    d.rectangle((10, 12, 22, 14), fill=WHITE)
    d.arc((20, 10, 28, 20), 270, 90, fill=INK, width=2)
    d.line((13, 8, 13, 22), fill=WHITE)
    return im


def troubadour():
    im, d = card((0x6A, 0x3E, 0x8C))
    d.ellipse((10, 12, 22, 24), fill=(0xC8, 0x80, 0x38), outline=INK)
    d.rectangle((14, 5, 17, 14), fill=(0x7A, 0x44, 0x14), outline=INK)
    d.ellipse((13, 16, 19, 22), fill=INK)
    for y in (13, 24):
        d.line((11, y, 21, y), fill=GOLD)
    d.polygon([(21, 6), (27, 5), (27, 10), (24, 11), (24, 8), (21, 8)], fill=GOLD, outline=INK)
    return im


def merry_andy():
    im, d = card((0xF4, 0xA0, 0xC0))
    d.ellipse((8, 8, 24, 24), fill=(0xFF, 0xE0, 0xC0), outline=INK)
    d.polygon([(8, 11), (6, 4), (13, 9)], fill=RED, outline=INK)
    d.polygon([(24, 11), (26, 4), (19, 9)], fill=BLUE, outline=INK)
    d.ellipse((10, 14, 12, 16), fill=INK)
    d.ellipse((20, 14, 22, 16), fill=INK)
    d.arc((11, 15, 21, 23), 20, 160, fill=RED, width=2)
    d.rectangle((13, 25, 19, 28), fill=GOLD, outline=INK)
    return im


def stuntman():
    im, d = card((0xC0, 0x30, 0x38))
    d.rectangle((3, 24, 28, 29), fill=(0x7A, 0x1C, 0x24))
    d.ellipse((11, 6, 21, 16), fill=(0xFF, 0xE0, 0xC0), outline=INK)
    d.rectangle((11, 5, 21, 8), fill=INK)
    d.rectangle((8, 16, 24, 26), fill=WHITE, outline=INK)
    d.polygon([(16, 16), (13, 26), (19, 26)], fill=RED)
    d.rectangle((5, 18, 8, 21), fill=(0xFF, 0xE0, 0xC0), outline=INK)
    d.rectangle((24, 18, 27, 21), fill=(0xFF, 0xE0, 0xC0), outline=INK)
    return im


def gift_card():
    im, d = card((0x4C, 0xA8, 0xA0))
    d.rectangle((6, 12, 25, 26), fill=RED, outline=INK)
    d.rectangle((5, 9, 26, 13), fill=(0xF0, 0x5A, 0x6A), outline=INK)
    d.rectangle((14, 9, 17, 26), fill=GOLD)
    d.ellipse((9, 3, 16, 9), outline=GOLD, width=2)
    d.ellipse((16, 3, 23, 9), outline=GOLD, width=2)
    return im


def swashbuckler():
    im, d = card((0x2C, 0x6A, 0x8C))
    d.polygon([(4, 12), (16, 3), (28, 12), (22, 13), (10, 13)], fill=INK)
    d.rectangle((9, 12, 23, 14), fill=GOLD, outline=GOLD_D)
    d.ellipse((10, 14, 22, 26), fill=(0xFF, 0xE0, 0xC0), outline=INK)
    d.rectangle((10, 17, 14, 19), fill=INK)
    d.point((18, 19), fill=INK)
    d.line((14, 23, 18, 23), fill=RED)
    d.polygon([(6, 28), (8, 20), (10, 28)], fill=(0xC8, 0xCE, 0xD8), outline=INK)
    return im


def golden_ticket():
    im, d = card((0x8C, 0x2C, 0x4C))
    d.polygon([(3, 9), (28, 9), (28, 14), (26, 16), (28, 18), (28, 23), (3, 23), (3, 18), (5, 16), (3, 14)], fill=GOLD, outline=INK)
    d.line((10, 10, 10, 22), fill=GOLD_D)
    d.polygon([(19, 12), (20, 15), (23, 15), (21, 17), (22, 20), (19, 18), (16, 20), (17, 17), (15, 15), (18, 15)], fill=WHITE, outline=GOLD_D)
    return im


def rough_gem():
    im, d = card((0x3A, 0x30, 0x58))
    d.polygon([(16, 4), (26, 13), (16, 28), (6, 13)], fill=(0x58, 0xC8, 0xF0), outline=INK)
    d.polygon([(6, 13), (26, 13), (16, 4)], fill=(0x9A, 0xE4, 0xF8), outline=INK)
    d.line((12, 13, 16, 28), fill=(0x2C, 0x88, 0xB8))
    d.line((20, 13, 16, 28), fill=(0x2C, 0x88, 0xB8))
    d.line((12, 13, 16, 4), fill=WHITE)
    d.point((10, 11), fill=WHITE)
    return im


JOKERS = [golden, delayed, to_the_moon, rocket, egg, credit_card, juggler, drunkard, troubadour,
          merry_andy, stuntman, gift_card, swashbuckler, golden_ticket, rough_gem]
FIRST_SHEET = 29


def main():
    for i, fn in enumerate(JOKERS):
        im = fn()
        n = len(set(p for p in im.getdata() if p[3]))
        assert n <= 15, (fn.__name__, n)
        idx = FIRST_SHEET + i
        im.save(f"graphics/joker_gfx{idx}.png")
        with open(f"graphics/joker_gfx{idx}.grit", "w") as f:
            f.write(GRIT)
    print("ok", len(JOKERS))


if __name__ == "__main__":
    sys.exit(main())
