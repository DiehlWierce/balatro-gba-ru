#!/usr/bin/env python3
"""Generates graphics/joker_gfx18..28.png (A8 batch 1 Jokers, 32x32 each, 15 colors + transparent).

Run from the repository root: python3 ru-tools/sprites/gen_a8_jokers_1.py
"""
import sys
from PIL import Image, ImageDraw

OUT = (0x38, 0x43, 0x4A)
RIM = (0xF4, 0xF4, 0xF4)
INK = (0x24, 0x28, 0x30)
WHITE = (255, 255, 255)


def card(bg):
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle((1, 0, 30, 31), 3, fill=OUT)
    d.rounded_rectangle((2, 1, 29, 30), 2, fill=RIM)
    d.rounded_rectangle((3, 2, 28, 29), 1, fill=bg)
    return im, d


def baron():
    im, d = card((0x5B, 0x3F, 0x8C))
    d.rectangle((3, 22, 28, 29), fill=(0x45, 0x2E, 0x6E))
    gold, dark = (0xF2, 0xC2, 0x30), (0xB8, 0x86, 0x1B)
    d.polygon([(8, 19), (7, 9), (12, 14), (16, 7), (20, 14), (25, 9), (24, 19)], fill=gold, outline=INK)
    d.rectangle((8, 19, 24, 23), fill=gold, outline=INK)
    d.line((9, 22, 23, 22), fill=dark)
    for x, c in ((11, (0xE0, 0x3C, 0x4C)), (16, (0x4C, 0xA8, 0xF0)), (21, (0xE0, 0x3C, 0x4C))):
        d.rectangle((x - 1, 20, x, 21), fill=c)
    for x, y in ((7, 8), (16, 6), (25, 8)):
        d.point((x, y), fill=WHITE)
    return im


def ride_the_bus():
    im, d = card((0xE7, 0xC0, 0x4A))
    d.rectangle((3, 24, 28, 29), fill=(0x8A, 0x8F, 0x96))
    d.rectangle((5, 24, 26, 25), fill=(0x5C, 0x62, 0x6A))
    d.rounded_rectangle((5, 10, 26, 23), 2, fill=(0xE8, 0x6A, 0x1C), outline=INK)
    d.rectangle((6, 19, 25, 20), fill=WHITE)
    for x in (7, 12, 17):
        d.rectangle((x, 12, x + 3, 16), fill=(0x9A, 0xD4, 0xF0), outline=INK)
    d.rectangle((22, 12, 25, 18), fill=(0x9A, 0xD4, 0xF0), outline=INK)
    for x in (9, 21):
        d.ellipse((x - 2, 21, x + 2, 25), fill=INK)
        d.point((x, 23), fill=(0xC8, 0xC8, 0xC8))
    return im


def supernova():
    im, d = card((0x1C, 0x22, 0x50))
    cx, cy = 16, 15
    ray, core = (0xF8, 0x9A, 0x28), (0xFF, 0xE8, 0x7A)
    for dx, dy in ((0, -1), (0, 1), (-1, 0), (1, 0)):
        d.line((cx, cy, cx + dx * 11, cy + dy * 11), fill=ray, width=2)
    for dx, dy in ((-1, -1), (1, -1), (-1, 1), (1, 1)):
        d.line((cx, cy, cx + dx * 8, cy + dy * 8), fill=(0xF0, 0x5A, 0x3C))
    d.ellipse((cx - 5, cy - 5, cx + 5, cy + 5), fill=ray)
    d.ellipse((cx - 3, cy - 3, cx + 3, cy + 3), fill=core)
    d.point((cx, cy), fill=WHITE)
    for x, y in ((6, 5), (26, 7), (5, 24), (27, 25), (22, 27), (9, 27)):
        d.point((x, y), fill=WHITE)
    return im


def runner():
    im, d = card((0x2F, 0x8F, 0x6F))
    red = (0xE0, 0x3C, 0x4C)
    for y in (11, 15, 19):
        d.line((5, y, 11, y), fill=(0xB8, 0xF0, 0xD8))
    d.polygon([(11, 8), (17, 8), (18, 15), (26, 19), (26, 24), (11, 24)], fill=WHITE, outline=INK)
    d.polygon([(11, 20), (26, 20), (26, 24), (11, 24)], fill=red, outline=INK)
    d.line((13, 12, 16, 12), fill=red)
    d.line((13, 15, 17, 15), fill=red)
    d.rectangle((11, 25, 26, 26), fill=INK)
    return im


def square():
    im, d = card((0x4A, 0x6E, 0xA8))
    d.rectangle((8, 8, 23, 23), fill=(0xF2, 0x9A, 0x30), outline=INK)
    d.rectangle((9, 9, 22, 22), outline=(0xFF, 0xD0, 0x80))
    for x, y in ((11, 11), (19, 11), (11, 19), (19, 19)):
        d.rectangle((x, y, x + 1, y + 1), fill=WHITE)
    d.rectangle((14, 14, 17, 17), fill=(0xB8, 0x5A, 0x14))
    return im


def ice_cream():
    im, d = card((0xF7, 0xC1, 0xD8))
    d.polygon([(10, 16), (22, 16), (16, 28)], fill=(0xD9, 0x9A, 0x4A), outline=INK)
    for a, b in (((12, 18), (19, 22)), ((20, 18), (13, 22)), ((14, 24), (18, 24))):
        d.line(a + b, fill=(0x9A, 0x62, 0x24))
    d.ellipse((9, 8, 23, 18), fill=(0xFF, 0xF3, 0xD6), outline=INK)
    d.ellipse((11, 4, 21, 12), fill=(0xF0, 0x6A, 0x9A), outline=INK)
    d.rectangle((15, 2, 16, 4), fill=(0xD0, 0x28, 0x38))
    d.point((14, 7), fill=WHITE)
    return im


def loyalty_card():
    im, d = card((0xB5, 0x33, 0x3C))
    d.rounded_rectangle((6, 8, 25, 24), 2, fill=WHITE, outline=INK)
    d.rectangle((7, 10, 24, 12), fill=(0xF2, 0xC2, 0x30))
    for i, x in enumerate((10, 15, 20)):
        for j, y in enumerate((16, 21)):
            filled = i + j * 3 < 2
            d.ellipse((x - 1, y - 1, x + 2, y + 2), fill=(0xB8, 0x86, 0x1B) if filled else (0xD8, 0xD8, 0xD8), outline=INK if not filled else (0x80, 0x5A, 0x10))
    return im


def spare_trousers():
    im, d = card((0xC8, 0xA8, 0x78))
    blue, dk = (0x3E, 0x60, 0xD4), (0x28, 0x3C, 0x90)
    d.polygon([(10, 7), (22, 7), (23, 25), (17, 25), (16, 14), (15, 25), (9, 25)], fill=blue, outline=INK)
    d.rectangle((10, 7, 22, 10), fill=dk, outline=INK)
    d.line((16, 11, 16, 15), fill=dk)
    d.rectangle((17, 12, 20, 15), outline=dk)
    d.rectangle((11, 18, 14, 22), fill=(0xF2, 0xC2, 0x30), outline=INK)
    d.line((12, 20, 13, 20), fill=INK)
    d.rectangle((13, 8, 14, 9), fill=(0xF2, 0xC2, 0x30))
    return im


def wee():
    im, d = card((0xA8, 0xC8, 0xF0))
    d.ellipse((11, 15, 21, 25), fill=(0xFF, 0xE0, 0xC0), outline=INK)
    d.polygon([(10, 16), (11, 8), (16, 13), (21, 8), (22, 16)], fill=(0xE0, 0x3C, 0x4C), outline=INK)
    d.rectangle((10, 15, 22, 16), fill=(0xF2, 0xC2, 0x30), outline=INK)
    d.rectangle((10, 7, 11, 8), fill=(0xF2, 0xC2, 0x30))
    d.rectangle((21, 7, 22, 8), fill=(0xF2, 0xC2, 0x30))
    d.point((14, 19), fill=INK)
    d.point((18, 19), fill=INK)
    d.line((14, 22, 18, 22), fill=(0xE0, 0x3C, 0x4C))
    d.point((13, 21), fill=(0xE0, 0x3C, 0x4C))
    d.point((19, 21), fill=(0xE0, 0x3C, 0x4C))
    return im


def onyx_agate():
    im, d = card((0x20, 0x24, 0x2A))
    d.polygon([(16, 4), (26, 10), (26, 22), (16, 28), (6, 22), (6, 10)], fill=(0x3A, 0x44, 0x58), outline=(0x8A, 0x9A, 0xB8))
    d.polygon([(16, 7), (23, 11), (23, 21), (16, 25), (9, 21), (9, 11)], fill=(0x18, 0x1C, 0x22))
    d.line((10, 11, 15, 8), fill=(0x6A, 0x7A, 0x98))
    for x, y in ((16, 11), (12, 17), (20, 17)):
        d.ellipse((x - 2, y - 2, x + 2, y + 2), fill=WHITE)
    d.rectangle((15, 17, 17, 23), fill=WHITE)
    d.point((13, 23), fill=WHITE)
    d.point((19, 23), fill=WHITE)
    return im


def arrowhead():
    im, d = card((0x6E, 0x7A, 0x58))
    d.polygon([(16, 4), (25, 18), (19, 17), (18, 26), (14, 26), (13, 17), (7, 18)], fill=(0xC8, 0xCE, 0xD8), outline=INK)
    d.polygon([(16, 6), (16, 17), (13, 17), (9, 17)], fill=(0xE8, 0xEC, 0xF4))
    d.line((16, 6, 16, 24), fill=(0x88, 0x92, 0xA4))
    d.rectangle((14, 22, 18, 24), fill=(0xB8, 0x72, 0x30))
    d.line((14, 23, 18, 23), fill=(0x7A, 0x44, 0x14))
    return im


JOKERS = [baron, ride_the_bus, supernova, runner, square, ice_cream, loyalty_card,
          spare_trousers, wee, onyx_agate, arrowhead]
FIRST_SHEET = 18

GRIT = "-gB4 -Mw4 -Mh4 -m! -pn 16"


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
