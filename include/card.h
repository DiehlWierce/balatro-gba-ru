#ifndef CARD_H
#define CARD_H

#include "deck_types.h"
#include "game/common_ui.h"
#include "item.h"

#include <maxmod.h>
#include <tonc.h>

#define MAX_CARDS           128                 // Arbitrary high value so we don't have issues
#define MAX_CARDS_ON_SCREEN (MAX_HAND_SIZE + 1) // Add room for the Deck sprite

// Card suits
#define DIAMONDS  0
#define CLUBS     1
#define HEARTS    2
#define SPADES    3
#define NUM_SUITS 4

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

#define IMPOSSIBLY_HIGH_CARD_VALUE 100

// Card sprites
#define DEFAULT_HIGH_CONTRAST false
#define DEFAULT_MORE_READABLE false

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
    u8 suit;
    u8 rank;
    u8 enhancement; // enum CardEnhancement
    u8 seal;        // enum CardSeal
    bool debuffed;  // Set by Boss Blinds: the card doesn't score and doesn't trigger Jokers
    bool face_down; // Set by Boss Blinds: the card is drawn face down
    u8 played_ante; // Last Ante the card was played in, 0 if never (The Pillar)
} Card;

typedef struct CardObject
{
    Item; // First member struct inheritance
    Card* card;
    bool selected;
} CardObject;

void card_init(void);

// Card sprites accessibility functions
void set_cards_high_contrast(bool enable);
void set_cards_more_readable(bool enable);
bool get_cards_high_contrast(void);
bool get_cards_more_readable(void);

// Card methods
Card* card_new(u8 suit, u8 rank);
void card_destroy(Card** card);
u8 card_get_value(Card* card);

// CardObject methods
CardObject* card_object_new(Card* card);
void card_object_destroy(CardObject** card_object);
void card_object_set_sprite(CardObject* card_object, s16 layer);
void card_object_set_sprite_face_down(CardObject* card_object, enum DeckType deck, s16 layer);

/**
 * @brief Copies the 32x32 back of the run's Deck (4bpp, palette bank DECK_PB) into @p dst_tiles,
 *        e.g. to show a face down Joker.
 */
void card_copy_deck_back_tiles(void* dst_tiles);
void card_object_shake(CardObject* card_object, mm_word sound_id);

void card_object_set_selected(CardObject* card_object, bool selected);
bool card_object_is_selected(CardObject* card_object);
Sprite* card_object_get_sprite(CardObject* card_object);

int card_object_get_buy_price(Item* card_object);
void card_object_dispose(Item** card_object);

#endif // CARD_H
