#include <assert.h>
#include <joker_batch5.h>
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

static void test_first_hand_and_discard(void)
{
    Card six = make_card(SIX, HEARTS, 0), seven = make_card(SEVEN, HEARTS, 0);
    Card stone = make_card(SIX, HEARTS, CARD_ENHANCEMENT_STONE);
    assert(joker_batch5_sixth_sense_hit(1, 1, &six));
    assert(!joker_batch5_sixth_sense_hit(2, 1, &six)); // not the first hand
    assert(!joker_batch5_sixth_sense_hit(1, 2, &six)); // not a single card
    assert(!joker_batch5_sixth_sense_hit(1, 1, &seven));
    assert(!joker_batch5_sixth_sense_hit(1, 1, &stone)); // no rank
    assert(!joker_batch5_sixth_sense_hit(1, 1, NULL));

    assert(joker_batch5_dna_hit(1, 1));
    assert(!joker_batch5_dna_hit(1, 2) && !joker_batch5_dna_hit(2, 1) && !joker_batch5_dna_hit(0, 1));

    assert(joker_batch5_trading_card_hit(1, 1));
    assert(!joker_batch5_trading_card_hit(2, 1) && !joker_batch5_trading_card_hit(1, 3));

    assert(joker_batch5_is_straight_flush(STRAIGHT_FLUSH) && joker_batch5_is_straight_flush(ROYAL_FLUSH));
    assert(!joker_batch5_is_straight_flush(STRAIGHT) && !joker_batch5_is_straight_flush(FLUSH));

    assert(joker_batch5_vagabond_hit(4) && joker_batch5_vagabond_hit(-3) && !joker_batch5_vagabond_hit(5));
}

static void test_food(void)
{
    assert(joker_batch5_popcorn_next(POPCORN_START_MULT) == 16);
    assert(joker_batch5_popcorn_next(4) == 0); // eaten up at 0
    assert(joker_batch5_ramen_discard(RAMEN_START_PERCENT, 5) == 195);
    assert(!joker_batch5_ramen_gone(100) && joker_batch5_ramen_gone(99));
    assert(joker_batch5_ramen_discard(100, 1) == 99 && joker_batch5_ramen_gone(99));
    assert(joker_batch5_ramen_discard(150, 0) == 150 && joker_batch5_ramen_discard(150, -2) == 150);
    assert(joker_batch5_turtle_bean_next(TURTLE_BEAN_START) == 4);
    assert(joker_batch5_turtle_bean_next(1) == 0); // eaten up at 0

    int destroyed = 0;
    for (uint32_t roll = 0; roll < 600; roll++)
        destroyed += joker_batch5_one_in(roll, GROS_MICHEL_CHANCE);
    assert(destroyed == 100);
    assert(joker_batch5_one_in(0, CAVENDISH_CHANCE) && !joker_batch5_one_in(1, CAVENDISH_CHANCE));
    assert(!joker_batch5_one_in(0, 0));
}

static void test_castle_and_mail(void)
{
    int32_t state = joker_batch5_castle_pack(SPADES, 0);
    assert(joker_batch5_castle_suit(state) == SPADES && joker_batch5_castle_chips(state) == 0);
    state = joker_batch5_castle_discard(state, 3);
    assert(joker_batch5_castle_chips(state) == 9 && joker_batch5_castle_suit(state) == SPADES);
    state = joker_batch5_castle_pack(DIAMONDS, joker_batch5_castle_chips(state)); // new suit, chips stay
    assert(joker_batch5_castle_suit(state) == DIAMONDS && joker_batch5_castle_chips(state) == 9);
    state = joker_batch5_castle_pack(HEARTS, CASTLE_CHIPS_MAX + 100);
    assert(joker_batch5_castle_chips(state) == CASTLE_CHIPS_MAX);
    assert(joker_batch5_castle_suit(state) == HEARTS);

    assert(joker_batch5_mail_in_money(0) == 0 && joker_batch5_mail_in_money(3) == 15);

    DiscardCounts counts;
    memset(&counts, 0, sizeof(counts));
    Card kh = make_card(KING, HEARTS, 0), kd = make_card(KING, DIAMONDS, 0);
    Card jw = make_card(JACK, SPADES, CARD_ENHANCEMENT_WILD), st = make_card(KING, HEARTS, CARD_ENHANCEMENT_STONE);
    joker_batch5_discard_add(&counts, &kh, true);
    joker_batch5_discard_add(&counts, &kd, true);
    joker_batch5_discard_add(&counts, &jw, true);
    joker_batch5_discard_add(&counts, &st, false);
    assert(counts.nb_cards == 4 && counts.nb_face_cards == 3 && counts.nb_jacks == 1);
    assert(counts.ranks[KING] == 2 && counts.ranks[JACK] == 1); // Stone has no rank
    assert(counts.suits[HEARTS] == 2); // Heart king and the Wild card
    assert(counts.suits[DIAMONDS] == 2 && counts.suits[SPADES] == 1 && counts.suits[CLUBS] == 1);
}

static void test_obelisk(void)
{
    uint32_t played[HAND_TYPE_MAX] = {0};
    // Pair played twice and a High Card once: High Card is not the most played
    played[PAIR - 1] = 2;
    played[HIGH_CARD - 1] = 1;
    int32_t streak = 0;
    streak = joker_batch5_obelisk_next(streak, played, HIGH_CARD);
    assert(streak == 1);
    played[FLUSH - 1] = 1;
    streak = joker_batch5_obelisk_next(streak, played, FLUSH);
    assert(streak == 2);
    // Playing the most played hand again resets the streak
    played[PAIR - 1] = 3;
    assert(joker_batch5_obelisk_next(streak, played, PAIR) == 0);
    // A tie with the most played hand is not the most played: the streak goes on
    played[FLUSH - 1] = 3;
    assert(!joker_batch5_is_most_played(played, FLUSH));
    assert(joker_batch5_obelisk_next(5, played, FLUSH) == 6);
    played[FLUSH - 1] = 4;
    assert(joker_batch5_is_most_played(played, FLUSH));
    assert(joker_batch5_obelisk_next(5, played, FLUSH) == 0);
    // Invalid hand: nothing changes
    assert(joker_batch5_obelisk_next(4, played, NONE) == 4);

    assert(joker_batch5_obelisk_percent(0) == 100 && joker_batch5_obelisk_percent(3) == 160);
    assert(joker_batch5_obelisk_percent(-1) == 100);

    // Royal Flush counts with the Straight Flush
    uint32_t sf[HAND_TYPE_MAX] = {0};
    sf[STRAIGHT_FLUSH - 1] = 1;
    sf[ROYAL_FLUSH - 1] = 1;
    sf[PAIR - 1] = 2;
    assert(joker_batch5_played_count(sf, ROYAL_FLUSH) == 2);
    assert(!joker_batch5_is_most_played(sf, ROYAL_FLUSH)); // 2 = 2 is a tie
    sf[PAIR - 1] = 1;
    assert(joker_batch5_is_most_played(sf, ROYAL_FLUSH));
    sf[PAIR - 1] = 3;
    assert(!joker_batch5_is_most_played(sf, STRAIGHT_FLUSH));
}

static void test_dagger_and_cloud(void)
{
    assert(joker_batch5_dagger_gain(3) == 6 && joker_batch5_dagger_gain(0) == 0);
    assert(joker_batch5_dagger_gain(-1) == 0);
    assert(joker_batch5_dagger_add(4, 5) == 14);
    assert(joker_batch5_dagger_add(DAGGER_MULT_MAX, 5) == DAGGER_MULT_MAX);
    assert(joker_batch5_cloud_nine_money(4) == 4 && joker_batch5_cloud_nine_money(0) == 0);
    assert(joker_batch5_cloud_nine_money(-1) == 0);
}

int main(void)
{
    test_first_hand_and_discard();
    test_food();
    test_castle_and_mail();
    test_obelisk();
    test_dagger_and_cloud();
    printf("joker_batch5: all tests passed\n");
    return 0;
}
