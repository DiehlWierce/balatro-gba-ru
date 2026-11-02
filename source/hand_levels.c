/**
 * @file hand_levels.c
 *
 * @brief Chips and mult of poker hands by level, see hand_levels.h.
 */
#include "hand_levels.h"

typedef struct
{
    HandLevelValues base;
    HandLevelValues increment;
} HandLevelInfo;

// clang-format off
static const HandLevelInfo HAND_LEVEL_INFO[] = {
    [NONE]            = {.base = {0,   0 }, .increment = {0,  0}},
    [HIGH_CARD]       = {.base = {5,   1 }, .increment = {10, 1}},
    [PAIR]            = {.base = {10,  2 }, .increment = {15, 1}},
    [TWO_PAIR]        = {.base = {20,  2 }, .increment = {20, 1}},
    [THREE_OF_A_KIND] = {.base = {30,  3 }, .increment = {20, 2}},
    [STRAIGHT]        = {.base = {30,  4 }, .increment = {30, 3}},
    [FLUSH]           = {.base = {35,  4 }, .increment = {15, 2}},
    [FULL_HOUSE]      = {.base = {40,  4 }, .increment = {25, 2}},
    [FOUR_OF_A_KIND]  = {.base = {60,  7 }, .increment = {30, 3}},
    [STRAIGHT_FLUSH]  = {.base = {100, 8 }, .increment = {40, 4}},
    [ROYAL_FLUSH]     = {.base = {100, 8 }, .increment = {40, 4}},
    [FIVE_OF_A_KIND]  = {.base = {120, 12}, .increment = {35, 3}},
    [FLUSH_HOUSE]     = {.base = {140, 14}, .increment = {40, 4}},
    [FLUSH_FIVE]      = {.base = {160, 16}, .increment = {50, 3}},
};
// clang-format on

static inline int hand_type_is_valid(enum HandType hand_type)
{
    return (int)hand_type > NONE && hand_type <= HAND_TYPE_MAX;
}

HandLevelValues hand_level_increment(enum HandType hand_type)
{
    if (!hand_type_is_valid(hand_type))
        return HAND_LEVEL_INFO[NONE].increment;

    return HAND_LEVEL_INFO[hand_type].increment;
}

HandLevelValues hand_level_values(enum HandType hand_type, int level)
{
    if (!hand_type_is_valid(hand_type))
        return HAND_LEVEL_INFO[NONE].base;

    if (level < HAND_LEVEL_MIN)
        level = HAND_LEVEL_MIN;

    const HandLevelInfo* info = &HAND_LEVEL_INFO[hand_type];
    uint32_t extra_levels = (uint32_t)(level - HAND_LEVEL_MIN);

    HandLevelValues values = {
        .chips = info->base.chips + extra_levels * info->increment.chips,
        .mult = info->base.mult + extra_levels * info->increment.mult,
    };

    return values;
}
