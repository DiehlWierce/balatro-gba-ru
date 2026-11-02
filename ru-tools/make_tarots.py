#!/usr/bin/env python3
"""Generates graphics/tarot_gfx.png: 22 Tarot cards (32x32 each, stacked vertically).

Tarots use the palette of the Planets (planet_gfx.png, bank CONSUMABLE_PB, see
docs/ru/PALETTES.md). Each card has its number on top and a simple symbol; the order matches
enum TarotType in include/tarot_rules.h.

usage: python3 ru-tools/make_tarots.py [out.png]
"""
import math
import sys

from PIL import Image

sys.path.insert(0, __file__.rsplit('/', 1)[0])
from make_planets import PALETTE, CARD_LEFT, CARD_RIGHT, card_frame  # noqa: E402

BG, FRAME_IN = 3, 15
DIGITS = {
    '0': ['111', '101', '101', '101', '111'], '1': ['010', '110', '010', '010', '111'],
    '2': ['111', '001', '111', '100', '111'], '3': ['111', '001', '111', '001', '111'],
    '4': ['101', '101', '111', '001', '001'], '5': ['111', '100', '111', '001', '111'],
    '6': ['111', '100', '111', '101', '111'], '7': ['111', '001', '010', '010', '010'],
    '8': ['111', '101', '111', '101', '111'], '9': ['111', '101', '111', '001', '111'],
}
CX, CY = 16, 18


def put(px, x, y, c):
    if CARD_LEFT + 2 <= x <= CARD_RIGHT - 2 and 3 <= y <= 28:
        px[x, y] = c


def disc(px, cx, cy, r, c):
    for y in range(int(cy - r - 1), int(cy + r + 2)):
        for x in range(int(cx - r - 1), int(cx + r + 2)):
            if math.hypot(x + .5 - cx, y + .5 - cy) <= r:
                put(px, x, y, c)


def ring(px, cx, cy, r, c, w=1.2):
    for y in range(int(cy - r - 2), int(cy + r + 3)):
        for x in range(int(cx - r - 2), int(cx + r + 3)):
            if abs(math.hypot(x + .5 - cx, y + .5 - cy) - r) <= w / 2 + .3:
                put(px, x, y, c)


def line(px, x0, y0, x1, y1, c):
    n = int(max(abs(x1 - x0), abs(y1 - y0))) * 2 + 1
    for i in range(n + 1):
        t = i / n
        put(px, int(round(x0 + (x1 - x0) * t)), int(round(y0 + (y1 - y0) * t)), c)


def rect(px, x0, y0, x1, y1, c):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            put(px, x, y, c)


def star(px, cx, cy, r, c, points=5):
    pts = []
    for i in range(points * 2):
        a = -math.pi / 2 + i * math.pi / points
        rr = r if i % 2 == 0 else r * 0.45
        pts.append((cx + rr * math.cos(a), cy + rr * math.sin(a)))
    for y in range(int(cy - r - 1), int(cy + r + 2)):
        for x in range(int(cx - r - 1), int(cx + r + 2)):
            inside = False
            j = len(pts) - 1
            for i in range(len(pts)):
                xi, yi = pts[i]
                xj, yj = pts[j]
                if (yi > y + .5) != (yj > y + .5) and x + .5 < (xj - xi) * (y + .5 - yi) / (yj - yi) + xi:
                    inside = not inside
                j = i
            if inside:
                put(px, x, y, c)


def heart(px, cx, cy, s, c):
    for y in range(-int(s * 1.4), int(s * 1.4)):
        for x in range(-int(s * 1.4), int(s * 1.4)):
            u, v = x / s, -y / s
            if (u * u + v * v - 1) ** 3 - u * u * v ** 3 <= 0:
                put(px, int(cx + x), int(cy + y), c)


def spade(px, cx, cy, s, c):
    heart_flip = [(x, y) for y in range(-int(s * 1.3), int(s * 1.3)) for x in range(-int(s * 1.3), int(s * 1.3))
                  if ((x / s) ** 2 + (y / s) ** 2 - 1) ** 3 - (x / s) ** 2 * (y / s) ** 3 <= 0]
    for x, y in heart_flip:
        put(px, int(cx + x), int(cy + y), c)
    rect(px, int(cx) - 1, int(cy + s * .6), int(cx), int(cy + s * 1.4), c)


def club(px, cx, cy, s, c):
    disc(px, cx, cy - s * .6, s * .5, c)
    disc(px, cx - s * .6, cy + s * .2, s * .5, c)
    disc(px, cx + s * .6, cy + s * .2, s * .5, c)
    rect(px, int(cx) - 1, int(cy), int(cx), int(cy + s * 1.2), c)


def diamond(px, cx, cy, r, c):
    for y in range(-r, r + 1):
        w = r - abs(y)
        rect(px, int(cx - w), int(cy + y), int(cx + w - 1), int(cy + y), c)


def moon(px, cx, cy, r, c):
    for y in range(int(cy - r), int(cy + r + 1)):
        for x in range(int(cx - r), int(cx + r + 1)):
            if math.hypot(x + .5 - cx, y + .5 - cy) <= r and math.hypot(x + .5 - cx - r * .55, y + .5 - cy) > r * .85:
                put(px, x, y, c)


def sun(px, cx, cy, r, c):
    disc(px, cx, cy, r, c)
    for k in range(8):
        a = k * math.pi / 4
        line(px, cx + (r + 2) * math.cos(a), cy + (r + 2) * math.sin(a),
             cx + (r + 5) * math.cos(a), cy + (r + 5) * math.sin(a), 8)


# Symbol drawers: (px) -> None, in the same order as enum TarotType
def s_fool(px):
    disc(px, 16, 17, 4, 2); line(px, 16, 22, 16, 27, 2); line(px, 11, 12, 21, 12, 10)
    line(px, 21, 12, 24, 20, 8); disc(px, 24, 21, 2, 8)


def s_magician(px):
    line(px, 12, 26, 21, 11, 8); star(px, 22, 10, 4, 15); line(px, 8, 16, 24, 16, 6)


def s_priestess(px):
    moon(px, 16, 18, 8, 6); rect(px, 9, 11, 10, 27, 7); rect(px, 21, 11, 22, 27, 7)


def s_empress(px):
    heart(px, 16, 19, 5, 10); star(px, 16, 10, 4, 8)


def s_emperor(px):
    rect(px, 10, 13, 21, 25, 9); rect(px, 10, 13, 21, 14, 8); rect(px, 14, 17, 17, 25, 4)
    diamond(px, 16, 10, 3, 8)


def s_hierophant(px):
    rect(px, 15, 9, 16, 27, 8); rect(px, 10, 13, 21, 14, 8); rect(px, 12, 19, 19, 20, 8)


def s_lovers(px):
    heart(px, 12, 20, 4, 10); heart(px, 20, 17, 4, 11)


def s_chariot(px):
    rect(px, 8, 18, 23, 22, 6); disc(px, 11, 25, 3, 7); disc(px, 20, 25, 3, 7); rect(px, 12, 12, 19, 17, 11)


def s_justice(px):
    rect(px, 15, 9, 16, 26, 8); line(px, 8, 13, 23, 13, 8); line(px, 8, 13, 8, 18, 8)
    line(px, 23, 13, 23, 18, 8); rect(px, 5 + 2, 18, 11, 19, 6); rect(px, 20, 18, 25, 19, 6)


def s_hermit(px):
    rect(px, 15, 14, 16, 27, 7); disc(px, 16, 11, 3, 14); line(px, 22, 12, 22, 27, 9)
    disc(px, 22, 11, 2, 8)


def s_wheel(px):
    ring(px, 16, 18, 9, 8, 2)
    for k in range(4):
        a = k * math.pi / 4
        line(px, 16 + 9 * math.cos(a), 18 + 9 * math.sin(a), 16 - 9 * math.cos(a), 18 - 9 * math.sin(a), 8)
    disc(px, 16, 18, 2, 10)


def s_strength(px):
    disc(px, 16, 17, 7, 8); disc(px, 13, 15, 1.2, 3); disc(px, 19, 15, 1.2, 3); rect(px, 13, 21, 18, 21, 9)
    star(px, 16, 8, 3, 10)


def s_hanged(px):
    rect(px, 8, 8, 23, 9, 9); rect(px, 15, 10, 16, 15, 6); disc(px, 16, 22, 3, 2)
    line(px, 16, 15, 16, 19, 6)


def s_death(px):
    disc(px, 16, 16, 7, 2); rect(px, 12, 21, 19, 25, 2)
    disc(px, 13, 16, 2, 3); disc(px, 19, 16, 2, 3); rect(px, 15, 19, 16, 20, 3)
    for x in (13, 15, 17, 19):
        put(px, x, 23, 3)


def s_temperance(px):
    rect(px, 8, 14, 13, 24, 11); rect(px, 19, 17, 24, 26, 10); line(px, 13, 14, 19, 17, 14)


def s_devil(px):
    disc(px, 16, 19, 7, 10); line(px, 10, 12, 8, 7, 10); line(px, 22, 12, 24, 7, 10)
    disc(px, 13, 18, 1.2, 8); disc(px, 19, 18, 1.2, 8); rect(px, 13, 23, 18, 23, 3)


def s_tower(px):
    rect(px, 12, 12, 19, 27, 7); rect(px, 11, 9, 20, 12, 7)
    line(px, 22, 6, 17, 16, 8); line(px, 17, 16, 21, 16, 8); line(px, 21, 16, 15, 26, 8)


def s_star(px):
    star(px, 16, 19, 9, 8)


def s_moon(px):
    moon(px, 16, 19, 9, 5)


def s_sun(px):
    sun(px, 16, 19, 5, 8)


def s_judgement(px):
    rect(px, 12, 12, 20, 14, 8); line(px, 20, 12, 25, 8, 8); line(px, 12, 13, 8, 24, 6)
    line(px, 20, 13, 24, 24, 6); line(px, 8, 24, 24, 24, 6)


def s_world(px):
    disc(px, 16, 19, 8, 11); disc(px, 13, 17, 3, 13); disc(px, 19, 22, 3, 13); ring(px, 16, 19, 10, 8, 1)


SYMBOLS = [s_fool, s_magician, s_priestess, s_empress, s_emperor, s_hierophant, s_lovers, s_chariot,
           s_justice, s_hermit, s_wheel, s_strength, s_hanged, s_death, s_temperance, s_devil,
           s_tower, s_star, s_moon, s_sun, s_judgement, s_world]


def number(px, n):
    text = str(n)
    x0 = 16 - (len(text) * 4 - 1) // 2
    for i, ch in enumerate(text):
        for y, row in enumerate(DIGITS[ch]):
            for x, bit in enumerate(row):
                if bit == '1':
                    px[x0 + i * 4 + x, 4 + y] = 5


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else 'graphics/tarot_gfx.png'
    sheet = Image.new('P', (32, 32 * len(SYMBOLS)), 0)
    flat = [c for rgb in PALETTE for c in rgb] + [0] * (768 - 3 * len(PALETTE))
    sheet.putpalette(flat)
    for i, symbol in enumerate(SYMBOLS):
        card = Image.new('P', (32, 32), 0)
        card.putpalette(flat)
        px = card.load()
        card_frame(px)
        # Purple inner line marks Tarots
        for y in range(2, 30):
            for x in range(CARD_LEFT + 2, CARD_RIGHT - 1):
                if y in (2, 29) or x in (CARD_LEFT + 2, CARD_RIGHT - 2):
                    px[x, y] = FRAME_IN
        number(px, i)
        symbol(px)
        sheet.paste(card, (0, 32 * i))
    sheet.save(out, transparency=0)
    print(out)


if __name__ == '__main__':
    main()
