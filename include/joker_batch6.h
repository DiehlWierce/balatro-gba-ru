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

#define FIRST_BATCH6_JOKER_ID CANIO_JOKER_ID
#define LAST_BATCH6_JOKER_ID  PERKEO_JOKER_ID

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

#endif // JOKER_BATCH6_H
