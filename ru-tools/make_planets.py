#!/usr/bin/env python3
"""Generates graphics/planet_gfx.png: 12 Planet cards (32x32 each, stacked vertically).

All Planets share one 16-color palette (one OBJ palette bank, see docs/ru/PALETTES.md).
The card frame matches the Joker cards (24x32, light outline, rounded corners).

usage: python3 ru-tools/make_planets.py [out.png]
"""
import math
import random
import sys

from PIL import Image

PALETTE = [
    (255, 0, 255),    # 0  transparent
    (192, 200, 214),  # 1  card outline (same as Jokers)
    (255, 255, 255),  # 2  card inner border
    (18, 24, 54),     # 3  space
    (38, 50, 98),     # 4  space, lighter
    (235, 235, 255),  # 5  star
    (205, 208, 220),  # 6  light gray
    (128, 132, 150),  # 7  gray
    (240, 204, 128),  # 8  sand
    (196, 132, 64),   # 9  brown/orange
    (222, 84, 64),    # 10 red
    (84, 160, 232),   # 11 blue
    (40, 88, 180),    # 12 deep blue
    (92, 188, 116),   # 13 green
    (150, 230, 240),  # 14 cyan
    (172, 112, 222),  # 15 purple
]

CARD_LEFT, CARD_RIGHT = 4, 27  # inclusive, like the Joker cards
CX, CY = 16, 15

# Order matches enum PlanetType in include/consumable.h
PLANETS = [
    # name,       radius, colors (light, main, shade), style
    ('mercury', 6, (6, 7, 7), 'craters'),
    ('venus', 7, (8, 8, 9), 'plain'),
    ('earth', 7, (11, 11, 12), 'earth'),
    ('mars', 6, (8, 10, 9), 'plain'),
    ('jupiter', 8, (8, 9, 6), 'bands'),
    ('saturn', 6, (8, 8, 9), 'ring'),
    ('uranus', 6, (14, 14, 11), 'ring_v'),
    ('neptune', 7, (14, 11, 12), 'plain'),
    ('pluto', 4, (6, 7, 9), 'craters'),
    ('planet_x', 7, (15, 15, 12), 'bands'),
    ('ceres', 4, (6, 7, 7), 'craters'),
    ('eris', 4, (5, 6, 14), 'plain'),
]


def card_frame(px):
    for y in range(32):
        for x in range(CARD_LEFT, CARD_RIGHT + 1):
            corner = (y in (0, 31)) and x in (CARD_LEFT, CARD_RIGHT)
            if corner:
                continue
            edge = y in (0, 31) or x in (CARD_LEFT, CARD_RIGHT)
            inner = y in (1, 30) or x in (CARD_LEFT + 1, CARD_RIGHT - 1)
            px[x, y] = 1 if edge else (2 if inner else 3)


def stars(px, seed):
    rnd = random.Random(seed)
    for _ in range(9):
        x = rnd.randint(CARD_LEFT + 3, CARD_RIGHT - 3)
        y = rnd.randint(3, 28)
        px[x, y] = 5 if rnd.random() < 0.5 else 4


def planet(px, radius, colors, style, seed):
    light, main, shade = colors
    rnd = random.Random(seed)
    for y in range(32):
        for x in range(CARD_LEFT + 2, CARD_RIGHT - 1):
            dx, dy = x - CX + 0.5, y - CY + 0.5
            d = math.hypot(dx, dy)
            if d > radius:
                continue
            # Light comes from the top left
            lit = (-dx - dy) / (radius * 1.4)
            c = light if lit > 0.35 else (shade if lit < -0.35 else main)
            if style == 'bands' and int((y - CY + radius) / 2.5) % 2 == 1 and c != shade:
                c = shade if c == main else main
            if style == 'earth' and c != shade and math.sin(x * 1.3) + math.cos(y * 1.1) > 0.9:
                c = 13
            px[x, y] = c
    if style == 'craters':
        for _ in range(3):
            a = rnd.uniform(0, 2 * math.pi)
            r = rnd.uniform(0, radius - 2)
            px[int(CX + r * math.cos(a)), int(CY + r * math.sin(a))] = shade
    if style in ('ring', 'ring_v'):
        for t in range(0, 360, 4):
            a = math.radians(t)
            if style == 'ring':
                x, y = CX + (radius + 4) * math.cos(a), CY + 2.2 * math.sin(a)
            else:
                x, y = CX + 2.2 * math.cos(a), CY + (radius + 4) * math.sin(a)
            # The back half of the ring is hidden by the planet
            behind = (math.sin(a) < 0) if style == 'ring' else (math.cos(a) < 0)
            xi, yi = int(round(x - 0.5)), int(round(y - 0.5))
            inside = math.hypot(xi - CX + 0.5, yi - CY + 0.5) <= radius
            if CARD_LEFT + 2 <= xi <= CARD_RIGHT - 2 and not (behind and inside):
                px[xi, yi] = 6


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else 'graphics/planet_gfx.png'
    sheet = Image.new('P', (32, 32 * len(PLANETS)), 0)
    flat = [c for rgb in PALETTE for c in rgb] + [0] * (768 - 3 * len(PALETTE))
    sheet.putpalette(flat)
    for i, (name, radius, colors, style) in enumerate(PLANETS):
        card = Image.new('P', (32, 32), 0)
        card.putpalette(flat)
        px = card.load()
        card_frame(px)
        stars(px, i)
        planet(px, radius, colors, style, i)
        sheet.paste(card, (0, 32 * i))
    sheet.save(out, transparency=0)
    print(out)


if __name__ == '__main__':
    main()
