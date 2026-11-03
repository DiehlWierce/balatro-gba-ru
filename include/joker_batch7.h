/**
 * @file joker_batch7.h
 *
 * @brief Pure rules of the Jokers of A8 batch 7 (ID 135+). Plain C without libtonc so it can be
 *        tested on the host (tests/joker_batch7).
 *
 * "X" multipliers are kept as a percent (100 = X1.00), see joker_cards.h.
 */
#ifndef JOKER_BATCH7_H
#define JOKER_BATCH7_H

#include <stdbool.h>
#include <stdint.h>

#define THROWBACK_JOKER_ID 135
#define MADNESS_JOKER_ID   136
#define CAMPFIRE_JOKER_ID  137

#define FIRST_BATCH7_JOKER_ID THROWBACK_JOKER_ID
#define LAST_BATCH7_JOKER_ID  CAMPFIRE_JOKER_ID

#define THROWBACK_PERCENT_PER_SKIP 25
#define MADNESS_PERCENT_PER_BLIND  50
#define CAMPFIRE_PERCENT_PER_SALE  25
#define BATCH7_STATE_MAX           99999

static inline int joker_batch7_clamp(int count, int percent_per_step)
{
    if (count < 0)
        count = 0;
    if (count > BATCH7_STATE_MAX / percent_per_step)
        count = BATCH7_STATE_MAX / percent_per_step;
    return count;
}

/**
 * @brief Throwback: X1 plus X0.25 for every Blind skipped this run, as a percent.
 */
static inline int joker_batch7_throwback_percent(int skipped)
{
    return 100 + THROWBACK_PERCENT_PER_SKIP * joker_batch7_clamp(skipped, THROWBACK_PERCENT_PER_SKIP);
}

/**
 * @brief Madness: X1 plus X0.5 for every Small or Big Blind selected, as a percent.
 */
static inline int joker_batch7_madness_percent(int blinds)
{
    return 100 + MADNESS_PERCENT_PER_BLIND * joker_batch7_clamp(blinds, MADNESS_PERCENT_PER_BLIND);
}

/**
 * @brief Madness triggers on the Small and Big Blinds only (@p is_boss_blind false).
 */
static inline bool joker_batch7_madness_triggers(bool is_boss_blind)
{
    return !is_boss_blind;
}

/**
 * @brief Madness: which of the @p nb_candidates other destroyable Jokers is destroyed, -1 if none.
 */
static inline int joker_batch7_madness_pick(uint32_t roll, int nb_candidates)
{
    return nb_candidates <= 0 ? -1 : (int)(roll % (uint32_t)nb_candidates);
}

/**
 * @brief Campfire: X1 plus X0.25 for every card sold, as a percent.
 */
static inline int joker_batch7_campfire_percent(int sold)
{
    return 100 + CAMPFIRE_PERCENT_PER_SALE * joker_batch7_clamp(sold, CAMPFIRE_PERCENT_PER_SALE);
}

/**
 * @brief Campfire: the counter resets when a Boss Blind is defeated.
 */
static inline int joker_batch7_campfire_after_round(int sold, bool boss_defeated)
{
    return boss_defeated ? 0 : sold;
}

#endif // JOKER_BATCH7_H
