/**
 * @file edition_gfx.c
 *
 * @brief Edition effect on sprite tiles, see edition_gfx.h.
 */
#include "edition_gfx.h"

#include "graphic_utils.h"

#include <tonc.h>

#define SPRITE_PX   32
#define PAL_COLORS  PAL_ROW_LEN
#define FOIL_PERIOD 10
#define POLY_PERIOD 9

typedef struct
{
    u8 r, g, b;
} Rgb;

static inline int tiles_px_get(const u32* tiles, int x, int y)
{
    int tile = (y / 8) * 4 + (x / 8);
    u32 word = tiles[tile * TILE_SIZE + (y % 8)];
    return (word >> ((x % 8) * 4)) & 0xF;
}

static inline void tiles_px_set(u32* tiles, int x, int y, int color)
{
    int tile = (y / 8) * 4 + (x / 8);
    u32* word = &tiles[tile * TILE_SIZE + (y % 8)];
    int shift = (x % 8) * 4;
    *word = (*word & ~(0xFu << shift)) | ((u32)color << shift);
}

static inline int color_luma(COLOR color)
{
    // BGR555, weights 2:5:1 are good enough to order colours by lightness
    return 2 * (color & 31) + 5 * ((color >> 5) & 31) + ((color >> 10) & 31);
}

static int palette_closest(int pb, Rgb c)
{
    int best = 1;
    int best_dist = 0x7FFFFFFF;
    for (int i = 1; i < PAL_COLORS; i++)
    {
        COLOR pal = pal_obj_mem[pb * PAL_ROW_LEN + i];
        int dr = ((pal & 0x1F) << 3) - c.r;
        int dg = (((pal >> 5) & 0x1F) << 3) - c.g;
        int db = (((pal >> 10) & 0x1F) << 3) - c.b;
        int dist = dr * dr + dg * dg + db * db;
        if (dist < best_dist)
        {
            best_dist = dist;
            best = i;
        }
    }
    return best;
}

/**
 * @brief Index of the lightest (or darkest) colour of the bank, ignoring the transparent index 0.
 */
static int palette_extreme(int pb, bool lightest)
{
    int best = 1;
    int best_luma = color_luma(pal_obj_mem[pb * PAL_ROW_LEN + 1]);
    for (int i = 2; i < PAL_COLORS; i++)
    {
        int luma = color_luma(pal_obj_mem[pb * PAL_ROW_LEN + i]);
        if (lightest ? luma > best_luma : luma < best_luma)
        {
            best = i;
            best_luma = luma;
        }
    }
    return best;
}

/**
 * @brief Colour to draw over a pixel: @p wanted, unless it is the pixel's own colour (the streak
 *        would be invisible), then the lightest colour on a dark pixel or the darkest otherwise.
 */
static int streak_color(int pb, int wanted, int pixel)
{
    if (wanted != pixel)
        return wanted;
    bool dark_pixel = color_luma(pal_obj_mem[pb * PAL_ROW_LEN + pixel]) < 3 * 31 * 8 / 2;
    return palette_extreme(pb, dark_pixel);
}

static void draw_negative(u32* tiles, int pb)
{
    // Indices 1..15 ordered by lightness, then each one is replaced by its mirror in that order
    u8 order[PAL_COLORS - 1];
    for (int i = 0; i < PAL_COLORS - 1; i++)
        order[i] = i + 1;
    for (int i = 1; i < PAL_COLORS - 1; i++)
    {
        u8 idx = order[i];
        int luma = color_luma(pal_obj_mem[pb * PAL_ROW_LEN + idx]);
        int j = i - 1;
        while (j >= 0 && color_luma(pal_obj_mem[pb * PAL_ROW_LEN + order[j]]) > luma)
        {
            order[j + 1] = order[j];
            j--;
        }
        order[j + 1] = idx;
    }

    u8 lut[PAL_COLORS];
    lut[0] = 0;
    for (int k = 0; k < PAL_COLORS - 1; k++)
        lut[order[k]] = order[PAL_COLORS - 2 - k];

    for (int y = 0; y < SPRITE_PX; y++)
    {
        for (int x = 0; x < SPRITE_PX; x++)
        {
            int px = tiles_px_get(tiles, x, y);
            if (px != 0)
                tiles_px_set(tiles, x, y, lut[px]);
        }
    }
}

void edition_draw_tiles(u32* tiles, int pb, enum Edition edition)
{
    if (edition == EDITION_NONE || edition >= EDITION_MAX)
        return;

    if (edition == EDITION_NEGATIVE)
    {
        draw_negative(tiles, pb);
        return;
    }

    int blue = palette_closest(pb, (Rgb){41, 173, 255});
    int pink = palette_closest(pb, (Rgb){240, 52, 160});
    int red = palette_closest(pb, (Rgb){254, 60, 50});
    int green = palette_closest(pb, (Rgb){40, 200, 90});

    for (int y = 0; y < SPRITE_PX; y++)
    {
        for (int x = 0; x < SPRITE_PX; x++)
        {
            int px = tiles_px_get(tiles, x, y);
            if (px == 0)
                continue;

            int wanted = 0;
            switch (edition)
            {
                case EDITION_FOIL:
                    if ((x + y) % FOIL_PERIOD < 2)
                        wanted = blue;
                    break;
                case EDITION_HOLOGRAPHIC:
                    if ((x % 4 == 0 && y % 4 == 0) || (x % 4 == 2 && y % 4 == 2))
                        wanted = pink;
                    break;
                case EDITION_POLYCHROME:
                {
                    int d = (x + y) % POLY_PERIOD;
                    if (d == 0)
                        wanted = red;
                    else if (d == 3)
                        wanted = green;
                    else if (d == 6)
                        wanted = blue;
                    break;
                }
                default:
                    break;
            }

            if (wanted != 0)
                tiles_px_set(tiles, x, y, streak_color(pb, wanted, px));
        }
    }
}
