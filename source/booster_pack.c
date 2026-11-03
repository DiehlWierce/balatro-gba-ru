/**
 * @file booster_pack.c
 *
 * @brief Booster Packs, see booster_pack.h.
 *
 * The pack sprite is drawn by code with the colours of the card palette (CARD_PB): no graphics
 * file and no palette bank of its own (see docs/ru/PALETTES.md).
 */
#include "booster_pack.h"

#include "card.h"
#include "consumable.h"
#include "game.h"
#include "graphic_utils.h"
#include "item.h"
#include "pool.h"
#include "random.h"
#include "sprite.h"
#include "util.h"
#include "voucher.h"

#include <string.h>

// Standard Pack contents, as in the original game
#define STANDARD_ENHANCEMENT_CHANCE 40 // %
#define STANDARD_SEAL_CHANCE        20 // %

#define PACK_TILES_WORDS (CARD_SPRITE_TILES * TILE_SIZE)
#define PACK_ART_LEFT    6
#define PACK_ART_RIGHT   25
#define PACK_ART_TOP     2
#define PACK_ART_BOTTOM  29

static bool s_used_layers[MAX_SHOP_PACKS] = {false};

#pragma region Standard Pack

static Item* standard_pack_roll_choice(int idx)
{
    u8 suit = rng_get_u32(RNG_SEQ_PACK_CONTENTS) % NUM_SUITS;
    u8 rank = rng_get_u32(RNG_SEQ_PACK_CONTENTS) % NUM_RANKS;
    Card* card = card_new(suit, rank);

    if (rng_get_u32(RNG_SEQ_PACK_CONTENTS) % 100 < STANDARD_ENHANCEMENT_CHANCE)
    {
        card->enhancement = CARD_ENHANCEMENT_BONUS +
                            rng_get_u32(RNG_SEQ_PACK_CONTENTS) % (CARD_ENHANCEMENT_MAX - 1);
    }
    if (rng_get_u32(RNG_SEQ_PACK_CONTENTS) % 100 < STANDARD_SEAL_CHANCE)
    {
        card->seal = CARD_SEAL_GOLD + rng_get_u32(RNG_SEQ_PACK_CONTENTS) % (CARD_SEAL_MAX - 1);
    }
    // Own sequence so the contents above stay the same for a given seed. No Negative cards.
    card->edition = edition_from_roll(
        rng_get_u32(RNG_SEQ_PACK_EDITION),
        voucher_get_edition_rate_multiplier(),
        false
    );

    CardObject* card_object = card_object_new(card);
    card_object_set_sprite(card_object, idx);
    return (Item*)card_object;
}

static void standard_pack_pick(Item** choice)
{
    CardObject* card_object = (CardObject*)(*choice);
    if (deck_get_size() < MAX_DECK_SIZE)
        deck_push(card_object->card);
    else
        card_destroy(&card_object->card); // The deck is full, the card is lost
    card_object_destroy(&card_object);
    *choice = NULL;
    display_deck_size_max();
}

static void standard_pack_discard(Item** choice)
{
    CardObject* card_object = (CardObject*)(*choice);
    card_destroy(&card_object->card);
    card_object_destroy(&card_object);
    *choice = NULL;
}

#pragma endregion

#pragma region Arcana Pack

static Item* arcana_pack_roll_choice(int idx)
{
    (void)idx;
    enum TarotType tarot = tarot_roll(RNG_SEQ_PACK_TAROT);
    if (tarot == TAROT_TYPE_MAX)
        return NULL;
    return (Item*)consumable_object_new(CONSUMABLE_TYPE_TAROT, tarot);
}

static void arcana_pack_pick(Item** choice)
{
    ConsumableObject* consumable = (ConsumableObject*)(*choice);

    // The Tarot goes to the consumables row where it is, to be used later on the cards of a round.
    // Without a free slot it is lost.
    if (item_can_acquire(*choice))
        item_acquire(*choice);
    else
        consumable_object_destroy(&consumable);

    *choice = NULL;
}

static void arcana_pack_discard(Item** choice)
{
    ConsumableObject* consumable = (ConsumableObject*)(*choice);
    consumable_object_destroy(&consumable);
    *choice = NULL;
}

#pragma endregion

// clang-format off
static const PackInfo PACK_INFOS[PACK_TYPE_MAX] = {
    [PACK_TYPE_STANDARD] = {
        .name = "Стандартный бустер",
        .price = 4,
        .nb_choices = 3,
        .nb_picks = 1,
        .roll_choice = standard_pack_roll_choice,
        .pick = standard_pack_pick,
        .discard = standard_pack_discard,
        .body_r = 41, .body_g = 173, .body_b = 255,
    },
    [PACK_TYPE_ARCANA] = {
        .name = "Бустер Аркана",
        .price = 4,
        .nb_choices = 3,
        .nb_picks = 1,
        .roll_choice = arcana_pack_roll_choice,
        .pick = arcana_pack_pick,
        .discard = arcana_pack_discard,
        .body_r = 137, .body_g = 105, .body_b = 230,
    },
    // TODO(A7): Planet cards
    [PACK_TYPE_CELESTIAL] = {
        .name = "Небесный бустер",
        .price = 4,
        .nb_choices = 3,
        .nb_picks = 1,
        .roll_choice = NULL,
        .pick = NULL,
        .discard = NULL,
        .body_r = 35, .body_g = 89, .body_b = 85,
    },
};
// clang-format on

const PackInfo* booster_pack_get_info(enum PackType pack_type)
{
    if ((int)pack_type < 0 || pack_type >= PACK_TYPE_MAX)
        return NULL;
    return &PACK_INFOS[pack_type];
}

bool booster_pack_is_rollable(enum PackType pack_type)
{
    const PackInfo* info = booster_pack_get_info(pack_type);
    return info != NULL && info->roll_choice != NULL && info->pick != NULL && info->discard != NULL;
}

#pragma region Sprite

static inline void pack_px_set(u32* tiles, int x, int y, int color)
{
    int tile = (y / 8) * 4 + (x / 8);
    u32* word = &tiles[tile * TILE_SIZE + (y % 8)];
    int shift = (x % 8) * 4;
    *word = (*word & ~(0xFu << shift)) | ((u32)color << shift);
}

static void pack_fill_rect(u32* tiles, int left, int top, int right, int bottom, int color)
{
    for (int y = top; y <= bottom; y++)
    {
        for (int x = left; x <= right; x++)
            pack_px_set(tiles, x, y, color);
    }
}

/**
 * @brief Draws a pack: a wrapper with crimped ends, a gold band and a small card on a label.
 */
static void pack_draw(u32* tiles, const PackInfo* info)
{
    int body = card_palette_find_color(info->body_r, info->body_g, info->body_b);
    int outline = card_palette_find_color(60, 67, 104);
    int band = card_palette_find_color(255, 163, 0);
    int white = card_palette_find_color(255, 255, 255);
    int grey = card_palette_find_color(192, 200, 214);
    int red = card_palette_find_color(240, 52, 100);

    memset32(tiles, 0, PACK_TILES_WORDS);

    // Wrapper and outline
    pack_fill_rect(tiles, PACK_ART_LEFT, PACK_ART_TOP, PACK_ART_RIGHT, PACK_ART_BOTTOM, outline);
    pack_fill_rect(
        tiles,
        PACK_ART_LEFT + 1,
        PACK_ART_TOP + 1,
        PACK_ART_RIGHT - 1,
        PACK_ART_BOTTOM - 1,
        body
    );

    // Crimped ends: every other pixel of the first and last rows is transparent
    for (int x = PACK_ART_LEFT; x <= PACK_ART_RIGHT; x += 2)
    {
        pack_px_set(tiles, x, PACK_ART_TOP, 0);
        pack_px_set(tiles, x + 1, PACK_ART_BOTTOM, 0);
    }

    // Gold bands
    pack_fill_rect(
        tiles,
        PACK_ART_LEFT + 1,
        PACK_ART_TOP + 3,
        PACK_ART_RIGHT - 1,
        PACK_ART_TOP + 4,
        band
    );
    pack_fill_rect(
        tiles,
        PACK_ART_LEFT + 1,
        PACK_ART_BOTTOM - 4,
        PACK_ART_RIGHT - 1,
        PACK_ART_BOTTOM - 3,
        band
    );

    // A small playing card on the front
    pack_fill_rect(tiles, 11, 9, 20, 22, grey);
    pack_fill_rect(tiles, 12, 10, 19, 21, white);
    static const char* const PIP[] = {
        "..#..",
        ".###.",
        "#####",
        ".###.",
        "..#..",
    };
    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < 5; x++)
        {
            if (PIP[y][x] == '#')
                pack_px_set(tiles, 13 + x, 13 + y, red);
        }
    }
}

#pragma endregion

BoosterPackObject* booster_pack_new(enum PackType pack_type)
{
    const PackInfo* info = booster_pack_get_info(pack_type);
    GBAL_RETURN_IF_NULL(info, NULL);

    s8 layer = UNDEFINED;
    for (int i = 0; i < MAX_SHOP_PACKS; i++)
    {
        if (!s_used_layers[i])
        {
            layer = i;
            break;
        }
    }
    if (layer == UNDEFINED)
    {
        MGBA_FUNC_WARN("No free pack sprite layer");
        return NULL;
    }

    BoosterPackObject* pack = POOL_GET(BoosterPackObject);
    sprite_object_init((SpriteObject*)pack);
    pack->type = ITEM_TYPE_PACK;
    pack->is_owned = false;
    pack->pack_type = pack_type;
    pack->layer = layer;

    u32 tiles[PACK_TILES_WORDS];
    pack_draw(tiles, info);
    int tile_index = sprite_get_tid(PACK_SPRITE, layer);
    memcpy32(&tile_mem[TILE_MEM_OBJ_CHARBLOCK0_IDX][tile_index], tiles, PACK_TILES_WORDS);

    Sprite* sprite = sprite_new(
        ATTR0_SQUARE | ATTR0_4BPP | ATTR0_AFF,
        ATTR1_SIZE_32,
        tile_index,
        CARD_PB,
        sprite_get_starting_layer(PACK_SPRITE) + layer
    );
    if (sprite == NULL)
    {
        MGBA_FUNC_WARN("Could not create a pack sprite");
        sprite_object_destroy((SpriteObject*)pack);
        POOL_FREE(BoosterPackObject, pack);
        return NULL;
    }

    s_used_layers[layer] = true;
    sprite_object_set_sprite((SpriteObject*)pack, sprite);
    return pack;
}

void booster_pack_destroy(BoosterPackObject** pack)
{
    if (pack == NULL || *pack == NULL)
        return;

    if ((*pack)->layer >= 0 && (*pack)->layer < MAX_SHOP_PACKS)
        s_used_layers[(int)(*pack)->layer] = false;

    sprite_object_destroy((SpriteObject*)(*pack));
    POOL_FREE(BoosterPackObject, *pack);
    *pack = NULL;
}

Item* booster_pack_roll_new(enum RngSequence key)
{
    enum PackType rollable[PACK_TYPE_MAX];
    int nb_rollable = 0;
    for (enum PackType type = 0; type < PACK_TYPE_MAX; type++)
    {
        if (booster_pack_is_rollable(type))
            rollable[nb_rollable++] = type;
    }
    if (nb_rollable == 0)
        return NULL;

    return (Item*)booster_pack_new(rollable[rng_get_u32(key) % nb_rollable]);
}

int booster_pack_get_buy_price(Item* item)
{
    GBAL_RETURN_IF_NULL(item, UNDEFINED);
    const PackInfo* info = booster_pack_get_info(((BoosterPackObject*)item)->pack_type);
    GBAL_RETURN_IF_NULL(info, UNDEFINED);
    return info->price;
}

const char* booster_pack_get_name(Item* item)
{
    GBAL_RETURN_IF_NULL(item, ITEM_NAME_UNDEFINED);
    const PackInfo* info = booster_pack_get_info(((BoosterPackObject*)item)->pack_type);
    GBAL_RETURN_IF_NULL(info, ITEM_NAME_UNDEFINED);
    return info->name;
}

ItemDescSubtypeInfo booster_pack_get_subtype_info(Item* item)
{
    ItemDescSubtypeInfo subtype_info = ITEM_SUBTYPE_INFO_DEFAULT;
    GBAL_RETURN_IF_NULL(item, subtype_info);

    // Same colours as the item price text (yellow) to stand out from the card types
    subtype_info.main_color = RGB15(31, 20, 0);
    subtype_info.shadow_color = RGB15(20, 12, 0);
    subtype_info.name_str = "Бустер";
    return subtype_info;
}

bool booster_pack_can_acquire(Item* item)
{
    GBAL_RETURN_IF_NULL(item, false);
    return booster_pack_is_rollable(((BoosterPackObject*)item)->pack_type);
}

void booster_pack_acquire(Item* item)
{
    // Packs are opened by the Shop right after being bought, see shop_packs.c
}

void booster_pack_dispose(Item** item)
{
    GBAL_RETURN_IF_NULL(item, RET_NONE);
    GBAL_RETURN_IF_NULL(*item, RET_NONE);
    BoosterPackObject* pack = (BoosterPackObject*)(*item);
    booster_pack_destroy(&pack);
    *item = NULL;
}

int booster_pack_print_description(Item* item, Rect dest_rect)
{
    GBAL_RETURN_IF_NULL(item, 0);
    const PackInfo* info = booster_pack_get_info(((BoosterPackObject*)item)->pack_type);
    GBAL_RETURN_IF_NULL(info, 0);

    char desc[128];
    snprintf(
        desc,
        sizeof(desc),
        TTE_BLACK_TAG "Выберите " TTE_YELLOW_TAG "%d" TTE_BLACK_TAG " из " TTE_YELLOW_TAG
                      "%d" TTE_BLACK_TAG " игральных карт и добавьте в колоду",
        info->nb_picks,
        info->nb_choices
    );

    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}
