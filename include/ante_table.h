/**
 * @file ante_table.h
 *
 * @brief Base score requirement per ante, for the three Balatro scalings (White, Green, Purple
 *        stakes), up to the last ante the game can reach (Endless mode). Plain C without libtonc
 *        so it can be tested on the host (tests/ante_table).
 *
 * Antes 0-8 are the original tables. Antes 9-12 are what the original game computes with
 * `a * (1.6 + (0.75 * c) ^ (1 + 0.2 * c)) ^ c` (a = ante 8 value, c = ante - 8) rounded down to
 * two significant digits. Ante 13 needs 4.7 * 10^10 and does not fit in a u32, so ante 12 is the
 * last one: beating its Boss Blind is the final win.
 */
#ifndef ANTE_TABLE_H
#define ANTE_TABLE_H

#include <stdint.h>

/// Ante at which the normal run is won; Endless mode starts after it
#define ANTE_WIN 8
/// Last ante that exists. Its base requirement still fits in a u32 on every stake
#define ANTE_LAST 12

#define ANTE_SCALING_COUNT 3

// clang-format off
static const uint32_t ANTE_TABLE_BASE[ANTE_SCALING_COUNT][ANTE_LAST + 1] = {
    // White (and Red, Blue)
    {100, 300, 800,  2000, 5000, 11000, 20000, 35000,  50000,  110000, 560000,  7200000,  300000000},
    // Green and Black, Orange
    {100, 300, 900,  2600, 8000, 20000, 36000, 60000,  100000, 230000, 1100000, 14000000, 600000000},
    // Purple and Gold
    {100, 300, 1000, 3200, 9000, 25000, 60000, 110000, 200000, 460000, 2200000, 29000000, 1200000000}
};
// clang-format on

/**
 * @brief Base requirement of an ante.
 *
 * @param scaling 0 = White, 1 = Green, 2 = Purple scaling; out of range clamps
 * @param ante Out of range (below 0 or above ANTE_LAST) gives the ante 0 value
 */
static inline uint32_t ante_table_base(int scaling, int ante)
{
    if (scaling < 0)
        scaling = 0;
    if (scaling >= ANTE_SCALING_COUNT)
        scaling = ANTE_SCALING_COUNT - 1;
    if (ante < 0 || ante > ANTE_LAST)
        ante = 0;
    return ANTE_TABLE_BASE[scaling][ante];
}

/**
 * @brief Base requirement times a Blind multiplier, in 64 bits and saturated to UINT32_MAX.
 *
 * @param base Base requirement of the ante
 * @param mult_fx8 Blind multiplier as a 24.8 fixed point number (FIX_ONE = 256)
 */
static inline uint32_t ante_table_requirement(uint32_t base, uint32_t mult_fx8)
{
    uint64_t req = ((uint64_t)base * mult_fx8) >> 8;
    return req > UINT32_MAX ? UINT32_MAX : (uint32_t)req;
}

#endif // ANTE_TABLE_H
