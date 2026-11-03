/**
 * @file edition_gfx.h
 *
 * @brief Drawing of the edition effect into the tiles of a 32x32 card or Joker sprite.
 *
 * The effect is drawn into the sprite's own copy of the tiles with the colours of the palette
 * bank it already uses: no extra palette bank, no extra sprite (see docs/ru/PALETTES.md).
 */
#ifndef EDITION_GFX_H
#define EDITION_GFX_H

#include "edition_rules.h"

#include <tonc.h>

/**
 * @brief Draws the effect of @p edition on the tiles of a 32x32 4bpp sprite, in place.
 *
 * Foil: blue diagonal streaks. Holographic: a lattice of pink dots. Polychrome: red, green and
 * blue streaks. Negative: the lightness of the palette is inverted.
 *
 * @param tiles 16 tiles (128 words) in the 1D layout of the sprites
 * @param pb palette bank (OBJ) the sprite uses
 * @param edition the edition, nothing is drawn for EDITION_NONE
 */
void edition_draw_tiles(u32* tiles, int pb, enum Edition edition);

#endif // EDITION_GFX_H
