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

static void test_discounted_price(void)
{
    assert(shop_economy_discounted_price(10, 0) == 10);
    assert(shop_economy_discounted_price(10, 25) == 7);  // Voucher, Booster Pack: $10 -> $7
    assert(shop_economy_discounted_price(10, 50) == 5);
    assert(shop_economy_discounted_price(1, 50) == 1);   // never free
    assert(shop_economy_discounted_price(0, 50) == 0);
    assert(shop_economy_discounted_price(4, 25) == 3);
    assert(shop_economy_discounted_price(6, 50) == 3);
}

static void test_sell_price(void)
{
    assert(shop_economy_sell_price(6, 0, 0) == 3);
    assert(shop_economy_sell_price(7, 0, 0) == 3);
    assert(shop_economy_sell_price(1, 0, 0) == 1);  // at least $1
    assert(shop_economy_sell_price(2, 0, 0) == 1);
    assert(shop_economy_sell_price(10, 50, 0) == 2); // bought for $5: sold for $2
    assert(shop_economy_sell_price(10, 25, 0) == 3); // bought for $7
    assert(shop_economy_sell_price(10, 0, 3) == 8);  // Gift Card extra value
}

static int count_kinds(int joker, int cons, int pack, int card, int kinds[4])
{
    int total = joker + cons + pack + card;
    for (int i = 0; i < 4; i++)
        kinds[i] = 0;
    for (int r = 0; r < total; r++)
        kinds[shop_economy_top_row_kind(joker, cons, pack, card, (uint32_t)r)]++;
    return total;
}

static void test_top_row_weights(void)
{
    // Default: Joker 20, Tarot 4 + Planet 4, no Magic Trick, no Ghost Deck
    int cons = shop_economy_consumable_weight_x10(40, 40, false);
    assert(cons == 80);
    assert(shop_economy_consumable_weight_x10(40, 40, true) == 100); // Spectral 2
    assert(shop_economy_consumable_weight_x10(96, 40, false) == 136); // Tarot Merchant

    int kinds[4];
    int total = count_kinds(SHOP_JOKER_WEIGHT_X10, cons, 0, 0, kinds);
    assert(total == 280);
    assert(kinds[SHOP_TOP_ROW_JOKER] == 200);
    assert(kinds[SHOP_TOP_ROW_CONSUMABLE] == 80); // 8 / 28, not 4 / 24
    assert(kinds[SHOP_TOP_ROW_CARD] == 0);

    // Packs keep about a quarter of the rolls
    total = count_kinds(SHOP_JOKER_WEIGHT_X10, cons, SHOP_PACK_WEIGHT_X10, 0, kinds);
    assert(kinds[SHOP_TOP_ROW_PACK] * 100 / total >= 24);
    assert(kinds[SHOP_TOP_ROW_PACK] * 100 / total <= 26);

    // Magic Trick: playing cards weigh 4
    total = count_kinds(SHOP_JOKER_WEIGHT_X10, cons, 0, 40, kinds);
    assert(kinds[SHOP_TOP_ROW_CARD] == 40);
}

int main(void)
{
    test_joker_prices();
    test_reroll_base_cost();
    test_reroll_voucher_midshop();
    test_reroll_progression();
    test_chaos_the_clown();
    test_free_reroll_at_zero_cost();
    test_discounted_price();
    test_sell_price();
    test_top_row_weights();
    puts("shop_economy: all tests passed");
    return 0;
}
