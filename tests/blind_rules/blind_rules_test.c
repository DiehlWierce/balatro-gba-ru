#include <assert.h>
#include <blind_rules.h>
#include <stdio.h>

static int count_eligible(bool showdown, int ante)
{
    uint8_t types[BLIND_TYPE_MAX];
    int n = 0;
    for (int t = showdown ? BLIND_TYPE_SHOWDOWN : BLIND_TYPE_BOSS;
         t < (showdown ? BLIND_TYPE_MAX : BLIND_TYPE_SHOWDOWN);
         t++)
        types[n++] = t;
    int eligible = 0;
    blind_rules_pick(types, n, ante, 0, &eligible);
    return eligible;
}

static void test_min_ante(void)
{
    // Ante 1: Hook, Club, Psychic, Goad, Window, Manacle, Pillar, Head
    assert(count_eligible(false, 1) == 8);
    assert(blind_rules_can_roll(BLIND_TYPE_HOOK, 1));
    assert(!blind_rules_can_roll(BLIND_TYPE_OX, 1));
    assert(!blind_rules_can_roll(BLIND_TYPE_PLANT, 1));
    assert(!blind_rules_can_roll(BLIND_TYPE_SERPENT, 1));
    assert(!blind_rules_can_roll(BLIND_TYPE_WALL, 1));
    // Ante 2 adds House, Wall, Wheel, Arm, Fish, Water, Mouth, Needle, Flint, Mark
    assert(count_eligible(false, 2) == 18);
    // Ante 3: Eye, Tooth; 4: Plant; 5: Serpent; 6: Ox
    assert(count_eligible(false, 3) == 20);
    assert(count_eligible(false, 4) == 21);
    assert(count_eligible(false, 5) == 22);
    assert(count_eligible(false, 6) == 23);
    // Ante 0 counts as ante 1
    assert(count_eligible(false, 0) == 8);
    // Showdowns only from ante 8
    assert(count_eligible(true, 7) == 0);
    assert(count_eligible(true, 8) == 5);
    // Every Boss is eligible at the last ante
    for (int t = BLIND_TYPE_BOSS; t < BLIND_TYPE_MAX; t++)
        assert(blind_rules_can_roll(t, 12));
}

static void test_pick(void)
{
    uint8_t types[] = {BLIND_TYPE_OX, BLIND_TYPE_HOOK, BLIND_TYPE_PLANT, BLIND_TYPE_CLUB};
    int eligible;
    // Only Hook and Club at ante 1, in list order
    assert(blind_rules_pick(types, 4, 1, 0, &eligible) == BLIND_TYPE_HOOK && eligible == 2);
    assert(blind_rules_pick(types, 4, 1, 1, NULL) == BLIND_TYPE_CLUB);
    assert(blind_rules_pick(types, 4, 1, 2, NULL) == BLIND_TYPE_HOOK);
    assert(blind_rules_pick(types, 4, 6, 0, NULL) == BLIND_TYPE_OX);
    assert(blind_rules_pick(types, 0, 1, 0, &eligible) == -1 && eligible == 0);
    uint8_t late[] = {BLIND_TYPE_SERPENT};
    assert(blind_rules_pick(late, 1, 3, 5, &eligible) == -1 && eligible == 0);
}

static void test_requirement_mult(void)
{
    assert(blind_rules_req_mult_fx8(BLIND_TYPE_SMALL) == 256);
    assert(blind_rules_req_mult_fx8(BLIND_TYPE_BIG) == 384);
    // The Needle asks as much as the Small Blind
    assert(blind_rules_req_mult_fx8(BLIND_TYPE_NEEDLE) == 256);
    assert(blind_rules_req_mult_fx8(BLIND_TYPE_WALL) == 1024);
    assert(blind_rules_req_mult_fx8(BLIND_TYPE_VESSEL) == 1536);
    assert(blind_rules_req_mult_fx8(BLIND_TYPE_HOOK) == 512);
    assert(blind_rules_req_mult_fx8(BLIND_TYPE_BELL) == 512);
}

int main(void)
{
    test_min_ante();
    test_pick();
    test_requirement_mult();
    printf("blind_rules: all tests passed\n");
    return 0;
}
