#!/usr/bin/env python3
"""Extends graphics/decks_face_down_gfx.png (card backs, 32x32 each) from 6 to 14 decks.

The first 6 backs (Red..Painted) are kept as they are. The new ones reuse the pattern of the Red
back (palette index 4 pixels) with other colors, using the free palette entries 10-15 of the
shared deck palette (bank DECK_PB, see docs/ru/PALETTES.md). Run from the repository root.
"""
from PIL import Image

PATH = "graphics/decks_face_down_gfx.png"
OLD = 6
NEW_COLORS = {  # palette index -> RGB (indices 10-15 were unused)
    10: (150, 80, 220),   # Magic: purple
    11: (40, 60, 160),    # Nebula: deep blue
    12: (40, 190, 190),   # Zodiac: teal
    13: (130, 90, 60),    # Abandoned: brown
    14: (255, 120, 190),  # Plasma: pink
    15: (20, 24, 40),     # Checkered / dark
}
PATTERN = 4  # Red back pattern color


def main():
    im = Image.open(PATH)
    pal = im.getpalette()
    for idx, rgb in NEW_COLORS.items():
        pal[idx * 3: idx * 3 + 3] = list(rgb)

    template = im.crop((0, 0, 32, 32))
    mask = [[template.getpixel((x, y)) == PATTERN for x in range(32)] for y in range(32)]

    def make(color_fn):
        tile = template.copy()
        for y in range(32):
            for x in range(32):
                if mask[y][x]:
                    tile.putpixel((x, y), color_fn(x, y))
        return tile

    # Erratic: pseudo-random mosaic of 2x2 blocks (fixed, so the ROM stays reproducible)
    scatter = [4, 5, 7, 9, 8, 6]
    state = [12345]

    def noise(x, y):
        h = ((x // 2) * 73856093) ^ ((y // 2) * 19349663) ^ state[0]
        return scatter[(h >> 4) % len(scatter)]

    new_tiles = [
        make(lambda x, y: 10),                                   # Magic
        make(lambda x, y: 11),                                   # Nebula
        make(lambda x, y: 2),                                    # Ghost: pale gray on white
        make(lambda x, y: 13),                                   # Abandoned
        make(lambda x, y: 4 if ((x // 4) + (y // 4)) % 2 == 0 else 15),  # Checkered
        make(lambda x, y: 12),                                   # Zodiac
        make(lambda x, y: 14 if (x + y) // 4 % 2 == 0 else 7),   # Plasma: pink and blue stripes
        make(noise),                                             # Erratic
    ]

    out = Image.new("P", (32 * (OLD + len(new_tiles)), 32), 0)
    out.putpalette(pal)
    out.paste(im.crop((0, 0, 32 * OLD, 32)), (0, 0))
    for i, tile in enumerate(new_tiles):
        out.paste(tile, (32 * (OLD + i), 0))
    out.save(PATH)


if __name__ == "__main__":
    main()
