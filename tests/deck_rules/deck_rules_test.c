#include <assert.h>
#include <deck_rules.h>
#include <stdio.h>

static uint32_t s_seq[DECK_STANDARD_SIZE * 2];
static int s_seq_idx = 0;

static uint32_t next_from_seq(void* ctx)
{
    (void)ctx;
    return s_seq[s_seq_idx++];
}

static int build(enum DeckType deck, uint8_t* suits, uint8_t* ranks)
{
    s_seq_idx = 0;
    return deck_rules_build(deck, suits, ranks, DECK_STANDARD_SIZE, next_from_seq, NULL);
}

static void test_standard(void)
{
    uint8_t suits[DECK_STANDARD_SIZE], ranks[DECK_STANDARD_SIZE];
    for (enum DeckType deck = 0; deck < DECK_TYPE_MAX; deck++)
    {
        if (deck == DECK_TYPE_ABANDONED || deck == DECK_TYPE_CHECKERED || deck == DECK_TYPE_ERRATIC)
            continue;
        // The random source is never used by the other Decks
        assert(build(deck, suits, ranks) == 52);
        assert(s_seq_idx == 0);
        int counts[NUM_SUITS][NUM_RANKS] = {{0}};
        for (int i = 0; i < 52; i++)
            counts[suits[i]][ranks[i]]++;
        for (int s = 0; s < NUM_SUITS; s++)
            for (int r = 0; r < NUM_RANKS; r++)
                assert(counts[s][r] == 1);
    }
}

static void test_abandoned(void)
{
    uint8_t suits[DECK_STANDARD_SIZE], ranks[DECK_STANDARD_SIZE];
    int count = build(DECK_TYPE_ABANDONED, suits, ranks);
    assert(count == 40);
    int per_suit[NUM_SUITS] = {0};
    for (int i = 0; i < count; i++)
    {
        assert(ranks[i] != JACK && ranks[i] != QUEEN && ranks[i] != KING);
        per_suit[suits[i]]++;
    }
    for (int s = 0; s < NUM_SUITS; s++)
        assert(per_suit[s] == 10); // 2-10 and Ace
    assert(deck_rules_has_card(DECK_TYPE_ABANDONED, SPADES, ACE));
    assert(!deck_rules_has_card(DECK_TYPE_ABANDONED, HEARTS, KING));
    assert(deck_rules_has_card(DECK_TYPE_RED, HEARTS, KING));
}

static void test_checkered(void)
{
    uint8_t suits[DECK_STANDARD_SIZE], ranks[DECK_STANDARD_SIZE];
    int count = build(DECK_TYPE_CHECKERED, suits, ranks);
    assert(count == 52);
    int per_suit[NUM_SUITS] = {0};
    int per_rank[NUM_RANKS] = {0};
    for (int i = 0; i < count; i++)
    {
        per_suit[suits[i]]++;
        per_rank[ranks[i]]++;
    }
    assert(per_suit[SPADES] == 26 && per_suit[HEARTS] == 26);
    assert(per_suit[CLUBS] == 0 && per_suit[DIAMONDS] == 0);
    for (int r = 0; r < NUM_RANKS; r++)
        assert(per_rank[r] == 4); // every rank twice per suit

    assert(deck_rules_starting_suit(DECK_TYPE_CHECKERED, CLUBS) == SPADES);
    assert(deck_rules_starting_suit(DECK_TYPE_CHECKERED, DIAMONDS) == HEARTS);
    assert(deck_rules_starting_suit(DECK_TYPE_CHECKERED, HEARTS) == HEARTS);
    assert(deck_rules_starting_suit(DECK_TYPE_RED, CLUBS) == CLUBS);
}

static void test_erratic(void)
{
    uint8_t suits[DECK_STANDARD_SIZE], ranks[DECK_STANDARD_SIZE];
    for (int i = 0; i < DECK_STANDARD_SIZE; i++)
        s_seq[i] = (uint32_t)i * 2654435761u + 7;

    // Always 52 cards, every one valid, taking one random number each
    assert(build(DECK_TYPE_ERRATIC, suits, ranks) == 52);
    assert(s_seq_idx == 52);
    for (int i = 0; i < 52; i++)
    {
        assert(suits[i] < NUM_SUITS && ranks[i] < NUM_RANKS);
        uint32_t pick = s_seq[i] % DECK_STANDARD_SIZE;
        assert(suits[i] == pick / NUM_RANKS && ranks[i] == pick % NUM_RANKS);
    }

    // The same numbers give the same Deck (determinism by seed), others give another one
    uint8_t suits2[DECK_STANDARD_SIZE], ranks2[DECK_STANDARD_SIZE];
    build(DECK_TYPE_ERRATIC, suits2, ranks2);
    for (int i = 0; i < 52; i++)
        assert(suits[i] == suits2[i] && ranks[i] == ranks2[i]);
    for (int i = 0; i < DECK_STANDARD_SIZE; i++)
        s_seq[i] += 13;
    build(DECK_TYPE_ERRATIC, suits2, ranks2);
    int differences = 0;
    for (int i = 0; i < 52; i++)
        differences += (ranks[i] != ranks2[i]) || (suits[i] != suits2[i]);
    assert(differences > 0);

    // Repeats are allowed: the same number twice makes two identical cards
    for (int i = 0; i < DECK_STANDARD_SIZE; i++)
        s_seq[i] = 20;
    build(DECK_TYPE_ERRATIC, suits, ranks);
    assert(suits[0] == suits[51] && ranks[0] == ranks[51]);

    // Small capacity is respected
    s_seq_idx = 0;
    assert(deck_rules_build(DECK_TYPE_ERRATIC, suits, ranks, 10, next_from_seq, NULL) == 10);
    assert(s_seq_idx == 10);
}

static void test_plasma(void)
{
    assert(deck_rules_balances_score(DECK_TYPE_PLASMA));
    assert(!deck_rules_balances_score(DECK_TYPE_RED));
    assert(!deck_rules_balances_score(DECK_TYPE_ERRATIC));

    uint32_t chips = 40, mult = 2;
    deck_rules_plasma_balance(&chips, &mult);
    assert(chips == 21 && mult == 21);

    chips = 5;
    mult = 8; // average rounds down
    deck_rules_plasma_balance(&chips, &mult);
    assert(chips == 6 && mult == 6);

    chips = 0;
    mult = 0;
    deck_rules_plasma_balance(&chips, &mult);
    assert(chips == 0 && mult == 0);

    // No overflow when both are huge
    chips = UINT32_MAX;
    mult = UINT32_MAX - 1;
    deck_rules_plasma_balance(&chips, &mult);
    assert(chips == UINT32_MAX - 1 && mult == chips);

    // Score of a Pair (10 chips + 2 cards of 10, mult 2): 30 x 2 = 60 becomes 16 x 16 = 256
    chips = 30;
    mult = 2;
    deck_rules_plasma_balance(&chips, &mult);
    assert((uint64_t)chips * mult == 256);

    assert(deck_rules_blind_scale(DECK_TYPE_PLASMA) == 2);
    assert(deck_rules_blind_scale(DECK_TYPE_RED) == 1);
    assert(deck_rules_blind_scale(DECK_TYPE_GHOST) == 1);
}

static void test_ghost_shop(void)
{
    // Spectral cards in the Shop only on the Ghost Deck, 20 in 100 with the default weights
    int spectral = 0;
    for (uint32_t r = 0; r < 100; r++)
    {
        assert(!deck_rules_shop_is_spectral(DECK_TYPE_RED, 40, 40, r));
        spectral += deck_rules_shop_is_spectral(DECK_TYPE_GHOST, 40, 40, r);
    }
    assert(spectral == 20);
}

static void test_anaglyph(void)
{
    assert(DECK_TYPE_MAX == 15);
    assert(DECK_TYPE_ANAGLYPH == 14);
    // Double Tag only on the Anaglyph Deck and only after a Boss Blind
    assert(deck_rules_gives_double_tag(DECK_TYPE_ANAGLYPH, true));
    assert(!deck_rules_gives_double_tag(DECK_TYPE_ANAGLYPH, false));
    for (enum DeckType deck = 0; deck < DECK_TYPE_ANAGLYPH; deck++)
        assert(!deck_rules_gives_double_tag(deck, true));
    // A standard 52-card Deck, no score or Blind change (checked by test_standard() as well)
    assert(!deck_rules_balances_score(DECK_TYPE_ANAGLYPH));
    assert(deck_rules_blind_scale(DECK_TYPE_ANAGLYPH) == 1);
    assert(!deck_rules_shop_is_spectral(DECK_TYPE_ANAGLYPH, 40, 40, 0));
}

int main(void)
{
    test_standard();
    test_abandoned();
    test_checkered();
    test_erratic();
    test_plasma();
    test_ghost_shop();
    test_anaglyph();
    printf("deck_rules: all tests passed\n");
    return 0;
}
