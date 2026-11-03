/**
 * @file joker_batch5.h
 *
 * @brief Pure rules of the Jokers of A8 batch 5 (ID 109-123): Spectral makers, discard rewards,
 *        food Jokers and the Jokers that grow. Plain C without libtonc so it can be tested on the
 *        host (tests/joker_batch5).
 *
 * "X" multipliers are kept as a percent (100 = X1.00) and applied by adding the difference to the
 * current mult (see joker_cards.h).
 */
#ifndef JOKER_BATCH5_H
#define JOKER_BATCH5_H

#include "card_rules.h"
#include "hand_type.h"

#include "probability.h"

#include <stdbool.h>
#include <stdint.h>

#define SEANCE_JOKER_ID            109
#define SIXTH_SENSE_JOKER_ID       110
#define DNA_JOKER_ID               111
#define GROS_MICHEL_JOKER_ID       112
#define CAVENDISH_JOKER_ID         113
#define POPCORN_JOKER_ID           114
#define RAMEN_JOKER_ID             115
#define CASTLE_JOKER_ID            116
#define MAIL_IN_REBATE_JOKER_ID    117
#define TRADING_CARD_JOKER_ID      118
#define VAGABOND_JOKER_ID          119
#define OBELISK_JOKER_ID           120
#define TURTLE_BEAN_JOKER_ID       121
#define CEREMONIAL_DAGGER_JOKER_ID 122
#define CLOUD_NINE_JOKER_ID        123

#define FIRST_BATCH5_JOKER_ID SEANCE_JOKER_ID
#define LAST_BATCH5_JOKER_ID  CLOUD_NINE_JOKER_ID

#define GROS_MICHEL_MULT       15
#define GROS_MICHEL_CHANCE     6    // 1 in 6 to be destroyed at the end of the round
#define CAVENDISH_PERCENT      300  // X3
#define CAVENDISH_CHANCE       1000 // 1 in 1000 to be destroyed at the end of the round
#define POPCORN_START_MULT     20
#define POPCORN_ROUND_LOSS     4
#define RAMEN_START_PERCENT    200 // X2
#define RAMEN_CARD_LOSS        1   // X0.01 per discarded card
#define RAMEN_MIN_PERCENT      100 // Destroyed when it would go below X1
#define CASTLE_CHIPS_PER_CARD  3
#define CASTLE_CHIPS_MAX       0xFFFFFF
#define MAIL_IN_REBATE_MONEY   5
#define TRADING_CARD_MONEY     3
#define VAGABOND_MONEY_LIMIT   4
#define OBELISK_STEP_PERCENT   20
#define TURTLE_BEAN_START      5
#define DAGGER_VALUE_FACTOR    2
#define DAGGER_MULT_MAX        99999
#define CLOUD_NINE_MONEY       1

static inline bool joker_batch5_one_in(uint32_t roll, uint32_t chance)
{
    return probability_one_in(roll, chance); // Oops! All 6s
}

/**
 * @brief Seance: the played hand is a Straight Flush (a Royal Flush is one too).
 */
static inline bool joker_batch5_is_straight_flush(int hand_type)
{
    return hand_type == STRAIGHT_FLUSH || hand_type == ROYAL_FLUSH;
}

/**
 * @brief Sixth Sense: the first hand of the round is a single 6.
 *        @p hands_played counts the hand being played.
 */
static inline bool joker_batch5_sixth_sense_hit(int hands_played, int nb_played_cards, const Card* card)
{
    return hands_played == 1 && nb_played_cards == 1 && card != NULL && card_has_rank(card) &&
           card->rank == SIX;
}

/**
 * @brief DNA: the first hand of the round is a single card.
 */
static inline bool joker_batch5_dna_hit(int hands_played, int nb_played_cards)
{
    return hands_played == 1 && nb_played_cards == 1;
}

/**
 * @brief Trading Card: the first discard of the round is a single card.
 */
static inline bool joker_batch5_trading_card_hit(int discards_used, int nb_discarded_cards)
{
    return discards_used == 1 && nb_discarded_cards == 1;
}

/**
 * @brief Vagabond: the player has $4 or less.
 */
static inline bool joker_batch5_vagabond_hit(int money)
{
    return money <= VAGABOND_MONEY_LIMIT;
}

/**
 * @brief Popcorn: mult left after a round, 0 or less means it is eaten up.
 */
static inline int32_t joker_batch5_popcorn_next(int32_t mult)
{
    return mult - POPCORN_ROUND_LOSS;
}

/**
 * @brief Ramen: percent left after discarding @p nb_cards, below X1 it is eaten up.
 */
static inline int32_t joker_batch5_ramen_discard(int32_t percent, int nb_cards)
{
    return percent - RAMEN_CARD_LOSS * (nb_cards > 0 ? nb_cards : 0);
}

static inline bool joker_batch5_ramen_gone(int32_t percent)
{
    return percent < RAMEN_MIN_PERCENT;
}

/**
 * @brief Castle keeps its suit above the +chips counter in persistent_state.
 */
static inline int32_t joker_batch5_castle_pack(int suit, int32_t chips)
{
    if (chips > CASTLE_CHIPS_MAX)
        chips = CASTLE_CHIPS_MAX;
    return (int32_t)(((uint32_t)(suit % NUM_SUITS) << 24) | (uint32_t)chips);
}
static inline int joker_batch5_castle_suit(int32_t state)
{
    return (int)(((uint32_t)state >> 24) & 0xFF) % NUM_SUITS;
}
static inline int32_t joker_batch5_castle_chips(int32_t state)
{
    return (int32_t)((uint32_t)state & 0xFFFFFF);
}

/**
 * @brief Castle: +3 chips for each discarded card of its suit, the suit is kept.
 */
static inline int32_t joker_batch5_castle_discard(int32_t state, int cards_of_suit)
{
    int32_t chips = joker_batch5_castle_chips(state) + CASTLE_CHIPS_PER_CARD * cards_of_suit;
    return joker_batch5_castle_pack(joker_batch5_castle_suit(state), chips);
}

/**
 * @brief Mail-In Rebate: $5 per discarded card of the chosen rank.
 */
static inline int joker_batch5_mail_in_money(int cards_of_rank)
{
    return MAIL_IN_REBATE_MONEY * cards_of_rank;
}

/**
 * @brief Obelisk: number of times @p hand_type was played, a Royal Flush counts as a Straight Flush.
 *        @p played is indexed with hand_type - 1 and has HAND_TYPE_MAX elements.
 */
static inline uint32_t joker_batch5_played_count(const uint32_t* played, int hand_type)
{
    if (hand_type == ROYAL_FLUSH)
        hand_type = STRAIGHT_FLUSH;
    uint32_t count = played[hand_type - 1];
    if (hand_type == STRAIGHT_FLUSH)
        count += played[ROYAL_FLUSH - 1];
    return count;
}

/**
 * @brief Obelisk: whether @p hand_type is strictly the most played hand, a tie with another hand
 *        does not count. The hand being played is already counted in @p played.
 */
static inline bool joker_batch5_is_most_played(const uint32_t* played, int hand_type)
{
    if (hand_type <= NONE || hand_type > HAND_TYPE_MAX)
        return false;

    uint32_t current = joker_batch5_played_count(played, hand_type);
    for (int other = HIGH_CARD; other <= HAND_TYPE_MAX; other++)
    {
        // Royal Flush is counted with the Straight Flush, and a hand is not its own rival
        if (other == ROYAL_FLUSH || other == hand_type ||
            (hand_type == ROYAL_FLUSH && other == STRAIGHT_FLUSH))
            continue;
        if (joker_batch5_played_count(played, other) >= current)
            return false;
    }
    return true;
}

/**
 * @brief Obelisk: consecutive hands without the most played hand after playing @p hand_type.
 */
static inline int32_t joker_batch5_obelisk_next(int32_t streak, const uint32_t* played, int hand_type)
{
    if (hand_type <= NONE || hand_type > HAND_TYPE_MAX)
        return streak;
    return joker_batch5_is_most_played(played, hand_type) ? 0 : streak + 1;
}

static inline int joker_batch5_obelisk_percent(int32_t streak)
{
    return 100 + OBELISK_STEP_PERCENT * (streak > 0 ? streak : 0);
}

/**
 * @brief Turtle Bean: hand size bonus left after a round, 0 or less means it is eaten up.
 */
static inline int32_t joker_batch5_turtle_bean_next(int32_t bonus)
{
    return bonus - 1;
}

/**
 * @brief Ceremonial Dagger: mult gained by destroying a Joker with the given sell value.
 */
static inline int32_t joker_batch5_dagger_gain(int sell_value)
{
    return sell_value > 0 ? DAGGER_VALUE_FACTOR * sell_value : 0;
}

static inline int32_t joker_batch5_dagger_add(int32_t mult, int sell_value)
{
    int32_t total = mult + joker_batch5_dagger_gain(sell_value);
    return total > DAGGER_MULT_MAX ? DAGGER_MULT_MAX : total;
}

/**
 * @brief Cloud 9: $1 for each 9 in the full deck.
 */
static inline int joker_batch5_cloud_nine_money(int nines)
{
    return CLOUD_NINE_MONEY * (nines > 0 ? nines : 0);
}

/**
 * @brief How many cards of a discard have each rank and suit (Mail-In Rebate, Castle).
 */
typedef struct DiscardCounts
{
    int nb_cards;
    int nb_face_cards;
    int nb_jacks;
    uint8_t ranks[NUM_RANKS];
    uint8_t suits[NUM_SUITS];
} DiscardCounts;

/**
 * @brief Adds a discarded card. Wild cards count once for every suit, Stone cards for no rank
 *        and no suit.
 */
static inline void joker_batch5_discard_add(DiscardCounts* counts, const Card* card, bool is_face)
{
    counts->nb_cards++;
    if (is_face)
        counts->nb_face_cards++;
    if (card_has_rank(card) && card->rank == JACK)
        counts->nb_jacks++;
    card_add_to_distribution(card, counts->ranks, counts->suits);
}

#endif // JOKER_BATCH5_H
