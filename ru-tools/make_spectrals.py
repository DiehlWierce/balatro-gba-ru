#!/usr/bin/env python3
"""Generates graphics/spectral_gfx.png: 18 Spectral cards (32x32 each, stacked vertically).

Spectrals use the palette of the Planets (planet_gfx.png, bank CONSUMABLE_PB, see
docs/ru/PALETTES.md) like the Tarots. A blue inner line marks them. The order matches
enum SpectralType in include/spectral_rules.h.

usage: python3 ru-tools/make_spectrals.py [out.png]
"""
import math
import sys

from PIL import Image

sys.path.insert(0, __file__.rsplit('/', 1)[0])
from make_planets import PALETTE, CARD_LEFT, CARD_RIGHT, card_frame  # noqa: E402
from make_tarots import (put, disc, ring, line, rect, star, heart, spade, club, diamond)  # noqa: E402

FRAME_IN = 11  # blue line: Spectral


def card_small(px, cx, cy, c):
    rect(px, cx - 3, cy - 4, cx + 3, cy + 4, 2)
    rect(px, cx - 2, cy - 3, cx + 2, cy + 3, c)


def s_familiar(px):  # a ghost-like familiar with a star
    disc(px, 16, 14, 5, 2); rect(px, 11, 14, 21, 22, 2)
    for x in (11, 14, 17, 20):
        rect(px, x, 23, x + 1, 24, 2)
    disc(px, 14, 14, 1, 3); disc(px, 18, 14, 1, 3); star(px, 16, 7, 2, 8)


def s_grim(px):  # skull
    disc(px, 16, 15, 7, 2); rect(px, 12, 19, 20, 25, 2)
    disc(px, 13, 15, 2, 3); disc(px, 19, 15, 2, 3); rect(px, 15, 18, 16, 19, 3)
    for x in (13, 15, 17, 19):
        put(px, x, 23, 3)


def s_incantation(px):  # rune circle
    ring(px, 16, 17, 9, 12, 2); line(px, 16, 8, 10, 24, 11); line(px, 16, 8, 22, 24, 11)
    line(px, 9, 14, 23, 14, 11); disc(px, 16, 17, 2, 14)


def s_talisman(px):  # gold seal medallion
    disc(px, 16, 17, 8, 8); disc(px, 16, 17, 5, 9); star(px, 16, 17, 4, 8)


def s_aura(px):  # glow rings
    disc(px, 16, 17, 3, 5)
    ring(px, 16, 17, 6, 14, 1.5); ring(px, 16, 17, 9, 11, 1.5)


def s_wraith(px):  # hooded wraith
    disc(px, 16, 12, 5, 7); rect(px, 10, 13, 22, 25, 7)
    disc(px, 14, 12, 1.2, 3); disc(px, 18, 12, 1.2, 3)
    for x in (10, 13, 16, 19):
        rect(px, x, 26, x + 1, 27, 7)


def s_sigil(px):  # four suits
    heart(px, 11, 12, 3, 10); diamond(px, 21, 12, 3, 10)
    club(px, 11, 22, 3, 13); spade(px, 21, 21, 3, 13)


def s_ouija(px):  # planchette
    disc(px, 16, 18, 8, 9); disc(px, 16, 18, 3, 3)
    line(px, 16, 10, 16, 6, 8); line(px, 12, 24, 20, 24, 8)


def s_ectoplasm(px):  # slime blob
    disc(px, 16, 19, 7, 13); disc(px, 11, 21, 3, 13); disc(px, 21, 21, 3, 13)
    disc(px, 13, 17, 1.2, 3); disc(px, 19, 17, 1.2, 3); line(px, 13, 21, 19, 21, 3)


def s_immolate(px):  # flame
    disc(px, 16, 21, 6, 10); disc(px, 16, 14, 4, 10); line(px, 16, 7, 16, 14, 10)
    disc(px, 16, 21, 3, 8)


def s_ankh(px):
    ring(px, 16, 11, 4, 8, 2); rect(px, 15, 15, 16, 27, 8); rect(px, 10, 19, 21, 20, 8)


def s_deja_vu(px):  # red seal: two overlapping cards
    card_small(px, 13, 17, 3); card_small(px, 19, 15, 10)


def s_hex(px):  # pentagram
    ring(px, 16, 17, 9, 15, 1)
    star(px, 16, 18, 8, 15)


def s_trance(px):  # spiral
    for i in range(60):
        a = i * 0.35
        r = 1 + i * 0.14
        put(px, int(16 + r * math.cos(a)), int(17 + r * math.sin(a)), 11)
        put(px, int(16 + r * math.cos(a)) + 1, int(17 + r * math.sin(a)), 14)


def s_medium(px):  # crystal ball
    disc(px, 16, 16, 8, 15); disc(px, 13, 13, 2, 2); rect(px, 11, 24, 21, 26, 7)


def s_cryptid(px):  # a card and its copy
    card_small(px, 12, 17, 10); card_small(px, 20, 17, 10)
    put(px, 16, 17, 5)


def s_soul(px):  # soul flame in a diamond
    diamond(px, 16, 17, 9, 15); disc(px, 16, 18, 4, 2); disc(px, 16, 14, 2, 2)


def s_black_hole(px):
    disc(px, 16, 17, 9, 12); disc(px, 16, 17, 6, 4); disc(px, 16, 17, 3, 3)
    ring(px, 16, 17, 10, 15, 1)


SYMBOLS = [s_familiar, s_grim, s_incantation, s_talisman, s_aura, s_wraith, s_sigil, s_ouija,
           s_ectoplasm, s_immolate, s_ankh, s_deja_vu, s_hex, s_trance, s_medium, s_cryptid,
           s_soul, s_black_hole]


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else 'graphics/spectral_gfx.png'
    sheet = Image.new('P', (32, 32 * len(SYMBOLS)), 0)
    flat = [c for rgb in PALETTE for c in rgb] + [0] * (768 - 3 * len(PALETTE))
    sheet.putpalette(flat)
    for i, symbol in enumerate(SYMBOLS):
        card = Image.new('P', (32, 32), 0)
        card.putpalette(flat)
        px = card.load()
        card_frame(px)
        for y in range(2, 30):
            for x in range(CARD_LEFT + 2, CARD_RIGHT - 1):
                if y in (2, 29) or x in (CARD_LEFT + 2, CARD_RIGHT - 2):
                    px[x, y] = FRAME_IN
        symbol(px)
        sheet.paste(card, (0, 32 * i))
    sheet.save(out, transparency=0)
    print(out)


if __name__ == '__main__':
    main()
