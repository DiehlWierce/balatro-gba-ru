/**
 * @file voucher.c
 *
 * @brief Vouchers, see voucher.h.
 */
#include "voucher.h"

#include "game.h"
#include "game/common_ui.h"
#include "game_variables.h"
#include "graphic_utils.h"
#include "pool.h"
#include "spectral_rules.h"
#include "sprite.h"
#include "util.h"

// Tiles and palettes
#include "card_rarity_pal_gfx.h"
#include "voucher_gfx.h"

#include <tonc.h>

#define VOUCHER_SUBTYPE_PAL_IDX 13 // Voucher colors in card_rarity_pal_gfx.png

#define CLEARANCE_SALE_DISCOUNT_PERCENT 25
#define LIQUIDATION_DISCOUNT_PERCENT    50
#define REROLL_VOUCHER_DISCOUNT         2
#define SEED_MONEY_INTEREST_CAP         10
#define MONEY_TREE_INTEREST_CAP         20

// Weights of Planets in the Shop top row, in tenths (Jokers weigh 200)
#define PLAYING_CARD_SHOP_WEIGHT_X10    40 // Magic Trick, like the Planets
#define PLANET_SHOP_WEIGHT_X10          40
#define PLANET_MERCHANT_SHOP_WEIGHT_X10 96
#define PLANET_TYCOON_SHOP_WEIGHT_X10   320

typedef struct
{
    const char* name;
    const char* description;
    bool enabled; // false while the system the Voucher upgrades doesn't exist
} VoucherInfo;

// clang-format off
static const VoucherInfo VOUCHER_INFO[VOUCHER_TYPE_MAX] = {
    [VOUCHER_OVERSTOCK]       = {"Затоварка",
        TTE_YELLOW_TAG "+1 " TTE_BLACK_TAG "слот карт в лавке", true},
    [VOUCHER_OVERSTOCK_PLUS]  = {"Затоварка+",
        TTE_YELLOW_TAG "+1 " TTE_BLACK_TAG "слот карт в лавке", true},
    [VOUCHER_CLEARANCE_SALE]  = {"Распродажа",
        TTE_BLACK_TAG "Все карты в лавке дешевле на " TTE_YELLOW_TAG "25%", true},
    [VOUCHER_LIQUIDATION]     = {"Ликвидация",
        TTE_BLACK_TAG "Все карты в лавке дешевле на " TTE_YELLOW_TAG "50%", true},
    [VOUCHER_HONE]            = {"Заточка",
        TTE_BLACK_TAG "Издания джокеров в " TTE_YELLOW_TAG "2 раза " TTE_BLACK_TAG "чаще", false},
    [VOUCHER_GLOW_UP]         = {"Блеск",
        TTE_BLACK_TAG "Издания джокеров в " TTE_YELLOW_TAG "4 раза " TTE_BLACK_TAG "чаще", false},
    [VOUCHER_REROLL_SURPLUS]  = {"Лишний реролл",
        TTE_BLACK_TAG "Реролл дешевле на " TTE_YELLOW_TAG "$2", true},
    [VOUCHER_REROLL_GLUT]     = {"Избыток реролла",
        TTE_BLACK_TAG "Реролл дешевле ещё на " TTE_YELLOW_TAG "$2", true},
    [VOUCHER_CRYSTAL_BALL]    = {"Хрустальный шар",
        TTE_YELLOW_TAG "+1 " TTE_BLACK_TAG "слот расходников", true},
    [VOUCHER_OMEN_GLOBE]      = {"Шар знамений",
        TTE_BLACK_TAG "В бустерах Таро бывают спектральные карты", true},
    [VOUCHER_TELESCOPE]       = {"Телескоп",
        TTE_BLACK_TAG "Бустеры планет дают планету самой частой руки", true},
    [VOUCHER_OBSERVATORY]     = {"Обсерватория",
        TTE_BLACK_TAG "Планеты в слотах дают " TTE_RED_TAG "X1.5 " TTE_BLACK_TAG "множ. своей руке",
        true},
    [VOUCHER_GRABBER]         = {"Загребущий",
        TTE_BLUE_TAG "+1 " TTE_BLACK_TAG "рука каждый раунд", true},
    [VOUCHER_NACHO_TONG]      = {"Щипцы для начос",
        TTE_BLUE_TAG "+1 " TTE_BLACK_TAG "рука каждый раунд", true},
    [VOUCHER_WASTEFUL]        = {"Транжира",
        TTE_RED_TAG "+1 " TTE_BLACK_TAG "сброс каждый раунд", true},
    [VOUCHER_RECYCLOMANCY]    = {"Переработка",
        TTE_RED_TAG "+1 " TTE_BLACK_TAG "сброс каждый раунд", true},
    [VOUCHER_TAROT_MERCHANT]  = {"Торговец Таро",
        TTE_BLACK_TAG "Таро в лавке в " TTE_YELLOW_TAG "2 раза " TTE_BLACK_TAG "чаще", true},
    [VOUCHER_TAROT_TYCOON]    = {"Магнат Таро",
        TTE_BLACK_TAG "Таро в лавке в " TTE_YELLOW_TAG "4 раза " TTE_BLACK_TAG "чаще", true},
    [VOUCHER_PLANET_MERCHANT] = {"Торговец планет",
        TTE_BLACK_TAG "Планеты в лавке в " TTE_YELLOW_TAG "2 раза " TTE_BLACK_TAG "чаще", true},
    [VOUCHER_PLANET_TYCOON]   = {"Магнат планет",
        TTE_BLACK_TAG "Планеты в лавке в " TTE_YELLOW_TAG "4 раза " TTE_BLACK_TAG "чаще", true},
    [VOUCHER_SEED_MONEY]      = {"Стартовый капитал",
        TTE_BLACK_TAG "Максимум процента " TTE_YELLOW_TAG "$10", true},
    [VOUCHER_MONEY_TREE]      = {"Денежное дерево",
        TTE_BLACK_TAG "Максимум процента " TTE_YELLOW_TAG "$20", true},
    [VOUCHER_BLANK]           = {"Пустышка",
        TTE_BLACK_TAG "Ничего не делает?", true},
    [VOUCHER_ANTIMATTER]      = {"Антиматерия",
        TTE_YELLOW_TAG "+1 " TTE_BLACK_TAG "слот джокера", true},
    [VOUCHER_MAGIC_TRICK]     = {"Фокус",
        TTE_BLACK_TAG "Игральные карты продаются в лавке", true},
    [VOUCHER_ILLUSION]        = {"Иллюзия",
        TTE_BLACK_TAG "Карты в лавке бывают с улучшениями и печатями", true},
    [VOUCHER_HIEROGLYPH]      = {"Иероглиф",
        TTE_YELLOW_TAG "-1 " TTE_BLACK_TAG "анте, " TTE_BLUE_TAG "-1 " TTE_BLACK_TAG
        "рука каждый раунд", true},
    [VOUCHER_PETROGLYPH]      = {"Петроглиф",
        TTE_YELLOW_TAG "-1 " TTE_BLACK_TAG "анте, " TTE_RED_TAG "-1 " TTE_BLACK_TAG
        "сброс каждый раунд", true},
    [VOUCHER_DIRECTORS_CUT]   = {"Режиссёрская",
        TTE_BLACK_TAG "Реролл босса за " TTE_YELLOW_TAG "$10" TTE_BLACK_TAG
        ", раз за анте (R на выборе блайнда)", true},
    [VOUCHER_RETCON]          = {"Реткон",
        TTE_BLACK_TAG "Реролл босса за " TTE_YELLOW_TAG "$10 " TTE_BLACK_TAG "без ограничений",
        true},
    [VOUCHER_PAINT_BRUSH]     = {"Кисть",
        TTE_YELLOW_TAG "+1 " TTE_BLACK_TAG "к размеру руки", true},
    [VOUCHER_PALETTE]         = {"Палитра",
        TTE_YELLOW_TAG "+1 " TTE_BLACK_TAG "к размеру руки", true},
};
// clang-format on

// Voucher offered in the Shop during s_offer_ante, VOUCHER_NONE once bought
static u8 s_offer = VOUCHER_NONE;
static s16 s_offer_ante = 0;
// Ante of the last Boss reroll paid with Director's Cut
static s16 s_boss_reroll_ante = 0;
static bool s_used_layers[MAX_ACTIVE_VOUCHERS] = {false};

static inline bool voucher_is_valid(int voucher)
{
    return voucher >= 0 && voucher < VOUCHER_TYPE_MAX;
}

static inline bool voucher_is_upgrade(enum VoucherType voucher)
{
    return (voucher % 2) == 1;
}

void vouchers_reset(void)
{
    s_offer = VOUCHER_NONE;
    s_offer_ante = 0;
    s_boss_reroll_ante = 0;
}

bool voucher_is_owned(enum VoucherType voucher)
{
    return voucher_is_valid(voucher) && (g_game_vars.owned_vouchers & (1u << voucher));
}

bool voucher_is_rollable(enum VoucherType voucher)
{
    if (!voucher_is_valid(voucher) || !VOUCHER_INFO[voucher].enabled || voucher_is_owned(voucher))
        return false;

    return !voucher_is_upgrade(voucher) || voucher_is_owned(voucher - 1);
}

static enum VoucherType voucher_roll(enum RngSequence key)
{
    int nb_rollable = 0;
    for (int voucher = 0; voucher < VOUCHER_TYPE_MAX; voucher++)
    {
        if (voucher_is_rollable(voucher))
            nb_rollable++;
    }

    if (nb_rollable == 0)
        return VOUCHER_NONE;

    int pick = rng_get_u32(key) % nb_rollable;
    for (int voucher = 0; voucher < VOUCHER_TYPE_MAX; voucher++)
    {
        if (voucher_is_rollable(voucher) && pick-- == 0)
            return voucher;
    }

    return VOUCHER_NONE;
}

enum VoucherType voucher_get_shop_offer(void)
{
    if (s_offer_ante != g_game_vars.ante)
    {
        s_offer_ante = g_game_vars.ante;
        s_offer = voucher_roll(RNG_SEQ_SHOP_VOUCHERS);
    }

    // The offer may have become unavailable (e.g. owned through another way)
    if (s_offer != VOUCHER_NONE && !voucher_is_rollable(s_offer))
        s_offer = VOUCHER_NONE;

    return s_offer;
}

static inline void clamp_min(s32* value, s32 min)
{
    if (*value < min)
        *value = min;
}

static inline void clamp_max(s32* value, s32 max)
{
    if (*value > max)
        *value = max;
}

static void voucher_lower_ante(void)
{
    g_game_vars.ante--;
    clamp_min(&g_game_vars.ante, 1);
    // The Voucher offer of this Ante was already bought, lowering the Ante must not roll a new one
    s_offer_ante = g_game_vars.ante;
    display_ante();
}

void voucher_redeem(enum VoucherType voucher)
{
    if (!voucher_is_valid(voucher))
        return;

    g_game_vars.owned_vouchers |= 1u << voucher;
    if (voucher == s_offer)
        s_offer = VOUCHER_NONE;

    switch (voucher)
    {
        case VOUCHER_OVERSTOCK:
        case VOUCHER_OVERSTOCK_PLUS:
            g_game_vars.shop_slots++;
            clamp_max(&g_game_vars.shop_slots, MAX_SHOP_ITEMS);
            break;

        case VOUCHER_CRYSTAL_BALL:
            g_game_vars.consumable_slots++;
            break;

        case VOUCHER_GRABBER:
        case VOUCHER_NACHO_TONG:
            g_game_vars.hands_per_round++;
            g_game_vars.hands++;
            display_hands();
            break;

        case VOUCHER_WASTEFUL:
        case VOUCHER_RECYCLOMANCY:
            g_game_vars.discards_per_round++;
            g_game_vars.discards++;
            display_discards();
            break;

        case VOUCHER_SEED_MONEY:
        case VOUCHER_MONEY_TREE:
        {
            // The Green Deck earns no interest at all
            if (g_game_vars.interest_cap > 0)
            {
                s32 cap = (voucher == VOUCHER_SEED_MONEY) ? SEED_MONEY_INTEREST_CAP
                                                          : MONEY_TREE_INTEREST_CAP;
                if (g_game_vars.interest_cap < cap)
                    g_game_vars.interest_cap = cap;
            }
            break;
        }

        case VOUCHER_ANTIMATTER:
            g_game_vars.joker_slots++;
            clamp_max(&g_game_vars.joker_slots, MAX_JOKERS_HELD_SIZE);
            break;

        case VOUCHER_HIEROGLYPH:
            voucher_lower_ante();
            g_game_vars.hands_per_round--;
            clamp_min(&g_game_vars.hands_per_round, 1);
            g_game_vars.hands--;
            clamp_min(&g_game_vars.hands, 1);
            display_hands();
            break;

        case VOUCHER_PETROGLYPH:
            voucher_lower_ante();
            g_game_vars.discards_per_round--;
            clamp_min(&g_game_vars.discards_per_round, 0);
            g_game_vars.discards--;
            clamp_min(&g_game_vars.discards, 0);
            display_discards();
            break;

        case VOUCHER_PAINT_BRUSH:
        case VOUCHER_PALETTE:
            g_game_vars.base_hand_size++;
            clamp_max(&g_game_vars.base_hand_size, MAX_HAND_SIZE);
            g_game_vars.hand_size++;
            clamp_max(&g_game_vars.hand_size, MAX_HAND_SIZE);
            break;

        // Queried by the Shop and the Blind Select screen, or not implemented yet
        default:
            break;
    }
}

int voucher_get_shop_discount_percent(void)
{
    if (voucher_is_owned(VOUCHER_LIQUIDATION))
        return LIQUIDATION_DISCOUNT_PERCENT;
    if (voucher_is_owned(VOUCHER_CLEARANCE_SALE))
        return CLEARANCE_SALE_DISCOUNT_PERCENT;
    return 0;
}

int voucher_apply_shop_discount(int price)
{
    int discount = voucher_get_shop_discount_percent();
    if (discount == 0 || price <= 0)
        return price;

    // Rounded to the nearest dollar like in Balatro, never free
    int discounted = (price * (100 - discount) + 50) / 100;
    return (discounted < 1) ? 1 : discounted;
}

int voucher_get_reroll_discount(void)
{
    int discount = 0;
    if (voucher_is_owned(VOUCHER_REROLL_SURPLUS))
        discount += REROLL_VOUCHER_DISCOUNT;
    if (voucher_is_owned(VOUCHER_REROLL_GLUT))
        discount += REROLL_VOUCHER_DISCOUNT;
    return discount;
}

int voucher_get_planet_shop_weight_x10(void)
{
    if (voucher_is_owned(VOUCHER_PLANET_TYCOON))
        return PLANET_TYCOON_SHOP_WEIGHT_X10;
    if (voucher_is_owned(VOUCHER_PLANET_MERCHANT))
        return PLANET_MERCHANT_SHOP_WEIGHT_X10;
    return PLANET_SHOP_WEIGHT_X10;
}

int voucher_get_tarot_shop_weight_x10(void)
{
    if (voucher_is_owned(VOUCHER_TAROT_TYCOON))
        return TAROT_TYCOON_SHOP_WEIGHT_X10;
    if (voucher_is_owned(VOUCHER_TAROT_MERCHANT))
        return TAROT_MERCHANT_SHOP_WEIGHT_X10;
    return TAROT_SHOP_WEIGHT_X10;
}

int voucher_get_playing_card_shop_weight_x10(void)
{
    return voucher_is_owned(VOUCHER_MAGIC_TRICK) ? PLAYING_CARD_SHOP_WEIGHT_X10 : 0;
}

bool voucher_can_reroll_boss(void)
{
    if (voucher_is_owned(VOUCHER_RETCON))
        return true;
    return voucher_is_owned(VOUCHER_DIRECTORS_CUT) && s_boss_reroll_ante != g_game_vars.ante;
}

void voucher_on_boss_rerolled(void)
{
    s_boss_reroll_ante = g_game_vars.ante;
}

// VoucherObject

VoucherObject* voucher_object_new(enum VoucherType voucher)
{
    if (!voucher_is_valid(voucher))
    {
        MGBA_FUNC_ERROR("Invalid voucher %d", voucher);
        return NULL;
    }

    s16 layer = UNDEFINED;
    for (s16 i = 0; i < MAX_ACTIVE_VOUCHERS; i++)
    {
        if (!s_used_layers[i])
        {
            layer = i;
            break;
        }
    }

    if (layer == UNDEFINED)
    {
        MGBA_FUNC_WARN("No free voucher sprite layer");
        return NULL;
    }

    VoucherObject* voucher_object = POOL_GET(VoucherObject);
    sprite_object_init((SpriteObject*)voucher_object);

    voucher_object->type = ITEM_TYPE_VOUCHER;
    voucher_object->is_owned = false;
    voucher_object->voucher_type = voucher;
    voucher_object->layer = layer;
    s_used_layers[layer] = true;

    // Vouchers are drawn with the Planet palette and share its bank, see docs/ru/PALETTES.md
    memcpy16(&pal_obj_mem[PAL_ROW_LEN * CONSUMABLE_PB], voucher_gfxPal, PAL_ROW_LEN);

    int tile_index = sprite_get_tid(VOUCHER_SPRITE, layer);
    memcpy32(
        &tile_mem[TILE_MEM_OBJ_CHARBLOCK0_IDX][tile_index],
        &voucher_gfxTiles[voucher * TILE_SIZE * VOUCHER_SPRITE_TILES],
        TILE_SIZE * VOUCHER_SPRITE_TILES
    );

    Sprite* sprite = sprite_new(
        ATTR0_SQUARE | ATTR0_4BPP | ATTR0_AFF,
        ATTR1_SIZE_32,
        tile_index,
        CONSUMABLE_PB,
        sprite_get_starting_layer(VOUCHER_SPRITE) + layer
    );

    if (sprite == NULL)
    {
        MGBA_FUNC_WARN("Could not create a voucher sprite");
        s_used_layers[layer] = false;
        sprite_object_destroy((SpriteObject*)voucher_object);
        POOL_FREE(VoucherObject, voucher_object);
        return NULL;
    }

    sprite_object_set_sprite((SpriteObject*)voucher_object, sprite);

    return voucher_object;
}

void voucher_object_destroy(VoucherObject** voucher_object)
{
    if (voucher_object == NULL || *voucher_object == NULL)
        return;

    s16 layer = (*voucher_object)->layer;
    if (layer >= 0 && layer < MAX_ACTIVE_VOUCHERS)
        s_used_layers[layer] = false;

    sprite_object_destroy((SpriteObject*)(*voucher_object));
    POOL_FREE(VoucherObject, *voucher_object);
    *voucher_object = NULL;
}

// Item API

Item* voucher_object_roll_new(enum RngSequence key)
{
    enum VoucherType voucher = voucher_roll(key);
    if (voucher == VOUCHER_NONE)
        return NULL;

    return (Item*)voucher_object_new(voucher);
}

int voucher_object_get_buy_price(Item* item)
{
    GBAL_RETURN_IF_NULL(item, UNDEFINED);
    return VOUCHER_BUY_PRICE;
}

int voucher_object_get_sell_price(Item* item)
{
    // Vouchers can't be sold
    GBAL_RETURN_IF_NULL(item, UNDEFINED);
    return 0;
}

const char* voucher_object_get_name(Item* item)
{
    GBAL_RETURN_IF_NULL(item, ITEM_NAME_UNDEFINED);
    VoucherObject* voucher_object = (VoucherObject*)item;
    return voucher_is_valid(voucher_object->voucher_type)
             ? VOUCHER_INFO[voucher_object->voucher_type].name
             : ITEM_NAME_UNDEFINED;
}

ItemDescSubtypeInfo voucher_object_get_subtype_info(Item* item)
{
    ItemDescSubtypeInfo subtype_info = ITEM_SUBTYPE_INFO_DEFAULT;
    GBAL_RETURN_IF_NULL(item, subtype_info);

    subtype_info.main_color = card_rarity_pal_gfxPal[VOUCHER_SUBTYPE_PAL_IDX];
    subtype_info.shadow_color = card_rarity_pal_gfxPal[VOUCHER_SUBTYPE_PAL_IDX + 1];
    subtype_info.name_str = "Ваучер";

    return subtype_info;
}

bool voucher_object_can_acquire(Item* item)
{
    GBAL_RETURN_IF_NULL(item, false);
    return voucher_is_rollable(((VoucherObject*)item)->voucher_type);
}

void voucher_object_acquire(Item* item)
{
    GBAL_RETURN_IF_NULL(item, RET_NONE);
    voucher_redeem(((VoucherObject*)item)->voucher_type);
    // Redeemed Vouchers are not kept as Items: fly away and get disposed
    item_start_discard_animation(item);
}

void voucher_object_dispose(Item** item)
{
    GBAL_RETURN_IF_NULL(item, RET_NONE);
    GBAL_RETURN_IF_NULL(*item, RET_NONE);

    VoucherObject* voucher_object = (VoucherObject*)(*item);
    voucher_object_destroy(&voucher_object);
    *item = NULL;
}

int voucher_object_print_description(Item* item, Rect dest_rect)
{
    GBAL_RETURN_IF_NULL(item, 0);
    VoucherObject* voucher_object = (VoucherObject*)item;
    if (!voucher_is_valid(voucher_object->voucher_type))
        return 0;

    return tte_printf_justified_in_rect(
        VOUCHER_INFO[voucher_object->voucher_type].description,
        dest_rect,
        JUSTIFY_CENTER,
        SCREEN_LEFT,
        true
    );
}
