/**
 * @file joker_batch8.h
 *
 * @brief Pure rules of the Jokers of A8 batch 8 (ID 147+): Matador and Mime. Plain C without
 *        libtonc so it can be tested on the host (tests/joker_batch8).
 */
#ifndef JOKER_BATCH8_H
#define JOKER_BATCH8_H

#include <stdbool.h>

#define MATADOR_JOKER_ID 147
#define MIME_JOKER_ID    148

#define FIRST_BATCH8_JOKER_ID MATADOR_JOKER_ID
#define LAST_BATCH8_JOKER_ID  MIME_JOKER_ID

#define MATADOR_MONEY 8

// Every Mime adds a pass over the held cards, the number of passes stays small to keep the scoring
// animation short
#define MIME_MAX_EXTRA_PASSES 3

/**
 * @brief Matador: $8 each time the played hand triggers the Boss Blind ability.
 */
static inline int joker_batch8_matador_money(bool boss_triggered)
{
    return boss_triggered ? MATADOR_MONEY : 0;
}

/**
 * @brief Mime: how many times the cards held in hand score (Steel, Baron, Shoot the Moon...) with
 *        @p nb_mimes active Mimes: once, plus once per Mime.
 */
static inline int joker_batch8_mime_passes(int nb_mimes)
{
    if (nb_mimes < 0)
        nb_mimes = 0;
    if (nb_mimes > MIME_MAX_EXTRA_PASSES)
        nb_mimes = MIME_MAX_EXTRA_PASSES;
    return 1 + nb_mimes;
}

#endif // JOKER_BATCH8_H
