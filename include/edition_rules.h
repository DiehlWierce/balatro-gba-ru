/**
 * @file edition_rules.h
 *
 * @brief Editions of Jokers and playing cards (Foil, Holographic, Polychrome, Negative): the list,
 *        their bonuses, prices, the chance to appear and the Wheel of Fortune rolls.
 *
 * Plain C without libtonc so it can be tested on the host (tests/edition_rules).
 */
#ifndef EDITION_RULES_H
#define EDITION_RULES_H

#include <stddef.h>
#include "probability.h"

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Editions, a Joker or a card has at most one (none by default)
 */
enum Edition
{
    EDITION_NONE,
    EDITION_FOIL,        // +50 chips
    EDITION_HOLOGRAPHIC, // +10 mult
    EDITION_POLYCHROME,  // X1.5 mult
    EDITION_NEGATIVE,    // Joker: +1 Joker slot. Card: nothing in this build
    EDITION_MAX
};

#define EDITION_FOIL_CHIPS           50
#define EDITION_HOLOGRAPHIC_MULT     10
#define EDITION_POLYCHROME_XMULT_NUM 3 // X1.5 = 3/2
#define EDITION_POLYCHROME_XMULT_DEN 2

// Chance of each edition, in tenths of a percent (per mille), for a rate of 1 (no Voucher).
// The ranges are stacked: Negative first, then Polychrome, Holographic and Foil.
#define EDITION_CHANCE_NEGATIVE    3
#define EDITION_CHANCE_POLYCHROME  3
#define EDITION_CHANCE_HOLOGRAPHIC 14
#define EDITION_CHANCE_FOIL        20
#define EDITION_ROLL_RANGE         1000

// Wheel of Fortune: 1 in 4 chance, then Foil 50%, Holographic 35%, Polychrome 15%
#define EDITION_WHEEL_ODDS 4

/**
 * @brief Whether the edition is a valid one (none included).
 */
static inline bool edition_is_valid(int edition)
{
    return edition >= EDITION_NONE && edition < EDITION_MAX;
}

/**
 * @brief Russian name of the edition, NULL for no edition (or an invalid one).
 */
static inline const char* edition_get_name(int edition)
{
    switch (edition)
    {
        case EDITION_FOIL:
            return "Фольга";
        case EDITION_HOLOGRAPHIC:
            return "Голограмма";
        case EDITION_POLYCHROME:
            return "Полихром";
        case EDITION_NEGATIVE:
            return "Негатив";
        default:
            return NULL;
    }
}

/**
 * @brief Money added to the price of a Joker (and so to its sell value) by its edition.
 */
static inline int edition_extra_price(int edition)
{
    switch (edition)
    {
        case EDITION_FOIL:
            return 2;
        case EDITION_HOLOGRAPHIC:
            return 3;
        case EDITION_POLYCHROME:
        case EDITION_NEGATIVE:
            return 5;
        default:
            return 0;
    }
}

/**
 * @brief Whether the edition changes the score (Foil, Holographic, Polychrome).
 */
static inline bool edition_has_score_effect(int edition)
{
    return edition == EDITION_FOIL || edition == EDITION_HOLOGRAPHIC ||
           edition == EDITION_POLYCHROME;
}

/**
 * @brief Multiplies @p mult by 1.5, rounded to the nearest and saturated at UINT32_MAX.
 */
static inline uint32_t edition_polychrome_apply(uint32_t mult)
{
    uint64_t result =
        ((uint64_t)mult * EDITION_POLYCHROME_XMULT_NUM + EDITION_POLYCHROME_XMULT_DEN / 2) /
        EDITION_POLYCHROME_XMULT_DEN;
    return result > UINT32_MAX ? UINT32_MAX : (uint32_t)result;
}

/**
 * @brief Edition obtained with a roll, for the Shop, Packs and so on.
 *
 * @param roll random number, only roll % EDITION_ROLL_RANGE is used
 * @param rate multiplier of the chances: 1, 2 with Hone, 4 with Glow Up
 * @param allow_negative false to never get Negative (playing cards)
 */
static inline enum Edition edition_from_roll(uint32_t roll, int rate, bool allow_negative)
{
    if (rate < 1)
        rate = 1;

    uint32_t value = roll % EDITION_ROLL_RANGE;
    uint32_t limit = 0;

    limit += EDITION_CHANCE_NEGATIVE * rate;
    if (value < limit)
        return allow_negative ? EDITION_NEGATIVE : EDITION_NONE;
    limit += EDITION_CHANCE_POLYCHROME * rate;
    if (value < limit)
        return EDITION_POLYCHROME;
    limit += EDITION_CHANCE_HOLOGRAPHIC * rate;
    if (value < limit)
        return EDITION_HOLOGRAPHIC;
    limit += EDITION_CHANCE_FOIL * rate;
    if (value < limit)
        return EDITION_FOIL;
    return EDITION_NONE;
}

/**
 * @brief Whether the Wheel of Fortune works this time (1 in EDITION_WHEEL_ODDS).
 */
static inline bool edition_wheel_succeeds(uint32_t roll)
{
    return probability_one_in(roll, EDITION_WHEEL_ODDS); // Oops! All 6s
}

/**
 * @brief Edition given by the Wheel of Fortune: Foil 50%, Holographic 35%, Polychrome 15%.
 */
static inline enum Edition edition_wheel_edition(uint32_t roll)
{
    uint32_t value = roll % 100;
    if (value < 50)
        return EDITION_FOIL;
    if (value < 85)
        return EDITION_HOLOGRAPHIC;
    return EDITION_POLYCHROME;
}

/**
 * @brief Index of the Joker the Wheel of Fortune picks among those without an edition.
 *
 * @param editions edition of each owned Joker
 * @param count number of Jokers
 * @param roll random number
 *
 * @return index in editions[], -1 if every Joker already has an edition
 */
static inline int edition_wheel_pick_joker(const uint8_t* editions, int count, uint32_t roll)
{
    int eligible = 0;
    for (int i = 0; i < count; i++)
    {
        if (editions[i] == EDITION_NONE)
            eligible++;
    }
    if (eligible == 0)
        return -1;

    int target = (int)(roll % (uint32_t)eligible);
    for (int i = 0; i < count; i++)
    {
        if (editions[i] != EDITION_NONE)
            continue;
        if (target-- == 0)
            return i;
    }
    return -1;
}

/**
 * @brief Whether a Joker with edition @p new_edition fits in the Joker row.
 *
 * Negative Jokers bring a slot of their own: they don't count against @p slots, but the row can
 * never go over @p capacity.
 *
 * @param editions edition of each owned Joker
 * @param count number of owned Jokers
 * @param slots Joker slots of the run
 * @param capacity maximum number of Jokers the row can show
 */
static inline bool edition_joker_fits(
    const uint8_t* editions,
    int count,
    int slots,
    int capacity,
    int new_edition
)
{
    if (count >= capacity)
        return false;
    if (new_edition == EDITION_NEGATIVE)
        return true;

    int used = 0;
    for (int i = 0; i < count; i++)
        used += editions[i] != EDITION_NEGATIVE;
    return used < slots;
}

#endif // EDITION_RULES_H
