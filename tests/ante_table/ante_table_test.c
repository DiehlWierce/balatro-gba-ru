#include <ante_table.h>
#include <assert.h>
#include <stdio.h>

// Blind multipliers x256: Small 1, Big 1.5, Boss 2, The Wall 4, Violet Vessel 6
static const uint32_t MULTS[] = {256, 384, 512, 1024, 1536};

static void test_monotonic(void)
{
    for (int s = 0; s < ANTE_SCALING_COUNT; s++)
        for (int ante = 1; ante <= ANTE_LAST; ante++)
            assert(ante_table_base(s, ante) > ante_table_base(s, ante - 1));

    // Harder stakes never ask for less, and Boss > Big > Small of the same ante
    for (int ante = 0; ante <= ANTE_LAST; ante++)
    {
        assert(ante_table_base(0, ante) <= ante_table_base(1, ante));
        assert(ante_table_base(1, ante) <= ante_table_base(2, ante));
        uint32_t base = ante_table_base(0, ante);
        assert(ante_table_requirement(base, 512) >= ante_table_requirement(base, 384));
        assert(ante_table_requirement(base, 384) >= ante_table_requirement(base, 256));
    }
}

static void test_original_values(void)
{
    assert(ante_table_base(0, 8) == 50000);
    assert(ante_table_base(0, 9) == 110000);
    assert(ante_table_base(0, 12) == 300000000);
    assert(ante_table_base(2, 12) == 1200000000);
    assert(ante_table_base(0, -1) == 100);
    assert(ante_table_base(0, ANTE_LAST + 1) == 100);
    assert(ante_table_base(7, 3) == ante_table_base(2, 3));
}

static void test_requirement_never_wraps(void)
{
    // Every Blind multiplier of every ante on every scaling is monotonic in the ante up to
    // saturation: the product is never smaller than the one of the previous ante
    for (int s = 0; s < ANTE_SCALING_COUNT; s++)
        for (int m = 0; m < 5; m++)
            for (int ante = 1; ante <= ANTE_LAST; ante++)
            {
                uint32_t prev = ante_table_requirement(ante_table_base(s, ante - 1), MULTS[m]);
                uint32_t cur = ante_table_requirement(ante_table_base(s, ante), MULTS[m]);
                assert(cur >= prev);
            }

    assert(ante_table_requirement(300000000, 512) == 600000000);
    assert(ante_table_requirement(300000000, 1536) == 1800000000);
    // The only combination above 2^32 is Purple/Gold ante 12 on The Wall: saturates, never wraps
    assert(ante_table_requirement(1200000000, 1024) == UINT32_MAX);
    assert(ante_table_requirement(1200000000, 512) == 2400000000u);
    assert(ante_table_requirement(UINT32_MAX, 1536) == UINT32_MAX);
    assert(ante_table_requirement(100, 256) == 100);
    assert(ante_table_requirement(100, 384) == 150);
}

int main(void)
{
    test_monotonic();
    test_original_values();
    test_requirement_never_wraps();
    printf("Ante Table Tests Passed :)\n");
    return 0;
}
