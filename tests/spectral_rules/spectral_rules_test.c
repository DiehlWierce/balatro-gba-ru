#include <assert.h>
#include <spectral_rules.h>
#include <stdio.h>

static uint32_t s_seq[64];
static int s_seq_idx = 0;

static uint32_t next_from_seq(void* ctx)
{
    (void)ctx;
    return s_seq[s_seq_idx++];
}

static void test_pool(void)
{
    // Soul and Black Hole are out of the common pool, Black Hole has its own chance, the Spectral cards of Editions are checked in test_editions
    assert(spectral_is_implemented(SPECTRAL_SOUL));
    assert(!spectral_is_in_common_pool(SPECTRAL_SOUL));
    assert(spectral_is_implemented(SPECTRAL_BLACK_HOLE));
    assert(!spectral_is_in_common_pool(SPECTRAL_BLACK_HOLE));
    assert(spectral_is_implemented(SPECTRAL_AURA) == SPECTRAL_EDITIONS_AVAILABLE);
    assert(spectral_is_implemented(SPECTRAL_HEX) == SPECTRAL_EDITIONS_AVAILABLE);
    assert(spectral_is_implemented(SPECTRAL_ECTOPLASM) == SPECTRAL_EDITIONS_AVAILABLE);
    assert(!spectral_is_implemented(SPECTRAL_TYPE_MAX));

    int size = spectral_common_pool_size();
    int expected = SPECTRAL_EDITIONS_AVAILABLE ? 16 : 13;
    assert(size == expected);
    for (int i = 0; i < size; i++)
        assert(spectral_is_in_common_pool(spectral_common_pool_get(i)));
    assert(spectral_common_pool_get(size) == SPECTRAL_TYPE_MAX);
    assert(spectral_common_pool_get(0) == SPECTRAL_FAMILIAR);
}

static void test_scope_and_seals(void)
{
    assert(spectral_get_scope(SPECTRAL_TALISMAN) == SPECTRAL_SCOPE_SELECTED_CARD);
    assert(spectral_get_scope(SPECTRAL_CRYPTID) == SPECTRAL_SCOPE_SELECTED_CARD);
    assert(spectral_get_scope(SPECTRAL_SIGIL) == SPECTRAL_SCOPE_HAND);
    assert(spectral_get_scope(SPECTRAL_IMMOLATE) == SPECTRAL_SCOPE_HAND);
    assert(spectral_get_scope(SPECTRAL_WRAITH) == SPECTRAL_SCOPE_ANYWHERE);
    assert(spectral_get_scope(SPECTRAL_BLACK_HOLE) == SPECTRAL_SCOPE_ANYWHERE);

    assert(spectral_get_seal(SPECTRAL_TALISMAN) == CARD_SEAL_GOLD);
    assert(spectral_get_seal(SPECTRAL_DEJA_VU) == CARD_SEAL_RED);
    assert(spectral_get_seal(SPECTRAL_TRANCE) == CARD_SEAL_BLUE);
    assert(spectral_get_seal(SPECTRAL_MEDIUM) == CARD_SEAL_PURPLE);
    assert(spectral_get_seal(SPECTRAL_GRIM) == CARD_SEAL_NONE);

    Card card = {.suit = HEARTS, .rank = FIVE, .seal = CARD_SEAL_RED};
    card_set_seal(&card, CARD_SEAL_GOLD);
    assert(card.seal == CARD_SEAL_GOLD);
    card_set_seal(&card, CARD_SEAL_NONE); // Never removes a seal
    assert(card.seal == CARD_SEAL_GOLD);
    card_set_seal(&card, CARD_SEAL_MAX);
    assert(card.seal == CARD_SEAL_GOLD);
}

static void test_created_cards(void)
{
    assert(spectral_get_nb_created_cards(SPECTRAL_FAMILIAR) == 3);
    assert(spectral_get_nb_created_cards(SPECTRAL_GRIM) == 2);
    assert(spectral_get_nb_created_cards(SPECTRAL_INCANTATION) == 4);
    assert(spectral_get_nb_created_cards(SPECTRAL_CRYPTID) == 2);
    assert(spectral_get_nb_created_cards(SPECTRAL_SIGIL) == 0);
    assert(spectral_get_nb_destroyed_cards(SPECTRAL_FAMILIAR) == 1);
    assert(spectral_get_nb_destroyed_cards(SPECTRAL_IMMOLATE) == 5);
    assert(spectral_get_nb_destroyed_cards(SPECTRAL_CRYPTID) == 0);

    for (uint32_t r = 0; r < 200; r++)
    {
        Card face = spectral_make_card(SPECTRAL_FAMILIAR, r, r / 3, r / 7);
        assert(face.rank >= JACK && face.rank <= KING);
        assert(face.suit < NUM_SUITS);
        assert(face.enhancement != CARD_ENHANCEMENT_NONE && face.enhancement < CARD_ENHANCEMENT_MAX);
        assert(face.seal == CARD_SEAL_NONE);

        Card ace = spectral_make_card(SPECTRAL_GRIM, r, r, r);
        assert(ace.rank == ACE && ace.enhancement != CARD_ENHANCEMENT_NONE);

        Card num = spectral_make_card(SPECTRAL_INCANTATION, r, r, r);
        assert(num.rank >= TWO && num.rank <= TEN);
        assert(num.enhancement != CARD_ENHANCEMENT_NONE);
    }

    // Every rank and enhancement can come up
    bool ranks[NUM_RANKS] = {false};
    bool enhancements[CARD_ENHANCEMENT_MAX] = {false};
    for (uint32_t r = 0; r < 100; r++)
    {
        ranks[spectral_make_card(SPECTRAL_INCANTATION, r, 0, 0).rank] = true;
        enhancements[spectral_make_card(SPECTRAL_FAMILIAR, 0, 0, r).enhancement] = true;
    }
    for (int rank = TWO; rank <= TEN; rank++)
        assert(ranks[rank]);
    for (int e = CARD_ENHANCEMENT_BONUS; e < CARD_ENHANCEMENT_MAX; e++)
        assert(enhancements[e]);
}

static void test_cryptid_copy(void)
{
    Card src = {
        .suit = SPADES,
        .rank = QUEEN,
        .enhancement = CARD_ENHANCEMENT_GLASS,
        .seal = CARD_SEAL_BLUE,
        .debuffed = true,
        .face_down = true,
        .played_ante = 3,
        .bonus_chips = 10,
    };
    Card copy = spectral_make_copy(&src);
    assert(copy.suit == SPADES && copy.rank == QUEEN);
    assert(copy.enhancement == CARD_ENHANCEMENT_GLASS && copy.seal == CARD_SEAL_BLUE);
    assert(copy.bonus_chips == 10);
    assert(!copy.debuffed && !copy.face_down && copy.played_ante == 0);
}

static void test_sigil_ouija(void)
{
    Card cards[3] = {
        {.suit = CLUBS, .rank = TWO},
        {.suit = HEARTS, .rank = KING, .enhancement = CARD_ENHANCEMENT_BONUS},
        {.suit = SPADES, .rank = ACE, .enhancement = CARD_ENHANCEMENT_STONE},
    };
    for (int i = 0; i < 3; i++)
        card_set_suit(&cards[i], DIAMONDS);
    assert(cards[0].suit == DIAMONDS && cards[1].suit == DIAMONDS);
    assert(cards[2].suit == SPADES); // Stone: no suit

    for (int i = 0; i < 3; i++)
        card_set_rank(&cards[i], SEVEN);
    assert(cards[0].rank == SEVEN && cards[1].rank == SEVEN);
    assert(cards[2].rank == ACE);
    card_set_rank(&cards[0], NUM_RANKS); // Invalid
    assert(cards[0].rank == SEVEN);
    assert(cards[1].enhancement == CARD_ENHANCEMENT_BONUS);

    assert(spectral_hand_size_minus_one(8) == 7);
    assert(spectral_hand_size_minus_one(1) == 1);
    assert(spectral_hand_size_minus_one(0) == 1);
}

static void test_pick_distinct(void)
{
    // Deterministic: always the first remaining candidate
    int out[8];
    for (int i = 0; i < 64; i++)
        s_seq[i] = 0;
    s_seq_idx = 0;
    assert(spectral_pick_distinct(8, 5, next_from_seq, NULL, out) == 5);
    for (int i = 0; i < 5; i++)
        assert(out[i] == i);

    // Never picks the same index twice, whatever the numbers
    for (uint32_t seed = 0; seed < 200; seed++)
    {
        for (int i = 0; i < 64; i++)
            s_seq[i] = seed * 2654435761u + i * 40503u;
        s_seq_idx = 0;
        int nb = spectral_pick_distinct(8, 5, next_from_seq, NULL, out);
        assert(nb == 5);
        bool seen[8] = {false};
        for (int i = 0; i < nb; i++)
        {
            assert(out[i] >= 0 && out[i] < 8 && !seen[out[i]]);
            seen[out[i]] = true;
        }
    }

    // Fewer candidates than asked: all of them
    s_seq_idx = 0;
    assert(spectral_pick_distinct(3, 5, next_from_seq, NULL, out) == 3);
    s_seq_idx = 0;
    assert(spectral_pick_distinct(0, 5, next_from_seq, NULL, out) == 0);
}

static void test_shop_odds(void)
{
    assert(!spectral_pack_rolls_black_hole(3));
    assert(spectral_pack_rolls_black_hole(2));
    assert(spectral_pack_rolls_black_hole(1000));
    assert(spectral_pack_rolls_soul(1000) && spectral_pack_rolls_soul(2) && !spectral_pack_rolls_soul(3));

    assert(!pack_choice_is_spectral(false, 0));
    assert(pack_choice_is_spectral(true, 19));
    assert(!pack_choice_is_spectral(true, 20));

    // Equal weights keep the old coin flip
    assert(shop_consumable_is_tarot(40, 40, 0));
    assert(!shop_consumable_is_tarot(40, 40, 1));
    // Tarot Merchant: 96 of 136
    int tarots = 0;
    for (uint32_t r = 0; r < 136; r++)
        tarots += shop_consumable_is_tarot(TAROT_MERCHANT_SHOP_WEIGHT_X10, 40, r);
    assert(tarots == 96);
    // Tarot Tycoon: 320 of 360
    tarots = 0;
    for (uint32_t r = 0; r < 360; r++)
        tarots += shop_consumable_is_tarot(TAROT_TYCOON_SHOP_WEIGHT_X10, 40, r);
    assert(tarots == 320);
}

static void test_most_played(void)
{
    uint32_t played[HAND_TYPE_MAX] = {0};
    assert(most_played_hand(played) == HIGH_CARD);

    played[PAIR - 1] = 3;
    assert(most_played_hand(played) == PAIR);

    played[FLUSH - 1] = 3; // Tie: the weakest wins
    assert(most_played_hand(played) == PAIR);

    played[FLUSH - 1] = 4;
    assert(most_played_hand(played) == FLUSH);

    // Royal Flush counts for Straight Flush
    played[STRAIGHT_FLUSH - 1] = 2;
    played[ROYAL_FLUSH - 1] = 3;
    assert(most_played_hand(played) == STRAIGHT_FLUSH);

    played[FLUSH_FIVE - 1] = 9;
    assert(most_played_hand(played) == FLUSH_FIVE);
}

static void test_editions(void)
{
    assert(SPECTRAL_EDITIONS_AVAILABLE == 1);
    assert(spectral_is_implemented(SPECTRAL_AURA));
    assert(spectral_is_implemented(SPECTRAL_ECTOPLASM));
    assert(spectral_is_implemented(SPECTRAL_HEX));

    // Aura never gives Negative or nothing, and every other edition is reachable
    int counts[EDITION_MAX] = {0};
    for (uint32_t roll = 0; roll < 37 * 4; roll++)
        counts[spectral_aura_edition(roll)]++;
    assert(counts[EDITION_NONE] == 0 && counts[EDITION_NEGATIVE] == 0);
    assert(counts[EDITION_FOIL] == 20 * 4);
    assert(counts[EDITION_HOLOGRAPHIC] == 14 * 4);
    assert(counts[EDITION_POLYCHROME] == 3 * 4);

    // Ectoplasm and Hex pick a Joker without edition (the Wheel rule), none if all have one
    uint8_t editions[3] = {EDITION_FOIL, EDITION_NONE, EDITION_NONE};
    assert(edition_wheel_pick_joker(editions, 3, 0) == 1);
    assert(edition_wheel_pick_joker(editions, 3, 1) == 2);
    editions[1] = editions[2] = EDITION_NEGATIVE;
    assert(edition_wheel_pick_joker(editions, 3, 0) == -1);

    // Ectoplasm: -1 hand size, never below 1
    assert(spectral_hand_size_minus_one(8) == 7);
    assert(spectral_hand_size_minus_one(1) == 1);
}

int main(void)
{
    test_pool();
    test_scope_and_seals();
    test_created_cards();
    test_cryptid_copy();
    test_sigil_ouija();
    test_pick_distinct();
    test_shop_odds();
    test_most_played();
    test_editions();
    printf("spectral_rules: all tests passed\n");
    return 0;
}
