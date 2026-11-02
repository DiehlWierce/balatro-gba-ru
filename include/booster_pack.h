/**
 * @file booster_pack.h
 *
 * @brief Booster Packs (ITEM_TYPE_PACK): bought in the Shop, opened right away, the player picks
 *        some of the items they contain.
 *
 * Each pack type is described by a PackInfo: how many choices it shows, how many can be picked,
 * how a choice is rolled and what picking it does. Only the Standard Pack (playing cards) is
 * implemented; Arcana (Tarots) and Celestial (Planets) are declared for later and never rolled.
 */
#ifndef BOOSTER_PACK_H
#define BOOSTER_PACK_H

#include "item.h"

#include <tonc.h>

#define MAX_SHOP_PACKS   2 // Packs for sale in the Shop at the same time
#define MAX_PACK_CHOICES 5 // Items shown when opening the biggest packs

enum PackType
{
    PACK_TYPE_STANDARD,  // Playing cards, added to the deck
    PACK_TYPE_ARCANA,    // Tarot cards, not implemented yet
    PACK_TYPE_CELESTIAL, // Planet cards, not implemented yet
    PACK_TYPE_MAX
};

typedef struct BoosterPackObject
{
    Item; // First member struct inheritance
    u8 pack_type; // enum PackType
    s8 layer;     // Layer of the PACK_SPRITE used by this pack
} BoosterPackObject;

/**
 * @brief Description of a pack type
 */
typedef struct PackInfo
{
    const char* name;
    int price;
    int nb_choices; // Items shown when opening the pack
    int nb_picks;   // Items the player can take

    /**
     * @brief Creates choice @p idx of an opened pack, NULL when the pack type isn't implemented.
     *        The Item is placed by the caller.
     */
    Item* (*roll_choice)(int idx);

    /**
     * @brief Takes a choice: applies it (e.g. adds the card to the deck) and disposes of @p choice.
     */
    void (*pick)(Item** choice);

    /**
     * @brief Disposes of a choice that was not picked.
     */
    void (*discard)(Item** choice);

    u8 body_r, body_g, body_b; // Colour of the wrapper, matched in the card palette
} PackInfo;

/**
 * @brief Info of a pack type, NULL if @p pack_type is invalid.
 */
const PackInfo* booster_pack_get_info(enum PackType pack_type);

/**
 * @brief Whether a pack type can appear in the Shop (implemented).
 */
bool booster_pack_is_rollable(enum PackType pack_type);

/**
 * @brief Creates a pack with its sprite, NULL if no sprite layer is free.
 */
BoosterPackObject* booster_pack_new(enum PackType pack_type);

void booster_pack_destroy(BoosterPackObject** pack);

// Item functions, see ItemFuncs
Item* booster_pack_roll_new(enum RngSequence key);
int booster_pack_get_buy_price(Item* item);
const char* booster_pack_get_name(Item* item);
ItemDescSubtypeInfo booster_pack_get_subtype_info(Item* item);
bool booster_pack_can_acquire(Item* item);
void booster_pack_acquire(Item* item);
void booster_pack_dispose(Item** item);
int booster_pack_print_description(Item* item, Rect dest_rect);

#endif // BOOSTER_PACK_H
