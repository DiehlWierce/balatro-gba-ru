/**
 * @file card_rules.h
 *
 * @brief Playing card data and the rules of ranks and suits (Wild and Stone cards).
 *
 * Plain C without libtonc so it can be tested on the host (tests/card_rules).
 */
#ifndef CARD_RULES_H
#define CARD_RULES_H

#include <stdbool.h>
#include <stdint.h>

#include "edition_rules.h"

// Card suits
#define DIAMONDS  0
#define CLUBS     1
#define HEARTS    2
#define SPADES    3
#define NUM_SUITS 4

// Smeared Joker: Hearts and Diamonds count as the same suit, Spades and Clubs too (set by game.c).
// A weak definition so the header works in every translation unit and in the host tests.
__attribute__((weak)) bool g_card_rules_smeared = false;

// Card ranks
#define TWO         0
#define THREE       1
#define FOUR        2
#define FIVE        3
#define SIX         4
#define SEVEN       5
#define EIGHT       6
#define NINE        7
#define TEN         8
#define JACK        9
#define QUEEN       10
#define KING        11
#define ACE         12
#define NUM_RANKS   13
#define RANK_OFFSET 2 // Because the first rank is 2 and ranks start at 0

/**
 * @brief Card enhancements, a card has at most one (none by default)
 */
enum CardEnhancement
{
    CARD_ENHANCEMENT_NONE,
    CARD_ENHANCEMENT_BONUS, // +30 chips
    CARD_ENHANCEMENT_MULT,  // +4 mult
    CARD_ENHANCEMENT_WILD,  // Counts as every suit
    CARD_ENHANCEMENT_GLASS, // X2 mult, 1 in 4 chance to be destroyed
    CARD_ENHANCEMENT_STEEL, // X1.5 mult while held in hand
    CARD_ENHANCEMENT_STONE, // +50 chips, no rank or suit, always scores
    CARD_ENHANCEMENT_GOLD,  // $3 if held in hand at the end of the round
    CARD_ENHANCEMENT_LUCKY, // 1 in 5 chance of +20 mult, 1 in 15 chance of $20
    CARD_ENHANCEMENT_MAX
};

/**
 * @brief Card seals, a card has at most one (none by default)
 */
enum CardSeal
{
    CARD_SEAL_NONE,
    CARD_SEAL_GOLD,   // $3 when scored
    CARD_SEAL_RED,    // Retrigger the card
    CARD_SEAL_BLUE,   // Creates the Planet of the final hand if held at the end of the round
    CARD_SEAL_PURPLE, // Creates a Tarot when discarded
    CARD_SEAL_MAX
};

// Card types
typedef struct Card
{
    uint8_t suit;
    uint8_t rank;
    uint8_t enhancement; // enum CardEnhancement
    uint8_t seal;        // enum CardSeal
    // Set by Boss Blinds: the card doesn't score and doesn't trigger Jokers
    bool debuffed : 1;
    // Set by Boss Blinds: the card is drawn face down
    bool face_down : 1;
    // enum Edition (Foil, Holographic, Polychrome), packed with the flags to keep Card at 8 bytes
    uint8_t edition : 6;
    uint8_t played_ante; // Last Ante the card was played in, 0 if never (The Pillar)
    uint8_t bonus_chips; // Chips added for good by Hiker
    uint8_t reserved;    // Free: keeps Card at 8 bytes
} Card;

_Static_assert(sizeof(Card) == 8, "Card pool (MAX_CARDS entries in EWRAM) and its tests assume 8 bytes");

/**
 * @brief Whether the card has a rank (Stone cards have none).
 */
static inline bool card_has_rank(const Card* card)
{
    return card->enhancement != CARD_ENHANCEMENT_STONE;
}

/**
 * @brief Whether the card counts as @p suit: Wild cards count as every suit, Stone cards as none.
 */
static inline bool card_has_suit(const Card* card, uint8_t suit)
{
    if (card->enhancement == CARD_ENHANCEMENT_STONE)
        return false;
    if (card->enhancement == CARD_ENHANCEMENT_WILD || card->suit == suit)
        return true;
    // Red suits are Diamonds and Hearts, black suits are Clubs and Spades: the same parity
    return g_card_rules_smeared && ((card->suit ^ suit) & 1) == 0;
}

/**
 * @brief Rank of the card for effects that compare ranks: NUM_RANKS (matches nothing) for Stone cards.
 */
static inline uint8_t card_effective_rank(const Card* card)
{
    return card_has_rank(card) ? card->rank : NUM_RANKS;
}

/**
 * @brief Whether both cards have a rank and it is the same.
 */
static inline bool cards_same_rank(const Card* a, const Card* b)
{
    return card_has_rank(a) && card_has_rank(b) && a->rank == b->rank;
}

/**
 * @brief Adds a card to rank and suit counts: Stone cards count for nothing, Wild cards count
 *        once for every suit.
 *
 * @param card the card to count
 * @param ranks ranks[rank] is the number of cards of that rank, size NUM_RANKS
 * @param suits suits[suit] is the number of cards of that suit, size NUM_SUITS
 */
static inline void card_add_to_distribution(
    const Card* card,
    uint8_t ranks[NUM_RANKS],
    uint8_t suits[NUM_SUITS]
)
{
    if (!card_has_rank(card))
        return; // Stone card: no rank, no suit

    ranks[card->rank]++;
    for (uint8_t suit = 0; suit < NUM_SUITS; suit++)
    {
        if (card_has_suit(card, suit))
            suits[suit]++;
    }
}

#endif // CARD_RULES_H
