/**
 * @file probability.h
 *
 * @brief "1 in N" rolls shared by the Jokers, cards and tarots. Oops! All 6s doubles the chances
 *        (g_probability_mult, set in game.c). Plain C so it can be tested on the host.
 */
#ifndef PROBABILITY_H
#define PROBABILITY_H

#include <stdbool.h>
#include <stdint.h>

// A weak definition so the header works in every translation unit and in the host tests
__attribute__((weak)) int g_probability_mult = 1;

/**
 * @brief Whether @p roll triggers a "1 in @p chance" effect: "mult in chance" with Oops! All 6s.
 */
static inline bool probability_one_in(uint32_t roll, uint32_t chance)
{
    return chance != 0 && roll % chance < (uint32_t)g_probability_mult;
}

#endif // PROBABILITY_H
