/**
 * @file voucher.h
 *
 * @brief Vouchers: permanent run upgrades bought in the Shop, one offered per Ante.
 *
 * Each Voucher has a base version (even value) and an upgraded version (base + 1) that can only
 * appear once the base version is owned. Owned Vouchers are the bits of
 * g_game_vars.owned_vouchers. Vouchers whose system doesn't exist yet (Hone, Glow Up: Editions) are disabled in the pool.
 */
#ifndef VOUCHER_H
#define VOUCHER_H

#include "item.h"
#include "random.h"

#include <tonc.h>

#define VOUCHER_BUY_PRICE 10

/**
 * @brief Maximum VoucherObjects alive at once: the one in the Shop and one flying away
 */
#define MAX_ACTIVE_VOUCHERS 2

/**
 * @brief All Vouchers, in the order of their sprites in graphics/voucher_gfx.png.
 *        Upgraded versions directly follow their base version.
 */
enum VoucherType
{
    VOUCHER_OVERSTOCK,
    VOUCHER_OVERSTOCK_PLUS,
    VOUCHER_CLEARANCE_SALE,
    VOUCHER_LIQUIDATION,
    VOUCHER_HONE,
    VOUCHER_GLOW_UP,
    VOUCHER_REROLL_SURPLUS,
    VOUCHER_REROLL_GLUT,
    VOUCHER_CRYSTAL_BALL,
    VOUCHER_OMEN_GLOBE,
    VOUCHER_TELESCOPE,
    VOUCHER_OBSERVATORY,
    VOUCHER_GRABBER,
    VOUCHER_NACHO_TONG,
    VOUCHER_WASTEFUL,
    VOUCHER_RECYCLOMANCY,
    VOUCHER_TAROT_MERCHANT,
    VOUCHER_TAROT_TYCOON,
    VOUCHER_PLANET_MERCHANT,
    VOUCHER_PLANET_TYCOON,
    VOUCHER_SEED_MONEY,
    VOUCHER_MONEY_TREE,
    VOUCHER_BLANK,
    VOUCHER_ANTIMATTER,
    VOUCHER_MAGIC_TRICK,
    VOUCHER_ILLUSION,
    VOUCHER_HIEROGLYPH,
    VOUCHER_PETROGLYPH,
    VOUCHER_DIRECTORS_CUT,
    VOUCHER_RETCON,
    VOUCHER_PAINT_BRUSH,
    VOUCHER_PALETTE,
    VOUCHER_TYPE_MAX,
    VOUCHER_NONE = VOUCHER_TYPE_MAX
};

_Static_assert(VOUCHER_TYPE_MAX <= 32, "owned_vouchers is a u32 bitset");

/**
 * @brief A Voucher in the Shop, inherits Item (first member struct inheritance)
 */
typedef struct VoucherObject
{
    Item;
    u8 voucher_type; // enum VoucherType
    s16 layer;       // Sprite layer within VOUCHER_SPRITE
} VoucherObject;

/**
 * @brief Forgets the Voucher offered in the Shop, to be called when a run starts (game_init()).
 */
void vouchers_reset(void);

/**
 * @brief Whether a Voucher is owned.
 */
bool voucher_is_owned(enum VoucherType voucher);

/**
 * @brief Whether a Voucher can currently be offered in the Shop: implemented, not owned and, for
 *        an upgraded Voucher, its base version owned.
 */
bool voucher_is_rollable(enum VoucherType voucher);

/**
 * @brief The Voucher offered in the Shop during the current Ante. A new one is rolled when the
 *        Ante changes, the same one is offered until it is bought.
 *
 * @return the Voucher, VOUCHER_NONE if it was bought this Ante or none is rollable
 */
enum VoucherType voucher_get_shop_offer(void);

/**
 * @brief Marks a Voucher as owned and applies its effect.
 */
void voucher_redeem(enum VoucherType voucher);

// Effects queried by other systems

/**
 * @brief Discount in percent applied to Shop cards (Clearance Sale, Liquidation).
 */
int voucher_get_shop_discount_percent(void);

/**
 * @brief Price after the Shop discount, at least $1 for a non-free item.
 */
int voucher_apply_shop_discount(int price);

/**
 * @brief Reroll cost reduction in $ (Reroll Surplus, Reroll Glut).
 */
int voucher_get_reroll_discount(void);

/**
 * @brief Weight of Planets in the Shop's top row, in tenths (Planet Merchant, Planet Tycoon).
 */
int voucher_get_planet_shop_weight_x10(void);

/**
 * @brief Weight of Tarots among the Shop consumables, in tenths (Tarot Merchant, Tarot Tycoon).
 */
int voucher_get_tarot_shop_weight_x10(void);

/**
 * @brief Weight of playing cards in the Shop's top row, in tenths: 0 without Magic Trick.
 *        Illusion (voucher_is_owned) gives them enhancements and seals.
 */
int voucher_get_playing_card_shop_weight_x10(void);

/**
 * @brief Whether the Boss Blind can be rerolled now (Director's Cut once per Ante, Retcon always).
 */
bool voucher_can_reroll_boss(void);

/**
 * @brief Records a Boss reroll paid through Director's Cut / Retcon.
 */
void voucher_on_boss_rerolled(void);

#define VOUCHER_BOSS_REROLL_PRICE 10

// VoucherObject

/**
 * @brief Creates the Shop item of a Voucher.
 *
 * @return the new VoucherObject, NULL if the pool or the sprites are exhausted
 */
VoucherObject* voucher_object_new(enum VoucherType voucher);

void voucher_object_destroy(VoucherObject** voucher_object);

// Item API implementation, see item_funcs.c

Item* voucher_object_roll_new(enum RngSequence key);
int voucher_object_get_buy_price(Item* item);
int voucher_object_get_sell_price(Item* item);
const char* voucher_object_get_name(Item* item);
ItemDescSubtypeInfo voucher_object_get_subtype_info(Item* item);
bool voucher_object_can_acquire(Item* item);
void voucher_object_acquire(Item* item);
void voucher_object_dispose(Item** item);
int voucher_object_print_description(Item* item, Rect dest_rect);

#endif // VOUCHER_H
