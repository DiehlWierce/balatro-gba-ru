#include "item_funcs.h"

#include "card.h"
#include "consumable.h"
#include "game.h"
#include "game/shop.h"
#include "joker.h"
#include "util.h"

static bool item_always_can_acquire(Item* item);

// Placeholders for the item types that are declared but not implemented yet
static Item* item_placeholder_roll_new(enum RngSequence key);
static int item_placeholder_get_price(Item* item);
static const char* item_placeholder_get_name(Item* item);
static ItemDescSubtypeInfo item_placeholder_get_subtype_info(Item* item);
static bool item_placeholder_can_acquire(Item* item);
static void item_placeholder_acquire(Item* item);
static void item_placeholder_dispose(Item** item);
static int item_placeholder_print_description(Item* item, Rect dest_rect);

/**
 * @brief Function table of an item type that is not implemented yet.
 *
 * It can't be rolled (roll_new returns NULL) nor acquired, so it never appears in the Shop.
 * Replace the item type's entry in item_func_table when implementing it.
 */
// clang-format off
#define ITEM_PLACEHOLDER_FUNCS {                                 \
        .roll_new = item_placeholder_roll_new,                   \
        .get_buy_price = item_placeholder_get_price,             \
        .get_sell_price = item_placeholder_get_price,            \
        .get_name = item_placeholder_get_name,                   \
        .get_subtype_info = item_placeholder_get_subtype_info,   \
        .acquire = item_placeholder_acquire,                     \
        .can_acquire = item_placeholder_can_acquire,             \
        .dispose = item_placeholder_dispose,                     \
        .print_description = item_placeholder_print_description \
    }
// clang-format on

// clang-format off
ItemFuncs item_func_table[] = {
    [ITEM_TYPE_JOKER] = {
        .roll_new = joker_object_roll_new,
        .get_buy_price = joker_object_get_buy_price,
        .get_sell_price = joker_object_get_sell_price,
        .get_name = joker_object_get_name,
        .get_subtype_info = joker_object_get_rarity_info,
        .acquire = joker_object_add_to_owned,
        .can_acquire = joker_object_can_acquire,
        .dispose = joker_object_dispose,
        .print_description = joker_object_print_description
    },
    
    /* Currently playing cards have partial implementations since Magic Trick is not implemented
     * and playing cards can't appear in the shop.
     * If the Magic Trick voucher is implemented, these need to be completed.
     * Playing cards must stay the only exception, the rest of the item types must implement
     * all the basic functions to appear in the shop.
     */
    [ITEM_TYPE_PLAYING_CARD] = {
        .roll_new = NULL,
        .get_buy_price = card_object_get_buy_price,
        .get_sell_price = NULL,
        .get_name = NULL,
        .get_subtype_info = NULL,
        .acquire = NULL,
        .can_acquire = item_always_can_acquire,
        .dispose = card_object_dispose,
        .print_description = NULL
    },

    [ITEM_TYPE_CONSUMABLE] = {
        .roll_new = consumable_object_roll_new,
        .get_buy_price = consumable_object_get_buy_price,
        .get_sell_price = consumable_object_get_sell_price,
        .get_name = consumable_object_get_name,
        .get_subtype_info = consumable_object_get_subtype_info,
        .acquire = consumable_object_acquire,
        .can_acquire = consumable_object_can_acquire,
        .dispose = consumable_object_dispose,
        .print_description = consumable_object_print_description
    },
    [ITEM_TYPE_VOUCHER] = ITEM_PLACEHOLDER_FUNCS,
    [ITEM_TYPE_PACK] = ITEM_PLACEHOLDER_FUNCS
};
// clang-format on

ItemFuncs* get_item_type_funcs(enum ItemType type)
{
    if ((int)type < 0 || type >= NUM_ELEM_IN_ARR(item_func_table))
    {
        MGBA_FUNC_ERROR("Invalid type %d", type);
        return NULL;
    }

    return &item_func_table[type];
}

static bool item_always_can_acquire(Item* item)
{
    return true;
}

static Item* item_placeholder_roll_new(enum RngSequence key)
{
    return NULL;
}

static int item_placeholder_get_price(Item* item)
{
    return 0;
}

static const char* item_placeholder_get_name(Item* item)
{
    return ITEM_NAME_UNDEFINED;
}

static ItemDescSubtypeInfo item_placeholder_get_subtype_info(Item* item)
{
    return (ItemDescSubtypeInfo)ITEM_SUBTYPE_INFO_DEFAULT;
}

static bool item_placeholder_can_acquire(Item* item)
{
    return false;
}

static void item_placeholder_acquire(Item* item)
{
}

static void item_placeholder_dispose(Item** item)
{
    if (item != NULL)
        *item = NULL;
}

static int item_placeholder_print_description(Item* item, Rect dest_rect)
{
    return 0;
}
