/**
 * @file tarot_rules.h
 *
 * @brief Tarot cards: the list, the number of cards each one targets and the pure rules that
 *        change a Card (suit, rank, enhancement, copy).
 *
 * Plain C without libtonc so it can be tested on the host (tests/tarot_rules).
 */
#ifndef TAROT_RULES_H
#define TAROT_RULES_H

#include "card_rules.h"

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Tarot cards, in the order of their sprites in graphics/tarot_gfx.png
 */
enum TarotType
{
    TAROT_FOOL,           // Creates the last used Planet or Tarot
    TAROT_MAGICIAN,       // 2 cards become Lucky
    TAROT_HIGH_PRIESTESS, // Creates 2 Planets
    TAROT_EMPRESS,        // 2 cards become Mult
    TAROT_EMPEROR,        // Creates 2 Tarots
    TAROT_HIEROPHANT,     // 2 cards become Bonus
    TAROT_LOVERS,         // 1 card becomes Wild
    TAROT_CHARIOT,        // 1 card becomes Steel
    TAROT_JUSTICE,        // 1 card becomes Glass
    TAROT_HERMIT,         // Doubles the money (up to $20)
    TAROT_WHEEL,          // Not implemented: needs Editions
    TAROT_STRENGTH,       // 2 cards get +1 rank
    TAROT_HANGED_MAN,     // Destroys 2 cards
    TAROT_DEATH,          // The left card becomes a copy of the right card
    TAROT_TEMPERANCE,     // Money equal to the sell value of the Jokers (up to $50)
    TAROT_DEVIL,          // 1 card becomes Gold
    TAROT_TOWER,          // 1 card becomes Stone
    TAROT_STAR,           // 3 cards become Diamonds
    TAROT_MOON,           // 3 cards become Clubs
    TAROT_SUN,            // 3 cards become Hearts
    TAROT_JUDGEMENT,      // Creates a random Joker
    TAROT_WORLD,          // 3 cards become Spades
    TAROT_TYPE_MAX
};

#define TAROT_HERMIT_MAX_MONEY      20
#define TAROT_TEMPERANCE_MAX_MONEY  50
#define TAROT_CREATE_MAX            2 // High Priestess and Emperor
#define TAROT_MAX_TARGETS           3

/**
 * @brief Whether the Tarot is implemented and can appear in the Shop and Packs.
 */
static inline bool tarot_is_rollable(enum TarotType tarot)
{
    return tarot >= 0 && tarot < TAROT_TYPE_MAX && tarot != TAROT_WHEEL;
}

/**
 * @brief Number of cards of the hand a Tarot needs selected: [*min, *max], 0 and 0 if it targets
 *        nothing.
 */
static inline void tarot_get_target_range(enum TarotType tarot, int* min, int* max)
{
    int lo = 0, hi = 0;
    switch (tarot)
    {
        case TAROT_MAGICIAN:
        case TAROT_EMPRESS:
        case TAROT_HIEROPHANT:
        case TAROT_STRENGTH:
        case TAROT_HANGED_MAN:
            lo = 1, hi = 2;
            break;
        case TAROT_LOVERS:
        case TAROT_CHARIOT:
        case TAROT_JUSTICE:
        case TAROT_DEVIL:
        case TAROT_TOWER:
            lo = 1, hi = 1;
            break;
        case TAROT_DEATH:
            lo = 2, hi = 2;
            break;
        case TAROT_STAR:
        case TAROT_MOON:
        case TAROT_SUN:
        case TAROT_WORLD:
            lo = 1, hi = 3;
            break;
        default:
            break;
    }
    *min = lo;
    *max = hi;
}

/**
 * @brief The enhancement a Tarot gives, CARD_ENHANCEMENT_NONE if it doesn't enhance.
 */
static inline enum CardEnhancement tarot_get_enhancement(enum TarotType tarot)
{
    switch (tarot)
    {
        case TAROT_MAGICIAN:
            return CARD_ENHANCEMENT_LUCKY;
        case TAROT_EMPRESS:
            return CARD_ENHANCEMENT_MULT;
        case TAROT_HIEROPHANT:
            return CARD_ENHANCEMENT_BONUS;
        case TAROT_LOVERS:
            return CARD_ENHANCEMENT_WILD;
        case TAROT_CHARIOT:
            return CARD_ENHANCEMENT_STEEL;
        case TAROT_JUSTICE:
            return CARD_ENHANCEMENT_GLASS;
        case TAROT_DEVIL:
            return CARD_ENHANCEMENT_GOLD;
        case TAROT_TOWER:
            return CARD_ENHANCEMENT_STONE;
        default:
            return CARD_ENHANCEMENT_NONE;
    }
}

/**
 * @brief The suit a Tarot gives, -1 if it doesn't change suits.
 */
static inline int tarot_get_suit(enum TarotType tarot)
{
    switch (tarot)
    {
        case TAROT_STAR:
            return DIAMONDS;
        case TAROT_MOON:
            return CLUBS;
        case TAROT_SUN:
            return HEARTS;
        case TAROT_WORLD:
            return SPADES;
        default:
            return -1;
    }
}

/**
 * @brief Whether the Tarot creates other consumables (Fool, High Priestess, Emperor).
 */
static inline bool tarot_creates_consumables(enum TarotType tarot)
{
    return tarot == TAROT_FOOL || tarot == TAROT_HIGH_PRIESTESS || tarot == TAROT_EMPEROR;
}

/**
 * @brief Changes the suit of a card. Stone cards have no suit and are left as they are.
 */
static inline void card_set_suit(Card* card, uint8_t suit)
{
    if (suit < NUM_SUITS && card->enhancement != CARD_ENHANCEMENT_STONE)
        card->suit = suit;
}

/**
 * @brief Raises the rank of a card by one, Ace goes back to Two (Strength).
 *        Stone cards have no rank and are left as they are.
 */
static inline void card_rank_up(Card* card)
{
    if (card->enhancement != CARD_ENHANCEMENT_STONE)
        card->rank = (card->rank + 1) % NUM_RANKS;
}

/**
 * @brief Death: @p dst becomes a copy of @p src, with its suit, rank, enhancement and seal.
 *        The runtime flags (debuffed, face down, played Ante) of @p dst are kept.
 */
static inline void card_copy_from(Card* dst, const Card* src)
{
    dst->suit = src->suit;
    dst->rank = src->rank;
    dst->enhancement = src->enhancement;
    dst->seal = src->seal;
}

/**
 * @brief Hermit: money won when it doubles @p money, never above TAROT_HERMIT_MAX_MONEY.
 *        Negative money (debt) gives nothing.
 */
static inline int tarot_hermit_gain(int money)
{
    if (money <= 0)
        return 0;
    return money > TAROT_HERMIT_MAX_MONEY ? TAROT_HERMIT_MAX_MONEY : money;
}

/**
 * @brief Temperance: money won for a total sell value of the Jokers.
 */
static inline int tarot_temperance_gain(int total_sell_value)
{
    if (total_sell_value <= 0)
        return 0;
    return total_sell_value > TAROT_TEMPERANCE_MAX_MONEY ? TAROT_TEMPERANCE_MAX_MONEY
                                                         : total_sell_value;
}

#endif // TAROT_RULES_H
