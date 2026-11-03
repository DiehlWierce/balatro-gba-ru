#include <assert.h>
#include <joker_economy.h>
#include <stdio.h>
#include <string.h>

static JokerRunBonus bonus_of(const int* ids, const int* states, int n)
{
    JokerRunBonus bonus;
    memset(&bonus, 0, sizeof(bonus));
    for (int i = 0; i < n; i++)
        joker_run_bonus_add(&bonus, ids[i], states ? states[i] : 0);
    return bonus;
}

static void test_run_limits(void)
{
    const int ids[] = {JUGGLER_JOKER_ID, DRUNKARD_JOKER_ID, TROUBADOUR_JOKER_ID};
    JokerRunBonus bonus = bonus_of(ids, NULL, 3);
    assert(bonus.hand_size == 3);
    assert(bonus.discards == 1);
    assert(bonus.hands == -1);
    assert(joker_economy_limit(8, bonus.hand_size, 1) == 11);
    assert(joker_economy_limit(4, bonus.hands, 1) == 3);

    // Selling the Troubadour rolls its bonus back, nothing was written to the base values
    bonus = bonus_of(ids, NULL, 2);
    assert(bonus.hand_size == 1 && bonus.hands == 0 && bonus.discards == 1);

    const int penalty[] = {MERRY_ANDY_JOKER_ID, STUNTMAN_JOKER_ID};
    bonus = bonus_of(penalty, NULL, 2);
    assert(bonus.discards == 3 && bonus.hand_size == -3);
    assert(joker_economy_limit(2, bonus.hand_size, 1) == 1); // never below the minimum
    assert(joker_economy_limit(1, -5, 0) == 0);
}

static void test_interest(void)
{
    // $5 cap, $1 per $5
    assert(joker_economy_interest(0, 1, 5, 0) == 0);
    assert(joker_economy_interest(-10, 1, 5, 0) == 0);
    assert(joker_economy_interest(4, 1, 5, 0) == 0);
    assert(joker_economy_interest(14, 1, 5, 0) == 2);
    assert(joker_economy_interest(100, 1, 5, 0) == 5);
    // One To the Moon: $2 per $5, cap $10
    assert(joker_economy_interest(14, 1, 5, 1) == 4);
    assert(joker_economy_interest(100, 1, 5, 1) == 10);
    // Two of them
    assert(joker_economy_interest(100, 1, 5, 2) == 15);
    // No interest deck stays at 0
    assert(joker_economy_interest(100, 1, 0, 1) == 0);
    // Seed Money style cap
    assert(joker_economy_interest(100, 1, 10, 1) == 20);
}

static void test_round_end_money(void)
{
    const int ids[] = {GOLDEN_JOKER_ID, ROCKET_JOKER_ID, DELAYED_GRATIFICATION_ID};
    const int states[] = {0, 3, 0};
    JokerRunBonus bonus = bonus_of(ids, states, 3);
    assert(bonus.round_end_money == 4 + 3);
    // Delayed Gratification: only when no discards were used
    assert(joker_economy_round_end_money(&bonus, 3, 3) == 7 + 2 * 3);
    assert(joker_economy_round_end_money(&bonus, 2, 3) == 7);
    assert(joker_economy_round_end_money(&bonus, 0, 0) == 7);
    // Extra discards (Drunkard) are counted too
    assert(joker_economy_round_end_money(&bonus, 4, 4) == 7 + 2 * 4);

    const int two[] = {DELAYED_GRATIFICATION_ID, DELAYED_GRATIFICATION_ID};
    bonus = bonus_of(two, NULL, 2);
    assert(joker_economy_round_end_money(&bonus, 3, 3) == 12);
}

static void test_debt(void)
{
    const int ids[] = {CREDIT_CARD_JOKER_ID, CREDIT_CARD_JOKER_ID};
    assert(bonus_of(ids, NULL, 0).debt_limit == 0);
    assert(bonus_of(ids, NULL, 1).debt_limit == 20);
    assert(bonus_of(ids, NULL, 2).debt_limit == 40);
}

static void test_values(void)
{
    assert(joker_economy_self_value_gain(EGG_JOKER_ID) == 6);
    assert(joker_economy_self_value_gain(GIFT_CARD_JOKER_ID) == 0);
    assert(joker_economy_others_value_gain(GIFT_CARD_JOKER_ID) == 2);
    assert(joker_economy_others_value_gain(EGG_JOKER_ID) == 0);
    // Sell value is value / 2: an Egg bought for 4 sells for 2, then 5, 8, ...
    int value = 4;
    assert(value / 2 == 2);
    value = joker_economy_add_value(value, joker_economy_self_value_gain(EGG_JOKER_ID));
    assert(value / 2 == 5);
    value = joker_economy_add_value(250, 6);
    assert(value == JOKER_VALUE_MAX);

    assert(joker_economy_rocket_next(1, false) == 1);
    assert(joker_economy_rocket_next(1, true) == 3);

    // Hand size: Painted Deck (10) + Juggler + Troubadour + Skip Tag never go past the hand arrays
    assert(joker_economy_hand_size(8, 3, 16) == 11);
    assert(joker_economy_hand_size(16, 3, 16) == 16);
    assert(joker_economy_hand_size(8, -9, 16) == 1);
}

int main(void)
{
    test_run_limits();
    test_interest();
    test_round_end_money();
    test_debt();
    test_values();
    printf("joker_economy: all tests passed\n");
    return 0;
}
