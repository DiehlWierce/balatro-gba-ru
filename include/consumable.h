/**
 * @file consumable.h
 *
 * @brief Consumable cards (Planets, Tarots, Spectrals): Item implementation, owned consumables
 *        row, use and sell.
 *
 * Planets and Tarots are implemented. Spectral cards are declared for the next waves.
 */
#ifndef CONSUMABLE_H
#define CONSUMABLE_H

#include "hand_type.h"
#include "item.h"
#include "list.h"
#include "random.h"
#include "tarot_rules.h"

#include <tonc.h>

/**
 * @brief Maximum ConsumableObjects alive at once: owned ones, the ones in the Shop and the ones
 *        flying away after being used or sold.
 */
#define MAX_ACTIVE_CONSUMABLES 6

#define PLANET_BUY_PRICE 3
#define TAROT_BUY_PRICE  3

enum ConsumableType
{
    CONSUMABLE_TYPE_PLANET,
    CONSUMABLE_TYPE_TAROT,    // enum TarotType, see tarot_rules.h
    CONSUMABLE_TYPE_SPECTRAL, // Not implemented yet
    CONSUMABLE_TYPE_MAX
};

/**
 * @brief Planet cards, in the order of their sprites in graphics/planet_gfx.png
 */
enum PlanetType
{
    PLANET_MERCURY, // Pair
    PLANET_VENUS,   // Three of a Kind
    PLANET_EARTH,   // Full House
    PLANET_MARS,    // Four of a Kind
    PLANET_JUPITER, // Flush
    PLANET_SATURN,  // Straight
    PLANET_URANUS,  // Two Pair
    PLANET_NEPTUNE, // Straight Flush (and Royal Flush)
    PLANET_PLUTO,   // High Card
    PLANET_X,       // Five of a Kind, secret
    PLANET_CERES,   // Flush House, secret
    PLANET_ERIS,    // Flush Five, secret
    PLANET_TYPE_MAX
};

/**
 * @brief A consumable card, inherits Item (first member struct inheritance)
 */
typedef struct ConsumableObject
{
    Item;
    u8 consumable_type; // enum ConsumableType
    u8 id;              // enum PlanetType for Planets, enum TarotType for Tarots
    s16 layer;          // Sprite layer within CONSUMABLE_SPRITE
} ConsumableObject;

/**
 * @brief Resets the owned consumables list, to be called when a run starts (game_init()).
 */
void consumables_init(void);

/**
 * @brief Destroys every owned consumable, to be called when a run is reset.
 */
void consumables_reset(void);

/**
 * @brief Owned consumables, in display order.
 */
List* get_consumables_list(void);

/**
 * @brief Moves the owned consumables to their place in the consumables panel. Called each frame.
 *
 * @param skipped_item an Item that must not be moved (the Shop's description item), can be NULL
 */
void consumables_update_loop(const Item* skipped_item);

/**
 * @brief Creates a consumable card.
 *
 * @return the new ConsumableObject, NULL if the pool or the sprites are exhausted
 */
ConsumableObject* consumable_object_new(enum ConsumableType type, int id);

void consumable_object_destroy(ConsumableObject** consumable_object);

/**
 * @brief Whether an owned consumable can be used now.
 */
bool consumable_object_can_use(const ConsumableObject* consumable_object);

/**
 * @brief Uses an owned consumable: applies its effect, removes it from the owned list and plays
 *        its discard animation (it is disposed when the animation ends).
 *
 * @return true if it was used
 */
bool consumable_object_use(ConsumableObject* consumable_object);

/**
 * @brief Sells an owned consumable through the Item API and removes it from the owned list.
 */
void consumable_object_sell(ConsumableObject* consumable_object);

/**
 * @brief The poker hand a Planet levels up.
 */
enum HandType planet_get_hand_type(enum PlanetType planet);

/**
 * @brief Whether a Planet can appear: secret Planets need their poker hand to have been played.
 */
bool planet_is_rollable(enum PlanetType planet);

/**
 * @brief Rolls a random rollable Planet.
 *
 * @return the Planet, PLANET_TYPE_MAX if none is rollable
 */
enum PlanetType planet_roll(enum RngSequence key);

/**
 * @brief Rolls a random rollable Tarot.
 *
 * @return the Tarot, TAROT_TYPE_MAX if none is rollable
 */
enum TarotType tarot_roll(enum RngSequence key);

/**
 * @brief Whether the Tarot can be used now with the cards selected in the hand.
 */
bool tarot_can_use_now(enum TarotType tarot);

/**
 * @brief Creates a consumable and puts it in the owned row at once, if a slot is free.
 *
 * @param x, y screen position where it appears
 *
 * @return true if it was created
 */
bool consumable_create_owned(enum ConsumableType type, int id, FIXED x, FIXED y);

/**
 * @brief Creates a random Tarot in the owned row, if a slot is free (Purple seal).
 */
bool consumable_create_random_tarot(enum RngSequence key, FIXED x, FIXED y);

/**
 * @brief Forgets the last used consumable (The Fool), when a run starts.
 */
void consumables_forget_last_used(void);

// Item API implementation, see item_funcs.c

Item* consumable_object_roll_new(enum RngSequence key);
int consumable_object_get_buy_price(Item* item);
int consumable_object_get_sell_price(Item* item);
const char* consumable_object_get_name(Item* item);
ItemDescSubtypeInfo consumable_object_get_subtype_info(Item* item);
bool consumable_object_can_acquire(Item* item);
void consumable_object_acquire(Item* item);
void consumable_object_dispose(Item** item);
int consumable_object_print_description(Item* item, Rect dest_rect);

#endif // CONSUMABLE_H
