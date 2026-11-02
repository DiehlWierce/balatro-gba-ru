/**
 * @file hand_levels.h
 *
 * @brief Chips and mult of a poker hand depending on its level (Planet cards level hands up).
 *
 * Pure functions without GBA dependencies, covered by the host tests in tests/hand_levels.
 */
#ifndef HAND_LEVELS_H
#define HAND_LEVELS_H

#include "hand_type.h"

#include <stdint.h>

#define HAND_LEVEL_MIN 1

/**
 * @brief Base chips and mult of a hand
 */
typedef struct HandLevelValues
{
    uint32_t chips;
    uint32_t mult;
} HandLevelValues;

/**
 * @brief Chips and mult of @p hand_type at @p level: `base + (level - 1) * increment`.
 *
 * @param hand_type the poker hand, NONE gives 0 chips and 0 mult
 * @param level the hand level, values below HAND_LEVEL_MIN are treated as HAND_LEVEL_MIN
 *
 * @return the chips and mult the hand starts scoring with
 */
HandLevelValues hand_level_values(enum HandType hand_type, int level);

/**
 * @brief Chips and mult gained by @p hand_type for each level above 1.
 *
 * @param hand_type the poker hand
 *
 * @return the increments, 0/0 for NONE or an invalid hand type
 */
HandLevelValues hand_level_increment(enum HandType hand_type);

#endif // HAND_LEVELS_H
