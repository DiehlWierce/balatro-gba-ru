#ifndef CARD_H
#define CARD_H

#include "card_rules.h"
#include "deck_types.h"
#include "game/common_ui.h"
#include "item.h"

#include <maxmod.h>
#include <tonc.h>

#define MAX_CARDS           128                 // Arbitrary high value so we don't have issues
#define MAX_CARDS_ON_SCREEN (MAX_HAND_SIZE + 1) // Add room for the Deck sprite

#define IMPOSSIBLY_HIGH_CARD_VALUE 100

// Card sprites
#define DEFAULT_HIGH_CONTRAST false
#define DEFAULT_MORE_READABLE false

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

/**
 * @brief Chips given by the card when it scores: its value, +30 for Bonus cards,
 *        50 for Stone cards.
 */
int card_get_chips(Card* card);

/**
 * @brief Russian name of the card's enhancement, NULL if it has none.
 */
const char* card_get_enhancement_name(const Card* card);

/**
 * @brief Russian name of the card's seal, NULL if it has none.
 */
const char* card_get_seal_name(const Card* card);

/**
 * @brief Index in the card palette (CARD_PB) of the colour closest to the given 8 bit RGB.
 */
int card_palette_find_color(u8 r, u8 g, u8 b);

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
