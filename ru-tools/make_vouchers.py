#!/usr/bin/env python3
"""Generates graphics/voucher_gfx.png: 32 Vouchers (32x32 each, stacked vertically).

Vouchers live in the same OBJ palette bank as Planets (CONSUMABLE_PB, see docs/ru/PALETTES.md),
so they are drawn with exactly the Planet palette from make_planets.py. A Voucher is a ticket
with notched sides: base Vouchers are sand colored, upgraded ones are cyan. Both versions of a
pair share their icon.

usage: python3 ru-tools/make_vouchers.py [out.png]
"""
import os
import sys

from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from make_planets import PALETTE  # noqa: E402  same 16 colors as the Planets

# Palette indices (see make_planets.PALETTE)
T, OUTLINE, WHITE, DARK, NAVY, STAR = 0, 1, 2, 3, 4, 5
LGRAY, GRAY, SAND, BROWN, RED, BLUE, DBLUE, GREEN, CYAN, PURPLE = range(6, 16)

TIERS = [
    # body, border
    (SAND, BROWN),
    (CYAN, DBLUE),
]

CHARS = {
    '.': None,
    '#': DARK,
    'r': RED,
    'b': BLUE,
    'B': DBLUE,
    'g': GREEN,
    'p': PURPLE,
    'w': STAR,
    'l': LGRAY,
    'k': GRAY,
    'o': BROWN,
    'y': SAND,
    'n': NAVY,
}

# One 11x11 icon per Voucher pair, in the order of enum VoucherType (include/voucher.h)
ICONS = [
    # Overstock: crate with a plus
    ['...........',
     '.#########.',
     '.#.......#.',
     '.#...g...#.',
     '.#...g...#.',
     '.#.ggggg.#.',
     '.#...g...#.',
     '.#...g...#.',
     '.#.......#.',
     '.#########.',
     '...........'],
    # Clearance Sale: percent sign
    ['...........',
     '.rr.....r..',
     '.rr....r...',
     '......r....',
     '.....r.....',
     '....r......',
     '...r.......',
     '..r....rr..',
     '.r.....rr..',
     '...........',
     '...........'],
    # Hone: gem
    ['...........',
     '...bbbbb...',
     '..bwbbbbb..',
     '.bwbbbbbbB.',
     '.BBBBBBBBB.',
     '..BbbbbbB..',
     '...BbbbB...',
     '....BbB....',
     '.....B.....',
     '...........',
     '...........'],
    # Reroll Surplus: circular arrows
    ['...........',
     '...ggggg...',
     '..g.....g..',
     '.g.......gg',
     '.g......ggg',
     '.g.........',
     '.........g.',
     'ggg......g.',
     'gg.......g.',
     '..g.....g..',
     '...ggggg...'],
    # Crystal Ball: ball on a stand
    ['...........',
     '...ppppp...',
     '..pwpppppp.',
     '.ppwpppppp.',
     '.ppppppppp.',
     '.ppppppppp.',
     '..ppppppp..',
     '...ppppp...',
     '..#######..',
     '.#########.',
     '...........'],
    # Telescope
    ['...........',
     '........###',
     '......##ll#',
     '....##llll#',
     '..##llll###',
     '.#llll##...',
     '.###.#.....',
     '....#.#....',
     '...#...#...',
     '..#.....#..',
     '...........'],
    # Grabber: open hand
    ['...........',
     '..#.#.#....',
     '..#.#.#.#..',
     '..#l#l#l#..',
     '..#l#l#l#..',
     '#.#lllll#..',
     '#l#lllll#..',
     '.#lllllll#.',
     '..#lllll#..',
     '...#####...',
     '...........'],
    # Wasteful: trash can
    ['....###....',
     '.#########.',
     '...........',
     '..#######..',
     '..#k#k#k#..',
     '..#k#k#k#..',
     '..#k#k#k#..',
     '..#k#k#k#..',
     '..#k#k#k#..',
     '...#####...',
     '...........'],
    # Tarot Merchant: card with a star
    ['..#######..',
     '..#ppppp#..',
     '..#pp.pp#..',
     '..#p...p#..',
     '..#.....#..',
     '..#p...p#..',
     '..#p.p.p#..',
     '..#ppppp#..',
     '..#######..',
     '...........',
     '...........'],
    # Planet Merchant: planet with a ring
    ['...........',
     '....bbb....',
     '...bwbbb...',
     '..bbbbbbb..',
     'll.bbbbb.ll',
     '..llllllll.',
     '..BbbbbbB..',
     '...BBbBB...',
     '....BBB....',
     '...........',
     '...........'],
    # Seed Money: coin with a dollar sign
    ['...........',
     '...#####...',
     '..#..r..#..',
     '.#..rrr..#.',
     '.#.r.r...#.',
     '.#..rrr..#.',
     '.#...r.r.#.',
     '.#..rrr..#.',
     '..#..r..#..',
     '...#####...',
     '...........'],
    # Blank: dashed empty frame
    ['...........',
     '.#.#.#.#.#.',
     '...........',
     '.#.......#.',
     '...........',
     '.#.......#.',
     '...........',
     '.#.......#.',
     '...........',
     '.#.#.#.#.#.',
     '...........'],
    # Magic Trick: top hat
    ['...........',
     '...#####...',
     '...#####...',
     '...#####...',
     '...#####...',
     '...rrrrr...',
     '...#####...',
     '.#########.',
     '...........',
     '...........',
     '...........'],
    # Hieroglyph: eye
    ['...........',
     '...........',
     '...#####...',
     '..#.....#..',
     '.#..bbb..#.',
     '#...b#b...#',
     '.#..bbb..#.',
     '..#.....#..',
     '...#####...',
     '.....#.....',
     '....#.#....'],
    # Director's Cut: clapperboard
    ['...........',
     '.#w#w#w#w#.',
     '.#########.',
     '...........',
     '.#########.',
     '.#.......#.',
     '.#.rrr...#.',
     '.#.......#.',
     '.#.......#.',
     '.#########.',
     '...........'],
    # Paint Brush
    ['...........',
     '........oo.',
     '.......ooo.',
     '......ooo..',
     '.....ooo...',
     '....kkk....',
     '...rrkk....',
     '..rrrr.....',
     '.rrrr......',
     '.rr........',
     '...........'],
]

TICKET_LEFT, TICKET_RIGHT, TICKET_TOP, TICKET_BOTTOM = 2, 29, 5, 26
NOTCH_Y = (TICKET_TOP + TICKET_BOTTOM) // 2
NOTCH_R = 3


def ticket(px, body, border):
    for y in range(TICKET_TOP, TICKET_BOTTOM + 1):
        for x in range(TICKET_LEFT, TICKET_RIGHT + 1):
            # Notches on both sides
            if min(abs(x - TICKET_LEFT), abs(x - TICKET_RIGHT)) < NOTCH_R - abs(y - NOTCH_Y) // 2 \
                    and abs(y - NOTCH_Y) < NOTCH_R:
                continue
            # Rounded corners
            if (y in (TICKET_TOP, TICKET_BOTTOM)) and x in (TICKET_LEFT, TICKET_RIGHT):
                continue
            edge = (y in (TICKET_TOP, TICKET_BOTTOM) or x in (TICKET_LEFT, TICKET_RIGHT))
            px[x, y] = border if edge else body
    # Outline the notches
    for y in range(TICKET_TOP, TICKET_BOTTOM + 1):
        for x in range(TICKET_LEFT, TICKET_RIGHT + 1):
            if px[x, y] == T:
                continue
            for nx, ny in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)):
                if TICKET_LEFT <= nx <= TICKET_RIGHT and TICKET_TOP <= ny <= TICKET_BOTTOM \
                        and px[nx, ny] == T:
                    px[x, y] = border
    # Perforations along the top and bottom
    for x in range(TICKET_LEFT + 3, TICKET_RIGHT - 2, 2):
        px[x, TICKET_TOP + 2] = border
        px[x, TICKET_BOTTOM - 2] = border


def icon(px, rows, upgraded):
    x0 = (TICKET_LEFT + TICKET_RIGHT + 1) // 2 - 5
    y0 = NOTCH_Y - 5
    for dy, line in enumerate(rows):
        for dx, ch in enumerate(line):
            c = CHARS[ch]
            if c is not None:
                px[x0 + dx, y0 + dy] = c
    if upgraded:
        # Small plus in the top right corner
        for dx, dy in ((0, -1), (-1, 0), (0, 0), (1, 0), (0, 1)):
            px[TICKET_RIGHT - 4 + dx, TICKET_TOP + 4 + dy] = RED


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else 'graphics/voucher_gfx.png'
    flat = [c for rgb in PALETTE for c in rgb] + [0] * (768 - 3 * len(PALETTE))
    sheet = Image.new('P', (32, 32 * 2 * len(ICONS)), 0)
    sheet.putpalette(flat)
    for i, rows in enumerate(ICONS):
        assert len(rows) == 11 and all(len(r) == 11 for r in rows), i
        for tier, (body, border) in enumerate(TIERS):
            card = Image.new('P', (32, 32), 0)
            card.putpalette(flat)
            px = card.load()
            ticket(px, body, border)
            icon(px, rows, tier == 1)
            sheet.paste(card, (0, 32 * (2 * i + tier)))
    sheet.save(out, transparency=0)
    print(out)


if __name__ == '__main__':
    main()
