/**
 * @file joker_batch4.h
 *
 * @brief Pure rules of the Jokers of A8 batch 4 (ID 94-108): suits, ranks, deck size and discards.
 *        Plain C without libtonc so it can be tested on the host (tests/joker_batch4).
 *
 * "X" multipliers are kept as a percent (100 = X1.00) when they are not whole numbers and applied
 * by adding the difference to the current mult (see joker_cards.h).
 */
#ifndef JOKER_BATCH4_H
#define JOKER_BATCH4_H

#include "card_rules.h"

#include <stdbool.h>
#include <stdint.h>

#define BLOODSTONE_JOKER_ID     94
#define FLOWER_POT_JOKER_ID     95
#define SEEING_DOUBLE_JOKER_ID  96
#define IDOL_JOKER_ID           97
#define ANCIENT_JOKER_ID        98
#define BASEBALL_CARD_JOKER_ID  99
#define EROSION_JOKER_ID        100
#define DRIVERS_LICENSE_JOKER_ID 101
#define CARD_SHARP_JOKER_ID     102
#define SUPERPOSITION_JOKER_ID  103
#define EIGHT_BALL_JOKER_ID     104
#define TO_DO_LIST_JOKER_ID     105
#define GREEN_JOKER_ID          106
#define FACELESS_JOKER_ID       107
#define HIT_THE_ROAD_JOKER_ID   108

#define BLOODSTONE_CHANCE       2  // 1 in 2
#define BLOODSTONE_PERCENT      150
#define EIGHT_BALL_CHANCE       4  // 1 in 4
#define EROSION_MULT_PER_CARD   4
#define EROSION_DECK_SIZE       52
#define DRIVERS_LICENSE_CARDS   16
#define DRIVERS_LICENSE_XMULT   3
#define FLOWER_POT_XMULT        3
#define SEEING_DOUBLE_XMULT     2
#define IDOL_XMULT              2
#define ANCIENT_PERCENT         150
#define BASEBALL_PERCENT        150
#define CARD_SHARP_XMULT        3
#define TO_DO_LIST_MONEY        4
#define FACELESS_MONEY          5
#define FACELESS_MIN_FACES      3
#define HIT_THE_ROAD_PERCENT    50 // per Jack discarded
#define GREEN_JOKER_MAX         9999

// Hand types a To Do List can ask for: High Card to Straight Flush (a Royal Flush counts as one)
#define TO_DO_LIST_FIRST_HAND 1
#define TO_DO_LIST_NUM_HANDS  9
#define TO_DO_LIST_ROYAL_FLUSH 10
#define TO_DO_LIST_STRAIGHT_FLUSH 9

static inline bool joker_batch4_one_in(uint32_t roll, uint32_t chance)
{
    return chance != 0 && roll % chance == 0;
}

/**
 * @brief Flower Pot: the scoring cards cover all four suits. Wild cards fill any missing suit,
 *        Stone cards have none.
 */
static inline bool joker_batch4_all_suits(const Card* const* cards, int count)
{
    unsigned mask = 0;
    int wilds = 0;
    for (int i = 0; i < count; i++)
    {
        if (cards[i]->enhancement == CARD_ENHANCEMENT_STONE)
            continue;
        if (cards[i]->enhancement == CARD_ENHANCEMENT_WILD)
            wilds++;
        else
            mask |= 1u << (cards[i]->suit % NUM_SUITS);
    }
    int distinct = 0;
    for (int s = 0; s < NUM_SUITS; s++)
        distinct += (mask >> s) & 1;
    return distinct + wilds >= NUM_SUITS;
}

/**
 * @brief Seeing Double: one scoring card is a Club and another one is of another suit.
 */
static inline bool joker_batch4_club_and_other(const Card* const* cards, int count)
{
    for (int i = 0; i < count; i++)
    {
        if (!card_has_suit(cards[i], CLUBS))
            continue;
        for (int j = 0; j < count; j++)
        {
            if (j == i || cards[j]->enhancement == CARD_ENHANCEMENT_STONE)
                continue;
            if (cards[j]->enhancement == CARD_ENHANCEMENT_WILD || cards[j]->suit != CLUBS)
                return true;
        }
    }
    return false;
}

/**
 * @brief The Idol: the card has the chosen rank and suit.
 */
static inline bool joker_batch4_idol_match(const Card* card, int rank, int suit)
{
    return card_has_rank(card) && card->rank == rank && card_has_suit(card, (uint8_t)suit);
}

/** The Idol keeps its card in persistent_state: rank in the low byte, suit above it. */
static inline int32_t joker_batch4_idol_pack(int rank, int suit)
{
    return (suit << 8) | rank;
}
static inline int joker_batch4_idol_rank(int32_t state)
{
    return (int)(state & 0xFF) % NUM_RANKS;
}
static inline int joker_batch4_idol_suit(int32_t state)
{
    return (int)((state >> 8) & 0xFF) % NUM_SUITS;
}

/**
 * @brief X(1.5^n) as a percent for Baseball Card with @p uncommon Uncommon Jokers, rounded.
 */
static inline int joker_batch4_baseball_percent(int uncommon)
{
    uint64_t percent = 100;
    for (int i = 0; i < uncommon && i < 8; i++)
        percent = (percent * BASEBALL_PERCENT + 50) / 100;
    return (int)percent;
}

/**
 * @brief Erosion: +4 mult for every card missing from the starting 52 cards.
 */
static inline int joker_batch4_erosion_mult(int deck_cards)
{
    return deck_cards >= EROSION_DECK_SIZE ? 0 : EROSION_MULT_PER_CARD * (EROSION_DECK_SIZE - deck_cards);
}

/**
 * @brief Driver's License: X3 with at least 16 enhanced cards in the full deck.
 */
static inline bool joker_batch4_drivers_license_active(int enhanced_cards)
{
    return enhanced_cards >= DRIVERS_LICENSE_CARDS;
}

/**
 * @brief Card Sharp keeps a bit per hand type played this round (1 << hand_type).
 *        A Royal Flush counts as a Straight Flush.
 */
static inline uint32_t joker_batch4_hand_bit(int hand_type)
{
    if (hand_type == TO_DO_LIST_ROYAL_FLUSH)
        hand_type = TO_DO_LIST_STRAIGHT_FLUSH;
    return 1u << hand_type;
}

/**
 * @brief Whether @p hand_type was already played this round; the caller then stores the new mask.
 */
static inline bool joker_batch4_card_sharp_seen(uint32_t mask, int hand_type)
{
    return (mask & joker_batch4_hand_bit(hand_type)) != 0;
}

/**
 * @brief Superposition: the hand is a Straight and one scoring card is an Ace.
 */
static inline bool joker_batch4_superposition(bool has_straight, const Card* const* cards, int count)
{
    if (!has_straight)
        return false;
    for (int i = 0; i < count; i++)
    {
        if (card_has_rank(cards[i]) && cards[i]->rank == ACE)
            return true;
    }
    return false;
}

/**
 * @brief To Do List: the hand type wanted for a random roll (High Card ... Straight Flush).
 */
static inline int joker_batch4_todo_target(uint32_t roll)
{
    return TO_DO_LIST_FIRST_HAND + (int)(roll % TO_DO_LIST_NUM_HANDS);
}

/**
 * @brief To Do List pays when the played hand is the wanted one (a Royal Flush is a Straight Flush).
 */
static inline bool joker_batch4_todo_hit(int target, int played_hand_type)
{
    return joker_batch4_hand_bit(played_hand_type) == joker_batch4_hand_bit(target);
}

/**
 * @brief Green Joker: +1 per hand played, -1 per discard, never below 0.
 */
static inline int32_t joker_batch4_green_hand(int32_t state)
{
    return state >= GREEN_JOKER_MAX ? GREEN_JOKER_MAX : state + 1;
}
static inline int32_t joker_batch4_green_discard(int32_t state)
{
    return state > 0 ? state - 1 : 0;
}

/**
 * @brief Faceless Joker: $5 when 3 or more face cards are discarded at once.
 */
static inline int joker_batch4_faceless_money(int face_cards)
{
    return face_cards >= FACELESS_MIN_FACES ? FACELESS_MONEY : 0;
}

/**
 * @brief Hit the Road: X1 plus X0.5 for every Jack discarded this round, as a percent.
 */
static inline int joker_batch4_hit_the_road_percent(int jacks)
{
    return 100 + HIT_THE_ROAD_PERCENT * (jacks > 0 ? jacks : 0);
}

#endif // JOKER_BATCH4_H
