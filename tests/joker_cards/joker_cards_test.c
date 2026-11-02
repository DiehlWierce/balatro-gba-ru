#include <assert.h>
#include <joker_cards.h>
#include <stdio.h>
#include <string.h>

static Card make_card(uint8_t enhancement)
{
    Card card;
    memset(&card, 0, sizeof(card));
    card.rank = KING;
    card.enhancement = enhancement;
    return card;
}

static void test_xmult_percent(void)
{
    assert(joker_cards_xmult_percent(STEEL_JOKER_ID, 0, 0) == 100);
    assert(joker_cards_xmult_percent(STEEL_JOKER_ID, 99, 3) == 160); // state is ignored
    assert(joker_cards_xmult_percent(GLASS_JOKER_ID, 2, 9) == 250);  // steel count is ignored
    assert(joker_cards_xmult_percent(LUCKY_CAT_JOKER_ID, 5, 0) == 225);
    assert(joker_cards_xmult_percent(VAMPIRE_JOKER_ID, 7, 0) == 170);
    assert(joker_cards_xmult_percent(CONSTELLATION_JOKER_ID, 3, 0) == 130);
    assert(joker_cards_xmult_percent(HIKER_JOKER_ID, 3, 3) == 100);
}

static void test_mult_gain(void)
{
    assert(joker_cards_mult_gain(10, 100) == 0);
    assert(joker_cards_mult_gain(10, 120) == 2);
    assert(joker_cards_mult_gain(4, 120) == 1);   // 4.8 rounds to 5
    assert(joker_cards_mult_gain(1, 120) == 0);   // 1.2 rounds to 1
    assert(joker_cards_mult_gain(50, 175) == 38); // 87.5 rounds to 88
    assert(joker_cards_mult_gain(0, 300) == 0);
    // Huge values must not wrap around
    assert(joker_cards_mult_gain(0xFFFFFFF0u, 300) == 0xFFFFFFFFu);
}

static void test_counters(void)
{
    assert(joker_cards_stone_chips(0) == 0);
    assert(joker_cards_stone_chips(4) == 100);
    assert(joker_cards_fortune_teller_mult(6) == 6);

    uint8_t bonus = 0;
    for (int i = 0; i < 10; i++)
        bonus = joker_cards_hiker_bonus(bonus);
    assert(bonus == 50);
    for (int i = 0; i < 100; i++)
        bonus = joker_cards_hiker_bonus(bonus);
    assert(bonus == BONUS_CHIPS_MAX);
}

static void test_cards(void)
{
    Card plain = make_card(CARD_ENHANCEMENT_NONE);
    Card glass = make_card(CARD_ENHANCEMENT_GLASS);
    Card stone = make_card(CARD_ENHANCEMENT_STONE);

    assert(!joker_cards_vampire_eat(&plain));
    assert(joker_cards_vampire_eat(&glass) && glass.enhancement == CARD_ENHANCEMENT_NONE);
    assert(joker_cards_vampire_eat(&stone) && stone.enhancement == CARD_ENHANCEMENT_NONE);
    assert(!joker_cards_vampire_eat(&glass)); // already eaten

    assert(!joker_cards_midas_gild(&plain, false));
    assert(plain.enhancement == CARD_ENHANCEMENT_NONE);
    assert(joker_cards_midas_gild(&plain, true) && plain.enhancement == CARD_ENHANCEMENT_GOLD);
    assert(!joker_cards_midas_gild(&plain, true)); // already Gold
    Card lucky = make_card(CARD_ENHANCEMENT_LUCKY);
    assert(joker_cards_midas_gild(&lucky, true) && lucky.enhancement == CARD_ENHANCEMENT_GOLD);
}

static void test_chances(void)
{
    int hits = 0;
    for (uint32_t roll = 0; roll < 400; roll++)
        hits += joker_cards_one_in(roll, SPACE_JOKER_CHANCE);
    assert(hits == 100);
    assert(joker_cards_one_in(2, HALLUCINATION_CHANCE));
    assert(!joker_cards_one_in(3, HALLUCINATION_CHANCE));
    assert(!joker_cards_one_in(0, 0));
    assert(joker_cards_burglar_hands(4) == 7);
}

int main(void)
{
    test_xmult_percent();
    test_mult_gain();
    test_counters();
    test_cards();
    test_chances();
    printf("joker_cards: all tests passed\n");
    return 0;
}
