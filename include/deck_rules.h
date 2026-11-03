/**
 * @file deck_rules.h
 *
 * @brief Pure rules of the Decks that change the starting cards, the score or the Blind size:
 *        Abandoned, Checkered, Erratic and Plasma, and the Anaglyph
 *        Deck's Double Tag.
 *
 * Plain C without libtonc so it can be tested on the host (tests/deck_rules).
 */
#ifndef DECK_RULES_H
#define DECK_RULES_H

#include "card_rules.h"

#include <stdbool.h>
#include <stdint.h>

enum DeckType
{
    DECK_TYPE_RED,
    DECK_TYPE_BLUE,
    DECK_TYPE_YELLOW,
    DECK_TYPE_GREEN,
    DECK_TYPE_BLACK,
    DECK_TYPE_PAINTED,
    DECK_TYPE_MAGIC,
    DECK_TYPE_NEBULA,
    DECK_TYPE_GHOST,
    DECK_TYPE_ABANDONED,
    DECK_TYPE_CHECKERED,
    DECK_TYPE_ZODIAC,
    DECK_TYPE_PLASMA,
    DECK_TYPE_ERRATIC,
    DECK_TYPE_ANAGLYPH,
    DECK_TYPE_MAX
};

/// Cards of a standard starting Deck
#define DECK_STANDARD_SIZE (NUM_SUITS * NUM_RANKS)

/// The Plasma Deck doubles the score requirement of every Blind
#define DECK_PLASMA_BLIND_SCALE 2

/**
 * @brief Source of random numbers, so the Erratic Deck can be tested without the game's RNG.
 */
typedef uint32_t (*DeckRandFn)(void* ctx);

/**
 * @brief Whether the standard Deck has this card for the given Deck: the Abandoned Deck has no
 *        face cards (Jack, Queen, King).
 */
static inline bool deck_rules_has_card(enum DeckType deck, int suit, int rank)
{
    (void)suit;
    if (deck == DECK_TYPE_ABANDONED)
        return rank != JACK && rank != QUEEN && rank != KING;
    return true;
}

/**
 * @brief Suit a standard card starts with: the Checkered Deck has only Spades and Hearts
 *        (Clubs become Spades, Diamonds become Hearts).
 */
static inline int deck_rules_starting_suit(enum DeckType deck, int suit)
{
    if (deck == DECK_TYPE_CHECKERED)
    {
        if (suit == CLUBS)
            return SPADES;
        if (suit == DIAMONDS)
            return HEARTS;
    }
    return suit;
}

/**
 * @brief Writes the suit and rank of every starting card of the Deck.
 *
 * Standard Decks list suit by suit, rank by rank. The Erratic Deck has 52 cards, each one with a
 * random suit and rank (repeats are possible), taking one number from @p rand_fn per card.
 *
 * @param suits out: suit of each card, @p capacity entries
 * @param ranks out: rank of each card, @p capacity entries
 * @param capacity size of both arrays, at least DECK_STANDARD_SIZE
 * @return number of cards
 */
static inline int deck_rules_build(
    enum DeckType deck,
    uint8_t* suits,
    uint8_t* ranks,
    int capacity,
    DeckRandFn rand_fn,
    void* ctx
)
{
    int count = 0;

    if (deck == DECK_TYPE_ERRATIC)
    {
        for (; count < DECK_STANDARD_SIZE && count < capacity; count++)
        {
            uint32_t pick = rand_fn(ctx) % DECK_STANDARD_SIZE;
            suits[count] = (uint8_t)(pick / NUM_RANKS);
            ranks[count] = (uint8_t)(pick % NUM_RANKS);
        }
        return count;
    }

    for (int suit = 0; suit < NUM_SUITS; suit++)
    {
        for (int rank = 0; rank < NUM_RANKS; rank++)
        {
            if (!deck_rules_has_card(deck, suit, rank) || count >= capacity)
                continue;
            suits[count] = (uint8_t)deck_rules_starting_suit(deck, suit);
            ranks[count] = (uint8_t)rank;
            count++;
        }
    }
    return count;
}

/**
 * @brief Whether the Deck balances chips and mult before the hand is scored (Plasma).
 */
static inline bool deck_rules_balances_score(enum DeckType deck)
{
    return deck == DECK_TYPE_PLASMA;
}

/**
 * @brief Plasma Deck: chips and mult both become the average of the two (rounded down).
 */
static inline void deck_rules_plasma_balance(uint32_t* chips, uint32_t* mult)
{
    uint32_t average = (uint32_t)(((uint64_t)*chips + *mult) / 2);
    *chips = average;
    *mult = average;
}

/**
 * @brief Multiplier of the score requirement of every Blind (2 on the Plasma Deck).
 */
static inline uint32_t deck_rules_blind_scale(enum DeckType deck)
{
    return deck == DECK_TYPE_PLASMA ? DECK_PLASMA_BLIND_SCALE : 1;
}

/// Ghost Deck: weight in tenths of Spectral cards among the Shop consumables (Tarots and Planets
/// weigh 40 each without Vouchers, so a Spectral card is 2 in 10 like in Balatro)
#define DECK_GHOST_SPECTRAL_WEIGHT_X10 20

/**
 * @brief Whether a Shop consumable slot of the Ghost Deck holds a Spectral card.
 *
 * @param tarot_weight_x10, planet_weight_x10 weights in tenths of the other consumables
 * @param r random number
 */
static inline bool deck_rules_shop_is_spectral(
    enum DeckType deck,
    int tarot_weight_x10,
    int planet_weight_x10,
    uint32_t r
)
{
    if (deck != DECK_TYPE_GHOST)
        return false;
    uint32_t total =
        (uint32_t)(tarot_weight_x10 + planet_weight_x10 + DECK_GHOST_SPECTRAL_WEIGHT_X10);
    return r % total < (uint32_t)DECK_GHOST_SPECTRAL_WEIGHT_X10;
}

/**
 * @brief Anaglyph Deck: a Double Tag after defeating a Boss Blind.
 *
 * @param boss_defeated true when the Blind that just ended was a Boss Blind
 */
static inline bool deck_rules_gives_double_tag(enum DeckType deck, bool boss_defeated)
{
    return deck == DECK_TYPE_ANAGLYPH && boss_defeated;
}

#endif // DECK_RULES_H
