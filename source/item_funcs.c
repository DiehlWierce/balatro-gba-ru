#include "item_funcs.h"

#include "booster_pack.h"
#include "card.h"
#include "consumable.h"
#include "game.h"
#include "game/shop.h"
#include "joker.h"
#include "util.h"
#include "voucher.h"

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
    
    // Playing cards appear in the Shop with Magic Trick. They can't be sold.
    [ITEM_TYPE_PLAYING_CARD] = {
        .roll_new = card_object_roll_new,
        .get_buy_price = card_object_get_buy_price,
        .get_sell_price = NULL,
        .get_name = card_object_get_name,
        .get_subtype_info = card_object_get_subtype_info,
        .acquire = card_object_acquire,
        .can_acquire = card_object_can_acquire,
        .dispose = card_object_dispose,
        .print_description = card_object_print_description
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
    [ITEM_TYPE_VOUCHER] = {
        .roll_new = voucher_object_roll_new,
        .get_buy_price = voucher_object_get_buy_price,
        .get_sell_price = voucher_object_get_sell_price,
        .get_name = voucher_object_get_name,
        .get_subtype_info = voucher_object_get_subtype_info,
        .acquire = voucher_object_acquire,
        .can_acquire = voucher_object_can_acquire,
        .dispose = voucher_object_dispose,
        .print_description = voucher_object_print_description
    },

    [ITEM_TYPE_PACK] = {
        .roll_new = booster_pack_roll_new,
        .get_buy_price = booster_pack_get_buy_price,
        .get_sell_price = NULL,
        .get_name = booster_pack_get_name,
        .get_subtype_info = booster_pack_get_subtype_info,
        .acquire = booster_pack_acquire,
        .can_acquire = booster_pack_can_acquire,
        .dispose = booster_pack_dispose,
        .print_description = booster_pack_print_description
    }
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
