#include <assert.h>
#include <card_rules.h>
#include <stdio.h>
#include <string.h>

static Card make(uint8_t rank, uint8_t suit, uint8_t enhancement)
{
    Card card = {.suit = suit, .rank = rank, .enhancement = enhancement};
    return card;
}

static void distribution(
    const Card* cards,
    int n,
    uint8_t ranks[NUM_RANKS],
    uint8_t suits[NUM_SUITS]
)
{
    memset(ranks, 0, NUM_RANKS);
    memset(suits, 0, NUM_SUITS);
    for (int i = 0; i < n; i++)
        card_add_to_distribution(&cards[i], ranks, suits);
}

static int max_of(const uint8_t* values, int n)
{
    int best = 0;
    for (int i = 0; i < n; i++)
        best = values[i] > best ? values[i] : best;
    return best;
}

static void test_wild_counts_for_every_suit(void)
{
    Card wild = make(ACE, HEARTS, CARD_ENHANCEMENT_WILD);
    for (uint8_t suit = 0; suit < NUM_SUITS; suit++)
        assert(card_has_suit(&wild, suit));
    assert(card_has_rank(&wild));
}

static void test_stone_has_no_rank_nor_suit(void)
{
    Card stone = make(ACE, HEARTS, CARD_ENHANCEMENT_STONE);
    Card ace = make(ACE, SPADES, CARD_ENHANCEMENT_NONE);
    assert(!card_has_rank(&stone));
    for (uint8_t suit = 0; suit < NUM_SUITS; suit++)
        assert(!card_has_suit(&stone, suit));
    assert(!cards_same_rank(&stone, &ace));
    assert(!cards_same_rank(&stone, &stone));

    uint8_t ranks[NUM_RANKS], suits[NUM_SUITS];
    distribution(&stone, 1, ranks, suits);
    assert(max_of(ranks, NUM_RANKS) == 0 && max_of(suits, NUM_SUITS) == 0);
}

static void test_flush_five_with_wild_cards(void)
{
    // Five Aces of different suits: Five of a Kind, and a Flush thanks to the Wild cards
    Card cards[5] = {
        make(ACE, HEARTS, CARD_ENHANCEMENT_NONE),
        make(ACE, HEARTS, CARD_ENHANCEMENT_NONE),
        make(ACE, SPADES, CARD_ENHANCEMENT_WILD),
        make(ACE, CLUBS, CARD_ENHANCEMENT_WILD),
        make(ACE, DIAMONDS, CARD_ENHANCEMENT_WILD),
    };
    uint8_t ranks[NUM_RANKS], suits[NUM_SUITS];
    distribution(cards, 5, ranks, suits);
    assert(ranks[ACE] == 5);
    assert(suits[HEARTS] == 5); // Flush Five
}

static void test_flush_house_with_wild_cards(void)
{
    Card cards[5] = {
        make(KING, SPADES, CARD_ENHANCEMENT_NONE),
        make(KING, SPADES, CARD_ENHANCEMENT_NONE),
        make(KING, HEARTS, CARD_ENHANCEMENT_WILD),
        make(TWO, DIAMONDS, CARD_ENHANCEMENT_WILD),
        make(TWO, SPADES, CARD_ENHANCEMENT_NONE),
    };
    uint8_t ranks[NUM_RANKS], suits[NUM_SUITS];
    distribution(cards, 5, ranks, suits);
    assert(ranks[KING] == 3 && ranks[TWO] == 2); // Full House
    assert(suits[SPADES] == 5);                  // Flush House
}

static void test_no_flush_without_wild(void)
{
    Card cards[5] = {
        make(KING, SPADES, CARD_ENHANCEMENT_NONE),
        make(QUEEN, SPADES, CARD_ENHANCEMENT_NONE),
        make(JACK, SPADES, CARD_ENHANCEMENT_BONUS),
        make(TEN, SPADES, CARD_ENHANCEMENT_NONE),
        make(NINE, HEARTS, CARD_ENHANCEMENT_STONE),
    };
    uint8_t ranks[NUM_RANKS], suits[NUM_SUITS];
    distribution(cards, 5, ranks, suits);
    assert(suits[SPADES] == 4); // The Stone card doesn't complete the flush
    assert(ranks[NINE] == 0);
}

static void test_effective_rank(void)
{
    Card stone = make(KING, SPADES, CARD_ENHANCEMENT_STONE);
    Card king = make(KING, SPADES, CARD_ENHANCEMENT_NONE);
    assert(card_effective_rank(&stone) == NUM_RANKS); // The hidden rank never matches
    assert(card_effective_rank(&king) == KING);
}

int main(void)
{
    test_effective_rank();
    test_wild_counts_for_every_suit();
    test_stone_has_no_rank_nor_suit();
    test_flush_five_with_wild_cards();
    test_flush_house_with_wild_cards();
    test_no_flush_without_wild();
    printf("Card Rules Tests Passed :)\n");
    return 0;
}
