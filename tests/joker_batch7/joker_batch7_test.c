#include <assert.h>
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

int main(void)
{
    test_xmult_counters();
    test_madness_campfire();
    printf("joker_batch7: all tests passed\n");
    return 0;
}
