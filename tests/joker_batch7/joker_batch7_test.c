#include <assert.h>
#include <card_rules.h>
#include <joker_batch7.h>
#include <stdio.h>

static void test_xmult_counters(void)
{
    assert(joker_batch7_throwback_percent(0) == 100);
    assert(joker_batch7_throwback_percent(4) == 200);
    assert(joker_batch7_throwback_percent(-1) == 100);
    assert(joker_batch7_throwback_percent(10000000) == 100 + 99975);

    assert(joker_batch7_madness_percent(0) == 100);
    assert(joker_batch7_madness_percent(3) == 250);
    assert(joker_batch7_madness_percent(-5) == 100);

    assert(joker_batch7_hologram_percent(0) == 100);
    assert(joker_batch7_hologram_percent(5) == 225);
    assert(joker_batch7_hologram_percent(-3) == 100);
    assert(joker_batch7_hologram_percent(10000000) == 100 + 99975);

    assert(joker_batch7_campfire_percent(0) == 100);
    assert(joker_batch7_campfire_percent(6) == 250);
    assert(joker_batch7_campfire_percent(10000000) == 100 + 99975);
}

static void test_madness_campfire(void)
{
    assert(joker_batch7_madness_triggers(false));
    assert(!joker_batch7_madness_triggers(true));
    assert(joker_batch7_madness_pick(7, 0) == -1);
    assert(joker_batch7_madness_pick(7, 3) == 1);
    assert(joker_batch7_campfire_after_round(5, false) == 5);
    assert(joker_batch7_campfire_after_round(5, true) == 0);
}

static void test_riff_raff_satellite(void)
{
    assert(joker_batch7_riff_raff_count(0) == 0);
    assert(joker_batch7_riff_raff_count(-1) == 0);
    assert(joker_batch7_riff_raff_count(1) == 1);
    assert(joker_batch7_riff_raff_count(4) == 2);

    uint32_t mask = 0;
    assert(joker_batch7_satellite_money(mask) == 0);
    mask = joker_batch7_planet_mask_add(mask, 0);
    mask = joker_batch7_planet_mask_add(mask, 0);
    mask = joker_batch7_planet_mask_add(mask, 5);
    assert(joker_batch7_satellite_money(mask) == 2);
    mask = joker_batch7_planet_mask_add(mask, 12);
    mask = joker_batch7_planet_mask_add(mask, -1);
    assert(joker_batch7_satellite_money(mask) == 2);
    for (int i = 0; i < 12; i++)
        mask = joker_batch7_planet_mask_add(mask, i);
    assert(joker_batch7_satellite_money(mask) == 12);
}

static void test_burnt_invisible_showman(void)
{
    assert(joker_batch7_burnt_target(PAIR, true) == PAIR);
    assert(joker_batch7_burnt_target(PAIR, false) == NONE);
    assert(joker_batch7_burnt_target(NONE, true) == NONE);
    assert(joker_batch7_burnt_target(ROYAL_FLUSH, true) == STRAIGHT_FLUSH);

    assert(joker_batch7_invisible_next(0) == 1);
    assert(joker_batch7_invisible_next(1) == 2);
    assert(joker_batch7_invisible_next(2) == 2);
    assert(joker_batch7_invisible_next(-4) == 1);
    assert(!joker_batch7_invisible_ready(1));
    assert(joker_batch7_invisible_ready(2));

    assert(joker_batch7_showman_locks_roll(false));
    assert(!joker_batch7_showman_locks_roll(true));
}

static void test_smeared(void)
{
    Card club = {.suit = CLUBS}, heart = {.suit = HEARTS}, diamond = {.suit = DIAMONDS}, spade = {.suit = SPADES};
    g_card_rules_smeared = false;
    assert(!card_has_suit(&club, SPADES) && !card_has_suit(&heart, DIAMONDS));
    g_card_rules_smeared = true;
    assert(card_has_suit(&club, SPADES) && card_has_suit(&spade, CLUBS));
    assert(card_has_suit(&heart, DIAMONDS) && card_has_suit(&diamond, HEARTS));
    assert(!card_has_suit(&club, HEARTS) && !card_has_suit(&diamond, SPADES));
    g_card_rules_smeared = false;
}

int main(void)
{
    test_xmult_counters();
    test_madness_campfire();
    test_riff_raff_satellite();
    test_burnt_invisible_showman();
    test_smeared();
    printf("joker_batch7: all tests passed\n");
    return 0;
}
