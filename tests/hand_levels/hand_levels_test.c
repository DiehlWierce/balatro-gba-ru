#include <assert.h>
#include <hand_levels.h>
#include <stdio.h>

static void check(enum HandType type, int level, uint32_t chips, uint32_t mult)
{
    HandLevelValues v = hand_level_values(type, level);
    if (v.chips != chips || v.mult != mult)
    {
        printf("hand %d level %d: got %u x %u, expected %u x %u\n", type, level, v.chips, v.mult,
               chips, mult);
        assert(0);
    }
}

static void test_level_one_is_base_value(void)
{
    check(HIGH_CARD, 1, 5, 1);
    check(PAIR, 1, 10, 2);
    check(TWO_PAIR, 1, 20, 2);
    check(THREE_OF_A_KIND, 1, 30, 3);
    check(STRAIGHT, 1, 30, 4);
    check(FLUSH, 1, 35, 4);
    check(FULL_HOUSE, 1, 40, 4);
    check(FOUR_OF_A_KIND, 1, 60, 7);
    check(STRAIGHT_FLUSH, 1, 100, 8);
    check(ROYAL_FLUSH, 1, 100, 8);
    check(FIVE_OF_A_KIND, 1, 120, 12);
    check(FLUSH_HOUSE, 1, 140, 14);
    check(FLUSH_FIVE, 1, 160, 16);
}

static void test_increments(void)
{
    // Level 2 values, as given by one Planet card in the original game
    check(HIGH_CARD, 2, 15, 2);
    check(PAIR, 2, 25, 3);
    check(TWO_PAIR, 2, 40, 3);
    check(THREE_OF_A_KIND, 2, 50, 5);
    check(STRAIGHT, 2, 60, 7);
    check(FLUSH, 2, 50, 6);
    check(FULL_HOUSE, 2, 65, 6);
    check(FOUR_OF_A_KIND, 2, 90, 10);
    check(STRAIGHT_FLUSH, 2, 140, 12);
    check(ROYAL_FLUSH, 2, 140, 12);
    check(FIVE_OF_A_KIND, 2, 155, 15);
    check(FLUSH_HOUSE, 2, 180, 18);
    check(FLUSH_FIVE, 2, 210, 19);

    check(PAIR, 5, 10 + 4 * 15, 2 + 4 * 1);
    check(FLUSH_FIVE, 10, 160 + 9 * 50, 16 + 9 * 3);
}

static void test_invalid_inputs(void)
{
    check(NONE, 1, 0, 0);
    check(NONE, 7, 0, 0);
    check((enum HandType)(HAND_TYPE_MAX + 1), 2, 0, 0);

    // Levels below the minimum are clamped
    check(PAIR, 0, 10, 2);
    check(PAIR, -3, 10, 2);

    HandLevelValues inc = hand_level_increment(STRAIGHT);
    assert(inc.chips == 30 && inc.mult == 3);
    inc = hand_level_increment(NONE);
    assert(inc.chips == 0 && inc.mult == 0);
}

int main(void)
{
    test_level_one_is_base_value();
    test_increments();
    test_invalid_inputs();
    printf("Hand Levels Tests Passed :)\n");
    return 0;
}
