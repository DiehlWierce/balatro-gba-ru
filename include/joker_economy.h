/**
 * @file joker_economy.h
 *
 * @brief Pure rules of the Jokers that change money, hands, discards, hand size and Joker values
 *        (A8 batch 2). Plain C without libtonc so it can be tested on the host (tests/joker_economy).
 *
 * The run limits are never modified by these Jokers: the bonus is recomputed from the owned Jokers
 * every time it is needed and added to the base value in g_game_vars, so selling a Joker rolls its
 * effect back by itself.
 */
#ifndef JOKER_ECONOMY_H
#define JOKER_ECONOMY_H

#include "joker_batch5.h"

#include <stdbool.h>

#define GOLDEN_JOKER_ID           64
#define DELAYED_GRATIFICATION_ID  65
#define TO_THE_MOON_JOKER_ID      66
#define ROCKET_JOKER_ID           67
#define EGG_JOKER_ID              68
#define CREDIT_CARD_JOKER_ID      69
#define JUGGLER_JOKER_ID          70
#define DRUNKARD_JOKER_ID         71
#define TROUBADOUR_JOKER_ID       72
#define MERRY_ANDY_JOKER_ID       73
#define STUNTMAN_JOKER_ID         74
#define GIFT_CARD_JOKER_ID        75
#define SWASHBUCKLER_JOKER_ID     76
#define GOLDEN_TICKET_JOKER_ID    77
#define ROUGH_GEM_JOKER_ID        78

#define GOLDEN_JOKER_MONEY        4
#define DELAYED_GRATIFICATION_PER_DISCARD 2
#define CREDIT_CARD_DEBT          20
#define EGG_VALUE_GAIN            6 // The sell value is value / 2, so this is +$3
#define GIFT_CARD_VALUE_GAIN      2 // +$1 sell value
#define ROCKET_START_MONEY        1
#define ROCKET_BOSS_GAIN          2 // Payout increase for every Boss Blind defeated
#define JOKER_VALUE_MAX           255

// Everything the owned Jokers add to the run, summed over all of them
typedef struct JokerRunBonus
{
    int hands;               // Added to the hands per round
    int discards;            // Added to the discards per round
    int hand_size;           // Added to the hand size
    int interest_extra;      // Added to the $ per $5 of interest (and multiplies the cap)
    int debt_limit;          // How far below $0 the money can go
    int round_end_money;     // $ at the end of the round (Golden Joker, Rocket)
    int delayed_gratification; // Number of Delayed Gratification Jokers
    int cloud_nine;          // Number of Cloud 9 Jokers (paid per 9 in the deck, see round_end.c)
} JokerRunBonus;

/**
 * @brief Adds the effect of one owned Joker to the bonus
 * @param persistent_state The Joker's persistent state (Rocket keeps its payout in it)
 */
static inline void joker_run_bonus_add(JokerRunBonus* bonus, int joker_id, int persistent_state)
{
    switch (joker_id)
    {
        case GOLDEN_JOKER_ID:
            bonus->round_end_money += GOLDEN_JOKER_MONEY;
            break;
        case DELAYED_GRATIFICATION_ID:
            bonus->delayed_gratification++;
            break;
        case TO_THE_MOON_JOKER_ID:
            bonus->interest_extra++;
            break;
        case ROCKET_JOKER_ID:
            bonus->round_end_money += persistent_state;
            break;
        case CREDIT_CARD_JOKER_ID:
            bonus->debt_limit += CREDIT_CARD_DEBT;
            break;
        case JUGGLER_JOKER_ID:
            bonus->hand_size += 1;
            break;
        case DRUNKARD_JOKER_ID:
            bonus->discards += 1;
            break;
        case TROUBADOUR_JOKER_ID:
            bonus->hand_size += 2;
            bonus->hands -= 1;
            break;
        case MERRY_ANDY_JOKER_ID:
            bonus->discards += 3;
            bonus->hand_size -= 1;
            break;
        case STUNTMAN_JOKER_ID:
            bonus->hand_size -= 2;
            break;
        case TURTLE_BEAN_JOKER_ID:
            bonus->hand_size += persistent_state > 0 ? persistent_state : 0;
            break;
        case CLOUD_NINE_JOKER_ID:
            bonus->cloud_nine++;
            break;
        default:
            break;
    }
}

/**
 * @brief Base value of a run limit plus the Joker bonus, never below @p min_value
 */
static inline int joker_economy_limit(int base, int bonus, int min_value)
{
    int value = base + bonus;
    return value < min_value ? min_value : value;
}

/**
 * @brief Hand size with the Joker bonus: at least 1 card, at most @p max_size (the hand arrays)
 */
static inline int joker_economy_hand_size(int base, int bonus, int max_size)
{
    int size = joker_economy_limit(base, bonus, 1);
    return size > max_size ? max_size : size;
}

/**
 * @brief Interest paid at the end of the round
 *
 * Every To the Moon adds $1 to the $1 per $5, and raises the cap by the same factor.
 * A cap of 0 (no interest at all) stays 0.
 */
static inline int joker_economy_interest(int money, int per_5, int cap, int interest_extra)
{
    if (money <= 0)
        return 0;
    int reward = (money / 5) * (per_5 + interest_extra);
    int max_reward = cap * (1 + interest_extra);
    return reward > max_reward ? max_reward : reward;
}

/**
 * @brief $ paid by Joker at the end of the round, shown as one row of the cash out
 * @param discards_left Discards the player still has
 * @param discards_per_round Discards the round started with
 */
static inline int joker_economy_round_end_money(
    const JokerRunBonus* bonus,
    int discards_left,
    int discards_per_round
)
{
    int money = bonus->round_end_money;
    if (discards_left > 0 && discards_left >= discards_per_round)
        money += bonus->delayed_gratification * DELAYED_GRATIFICATION_PER_DISCARD * discards_left;
    return money;
}

// Value added to the Joker itself at the end of the round (Egg)
static inline int joker_economy_self_value_gain(int joker_id)
{
    return joker_id == EGG_JOKER_ID ? EGG_VALUE_GAIN : 0;
}

// Value added to every other owned Joker at the end of the round by this Joker (Gift Card)
static inline int joker_economy_others_value_gain(int joker_id)
{
    return joker_id == GIFT_CARD_JOKER_ID ? GIFT_CARD_VALUE_GAIN : 0;
}

static inline int joker_economy_add_value(int value, int gain)
{
    value += gain;
    return value > JOKER_VALUE_MAX ? JOKER_VALUE_MAX : value;
}

// Rocket payout after the round: +$1 for every Boss Blind defeated
static inline int joker_economy_rocket_next(int payout, bool boss_defeated)
{
    return boss_defeated ? payout + ROCKET_BOSS_GAIN : payout;
}

#endif // JOKER_ECONOMY_H
