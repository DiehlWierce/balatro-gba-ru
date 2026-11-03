/**
 * @file joker_cards.h
 *
 * @brief Pure rules of the Jokers tied to card enhancements, Tarots and the deck (A8 batch 3).
 *        Plain C without libtonc so it can be tested on the host (tests/joker_cards).
 *
 * "X" multipliers are integers in this port, so they are kept as a percent (100 = X1.00) and
 * applied by adding the difference to the current mult, like Baron does.
 */
#ifndef JOKER_CARDS_H
#define JOKER_CARDS_H

#include "card_rules.h"

#include "probability.h"

#include <stdbool.h>
#include <stdint.h>

#define STEEL_JOKER_ID         79
#define GLASS_JOKER_ID         80
#define LUCKY_CAT_JOKER_ID     81
#define VAMPIRE_JOKER_ID       82
#define MIDAS_MASK_JOKER_ID    83
#define CARTOMANCER_JOKER_ID   84
#define CONSTELLATION_JOKER_ID 85
#define FORTUNE_TELLER_JOKER_ID 86
#define HALLUCINATION_JOKER_ID 87
#define STONE_JOKER_ID         88
#define MARBLE_JOKER_ID        89
#define CERTIFICATE_JOKER_ID   90
#define SPACE_JOKER_ID         91
#define HIKER_JOKER_ID         92
#define BURGLAR_JOKER_ID       93

#define XMULT_BASE_PERCENT        100
#define STEEL_JOKER_PERCENT       20 // per Steel card in the full deck
#define GLASS_JOKER_PERCENT       75 // per Glass card destroyed
#define LUCKY_CAT_PERCENT         25 // per Lucky card trigger
#define VAMPIRE_PERCENT           10 // per enhancement eaten
#define CONSTELLATION_PERCENT     10 // per Planet used
#define STONE_JOKER_CHIPS         25 // per Stone card in the full deck
#define HIKER_CHIPS               5
#define BURGLAR_HANDS             3
#define HALLUCINATION_CHANCE      2 // 1 in 2
#define SPACE_JOKER_CHANCE        4 // 1 in 4
#define BONUS_CHIPS_MAX           255

/**
 * @brief X multiplier of a scaling Joker as a percent (100 = X1.00), 100 for the other Jokers.
 * @param state the counter of the Joker (Glass cards destroyed, Lucky triggers, enhancements
 *              eaten, Planets used)
 * @param steel_cards Steel cards in the full deck (Steel Joker)
 */
static inline int joker_cards_xmult_percent(int joker_id, int state, int steel_cards)
{
    switch (joker_id)
    {
        case STEEL_JOKER_ID:
            return XMULT_BASE_PERCENT + STEEL_JOKER_PERCENT * steel_cards;
        case GLASS_JOKER_ID:
            return XMULT_BASE_PERCENT + GLASS_JOKER_PERCENT * state;
        case LUCKY_CAT_JOKER_ID:
            return XMULT_BASE_PERCENT + LUCKY_CAT_PERCENT * state;
        case VAMPIRE_JOKER_ID:
            return XMULT_BASE_PERCENT + VAMPIRE_PERCENT * state;
        case CONSTELLATION_JOKER_ID:
            return XMULT_BASE_PERCENT + CONSTELLATION_PERCENT * state;
        default:
            return XMULT_BASE_PERCENT;
    }
}

/**
 * @brief Mult to add so that @p mult is multiplied by @p percent / 100, rounded to the nearest.
 */
static inline uint32_t joker_cards_mult_gain(uint32_t mult, int percent)
{
    if (percent <= XMULT_BASE_PERCENT)
        return 0;
    uint64_t scaled = ((uint64_t)mult * (uint64_t)percent + 50) / 100;
    uint64_t gain = scaled - mult;
    return gain > 0xFFFFFFFFu ? 0xFFFFFFFFu : (uint32_t)gain;
}

/**
 * @brief Chips of Stone Joker for the Stone cards in the full deck.
 */
static inline int joker_cards_stone_chips(int stone_cards)
{
    return STONE_JOKER_CHIPS * stone_cards;
}

/**
 * @brief Fortune Teller: +1 mult for every Tarot used in the run.
 */
static inline int joker_cards_fortune_teller_mult(int tarots_used)
{
    return tarots_used;
}

/**
 * @brief Chips a Hiker adds for good to a scored card, saturates at BONUS_CHIPS_MAX.
 */
static inline uint8_t joker_cards_hiker_bonus(uint8_t bonus_chips)
{
    unsigned total = (unsigned)bonus_chips + HIKER_CHIPS;
    return total > BONUS_CHIPS_MAX ? BONUS_CHIPS_MAX : (uint8_t)total;
}

/**
 * @brief Vampire: removes the enhancement of a scored card.
 * @return true if the card had one
 */
static inline bool joker_cards_vampire_eat(Card* card)
{
    if (card->enhancement == CARD_ENHANCEMENT_NONE)
        return false;
    card->enhancement = CARD_ENHANCEMENT_NONE;
    return true;
}

/**
 * @brief Midas Mask: a scored face card becomes a Gold card.
 * @return true if the card changed
 */
static inline bool joker_cards_midas_gild(Card* card, bool is_face)
{
    if (!is_face || card->enhancement == CARD_ENHANCEMENT_GOLD)
        return false;
    card->enhancement = CARD_ENHANCEMENT_GOLD;
    return true;
}

/**
 * @brief Whether a random roll triggers a "1 in chance" effect.
 */
static inline bool joker_cards_one_in(uint32_t roll, uint32_t chance)
{
    return probability_one_in(roll, chance); // Oops! All 6s
}

/**
 * @brief Hands and discards after Burglar: +3 hands, no discards left.
 */
static inline int joker_cards_burglar_hands(int hands)
{
    return hands + BURGLAR_HANDS;
}

#endif // JOKER_CARDS_H
