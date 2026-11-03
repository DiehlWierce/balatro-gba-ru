/**
 * @file booster_pack.h
 *
 * @brief Booster Packs (ITEM_TYPE_PACK): bought in the Shop, opened right away, the player picks
 *        some of the items they contain.
 *
 * Each pack type is described by a PackInfo: how many choices it shows, how many can be picked,
 * how a choice is rolled and what picking it does. Five kinds (Standard, Arcana, Celestial, Spectral
 * and Buffoon) in three sizes (Normal $4, Jumbo $6, Mega $8), 15 pack types in all.
 */
#ifndef BOOSTER_PACK_H
#define BOOSTER_PACK_H

#include "item.h"

#include <tonc.h>

#define MAX_SHOP_PACKS   2 // Packs for sale in the Shop at the same time
#define MAX_PACK_CHOICES 5 // Items shown when opening the biggest packs

/**
 * @brief Pack types: Normal (the first five), then Jumbo and Mega of each kind. A Tag that gives a
 *        pack (Charm, Meteor, Ethereal, Standard, Buffoon) opens a Mega one.
 */
enum PackType
{
    PACK_TYPE_STANDARD,  // 3 playing cards, pick 1, added to the deck
    PACK_TYPE_ARCANA,    // 3 Tarot cards, pick 1, taken to the consumables row
    PACK_TYPE_CELESTIAL, // 3 Planet cards, pick 1, taken to the consumables row
    PACK_TYPE_SPECTRAL,  // 2 Spectral cards, pick 1, taken to the consumables row
    PACK_TYPE_BUFFOON,   // 2 Jokers, pick 1, taken to the Jokers row
    PACK_TYPE_JUMBO_STANDARD,  // 5, pick 1
    PACK_TYPE_JUMBO_ARCANA,    // 5, pick 1
    PACK_TYPE_JUMBO_CELESTIAL, // 5, pick 1
    PACK_TYPE_JUMBO_SPECTRAL,  // 4, pick 1
    PACK_TYPE_JUMBO_BUFFOON,   // 4, pick 1
    PACK_TYPE_MEGA_STANDARD,   // 5, pick 2
    PACK_TYPE_MEGA_ARCANA,     // 5, pick 2
    PACK_TYPE_MEGA_CELESTIAL,  // 5, pick 2
    PACK_TYPE_MEGA_SPECTRAL,   // 4, pick 2
    PACK_TYPE_MEGA_BUFFOON,    // 4, pick 2
    PACK_TYPE_MAX
};

/**
 * @brief Buffoon Packs are rolled in the Shop. The opened pack screen of the Shop must be able to
 *        show the name and description of a Joker choice; set to 0 to keep them out meanwhile.
 */
#ifndef BOOSTER_BUFFOON_ROLLABLE
#define BOOSTER_BUFFOON_ROLLABLE 1
#endif

enum PackKind
{
    PACK_KIND_STANDARD,
    PACK_KIND_ARCANA,
    PACK_KIND_CELESTIAL,
    PACK_KIND_SPECTRAL,
    PACK_KIND_BUFFOON,
    PACK_KIND_MAX
};

enum PackSize
{
    PACK_SIZE_NORMAL,
    PACK_SIZE_JUMBO,
    PACK_SIZE_MEGA
};

typedef struct BoosterPackObject
{
    Item;         // First member struct inheritance
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
    int weight;     // Chance to be in the Shop, relative to the other packs (hundredths)
    u8 kind;        // enum PackKind
    u8 size;        // enum PackSize
    const char* desc_what; // "Выберите N из M <desc_what>"

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
     * @brief Whether a choice can be taken now (e.g. a free consumable slot), NULL for always.
     */
    bool (*can_pick)(Item* choice);

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
