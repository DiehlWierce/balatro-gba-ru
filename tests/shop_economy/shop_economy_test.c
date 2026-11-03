#include <assert.h>
#include <shop_economy.h>
#include <stdio.h>

static void test_joker_prices(void)
{
    assert(shop_economy_joker_buy_price(6, false) == 6);
    assert(shop_economy_joker_sell_value(6, false) == 3);
    assert(shop_economy_joker_sell_value(7, false) == 3); // rounds down
    assert(shop_economy_joker_sell_value(1, false) == 0);

    // Rental Jokers: bought for $1 and sold for $1 whatever their value
    assert(shop_economy_joker_buy_price(8, true) == RENTAL_JOKER_BUY_PRICE);
    assert(shop_economy_joker_sell_value(8, true) == 1);
}

static void test_reroll_base_cost(void)
{
    assert(shop_economy_reroll_base_cost(0) == 5);
    assert(shop_economy_reroll_base_cost(2) == 3); // Reroll Surplus
    assert(shop_economy_reroll_base_cost(4) == 1); // + Reroll Glut
    assert(shop_economy_reroll_base_cost(9) == 0); // never negative
}

static void test_reroll_voucher_midshop(void)
{
    assert(shop_economy_reroll_cost_after_voucher(5) == 3);
    assert(shop_economy_reroll_cost_after_voucher(7) == 5); // keeps the increase of the rerolls
    assert(shop_economy_reroll_cost_after_voucher(1) == 0);
    assert(shop_economy_reroll_cost_after_voucher(0) == 0);
}

static void test_reroll_progression(void)
{
    bool used = false;
    int cost = SHOP_REROLL_BASE_COST;
    cost = shop_economy_reroll_cost_after_reroll(cost, 5, &used);
    assert(cost == 6 && !used);
    cost = shop_economy_reroll_cost_after_reroll(cost, 6, &used);
    assert(cost == 7 && !used);
}

static void test_chaos_the_clown(void)
{
    bool used = false;
    int cost = 5;

    // Without the Joker the reroll is paid in full
    assert(shop_economy_current_reroll_cost(cost, false, used) == 5);
    // With it the first reroll of the Shop is free...
    assert(shop_economy_current_reroll_cost(cost, true, used) == 0);
    cost = shop_economy_reroll_cost_after_reroll(cost, 0, &used);
    // ...and doesn't raise the price
    assert(cost == 5 && used);
    assert(shop_economy_current_reroll_cost(cost, true, used) == 5);
    cost = shop_economy_reroll_cost_after_reroll(cost, 5, &used);
    assert(cost == 6);
}

static void test_free_reroll_at_zero_cost(void)
{
    // At $0 a reroll is free anyway: it doesn't use up the Chaos the Clown one and raises the price
    bool used = false;
    int cost = shop_economy_reroll_cost_after_reroll(0, 0, &used);
    assert(cost == 1 && !used);

    // A used free reroll is not given twice
    used = true;
    cost = shop_economy_reroll_cost_after_reroll(5, 0, &used);
    assert(cost == 6 && used);
}

int main(void)
{
    test_joker_prices();
    test_reroll_base_cost();
    test_reroll_voucher_midshop();
    test_reroll_progression();
    test_chaos_the_clown();
    test_free_reroll_at_zero_cost();
    puts("shop_economy: all tests passed");
    return 0;
}
