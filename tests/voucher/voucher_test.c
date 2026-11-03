#include <assert.h>
#include <game_variables.h>
#include <stdio.h>
#include <string.h>
#include <voucher.h>

// Host stubs for the game state and the HUD updates the Vouchers call

GameVariables g_game_vars;

static u32 s_rng_next;
static int s_rng_calls;

u32 rng_get_u32(enum RngSequence key)
{
    (void)key;
    s_rng_calls++;
    return s_rng_next;
}

static int s_display_ante_calls;
static int s_display_hands_calls;
static int s_display_discards_calls;

void display_ante(void)
{
    s_display_ante_calls++;
}

void display_hands(void)
{
    s_display_hands_calls++;
}

void display_discards(void)
{
    s_display_discards_calls++;
}

// Helpers

static void reset(void)
{
    // Run defaults from game_variables.h
    memset(&g_game_vars, 0, sizeof(g_game_vars));
    g_game_vars.ante = 1;
    g_game_vars.shop_slots = 2;
    g_game_vars.consumable_slots = 2;
    g_game_vars.interest_cap = 5;
    g_game_vars.joker_slots = 5;
    g_game_vars.hands_per_round = 4;
    g_game_vars.hands = 4;
    g_game_vars.discards_per_round = 3;
    g_game_vars.discards = 3;
    g_game_vars.base_hand_size = 8;
    g_game_vars.hand_size = 8;

    vouchers_reset();
    s_rng_next = 0;
    s_rng_calls = 0;
    s_display_ante_calls = 0;
    s_display_hands_calls = 0;
    s_display_discards_calls = 0;
}

static int count_rollable(void)
{
    int count = 0;
    for (int voucher = 0; voucher < VOUCHER_TYPE_MAX; voucher++)
        count += voucher_is_rollable(voucher);
    return count;
}

static enum VoucherType first_rollable(void)
{
    for (int voucher = 0; voucher < VOUCHER_TYPE_MAX; voucher++)
    {
        if (voucher_is_rollable(voucher))
            return voucher;
    }
    return VOUCHER_NONE;
}

static enum VoucherType last_rollable(void)
{
    for (int voucher = VOUCHER_TYPE_MAX - 1; voucher >= 0; voucher--)
    {
        if (voucher_is_rollable(voucher))
            return voucher;
    }
    return VOUCHER_NONE;
}

// Tests: ownership and pairs of base and upgraded Vouchers

static void test_base_and_upgrade_pairs(void)
{
    reset();
    assert(voucher_is_rollable(VOUCHER_OVERSTOCK));
    assert(!voucher_is_rollable(VOUCHER_OVERSTOCK_PLUS)); // Upgrade needs the base owned

    voucher_redeem(VOUCHER_OVERSTOCK);
    assert(voucher_is_owned(VOUCHER_OVERSTOCK));
    assert(!voucher_is_rollable(VOUCHER_OVERSTOCK));      // Owned, never offered again
    assert(voucher_is_rollable(VOUCHER_OVERSTOCK_PLUS));  // Upgrade unlocked

    voucher_redeem(VOUCHER_OVERSTOCK_PLUS);
    assert(!voucher_is_rollable(VOUCHER_OVERSTOCK_PLUS));
}

static void test_invalid_vouchers_are_not_rollable(void)
{
    reset();
    assert(!voucher_is_rollable(VOUCHER_NONE));
    assert(!voucher_is_rollable((enum VoucherType)VOUCHER_TYPE_MAX));
    assert(!voucher_is_owned(VOUCHER_NONE));

    // Redeeming an invalid Voucher changes nothing
    voucher_redeem(VOUCHER_NONE);
    voucher_redeem((enum VoucherType)VOUCHER_TYPE_MAX);
    assert(g_game_vars.owned_vouchers == 0);
    assert(g_game_vars.shop_slots == 2);
}

// Tests: Shop prices

static void test_shop_discount_clearance_and_liquidation(void)
{
    reset();
    assert(voucher_get_shop_discount_percent() == 0);
    assert(voucher_apply_shop_discount(5) == 5);

    voucher_redeem(VOUCHER_CLEARANCE_SALE);
    assert(voucher_get_shop_discount_percent() == 25);
    assert(voucher_apply_shop_discount(5) == 4);  // floor(5.5 * 0.75) = 4
    assert(voucher_apply_shop_discount(6) == 4);  // floor(6.5 * 0.75) = 4
    assert(voucher_apply_shop_discount(1) == 1);  // Never free

    voucher_redeem(VOUCHER_LIQUIDATION);
    assert(voucher_get_shop_discount_percent() == 50); // Liquidation replaces Clearance Sale
    assert(voucher_apply_shop_discount(10) == 5);
    assert(voucher_apply_shop_discount(7) == 3);  // floor(7.5 * 0.5) = 3
    assert(voucher_apply_shop_discount(3) == 1);  // floor(3.5 * 0.5) = 1
    assert(voucher_apply_shop_discount(1) == 1);  // Never free
}

static void test_shop_discount_keeps_free_items_free(void)
{
    reset();
    voucher_redeem(VOUCHER_LIQUIDATION);
    assert(voucher_apply_shop_discount(0) == 0);
    assert(voucher_apply_shop_discount(-2) == -2);
}

static void test_reroll_discount_adds_up(void)
{
    reset();
    assert(voucher_get_reroll_discount() == 0);

    voucher_redeem(VOUCHER_REROLL_SURPLUS);
    assert(voucher_get_reroll_discount() == 2);

    voucher_redeem(VOUCHER_REROLL_GLUT);
    assert(voucher_get_reroll_discount() == 4);
}

static void test_planet_shop_weight(void)
{
    reset();
    assert(voucher_get_planet_shop_weight_x10() == 40);

    voucher_redeem(VOUCHER_PLANET_MERCHANT);
    assert(voucher_get_planet_shop_weight_x10() == 96);

    voucher_redeem(VOUCHER_PLANET_TYCOON);
    assert(voucher_get_planet_shop_weight_x10() == 320);
}

// Tests: Boss rerolls

static void test_boss_reroll_directors_cut_once_per_ante(void)
{
    reset();
    assert(!voucher_can_reroll_boss());

    voucher_redeem(VOUCHER_DIRECTORS_CUT);
    assert(voucher_can_reroll_boss());

    voucher_on_boss_rerolled();
    assert(!voucher_can_reroll_boss());

    g_game_vars.ante = 2;
    assert(voucher_can_reroll_boss());
}

static void test_boss_reroll_retcon_always(void)
{
    reset();
    voucher_redeem(VOUCHER_RETCON);
    assert(voucher_can_reroll_boss());

    voucher_on_boss_rerolled();
    assert(voucher_can_reroll_boss());
}

// Tests: effects on the run variables

static void test_slot_vouchers(void)
{
    reset();
    voucher_redeem(VOUCHER_OVERSTOCK);
    assert(g_game_vars.shop_slots == 3);
    voucher_redeem(VOUCHER_OVERSTOCK_PLUS);
    assert(g_game_vars.shop_slots == 4);
    voucher_redeem(VOUCHER_OVERSTOCK_PLUS);
    assert(g_game_vars.shop_slots == 4); // Clamped to MAX_SHOP_ITEMS

    voucher_redeem(VOUCHER_CRYSTAL_BALL);
    assert(g_game_vars.consumable_slots == 3);

    voucher_redeem(VOUCHER_ANTIMATTER);
    assert(g_game_vars.joker_slots == 6);
    voucher_redeem(VOUCHER_ANTIMATTER);
    assert(g_game_vars.joker_slots == 7);
    voucher_redeem(VOUCHER_ANTIMATTER);
    assert(g_game_vars.joker_slots == 7); // Clamped to MAX_JOKERS_HELD_SIZE
}

static void test_hand_and_discard_vouchers(void)
{
    reset();
    voucher_redeem(VOUCHER_GRABBER);
    assert(g_game_vars.hands_per_round == 5 && g_game_vars.hands == 5);
    assert(s_display_hands_calls == 1);

    voucher_redeem(VOUCHER_NACHO_TONG);
    assert(g_game_vars.hands_per_round == 6 && g_game_vars.hands == 6);

    voucher_redeem(VOUCHER_WASTEFUL);
    assert(g_game_vars.discards_per_round == 4 && g_game_vars.discards == 4);
    assert(s_display_discards_calls == 1);

    voucher_redeem(VOUCHER_RECYCLOMANCY);
    assert(g_game_vars.discards_per_round == 5 && g_game_vars.discards == 5);
}

static void test_hand_size_vouchers_clamp(void)
{
    reset();
    voucher_redeem(VOUCHER_PAINT_BRUSH);
    assert(g_game_vars.base_hand_size == 9 && g_game_vars.hand_size == 9);

    g_game_vars.base_hand_size = 15;
    g_game_vars.hand_size = 15;
    voucher_redeem(VOUCHER_PALETTE);
    assert(g_game_vars.base_hand_size == 16 && g_game_vars.hand_size == 16);
    voucher_redeem(VOUCHER_PALETTE);
    assert(g_game_vars.base_hand_size == 16 && g_game_vars.hand_size == 16); // MAX_HAND_SIZE
}

static void test_interest_cap_is_raised_never_lowered(void)
{
    reset();
    voucher_redeem(VOUCHER_SEED_MONEY);
    assert(g_game_vars.interest_cap == 10);

    voucher_redeem(VOUCHER_MONEY_TREE);
    assert(g_game_vars.interest_cap == 20);

    g_game_vars.interest_cap = 25;
    voucher_redeem(VOUCHER_SEED_MONEY);
    assert(g_game_vars.interest_cap == 25);
}

static void test_interest_cap_stays_zero_without_interest(void)
{
    // Green Deck: no interest at all, the Vouchers don't give it
    reset();
    g_game_vars.interest_cap = 0;
    voucher_redeem(VOUCHER_MONEY_TREE);
    assert(g_game_vars.interest_cap == 0);
}

static void test_hieroglyph_lowers_ante_and_hands(void)
{
    reset();
    g_game_vars.ante = 3;
    voucher_redeem(VOUCHER_HIEROGLYPH);
    assert(g_game_vars.ante == 2);
    assert(g_game_vars.hands_per_round == 3 && g_game_vars.hands == 3);
    assert(s_display_ante_calls == 1);
    assert(s_display_hands_calls == 1);

    // The Ante goes below 1 (0, then -1), hands stay at 1 at least
    g_game_vars.ante = 1;
    g_game_vars.hands_per_round = 1;
    g_game_vars.hands = 1;
    voucher_redeem(VOUCHER_HIEROGLYPH);
    assert(g_game_vars.ante == 0);
    voucher_redeem(VOUCHER_HIEROGLYPH);
    assert(g_game_vars.ante == -1);
    assert(g_game_vars.hands_per_round == 1 && g_game_vars.hands == 1);
}

static void test_petroglyph_lowers_ante_and_discards(void)
{
    reset();
    g_game_vars.ante = 3;
    voucher_redeem(VOUCHER_PETROGLYPH);
    assert(g_game_vars.ante == 2);
    assert(g_game_vars.discards_per_round == 2 && g_game_vars.discards == 2);
    assert(s_display_discards_calls == 1);

    g_game_vars.discards_per_round = 0;
    g_game_vars.discards = 0;
    voucher_redeem(VOUCHER_PETROGLYPH);
    assert(g_game_vars.discards_per_round == 0 && g_game_vars.discards == 0);
}

// Tests: the Voucher offered in the Shop

static void test_shop_offer_is_stable_within_an_ante(void)
{
    reset();
    s_rng_next = 0;
    enum VoucherType offer = voucher_get_shop_offer();
    assert(offer == first_rollable());
    assert(s_rng_calls == 1);

    assert(voucher_get_shop_offer() == offer);
    assert(s_rng_calls == 1); // Not rolled again
}

static void test_shop_offer_rerolls_on_new_ante(void)
{
    reset();
    s_rng_next = 0;
    assert(voucher_get_shop_offer() == first_rollable());

    g_game_vars.ante = 2;
    s_rng_next = count_rollable() - 1; // Picks the last rollable Voucher
    assert(voucher_get_shop_offer() == last_rollable());
    assert(s_rng_calls == 2);
}

static void test_shop_offer_gone_once_bought(void)
{
    reset();
    s_rng_next = 0;
    enum VoucherType offer = voucher_get_shop_offer();
    voucher_redeem(offer);

    assert(voucher_get_shop_offer() == VOUCHER_NONE);
    assert(s_rng_calls == 1); // Still the same Ante, no new offer
}

static void test_shop_offer_gone_when_no_longer_rollable(void)
{
    reset();
    s_rng_next = 0;
    enum VoucherType offer = voucher_get_shop_offer();

    // Owned through another way than the Shop
    g_game_vars.owned_vouchers |= 1u << offer;
    assert(voucher_get_shop_offer() == VOUCHER_NONE);
}

static void test_shop_offer_none_when_everything_owned(void)
{
    reset();
    g_game_vars.owned_vouchers = UINT32_MAX;
    s_rng_next = 0;
    assert(count_rollable() == 0);
    assert(voucher_get_shop_offer() == VOUCHER_NONE);
}

static void test_hieroglyph_does_not_reroll_the_offer(void)
{
    // Lowering the Ante must not roll a new Voucher for the Ante already in progress
    reset();
    g_game_vars.ante = 3;
    s_rng_next = 0;
    enum VoucherType offer = voucher_get_shop_offer();
    assert(s_rng_calls == 1);

    voucher_redeem(VOUCHER_HIEROGLYPH);
    assert(g_game_vars.ante == 2);
    assert(voucher_get_shop_offer() == offer);
    assert(s_rng_calls == 1);
}

static void test_hone_and_glow_up_edition_rate(void)
{
    reset();
    assert(voucher_get_edition_rate_multiplier() == 1);

    // Both are enabled, Glow Up needs Hone
    assert(voucher_is_rollable(VOUCHER_HONE));
    assert(!voucher_is_rollable(VOUCHER_GLOW_UP));

    voucher_redeem(VOUCHER_HONE);
    assert(voucher_get_edition_rate_multiplier() == 2);
    assert(!voucher_is_rollable(VOUCHER_HONE));
    assert(voucher_is_rollable(VOUCHER_GLOW_UP));

    voucher_redeem(VOUCHER_GLOW_UP);
    assert(voucher_get_edition_rate_multiplier() == 4); // X4 in total, not X8
}

int main(void)
{
    test_base_and_upgrade_pairs();
    test_invalid_vouchers_are_not_rollable();
    test_shop_discount_clearance_and_liquidation();
    test_shop_discount_keeps_free_items_free();
    test_reroll_discount_adds_up();
    test_planet_shop_weight();
    test_boss_reroll_directors_cut_once_per_ante();
    test_boss_reroll_retcon_always();
    test_slot_vouchers();
    test_hand_and_discard_vouchers();
    test_hand_size_vouchers_clamp();
    test_interest_cap_is_raised_never_lowered();
    test_interest_cap_stays_zero_without_interest();
    test_hieroglyph_lowers_ante_and_hands();
    test_petroglyph_lowers_ante_and_discards();
    test_shop_offer_is_stable_within_an_ante();
    test_shop_offer_rerolls_on_new_ante();
    test_shop_offer_gone_once_bought();
    test_shop_offer_gone_when_no_longer_rollable();
    test_shop_offer_none_when_everything_owned();
    test_hieroglyph_does_not_reroll_the_offer();
    test_hone_and_glow_up_edition_rate();
    printf("Voucher Tests Passed :)\n");
    return 0;
}
