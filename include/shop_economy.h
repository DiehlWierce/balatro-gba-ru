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

#endif // SHOP_ECONOMY_H
