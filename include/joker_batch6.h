/**
 * @file joker_batch6.h
 *
 * @brief Pure rules of the Jokers of A8 batch 6: the Legendary Jokers (ID 124-128, only created by
 *        The Soul) and the ordinary Jokers after them. Plain C without libtonc so it can be tested
 *        on the host (tests/joker_batch6).
 *
 * "X" multipliers are kept as a percent (100 = X1.00) and applied by adding the difference to the
 * current mult (see joker_cards.h).
 */
#ifndef JOKER_BATCH6_H
#define JOKER_BATCH6_H

#include "card_rules.h"

#include <stdbool.h>
#include <stdint.h>

#define CANIO_JOKER_ID     124
#define TRIBOULET_JOKER_ID 125
#define YORICK_JOKER_ID    126
#define CHICOT_JOKER_ID    127
#define PERKEO_JOKER_ID    128

#define FLASH_CARD_JOKER_ID 129
#define RED_CARD_JOKER_ID   130
#define LUCHADOR_JOKER_ID   131
#define MR_BONES_JOKER_ID   132
#define CHAOS_CLOWN_JOKER_ID 133
#define ASTRONOMER_JOKER_ID 134

#define FIRST_BATCH6_JOKER_ID CANIO_JOKER_ID
#define LAST_BATCH6_JOKER_ID  ASTRONOMER_JOKER_ID

#define FLASH_CARD_MULT_PER_REROLL 2
#define RED_CARD_MULT_PER_SKIP     3
#define MR_BONES_NUMERATOR         1 // Saves the run from a score of at least 1/4 of the requirement
#define MR_BONES_DENOMINATOR       4

#define LEGENDARY_JOKER_VALUE   20 // Base value (sells for half)
#define CANIO_PERCENT_PER_FACE  100 // +X1 per face card destroyed
#define TRIBOULET_XMULT         2
#define YORICK_DISCARDS         23  // +X1 every 23 cards discarded
#define YORICK_PERCENT_PER_STEP 100
#define STATE_PERCENT_MAX       99999

/**
 * @brief Canio: X1 plus X1 for each face card destroyed, as a percent.
 */
static inline int joker_batch6_canio_percent(int faces_destroyed)
{
    if (faces_destroyed < 0)
        faces_destroyed = 0;
    if (faces_destroyed > STATE_PERCENT_MAX / CANIO_PERCENT_PER_FACE)
        faces_destroyed = STATE_PERCENT_MAX / CANIO_PERCENT_PER_FACE;
    return 100 + CANIO_PERCENT_PER_FACE * faces_destroyed;
}

/**
 * @brief Triboulet: scored Kings and Queens (Stone cards have no rank).
 */
static inline bool joker_batch6_triboulet_hit(const Card* card)
{
    return card != NULL && card_has_rank(card) && (card->rank == KING || card->rank == QUEEN);
}

/**
 * @brief Yorick: X1 plus X1 for every 23 cards discarded (@p discarded is the total), as a percent.
 */
static inline int joker_batch6_yorick_percent(int discarded)
{
    if (discarded < 0)
        discarded = 0;
    int steps = discarded / YORICK_DISCARDS;
    if (steps > STATE_PERCENT_MAX / YORICK_PERCENT_PER_STEP)
        steps = STATE_PERCENT_MAX / YORICK_PERCENT_PER_STEP;
    return 100 + YORICK_PERCENT_PER_STEP * steps;
}

/**
 * @brief Yorick: cards left before the next X1.
 */
static inline int joker_batch6_yorick_remaining(int discarded)
{
    if (discarded < 0)
        discarded = 0;
    return YORICK_DISCARDS - discarded % YORICK_DISCARDS;
}

/**
 * @brief Chicot: the requirement multiplier of a Boss Blind when it is disabled (as a plain Blind
 *        of the Boss row: X2).
 */
static inline int joker_batch6_chicot_requirement_mult(int mult)
{
    return mult > 2 ? 2 : mult;
}

/**
 * @brief Perkeo: which of @p nb owned consumables is copied, -1 if there is none or no free slot.
 */
static inline int joker_batch6_perkeo_pick(uint32_t roll, int nb, int nb_free_slots)
{
    if (nb <= 0 || nb_free_slots <= 0)
        return -1;
    return (int)(roll % (uint32_t)nb);
}

/**
 * @brief Flash Card: +2 mult for each reroll (persistent_state counts them).
 */
static inline int joker_batch6_flash_card_mult(int rerolls)
{
    if (rerolls < 0)
        rerolls = 0;
    if (rerolls > STATE_PERCENT_MAX / FLASH_CARD_MULT_PER_REROLL)
        rerolls = STATE_PERCENT_MAX / FLASH_CARD_MULT_PER_REROLL;
    return FLASH_CARD_MULT_PER_REROLL * rerolls;
}

/**
 * @brief Red Card: +3 mult for each Booster Pack skipped (persistent_state counts them).
 */
static inline int joker_batch6_red_card_mult(int skipped)
{
    if (skipped < 0)
        skipped = 0;
    if (skipped > STATE_PERCENT_MAX / RED_CARD_MULT_PER_SKIP)
        skipped = STATE_PERCENT_MAX / RED_CARD_MULT_PER_SKIP;
    return RED_CARD_MULT_PER_SKIP * skipped;
}

/**
 * @brief Luchador: whether selling it disables the Boss (only a Boss Blind being played).
 */
static inline bool joker_batch6_luchador_disables(bool in_round, bool is_boss_blind)
{
    return in_round && is_boss_blind;
}

/**
 * @brief Mr. Bones: whether a lost round (no hands left) is saved, the score is at least 25% of the
 *        requirement.
 */
static inline bool joker_batch6_mr_bones_saves(uint32_t score, uint32_t requirement)
{
    return requirement > 0 &&
           (uint64_t)score * MR_BONES_DENOMINATOR >= (uint64_t)requirement * MR_BONES_NUMERATOR;
}

/**
 * @brief Chaos the Clown: whether the next reroll is free (owned, free reroll of this Shop unused).
 */
static inline bool joker_batch6_chaos_free_reroll(bool owned, bool used)
{
    return owned && !used;
}

/**
 * @brief Astronomer: Planet cards and Celestial Packs are free.
 */
static inline int joker_batch6_astronomer_price(int price, bool owned)
{
    return owned ? 0 : price;
}

#endif // JOKER_BATCH6_H
