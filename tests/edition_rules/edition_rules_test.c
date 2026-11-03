#include <assert.h>
#include <edition_rules.h>
#include <stdio.h>

static void test_names_and_prices(void)
{
    assert(edition_get_name(EDITION_NONE) == NULL);
    assert(edition_get_name(EDITION_MAX) == NULL);
    for (int e = EDITION_FOIL; e < EDITION_MAX; e++)
        assert(edition_get_name(e) != NULL);

    assert(edition_extra_price(EDITION_NONE) == 0);
    assert(edition_extra_price(EDITION_FOIL) == 2);
    assert(edition_extra_price(EDITION_HOLOGRAPHIC) == 3);
    assert(edition_extra_price(EDITION_POLYCHROME) == 5);
    assert(edition_extra_price(EDITION_NEGATIVE) == 5);

    assert(edition_has_score_effect(EDITION_FOIL));
    assert(edition_has_score_effect(EDITION_HOLOGRAPHIC));
    assert(edition_has_score_effect(EDITION_POLYCHROME));
    assert(!edition_has_score_effect(EDITION_NEGATIVE));
    assert(!edition_has_score_effect(EDITION_NONE));
}

static void test_polychrome_math(void)
{
    assert(edition_polychrome_apply(0) == 0);
    assert(edition_polychrome_apply(10) == 15);
    assert(edition_polychrome_apply(1) == 2); // Rounded to the nearest, like Steel
    assert(edition_polychrome_apply(3) == 5);
    assert(edition_polychrome_apply(UINT32_MAX) == UINT32_MAX); // Saturates
    assert(edition_polychrome_apply(UINT32_MAX - 1) == UINT32_MAX);
}

static int count_editions(int rate, bool allow_negative, int counts[EDITION_MAX])
{
    for (int e = 0; e < EDITION_MAX; e++)
        counts[e] = 0;
    for (uint32_t roll = 0; roll < EDITION_ROLL_RANGE; roll++)
        counts[edition_from_roll(roll, rate, allow_negative)]++;
    return 0;
}

static void test_roll_chances(void)
{
    int counts[EDITION_MAX];

    // Rate 1: 0.3% Negative, 0.3% Polychrome, 1.4% Holographic, 2% Foil
    count_editions(1, true, counts);
    assert(counts[EDITION_NEGATIVE] == 3);
    assert(counts[EDITION_POLYCHROME] == 3);
    assert(counts[EDITION_HOLOGRAPHIC] == 14);
    assert(counts[EDITION_FOIL] == 20);
    assert(counts[EDITION_NONE] == 960);

    // Hone doubles and Glow Up quadruples the chances of the other editions, not Negative's:
    // the thresholds of Polychrome, Holographic and Foil (0.6%, 2%, 4%) are what scales
    count_editions(2, true, counts);
    assert(counts[EDITION_NEGATIVE] == 3 && counts[EDITION_POLYCHROME] == 9);
    assert(counts[EDITION_HOLOGRAPHIC] == 28 && counts[EDITION_FOIL] == 40);
    assert(counts[EDITION_NONE] == 920);
    count_editions(4, true, counts);
    assert(counts[EDITION_NEGATIVE] == 3 && counts[EDITION_POLYCHROME] == 21);
    assert(counts[EDITION_HOLOGRAPHIC] == 56 && counts[EDITION_FOIL] == 80);
    assert(counts[EDITION_NONE] == 840);

    // Playing cards never get Negative, its band becomes Polychrome
    count_editions(4, false, counts);
    assert(counts[EDITION_NEGATIVE] == 0);
    assert(counts[EDITION_POLYCHROME] == 24);
    assert(counts[EDITION_NONE] == 840);
    count_editions(1, false, counts);
    assert(counts[EDITION_POLYCHROME] == 6);

    // A rate below 1 counts as 1, the roll wraps around the range
    assert(edition_from_roll(0, 0, true) == EDITION_NEGATIVE);
    assert(edition_from_roll(EDITION_ROLL_RANGE, 1, true) == EDITION_NEGATIVE);
    assert(edition_from_roll(3, 1, true) == EDITION_POLYCHROME);
    assert(edition_from_roll(6, 1, true) == EDITION_HOLOGRAPHIC);
    assert(edition_from_roll(20, 1, true) == EDITION_FOIL);
    assert(edition_from_roll(40, 1, true) == EDITION_NONE);
}

static void test_wheel(void)
{
    // 1 in 4
    for (uint32_t roll = 0; roll < 16; roll++)
        assert(edition_wheel_succeeds(roll) == (roll % 4 == 0));

    // Foil 50%, Holographic 35%, Polychrome 15%, never Negative
    int counts[EDITION_MAX] = {0};
    for (uint32_t roll = 0; roll < 100; roll++)
        counts[edition_wheel_edition(roll)]++;
    assert(counts[EDITION_FOIL] == 50);
    assert(counts[EDITION_HOLOGRAPHIC] == 35);
    assert(counts[EDITION_POLYCHROME] == 15);
    assert(counts[EDITION_NEGATIVE] == 0 && counts[EDITION_NONE] == 0);
}

static void test_wheel_pick_joker(void)
{
    const uint8_t none[3] = {EDITION_NONE, EDITION_NONE, EDITION_NONE};
    assert(edition_wheel_pick_joker(none, 3, 0) == 0);
    assert(edition_wheel_pick_joker(none, 3, 1) == 1);
    assert(edition_wheel_pick_joker(none, 3, 5) == 2);

    // Jokers with an edition are never picked
    const uint8_t mixed[4] = {EDITION_FOIL, EDITION_NONE, EDITION_NEGATIVE, EDITION_NONE};
    for (uint32_t roll = 0; roll < 8; roll++)
    {
        int idx = edition_wheel_pick_joker(mixed, 4, roll);
        assert(idx == 1 || idx == 3);
        assert(idx == (roll % 2 == 0 ? 1 : 3));
    }

    const uint8_t all[2] = {EDITION_FOIL, EDITION_POLYCHROME};
    assert(edition_wheel_pick_joker(all, 2, 0) == -1);
    assert(edition_wheel_pick_joker(none, 0, 0) == -1); // No Jokers at all
}

static void test_negative_joker_slot(void)
{
    const uint8_t two[2] = {EDITION_NONE, EDITION_FOIL};

    // 2 of 2 slots used: a normal Joker doesn't fit, a Negative one does
    assert(!edition_joker_fits(two, 2, 2, 7, EDITION_NONE));
    assert(!edition_joker_fits(two, 2, 2, 7, EDITION_POLYCHROME));
    assert(edition_joker_fits(two, 2, 2, 7, EDITION_NEGATIVE));
    assert(edition_joker_fits(two, 2, 3, 7, EDITION_NONE));

    // An owned Negative Joker frees a slot for a normal one
    const uint8_t with_negative[3] = {EDITION_NONE, EDITION_NEGATIVE, EDITION_FOIL};
    assert(edition_joker_fits(with_negative, 3, 3, 7, EDITION_NONE));
    assert(!edition_joker_fits(with_negative, 3, 2, 7, EDITION_NONE));

    // The row never exceeds its capacity, Negative or not
    const uint8_t full[3] = {EDITION_NEGATIVE, EDITION_NEGATIVE, EDITION_NEGATIVE};
    assert(!edition_joker_fits(full, 3, 5, 3, EDITION_NEGATIVE));
    assert(!edition_joker_fits(full, 3, 5, 3, EDITION_NONE));
    assert(edition_joker_fits(full, 3, 5, 4, EDITION_NONE));
    assert(edition_joker_fits(NULL, 0, 1, 7, EDITION_NONE));
    assert(!edition_joker_fits(NULL, 0, 0, 7, EDITION_NONE)); // No slots at all
}

int main(void)
{
    test_names_and_prices();
    test_polychrome_math();
    test_roll_chances();
    test_wheel();
    test_wheel_pick_joker();
    test_negative_joker_slot();
    printf("edition_rules: all tests passed\n");
    return 0;
}
