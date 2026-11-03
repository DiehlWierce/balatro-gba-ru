/**
 * @file shop_economy.h
 *
 * @brief Pure Shop money rules: Joker buy and sell prices and the reroll cost. Plain C without
 *        libtonc so it can be tested on the host (tests/shop_economy).
 */
#ifndef SHOP_ECONOMY_H
#define SHOP_ECONOMY_H

#include "joker_batch6.h"

#include <stdbool.h>
#include <stdint.h>

#define SHOP_REROLL_BASE_COST        5 // Reroll cost at the start of each Shop
#define SHOP_REROLL_VOUCHER_DISCOUNT 2 // Reroll Surplus / Reroll Glut
#define RENTAL_JOKER_BUY_PRICE       1 // A Rental Joker is bought for this much

/**
 * @brief Buy price of a Joker with the given value: Rental Jokers always cost the minimum.
 */
static inline int shop_economy_joker_buy_price(int value, bool rental)
{
    return rental ? RENTAL_JOKER_BUY_PRICE : value;
}

/**
 * @brief Sell value of a Joker with the given value: half of it, $1 for Rental Jokers.
 */
static inline int shop_economy_joker_sell_value(int value, bool rental)
{
    return rental ? 1 : value / 2;
}

/**
 * @brief Reroll cost at the start of each Shop, lowered by the Vouchers and never below $0.
 */
static inline int shop_economy_reroll_base_cost(int voucher_discount)
{
    int cost = SHOP_REROLL_BASE_COST - voucher_discount;
    return (cost < 0) ? 0 : cost;
}

/**
 * @brief Reroll cost after redeeming Reroll Surplus / Reroll Glut in the middle of a Shop.
 */
static inline int shop_economy_reroll_cost_after_voucher(int cost)
{
    cost -= SHOP_REROLL_VOUCHER_DISCOUNT;
    return (cost < 0) ? 0 : cost;
}

/**
 * @brief What the next reroll costs: $0 with Chaos the Clown until its free reroll is used.
 */
static inline int shop_economy_current_reroll_cost(int cost, bool chaos_owned, bool free_used)
{
    return joker_batch6_chaos_free_reroll(chaos_owned, free_used) ? 0 : cost;
}

/**
 * @brief Updates the reroll cost after a reroll that was paid @p paid_cost. The free reroll
 *        (Chaos the Clown, or a Skip Tag) doesn't raise the price, but it is used up only if the
 *        cost was positive; every other reroll adds $1.
 *
 * @param cost the reroll cost before the reroll.
 * @param free_used in/out: whether the Chaos the Clown free reroll of this Shop was used.
 *
 * @return the reroll cost after the reroll.
 */
static inline int shop_economy_reroll_cost_after_reroll(int cost, int paid_cost, bool* free_used)
{
    if (paid_cost == 0 && cost > 0 && !*free_used)
    {
        *free_used = true;
        return cost;
    }
    return cost + 1;
}

/**
 * @brief Price with the Clearance Sale / Liquidation discount, as in Balatro:
 *        floor((price + 0.5) * (100 - discount) / 100), never below $1.
 *
 * @param discount_percent 0 (no Voucher), 25 or 50. Free (<= 0) prices stay as they are.
 */
static inline int shop_economy_discounted_price(int price, int discount_percent)
{
    if (discount_percent <= 0 || price <= 0)
        return price;

    int discounted = ((2 * price + 1) * (100 - discount_percent)) / 200;
    return (discounted < 1) ? 1 : discounted;
}

/**
 * @brief Sell price of a card bought in the Shop: max($1, floor(buy price / 2)) + the extra value
 *        (Gift Card), where the buy price is the one after the Clearance Sale / Liquidation
 *        discount. Callers: Jokers (joker.c) and consumables (consumable.c).
 *
 * @param base_price the price without discounts (the card's value).
 * @param discount_percent the current Shop discount (voucher_get_shop_discount_percent()).
 * @param extra extra sell value, e.g. Gift Card (0 if none).
 */
static inline int shop_economy_sell_price(int base_price, int discount_percent, int extra)
{
    int half = shop_economy_discounted_price(base_price, discount_percent) / 2;
    return ((half < 1) ? 1 : half) + extra;
}

// Weights of the Shop's top row item types, in tenths (Jokers 20, Tarots 4, Planets 4, playing
// cards 4 with Magic Trick, Spectral cards 2 with the Ghost Deck)
#define SHOP_JOKER_WEIGHT_X10   200
#define SHOP_SPECTRAL_WEIGHT_X10 20
// Booster Packs have their own slots in the original game. They share the top row here, and keep
// about a quarter of its rolls (28 / 3 = 9.33 with the default weights)
#define SHOP_PACK_WEIGHT_X10    93

enum ShopTopRowKind
{
    SHOP_TOP_ROW_JOKER,
    SHOP_TOP_ROW_CONSUMABLE,
    SHOP_TOP_ROW_PACK,
    SHOP_TOP_ROW_CARD
};

/**
 * @brief Weight of a consumable slot: Tarot + Planet (+ Spectral on the Ghost Deck), in tenths.
 *        The slot itself is then split between those types when rolled (consumable.c).
 */
static inline int shop_economy_consumable_weight_x10(int tarot_x10, int planet_x10, bool ghost_deck)
{
    return tarot_x10 + planet_x10 + (ghost_deck ? SHOP_SPECTRAL_WEIGHT_X10 : 0);
}

/**
 * @brief Item type of a top row slot for the random number @p r, with weights in tenths.
 *        Cumulative order: Joker, consumable, Pack, playing card. The total must not be 0.
 */
static inline enum ShopTopRowKind shop_economy_top_row_kind(
    int joker_x10,
    int consumable_x10,
    int pack_x10,
    int card_x10,
    uint32_t r
)
{
    uint32_t roll = r % (uint32_t)(joker_x10 + consumable_x10 + pack_x10 + card_x10);
    if (roll < (uint32_t)joker_x10)
        return SHOP_TOP_ROW_JOKER;
    roll -= (uint32_t)joker_x10;
    if (roll < (uint32_t)consumable_x10)
        return SHOP_TOP_ROW_CONSUMABLE;
    roll -= (uint32_t)consumable_x10;
    if (roll < (uint32_t)pack_x10)
        return SHOP_TOP_ROW_PACK;
    return SHOP_TOP_ROW_CARD;
}

#endif // SHOP_ECONOMY_H
