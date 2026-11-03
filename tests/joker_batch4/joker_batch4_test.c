#include <assert.h>
#include <joker_batch4.h>
#include <stdio.h>
#include <string.h>

static Card make_card(uint8_t rank, uint8_t suit, uint8_t enhancement)
{
    Card card;
    memset(&card, 0, sizeof(card));
    card.rank = rank;
    card.suit = suit;
    card.enhancement = enhancement;
    return card;
}

static void test_suits(void)
{
    Card d = make_card(TWO, DIAMONDS, 0), c = make_card(THREE, CLUBS, 0);
    Card h = make_card(FOUR, HEARTS, 0), s = make_card(FIVE, SPADES, 0);
    Card w = make_card(SIX, SPADES, CARD_ENHANCEMENT_WILD);
    Card st = make_card(SEVEN, HEARTS, CARD_ENHANCEMENT_STONE);

    const Card* all[] = {&d, &c, &h, &s};
    assert(joker_batch4_all_suits(all, 4));
    const Card* three[] = {&d, &c, &h};
    assert(!joker_batch4_all_suits(three, 3));
    const Card* wild[] = {&d, &c, &h, &w};
    assert(joker_batch4_all_suits(wild, 4)); // Wild fills Spades
    const Card* stone[] = {&d, &c, &h, &st};
    assert(!joker_batch4_all_suits(stone, 4)); // Stone has no suit
    const Card* two_wild[] = {&d, &c, &w, &w};
    assert(joker_batch4_all_suits(two_wild, 4));

    const Card* cs[] = {&c, &h};
    assert(joker_batch4_club_and_other(cs, 2));
    const Card* cc[] = {&c, &c};
    assert(!joker_batch4_club_and_other(cc, 2));
    const Card* hs[] = {&h, &s};
    assert(!joker_batch4_club_and_other(hs, 2)); // no Club
    const Card* one_wild[] = {&w};
    assert(!joker_batch4_club_and_other(one_wild, 1)); // a lone Wild is only one card
    const Card* wc[] = {&w, &c};
    assert(joker_batch4_club_and_other(wc, 2));
    const Card* cstone[] = {&c, &st};
    assert(!joker_batch4_club_and_other(cstone, 2));
}

static void test_idol(void)
{
    int32_t state = joker_batch4_idol_pack(KING, HEARTS);
    assert(joker_batch4_idol_rank(state) == KING && joker_batch4_idol_suit(state) == HEARTS);
    Card kh = make_card(KING, HEARTS, 0), kd = make_card(KING, DIAMONDS, 0);
    Card kw = make_card(KING, SPADES, CARD_ENHANCEMENT_WILD), st = make_card(KING, HEARTS, CARD_ENHANCEMENT_STONE);
    assert(joker_batch4_idol_match(&kh, KING, HEARTS));
    assert(!joker_batch4_idol_match(&kd, KING, HEARTS));
    assert(joker_batch4_idol_match(&kw, KING, HEARTS));
    assert(!joker_batch4_idol_match(&st, KING, HEARTS));
}

static void test_counts(void)
{
    assert(joker_batch4_baseball_percent(0) == 100);
    assert(joker_batch4_baseball_percent(1) == 150);
    assert(joker_batch4_baseball_percent(2) == 225);
    assert(joker_batch4_baseball_percent(3) == 338);
    assert(joker_batch4_erosion_mult(52, 52) == 0 && joker_batch4_erosion_mult(60, 52) == 0);
    assert(joker_batch4_erosion_mult(50, 52) == 8);
    assert(joker_batch4_erosion_mult(0, 52) == 208);
    // Starting size of the Deck: 40 cards of the Abandoned Deck are not "missing"
    assert(joker_batch4_erosion_mult(40, 40) == 0 && joker_batch4_erosion_mult(38, 40) == 8);
    assert(joker_batch4_erosion_mult(53, 40) == 0 && joker_batch4_erosion_mult(30, 0) == 88);
    // Every Uncommon Joker counts (Negative ones too), the result saturates
    assert(joker_batch4_baseball_percent(8) == 2570 && joker_batch4_baseball_percent(9) == 3855);
    assert(joker_batch4_baseball_percent(12) > joker_batch4_baseball_percent(11));
    assert(joker_batch4_baseball_percent(255) == BASEBALL_PERCENT_MAX);
    assert(!joker_batch4_drivers_license_active(15) && joker_batch4_drivers_license_active(16));
    assert(joker_batch4_faceless_money(2) == 0 && joker_batch4_faceless_money(3) == 5);
    assert(joker_batch4_faceless_money(5) == 5);
    assert(joker_batch4_hit_the_road_percent(0) == 100);
    assert(joker_batch4_hit_the_road_percent(3) == 250);
}

static void test_hands(void)
{
    uint32_t mask = 0;
    assert(!joker_batch4_card_sharp_seen(mask, 2));
    mask |= joker_batch4_hand_bit(2);
    assert(joker_batch4_card_sharp_seen(mask, 2));
    assert(!joker_batch4_card_sharp_seen(mask, 3));
    mask |= joker_batch4_hand_bit(10); // Royal Flush
    assert(joker_batch4_card_sharp_seen(mask, 9)); // counts as Straight Flush

    uint32_t played[HAND_TYPE_MAX] = {0};
    assert(joker_batch4_todo_target(0, 0, played) == 1 && joker_batch4_todo_target(8, 0, played) == 9);
    assert(joker_batch4_todo_target(9, 0, played) == 1); // secret hands not played yet: 9 targets
    // The target never repeats the current one
    for (uint32_t roll = 0; roll < 40; roll++)
        assert(joker_batch4_todo_target(roll, 3, played) != 3);
    // A played secret hand can be asked for, an unplayed one never
    played[FLUSH_HOUSE - 1] = 1;
    bool seen_secret = false;
    for (uint32_t roll = 0; roll < 40; roll++)
    {
        int target = joker_batch4_todo_target(roll, 0, played);
        assert(target != FIVE_OF_A_KIND && target != FLUSH_FIVE && target != ROYAL_FLUSH);
        seen_secret |= target == FLUSH_HOUSE;
    }
    assert(seen_secret);
    // Card Sharp: a hand played earlier in the round (the current one is counted already)
    uint32_t now[HAND_TYPE_MAX] = {0}, start[HAND_TYPE_MAX] = {0};
    now[PAIR - 1] = 1;
    assert(!joker_batch4_played_earlier_in_round(now, start, PAIR)); // only the current hand
    now[PAIR - 1] = 2;
    assert(joker_batch4_played_earlier_in_round(now, start, PAIR));
    start[PAIR - 1] = 2; // two Pairs were played before this round
    assert(!joker_batch4_played_earlier_in_round(now, start, PAIR));
    now[ROYAL_FLUSH - 1] = 1;
    now[STRAIGHT_FLUSH - 1] = 1;
    assert(joker_batch4_played_earlier_in_round(now, start, STRAIGHT_FLUSH)); // Royal = Straight Flush
    assert(joker_batch4_todo_hit(2, 2) && !joker_batch4_todo_hit(2, 3));
    assert(joker_batch4_todo_hit(9, 10)); // Royal Flush pays for Straight Flush

    Card ace = make_card(ACE, SPADES, 0), two = make_card(TWO, SPADES, 0);
    Card stone = make_card(ACE, SPADES, CARD_ENHANCEMENT_STONE);
    const Card* with_ace[] = {&two, &ace};
    const Card* no_ace[] = {&two, &two};
    const Card* stone_ace[] = {&stone};
    assert(joker_batch4_superposition(true, with_ace, 2));
    assert(!joker_batch4_superposition(false, with_ace, 2));
    assert(!joker_batch4_superposition(true, no_ace, 2));
    assert(!joker_batch4_superposition(true, stone_ace, 1));
}

static void test_green_and_chances(void)
{
    int32_t state = 0;
    state = joker_batch4_green_discard(state);
    assert(state == 0);
    state = joker_batch4_green_hand(joker_batch4_green_hand(state));
    assert(state == 2);
    assert(joker_batch4_green_discard(state) == 1);
    assert(joker_batch4_green_hand(GREEN_JOKER_MAX) == GREEN_JOKER_MAX);

    int hits = 0;
    for (uint32_t roll = 0; roll < 400; roll++)
        hits += joker_batch4_one_in(roll, EIGHT_BALL_CHANCE);
    assert(hits == 100);
    assert(joker_batch4_one_in(2, BLOODSTONE_CHANCE) && !joker_batch4_one_in(3, BLOODSTONE_CHANCE));
    assert(!joker_batch4_one_in(1, 0));
}

int main(void)
{
    test_suits();
    test_idol();
    test_counts();
    test_hands();
    test_green_and_chances();
    printf("joker_batch4: all tests passed\n");
    return 0;
}
