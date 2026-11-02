/**
 * @file consumable.c
 *
 * @brief Consumable cards: Planets for now, see consumable.h.
 */
#include "consumable.h"

#include "game.h"
#include "game/common_ui.h"
#include "game_variables.h"
#include "graphic_utils.h"
#include "hand.h"
#include "layout.h"
#include "pool.h"
#include "soundbank.h"
#include "sprite.h"
#include "util.h"

// Tiles and palettes
#include "card_rarity_pal_gfx.h"
#include "planet_gfx.h"

#include <stdio.h>
#include <tonc.h>

// Owned consumables panel (pixels): one card centered, two cards side by side overlapping
#define CONSUMABLES_PANEL_CENTER_X 192
#define CONSUMABLES_SPACING_X      16

#define PLANET_SUBTYPE_PAL_IDX 9 // Planet colors in card_rarity_pal_gfx.png

typedef struct
{
    const char* name;
    enum HandType hand_type;
    bool secret;
} PlanetInfo;

// clang-format off
static const PlanetInfo PLANET_INFO[PLANET_TYPE_MAX] = {
    [PLANET_MERCURY] = {"Меркурий",  PAIR,            false},
    [PLANET_VENUS]   = {"Венера",    THREE_OF_A_KIND, false},
    [PLANET_EARTH]   = {"Земля",     FULL_HOUSE,      false},
    [PLANET_MARS]    = {"Марс",      FOUR_OF_A_KIND,  false},
    [PLANET_JUPITER] = {"Юпитер",    FLUSH,           false},
    [PLANET_SATURN]  = {"Сатурн",    STRAIGHT,        false},
    [PLANET_URANUS]  = {"Уран",      TWO_PAIR,        false},
    [PLANET_NEPTUNE] = {"Нептун",    STRAIGHT_FLUSH,  false},
    [PLANET_PLUTO]   = {"Плутон",    HIGH_CARD,       false},
    [PLANET_X]       = {"Планета X", FIVE_OF_A_KIND,  true },
    [PLANET_CERES]   = {"Церера",    FLUSH_HOUSE,     true },
    [PLANET_ERIS]    = {"Эрида",     FLUSH_FIVE,      true },
};
// clang-format on

static List s_owned_consumables;
static bool s_used_layers[MAX_ACTIVE_CONSUMABLES] = {false};

static inline bool planet_is_valid(int planet)
{
    return planet >= 0 && planet < PLANET_TYPE_MAX;
}

static inline bool is_planet(const ConsumableObject* consumable_object)
{
    return consumable_object->consumable_type == CONSUMABLE_TYPE_PLANET &&
           planet_is_valid(consumable_object->id);
}

void consumables_init(void)
{
    s_owned_consumables = list_init();
}

void consumables_reset(void)
{
    ListItr itr = list_itr_create(&s_owned_consumables);
    ConsumableObject* consumable_object;
    while ((consumable_object = list_itr_next(&itr)))
    {
        consumable_object_destroy(&consumable_object);
    }
    list_clear(&s_owned_consumables);
    s_owned_consumables = list_init();
}

List* get_consumables_list(void)
{
    return &s_owned_consumables;
}

void consumables_update_loop(const Item* skipped_item)
{
    int nb_owned = list_get_len(&s_owned_consumables);
    if (nb_owned == 0)
        return;

    // Cards are centered in the panel and overlap when there are more than 2
    int spacing = (nb_owned <= 2) ? CONSUMABLES_SPACING_X : (CONSUMABLES_SPACING_X * 2) / nb_owned;
    int first_x = CONSUMABLES_PANEL_CENTER_X - ((nb_owned - 1) * spacing) / 2;

    ListItr itr = list_itr_create(&s_owned_consumables);
    Item* item;
    int i = 0;
    while ((item = list_itr_next(&itr)))
    {
        if (item != skipped_item)
            item->tx = int2fx(first_x + i * spacing);
        i++;
    }
}

// Planets

enum HandType planet_get_hand_type(enum PlanetType planet)
{
    return planet_is_valid(planet) ? PLANET_INFO[planet].hand_type : NONE;
}

bool planet_is_rollable(enum PlanetType planet)
{
    if (!planet_is_valid(planet))
        return false;

    const PlanetInfo* info = &PLANET_INFO[planet];
    return !info->secret || g_game_vars.nb_played_hands[info->hand_type - 1] > 0;
}

enum PlanetType planet_roll(enum RngSequence key)
{
    int nb_rollable = 0;
    for (int planet = 0; planet < PLANET_TYPE_MAX; planet++)
    {
        if (planet_is_rollable(planet))
            nb_rollable++;
    }

    if (nb_rollable == 0)
        return PLANET_TYPE_MAX;

    int pick = rng_get_u32(key) % nb_rollable;
    for (int planet = 0; planet < PLANET_TYPE_MAX; planet++)
    {
        if (planet_is_rollable(planet) && pick-- == 0)
            return planet;
    }

    return PLANET_TYPE_MAX;
}

static void planet_use(enum PlanetType planet)
{
    enum HandType hand_type = planet_get_hand_type(planet);
    hand_level_up(hand_type, 1);
    hand_show_level_up(hand_type);
}

// ConsumableObject

ConsumableObject* consumable_object_new(enum ConsumableType type, int id)
{
    if (type != CONSUMABLE_TYPE_PLANET || !planet_is_valid(id))
    {
        MGBA_FUNC_ERROR("Unsupported consumable %d:%d", type, id);
        return NULL;
    }

    s16 layer = UNDEFINED;
    for (s16 i = 0; i < MAX_ACTIVE_CONSUMABLES; i++)
    {
        if (!s_used_layers[i])
        {
            layer = i;
            break;
        }
    }

    if (layer == UNDEFINED)
    {
        MGBA_FUNC_WARN("No free consumable sprite layer");
        return NULL;
    }

    ConsumableObject* consumable_object = POOL_GET(ConsumableObject);
    sprite_object_init((SpriteObject*)consumable_object);

    consumable_object->type = ITEM_TYPE_CONSUMABLE;
    consumable_object->is_owned = false;
    consumable_object->consumable_type = type;
    consumable_object->id = id;
    consumable_object->layer = layer;
    s_used_layers[layer] = true;

    // All Planets share one palette bank, see docs/ru/PALETTES.md
    memcpy16(&pal_obj_mem[PAL_ROW_LEN * CONSUMABLE_PB], planet_gfxPal, PAL_ROW_LEN);

    int tile_index = sprite_get_tid(CONSUMABLE_SPRITE, layer);
    memcpy32(
        &tile_mem[TILE_MEM_OBJ_CHARBLOCK0_IDX][tile_index],
        &planet_gfxTiles[id * TILE_SIZE * CONSUMABLE_SPRITE_TILES],
        TILE_SIZE * CONSUMABLE_SPRITE_TILES
    );

    Sprite* sprite = sprite_new(
        ATTR0_SQUARE | ATTR0_4BPP | ATTR0_AFF,
        ATTR1_SIZE_32,
        tile_index,
        CONSUMABLE_PB,
        sprite_get_starting_layer(CONSUMABLE_SPRITE) + layer
    );

    if (sprite == NULL)
    {
        MGBA_FUNC_WARN("Could not create a consumable sprite");
        s_used_layers[layer] = false;
        sprite_object_destroy((SpriteObject*)consumable_object);
        POOL_FREE(ConsumableObject, consumable_object);
        return NULL;
    }

    sprite_object_set_sprite((SpriteObject*)consumable_object, sprite);

    return consumable_object;
}

void consumable_object_destroy(ConsumableObject** consumable_object)
{
    if (consumable_object == NULL || *consumable_object == NULL)
        return;

    s16 layer = (*consumable_object)->layer;
    if (layer >= 0 && layer < MAX_ACTIVE_CONSUMABLES)
        s_used_layers[layer] = false;

    sprite_object_destroy((SpriteObject*)(*consumable_object));
    POOL_FREE(ConsumableObject, *consumable_object);
    *consumable_object = NULL;
}

bool consumable_object_can_use(const ConsumableObject* consumable_object)
{
    GBAL_RETURN_IF_NULL(consumable_object, false);

    // Planets can always be used
    return consumable_object->is_owned && is_planet(consumable_object);
}

static void consumable_object_remove_from_owned(ConsumableObject* consumable_object)
{
    sprite_object_erase_text_under((SpriteObject*)consumable_object);
    sprite_object_set_focus((SpriteObject*)consumable_object, false);
    list_remove_data(&s_owned_consumables, consumable_object);
    consumable_object->is_owned = false;
}

bool consumable_object_use(ConsumableObject* consumable_object)
{
    if (!consumable_object_can_use(consumable_object))
        return false;

    consumable_object_remove_from_owned(consumable_object);
    sprite_object_shake((SpriteObject*)consumable_object, SFX_CARD_SELECT);

    planet_use(consumable_object->id);

    item_start_discard_animation((Item*)consumable_object);
    return true;
}

void consumable_object_sell(ConsumableObject* consumable_object)
{
    GBAL_RETURN_IF_NULL(consumable_object, RET_NONE);

    consumable_object_remove_from_owned(consumable_object);
    // item_sell() starts the discard animation, the Item is disposed when it ends
    item_sell((Item*)consumable_object);
}

// Item API

Item* consumable_object_roll_new(enum RngSequence key)
{
    enum PlanetType planet = planet_roll(key);
    if (planet == PLANET_TYPE_MAX)
        return NULL;

    return (Item*)consumable_object_new(CONSUMABLE_TYPE_PLANET, planet);
}

int consumable_object_get_buy_price(Item* item)
{
    GBAL_RETURN_IF_NULL(item, UNDEFINED);
    return PLANET_BUY_PRICE;
}

int consumable_object_get_sell_price(Item* item)
{
    GBAL_RETURN_IF_NULL(item, UNDEFINED);
    return PLANET_BUY_PRICE / 2;
}

const char* consumable_object_get_name(Item* item)
{
    GBAL_RETURN_IF_NULL(item, ITEM_NAME_UNDEFINED);
    ConsumableObject* consumable_object = (ConsumableObject*)item;
    return is_planet(consumable_object) ? PLANET_INFO[consumable_object->id].name
                                        : ITEM_NAME_UNDEFINED;
}

ItemDescSubtypeInfo consumable_object_get_subtype_info(Item* item)
{
    ItemDescSubtypeInfo subtype_info = ITEM_SUBTYPE_INFO_DEFAULT;
    GBAL_RETURN_IF_NULL(item, subtype_info);

    if (is_planet((ConsumableObject*)item))
    {
        subtype_info.main_color = card_rarity_pal_gfxPal[PLANET_SUBTYPE_PAL_IDX];
        subtype_info.shadow_color = card_rarity_pal_gfxPal[PLANET_SUBTYPE_PAL_IDX + 1];
        subtype_info.name_str = "Планета";
    }

    return subtype_info;
}

bool consumable_object_can_acquire(Item* item)
{
    GBAL_RETURN_IF_NULL(item, false);
    return list_get_len(&s_owned_consumables) < g_game_vars.consumable_slots;
}

void consumable_object_acquire(Item* item)
{
    GBAL_RETURN_IF_NULL(item, RET_NONE);
    list_push_back(&s_owned_consumables, item);
    item->ty = int2fx(HELD_JOKERS_POS.y);
}

void consumable_object_dispose(Item** item)
{
    GBAL_RETURN_IF_NULL(item, RET_NONE);
    GBAL_RETURN_IF_NULL(*item, RET_NONE);

    ConsumableObject* consumable_object = (ConsumableObject*)(*item);
    consumable_object_destroy(&consumable_object);
    *item = NULL;
}

int consumable_object_print_description(Item* item, Rect dest_rect)
{
    GBAL_RETURN_IF_NULL(item, 0);
    ConsumableObject* consumable_object = (ConsumableObject*)item;
    if (!is_planet(consumable_object))
        return 0;

    enum HandType hand_type = PLANET_INFO[consumable_object->id].hand_type;
    HandLevelValues increment = hand_level_increment(hand_type);

    char desc[160];
    snprintf(
        desc,
        sizeof(desc),
        TTE_BLACK_TAG "(ур.%d) Повышает уровень руки " TTE_YELLOW_TAG "%s" TTE_BLACK_TAG
                      ": " TTE_RED_TAG "+%lu" TTE_BLACK_TAG " множ. и " TTE_BLUE_TAG
                      "+%lu" TTE_BLACK_TAG " фишек",
        hand_get_level(hand_type),
        get_hand_type_full_name(hand_type),
        (unsigned long)increment.mult,
        (unsigned long)increment.chips
    );

    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}
