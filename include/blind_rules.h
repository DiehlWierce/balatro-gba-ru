/**
 * @file blind_rules.h
 *
 * @brief Pure rules of the Blinds: the list of Blind types, the first ante where each Boss can
 *        show up and the score requirement multiplier. Plain C without libtonc so it can be
 *        tested on the host (tests/blind_rules).
 */
#ifndef BLIND_RULES_H
#define BLIND_RULES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// clang-format off
/**
 * @enum BlindType
 * @brief All Blind types in the game are listed here.
 *
 * Boss Blinds range from `BLIND_TYPE_BOSS` to `BLIND_TYPE_SHOWDOWN - 1`.
 *
 * Showdown Blinds range from `BLIND_TYPE_SHOWDOWN` to `BLIND_TYPE_MAX - 1`
 */
enum BlindType
{
    // Normal Blinds
    BLIND_TYPE_SMALL,
    BLIND_TYPE_BIG,

    // Boss Blinds
    BLIND_TYPE_BOSS,

    BLIND_TYPE_HOOK = BLIND_TYPE_BOSS,
    BLIND_TYPE_OX,
    BLIND_TYPE_HOUSE,
    BLIND_TYPE_WALL,
    BLIND_TYPE_WHEEL,
    BLIND_TYPE_ARM,
    BLIND_TYPE_CLUB,
    BLIND_TYPE_FISH,
    BLIND_TYPE_PSYCHIC,
    BLIND_TYPE_GOAD,
    BLIND_TYPE_WATER,
    BLIND_TYPE_WINDOW,
    BLIND_TYPE_MANACLE,
    BLIND_TYPE_EYE,
    BLIND_TYPE_MOUTH,
    BLIND_TYPE_PLANT,
    BLIND_TYPE_SERPENT,
    BLIND_TYPE_PILLAR,
    BLIND_TYPE_NEEDLE,
    BLIND_TYPE_HEAD,
    BLIND_TYPE_TOOTH,
    BLIND_TYPE_FLINT,
    BLIND_TYPE_MARK,

    // Showdown Blinds
    BLIND_TYPE_SHOWDOWN,

    BLIND_TYPE_ACORN = BLIND_TYPE_SHOWDOWN,
    BLIND_TYPE_LEAF,
    BLIND_TYPE_VESSEL,
    BLIND_TYPE_HEART,
    BLIND_TYPE_BELL,

    // End of Blinds' list
    BLIND_TYPE_MAX
};
// clang-format on

/// 24.8 fixed point one, same as FIX_ONE of libtonc
#define BLIND_RULES_FIX_ONE 256

/// Ante from which Showdown Blinds are rolled
#define BLIND_RULES_SHOWDOWN_MIN_ANTE 8

/**
 * @brief First ante where a Blind can be rolled (balatrowiki.org/w/Blinds_and_Antes). A switch
 *        instead of a table: this header is included everywhere and a table would be emitted
 *        in every translation unit that doesn't optimize it away.
 */
static inline int blind_rules_min_ante(enum BlindType type)
{
    switch (type)
    {
        case BLIND_TYPE_OX:
            return 6;
        case BLIND_TYPE_SERPENT:
            return 5;
        case BLIND_TYPE_PLANT:
            return 4;
        case BLIND_TYPE_EYE:
        case BLIND_TYPE_TOOTH:
            return 3;
        case BLIND_TYPE_HOUSE:
        case BLIND_TYPE_WALL:
        case BLIND_TYPE_WHEEL:
        case BLIND_TYPE_ARM:
        case BLIND_TYPE_FISH:
        case BLIND_TYPE_WATER:
        case BLIND_TYPE_MOUTH:
        case BLIND_TYPE_NEEDLE:
        case BLIND_TYPE_FLINT:
        case BLIND_TYPE_MARK:
            return 2;
        case BLIND_TYPE_ACORN:
        case BLIND_TYPE_LEAF:
        case BLIND_TYPE_VESSEL:
        case BLIND_TYPE_HEART:
        case BLIND_TYPE_BELL:
            return BLIND_RULES_SHOWDOWN_MIN_ANTE;
        default:
            return 1;
    }
}

/**
 * @brief Whether the Blind can be rolled at @p ante (antes below 1 count as 1).
 */
static inline bool blind_rules_can_roll(enum BlindType type, int ante)
{
    if ((int)type < 0 || type >= BLIND_TYPE_MAX)
        return false;
    return blind_rules_min_ante(type) <= (ante < 1 ? 1 : ante);
}

/**
 * @brief Score requirement multiplier of a Blind as a 24.8 fixed point number: Small x1,
 *        Big x1.5, The Needle x1, The Wall x4, Violet Vessel x6, every other Boss x2.
 */
static inline uint32_t blind_rules_req_mult_fx8(enum BlindType type)
{
    switch (type)
    {
        case BLIND_TYPE_SMALL:
        case BLIND_TYPE_NEEDLE:
            return BLIND_RULES_FIX_ONE;
        case BLIND_TYPE_BIG:
            return (BLIND_RULES_FIX_ONE * 3) / 2;
        case BLIND_TYPE_WALL:
            return BLIND_RULES_FIX_ONE * 4;
        case BLIND_TYPE_VESSEL:
            return BLIND_RULES_FIX_ONE * 6;
        default:
            return BLIND_RULES_FIX_ONE * 2;
    }
}

/**
 * @brief Picks among the Blinds of a list those that can be rolled at @p ante.
 *
 * @param types the candidate Blinds (unbeaten ones)
 * @param nb_types number of candidates
 * @param ante current ante
 * @param random random number; the result is the (random % eligible)-th eligible Blind
 * @param[out] out_nb_eligible number of eligible Blinds, can be NULL
 *
 * @return the chosen Blind, or -1 when no candidate is eligible
 */
static inline int blind_rules_pick(
    const uint8_t* types,
    int nb_types,
    int ante,
    uint32_t random,
    int* out_nb_eligible
)
{
    int nb_eligible = 0;
    for (int i = 0; i < nb_types; i++)
        nb_eligible += blind_rules_can_roll((enum BlindType)types[i], ante) ? 1 : 0;

    if (out_nb_eligible != NULL)
        *out_nb_eligible = nb_eligible;
    if (nb_eligible == 0)
        return -1;

    int pick = (int)(random % (uint32_t)nb_eligible);
    for (int i = 0; i < nb_types; i++)
    {
        if (!blind_rules_can_roll((enum BlindType)types[i], ante))
            continue;
        if (pick-- == 0)
            return types[i];
    }
    return -1;
}

#endif // BLIND_RULES_H
