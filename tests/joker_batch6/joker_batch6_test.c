#include <assert.h>
#include <joker_batch6.h>
#include <stdio.h>
#include <string.h>

static Card make_card(uint8_t rank, uint8_t enhancement)
{
    Card card;
    memset(&card, 0, sizeof(card));
    card.rank = rank;
    card.enhancement = enhancement;
    return card;
}

static void test_legendaries(void)
{
    assert(joker_batch6_canio_percent(0) == 100);
    assert(joker_batch6_canio_percent(3) == 400);
    assert(joker_batch6_canio_percent(-2) == 100);
    assert(joker_batch6_canio_percent(1000000) == 100 + 99900); // capped

    Card king = make_card(KING, 0), queen = make_card(QUEEN, 0), jack = make_card(JACK, 0);
    Card stone = make_card(KING, CARD_ENHANCEMENT_STONE);
    assert(joker_batch6_triboulet_hit(&king) && joker_batch6_triboulet_hit(&queen));
    assert(!joker_batch6_triboulet_hit(&jack));
    assert(!joker_batch6_triboulet_hit(&stone));
    assert(!joker_batch6_triboulet_hit(NULL));

    assert(joker_batch6_yorick_percent(0) == 100);
    assert(joker_batch6_yorick_percent(22) == 100);
    assert(joker_batch6_yorick_percent(23) == 200);
    assert(joker_batch6_yorick_percent(70) == 400);
    assert(joker_batch6_yorick_remaining(0) == 23);
    assert(joker_batch6_yorick_remaining(22) == 1);
    assert(joker_batch6_yorick_remaining(23) == 23);

    assert(joker_batch6_chicot_requirement_mult(4) == 2);
    assert(joker_batch6_chicot_requirement_mult(6) == 2);
    assert(joker_batch6_chicot_requirement_mult(2) == 2);

    assert(joker_batch6_perkeo_pick(7, 0, 1) == -1);
    assert(joker_batch6_perkeo_pick(7, 2, 0) == -1);
    assert(joker_batch6_perkeo_pick(7, 2, 1) == 1);
    assert(joker_batch6_perkeo_pick(8, 2, 1) == 0);
}

static void test_ordinary(void)
{
    assert(joker_batch6_flash_card_mult(0) == 0 && joker_batch6_flash_card_mult(3) == 6);
    assert(joker_batch6_flash_card_mult(-1) == 0);
    assert(joker_batch6_red_card_mult(2) == 6 && joker_batch6_red_card_mult(0) == 0);
    assert(joker_batch6_luchador_disables(true, true));
    assert(!joker_batch6_luchador_disables(true, false) && !joker_batch6_luchador_disables(false, true));
    assert(joker_batch6_mr_bones_saves(25, 100));
    assert(!joker_batch6_mr_bones_saves(24, 100));
    assert(joker_batch6_mr_bones_saves(100, 100));
    assert(!joker_batch6_mr_bones_saves(10, 0));
    assert(joker_batch6_chaos_free_reroll(true, false));
    assert(!joker_batch6_chaos_free_reroll(true, true) && !joker_batch6_chaos_free_reroll(false, false));
    assert(joker_batch6_astronomer_price(3, true) == 0 && joker_batch6_astronomer_price(3, false) == 3);
    assert(joker_batch6_mr_bones_saves(0x40000000u, 0xFFFFFFFFu)); // no overflow
}

int main(void)
{
    test_legendaries();
    test_ordinary();
    printf("joker_batch6: ok\n");
    return 0;
}
