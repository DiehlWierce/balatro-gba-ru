/**
 * @file hand_type.h
 *
 * @brief Poker hand types. Kept free of GBA headers so host tests can use it.
 */
#ifndef HAND_TYPE_H
#define HAND_TYPE_H

/**
 * @brief Poker hand types, from the weakest to the strongest.
 *
 * HAND_TYPE_MAX is the last hand type, not a count: arrays with one element per hand type
 * (e.g. g_game_vars.nb_played_hands, g_game_vars.hand_levels) have HAND_TYPE_MAX elements and
 * are indexed with `hand_type - 1`.
 */
enum HandType
{
    NONE,
    HIGH_CARD,
    PAIR,
    TWO_PAIR,
    THREE_OF_A_KIND,
    STRAIGHT,
    FLUSH,
    FULL_HOUSE,
    FOUR_OF_A_KIND,
    STRAIGHT_FLUSH,
    ROYAL_FLUSH,
    FIVE_OF_A_KIND,
    FLUSH_HOUSE,
    FLUSH_FIVE,
    HAND_TYPE_MAX = FLUSH_FIVE
};

#endif // HAND_TYPE_H
