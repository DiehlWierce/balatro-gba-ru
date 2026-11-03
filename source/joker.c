#include "joker.h"

#include "blind_effects.h"
#include "card.h"
#include "consumable.h"
#include "edition_gfx.h"
#include "hand.h"
#include "game/round.h"
#include "game_variables.h"
#include "graphic_utils.h"
#include "item.h"
#include "joker_batch4.h"
#include "joker_cards.h"
#include "layout.h"
#include "mgba_logger.h"
#include "pool.h"
#include "random.h"
#include "soundbank.h"
#include "stake.h"
#include "util.h"
#include "voucher.h"

// Tiles and palettes
#include "card_rarity_pal_gfx.h"
#include "joker_gfx.h"

#include <maxmod.h>
#include <stdlib.h>
#include <string.h>
#include <tonc.h>

// what it was before (MAX_DEFINEABLE_JOKERS / JOKERS_PER_SPRITESHEET)
#define MAX_NUM_JOKERS_SPRITESHEETS 75

static const unsigned int* joker_gfxTiles[] = {
#define DEF_JOKER_GFX(idx) joker_gfx##idx##Tiles,
#include "../include/def_joker_gfx_table.h"
#undef DEF_JOKER_GFX
};
static const unsigned short* joker_gfxPal[] = {
#define DEF_JOKER_GFX(idx) joker_gfx##idx##Pal,
#include "def_joker_gfx_table.h"
#undef DEF_JOKER_GFX
};

/* So for the card objects, I needed them to be properly sorted
   which is why they let you specify the layer index when creating a new card object.
   Since the cards would overlap a lot in your hand, If they weren't sorted properly, it would look
   like a mess. The joker objects are functionally identical to card objects, so they use the same
   logic. But I'm going to use a simpler approach for the joker objects since I'm lazy and sorting
   them wouldn't look good enough to warrant the effort.
*/
static bool s_used_layers[MAX_JOKER_OBJECTS] = {false}; // Track used layers for joker sprites
// TODO: Refactor sorting into SpriteObject?

// Maps the spritesheet index to the palette bank index allocated to it.
// Spritesheets that were not allocated are
static int s_joker_spritesheet_pb_map[MAX_NUM_JOKERS_SPRITESHEETS];
static int s_joker_pb_num_sprite_users[JOKER_LAST_PB - JOKER_BASE_PB + 1] = {0};

BITSET_DEFINE(s_rollable_jokers_bitset, MAX_DEFINABLE_JOKERS)

// See linked issue for context of maps
// https://github.com/GBALATRO/balatro-gba/issues/274#issue-3685075538

// clang-format off
// Map of Joker ID -> Spritesheet idx
static const int JOKER_ID_TO_SPRITE_MAP[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1,
    2, 2,
    3, 3, 3, 3, 3,
    4, 4, 4, 4, 4,
    5, 5, 5, 5,
    6, 6, 6, 6,
    7, 7,
    8, 8,
    9,
    10,
    11,
    12,
    13,
    14,
    15,
    16,
    17,
    18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
    29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43,
    44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58,
    59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73,
};

// Map of Spritesheet idx -> first Joker ID in sheet
// This is used to determine the sprite index of a Joker's sprite
// within its spritesheet by substracting its ID to this starting ID.
// Notice how spritesheets with only one Joker have sequential starting IDs.
static const int SPRITESHEET_IDX_TO_STARTING_JOKER_ID[] = {
     0, 18, 20, 22, 27, 32, 36, 40, 42, 44,
    45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78,
    79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93,
    94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108
};

// Lookup table of Joker Rarity strings. Used to display at the bottom of the description screen.
static const char* JOKER_RARITY_STRINGS_LUT[MAX_RARITIES] = {
    "Обычный", "Необычный", "Редкий", "Легендарный"
};
// clang-format on

static int s_get_num_spritesheets(void);
static int s_joker_get_spritesheet_idx(u8 joker_id);
static int s_joker_get_sprite_idx_in_sheet(u8 joker_id, int spritesheet_idx);
static void s_joker_pb_add_sprite_user(int pb);
static void s_joker_pb_remove_sprite_user(int pb);
static int s_joker_pb_get_num_sprite_users(int joker_pb);
static int s_get_unused_joker_pb(void);
static int s_allocate_pb_if_needed(u8 joker_id);
static void s_joker_draw_sticker_badges(int tile_index, int pb, u8 stickers);
static void s_joker_load_tiles(const Joker* joker, int tile_index, int pb);
static enum JokerRarity joker_get_random_rarity(enum RngSequence key);

void joker_init()
{
    // This should init once only so no need to free
    int num_spritesheets = s_get_num_spritesheets();

    for (int i = 0; i < num_spritesheets; i++)
    {
        s_joker_spritesheet_pb_map[i] = UNDEFINED;
    }
}

Joker* joker_new(u8 id)
{
    if (id >= get_joker_registry_size())
        return NULL;

    Joker* joker = POOL_GET(Joker);
    const JokerInfo* jinfo = get_joker_registry_entry(id);

    joker->id = id;
    joker->value = jinfo->base_value;
    joker->rarity = jinfo->rarity;
    joker->stickers = 0;
    joker->perish_rounds = 0;
    joker->edition = EDITION_NONE;
    joker->scoring_state = 0;
    joker->persistent_state = 0;

    // initialize persistent Joker data if needed
    JokerEffect* joker_effect = NULL;
    jinfo->joker_effect_func(joker, NULL, JOKER_EVENT_ON_JOKER_CREATED, &joker_effect);

    return joker;
}

void joker_destroy(Joker** joker)
{
    POOL_FREE(Joker, *joker);
    *joker = NULL;
}

u32 joker_get_score_effect(
    Joker* joker,
    Card* scored_card,
    enum JokerEvent joker_event,
    JokerEffect** joker_effect
)
{
    const JokerInfo* jinfo = get_joker_registry_entry(joker->id);
    if (!jinfo || joker_is_perished(joker))
        return JOKER_EFFECT_FLAG_NONE;

    return jinfo->joker_effect_func(joker, scored_card, joker_event, joker_effect);
}

const char* joker_object_get_name(Item* joker_object)
{
    GBAL_RETURN_IF_NULL(joker_object, NULL);
    Joker* joker = ((JokerObject*)joker_object)->joker;
    const JokerInfo* info = get_joker_registry_entry(joker->id);
    GBAL_RETURN_IF_NULL(info, NULL);

    return info->name;
}

ItemDescSubtypeInfo joker_object_get_rarity_info(Item* joker_object)
{
    ItemDescSubtypeInfo subtype_info = ITEM_SUBTYPE_INFO_DEFAULT;

    GBAL_RETURN_IF_NULL(joker_object, subtype_info);
    GBAL_RETURN_IF_ASSERT_FAILS(joker_object->type == ITEM_TYPE_JOKER, subtype_info);

    Joker* joker = ((JokerObject*)joker_object)->joker;
    GBAL_RETURN_IF_NULL(joker, subtype_info);

    const JokerInfo* info = get_joker_registry_entry(joker->id);
    GBAL_RETURN_IF_NULL(info, subtype_info);

    u8 rarity = joker->rarity;
    if (rarity >= MAX_RARITIES)
    {
        MGBA_FUNC_ERROR(
            "Invalid Joker rarity value %d, should not exceed %d",
            rarity,
            MAX_RARITIES
        );
        return subtype_info;
    }

    // +1 to account for the mandatory transparency in Asperite color palettes
    u32 pal_base = 1 + 2 * rarity;
    subtype_info.main_color = card_rarity_pal_gfxPal[pal_base];       // even indices for shadows
    subtype_info.shadow_color = card_rarity_pal_gfxPal[pal_base + 1]; // odd ones for main colors
    subtype_info.name_str = JOKER_RARITY_STRINGS_LUT[rarity];

    return subtype_info;
}

int joker_get_buy_price(const Joker* joker)
{
    GBAL_RETURN_IF_NULL(joker, UNDEFINED);

    if (joker->stickers & JOKER_STICKER_RENTAL)
        return RENTAL_JOKER_BUY_PRICE;

    return joker->value;
}

int joker_get_sell_value(const Joker* joker)
{
    GBAL_RETURN_IF_NULL(joker, UNDEFINED);

    // Rental Jokers are bought for RENTAL_JOKER_BUY_PRICE: they sell for the minimum, $1
    if (joker->stickers & JOKER_STICKER_RENTAL)
        return 1;

    return joker->value / 2;
}

bool joker_can_be_sold(const Joker* joker)
{
    return joker != NULL && !(joker->stickers & JOKER_STICKER_ETERNAL);
}

bool joker_is_perished(const Joker* joker)
{
    return joker != NULL && (joker->stickers & JOKER_STICKER_PERISHABLE) &&
           joker->perish_rounds == 0;
}

void joker_set_edition(Joker* joker, enum Edition edition)
{
    GBAL_RETURN_IF_NULL(joker, RET_NONE);
    if (!edition_is_valid(edition))
        return;

    // The edition is part of the price: the sell value follows (half of the price)
    int value = joker->value - edition_extra_price(joker->edition) + edition_extra_price(edition);
    joker->value = (u8)max(value, 0);
    joker->edition = edition;
}

void joker_roll_edition(Joker* joker)
{
    GBAL_RETURN_IF_NULL(joker, RET_NONE);

    enum Edition edition = edition_from_roll(
        rng_get_u32(RNG_SEQ_JOKER_EDITION),
        voucher_get_edition_rate_multiplier(),
        true
    );
    if (edition != EDITION_NONE)
        joker_set_edition(joker, edition);
}

// JokerObject methods
JokerObject* joker_object_new(Joker* joker)
{
    GBAL_RETURN_IF_NULL(joker, NULL);
    JokerObject* joker_object = POOL_GET(JokerObject);

    sprite_object_init((SpriteObject*)joker_object);

    s16 layer = 0;
    for (s16 i = 0; i < MAX_JOKER_OBJECTS; i++)
    {
        if (!s_used_layers[i])
        {
            layer = i;
            s_used_layers[i] = true; // Mark this layer as used
            break;
        }
    }

    joker_object->joker = joker;
    joker_object->type = ITEM_TYPE_JOKER;
    joker_object->is_owned = false;
    joker_object->face_down = false;

    int tile_index = sprite_get_tid(JOKER_SPRITE, layer);
    int joker_pb = s_allocate_pb_if_needed(joker->id);
    s_joker_pb_add_sprite_user(joker_pb);

    s_joker_load_tiles(joker, tile_index, joker_pb);

    sprite_object_set_sprite(
        (SpriteObject*)joker_object,
        sprite_new(
            ATTR0_SQUARE | ATTR0_4BPP | ATTR0_AFF,
            ATTR1_SIZE_32,
            tile_index,
            joker_pb,
            sprite_get_starting_layer(JOKER_SPRITE) + layer
        )
    );

    return joker_object;
}

void joker_object_destroy(JokerObject** joker_object)
{
    if (joker_object == NULL || *joker_object == NULL)
        return;

    s16 layer = sprite_get_layer(joker_object_get_sprite(*joker_object)) -
                sprite_get_starting_layer(JOKER_SPRITE);
    s_used_layers[layer] = false;
    // Not the sprite's bank: a face down Joker uses the Deck's palette bank
    int spritesheet_idx = s_joker_get_spritesheet_idx((*joker_object)->joker->id);
    int joker_pb = s_joker_spritesheet_pb_map[spritesheet_idx];
    s_joker_pb_remove_sprite_user(joker_pb);
    if (s_joker_pb_get_num_sprite_users(joker_pb) == 0)
    {
        s_joker_spritesheet_pb_map[spritesheet_idx] = UNDEFINED;
    }

    sprite_object_destroy((SpriteObject*)(*joker_object));
    joker_destroy(&(*joker_object)->joker);
    POOL_FREE(JokerObject, *joker_object);
    *joker_object = NULL;
}

void joker_object_dispose(Item** joker_object_item)
{
    GBAL_RETURN_IF_NULL(joker_object_item, RET_NONE);
    GBAL_RETURN_IF_NULL(*joker_object_item, RET_NONE);
    GBAL_RETURN_IF_ASSERT_FAILS((*joker_object_item)->type == ITEM_TYPE_JOKER, RET_NONE);

    JokerObject* joker_object = (JokerObject*)(*joker_object_item);
    GBAL_RETURN_IF_NULL(joker_object->joker, RET_NONE);

    joker_set_rollable(joker_object->joker->id, true);

    joker_object_destroy(&joker_object);
    *joker_object_item = NULL;
}

int joker_object_print_description(Item* joker_object_item, Rect dest_rect)
{
    GBAL_RETURN_IF_NULL(joker_object_item, 0);
    GBAL_RETURN_IF_ASSERT_FAILS(joker_object_item->type == ITEM_TYPE_JOKER, 0);

    JokerObject* joker_object = (JokerObject*)(joker_object_item);
    GBAL_RETURN_IF_NULL(joker_object->joker, 0);

    Joker* joker = joker_object->joker;
    const JokerInfo* info = get_joker_registry_entry(joker->id);
    GBAL_RETURN_IF_NULL(info, 0);

    int nb_lines;
    if (joker_object->face_down)
        nb_lines = tte_printf_justified_in_rect("???", dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
    else
        nb_lines = info->joker_print_desc(joker, dest_rect);

    if (joker->stickers == 0 && joker->edition == EDITION_NONE)
        return nb_lines;

    // Edition and Stake stickers on lines below the description
    char stickers_text[160] = {'\0'};
    int len = 0;
    switch (joker->edition)
    {
        case EDITION_FOIL:
            len += snprintf(
                stickers_text + len,
                sizeof(stickers_text) - len,
                TTE_BLUE_TAG "Фольга +%d фиш.\n",
                EDITION_FOIL_CHIPS
            );
            break;
        case EDITION_HOLOGRAPHIC:
            len += snprintf(
                stickers_text + len,
                sizeof(stickers_text) - len,
                TTE_RED_TAG "Голограмма +%d\n",
                EDITION_HOLOGRAPHIC_MULT
            );
            break;
        case EDITION_POLYCHROME:
            len += snprintf(
                stickers_text + len,
                sizeof(stickers_text) - len,
                TTE_RED_TAG "Полихром X1.5\n"
            );
            break;
        case EDITION_NEGATIVE:
            len += snprintf(
                stickers_text + len,
                sizeof(stickers_text) - len,
                TTE_PURPLE_TAG "Негатив +1 слот\n"
            );
            break;
        default:
            break;
    }
    if (joker->stickers & JOKER_STICKER_ETERNAL)
    {
        len +=
            snprintf(stickers_text + len, sizeof(stickers_text) - len, TTE_DARK_BLUE_TAG "Вечный ");
    }
    if (joker->stickers & JOKER_STICKER_PERISHABLE)
    {
        if (joker->perish_rounds > 0)
            len += snprintf(
                stickers_text + len,
                sizeof(stickers_text) - len,
                TTE_DARK_GREEN_TAG "Скоропорт. (%d) ",
                joker->perish_rounds
            );
        else
            len +=
                snprintf(stickers_text + len, sizeof(stickers_text) - len, TTE_RED_TAG "Испорчен ");
    }
    if (joker->stickers & JOKER_STICKER_RENTAL)
    {
        snprintf(
            stickers_text + len,
            sizeof(stickers_text) - len,
            TTE_YELLOW_TAG "Аренда $%d",
            RENTAL_JOKER_COST
        );
    }

    Rect stickers_rect = dest_rect;
    stickers_rect.top += nb_lines;
    return nb_lines + tte_printf_justified_in_rect(
                          stickers_text,
                          stickers_rect,
                          JUSTIFY_CENTER,
                          SCREEN_LEFT,
                          true
                      );
}

void joker_object_shake(JokerObject* joker_object, mm_word sound_id)
{
    sprite_object_shake((SpriteObject*)joker_object, sound_id);
}

int joker_object_get_buy_price(Item* joker_object)
{
    GBAL_RETURN_IF_NULL(joker_object, UNDEFINED);
    GBAL_RETURN_IF_ASSERT_FAILS(joker_object->type == ITEM_TYPE_JOKER, UNDEFINED);

    return joker_get_buy_price(((JokerObject*)joker_object)->joker);
}

int joker_object_get_sell_price(Item* joker_object)
{
    GBAL_RETURN_IF_NULL(joker_object, UNDEFINED);
    GBAL_RETURN_IF_ASSERT_FAILS(joker_object->type == ITEM_TYPE_JOKER, UNDEFINED);

    return joker_get_sell_value(((JokerObject*)joker_object)->joker);
}

void joker_object_add_to_owned(Item* joker_object)
{
    GBAL_RETURN_IF_NULL(joker_object, RET_NONE);
    GBAL_RETURN_IF_ASSERT_FAILS(joker_object->type == ITEM_TYPE_JOKER, RET_NONE);

    add_joker((JokerObject*)joker_object);
}

void joker_set_rollable(int joker_id, bool rollable)
{
    bitset_set_idx(&s_rollable_jokers_bitset, joker_id, rollable);
}

/**
 * @brief Computes the number of Jokers we can currently roll in the Shop.
 *         The Jokers we own is taken into account and can't be rolled again.
 */
static inline int get_num_rollable_jokers(void)
{
    return bitset_num_set_bits(&s_rollable_jokers_bitset);
}

/**
 * @brief Returns true if we can't roll any Joker
 */
static inline bool no_rollable_jokers(void)
{
    return bitset_is_empty(&s_rollable_jokers_bitset);
}

GBAL_UNUSED
static inline bool joker_is_rollable(int joker_id)
{
    return bitset_get_idx(&s_rollable_jokers_bitset, joker_id);
}

void joker_reset_rollable_jokers(void)
{
    int num_jokers = get_joker_registry_size();

    bitset_clear(&s_rollable_jokers_bitset);
    for (int i = 0; i < num_jokers; i++)
    {
        bitset_set_idx(&s_rollable_jokers_bitset, i, true);
    }
}

int joker_roll_id(enum JokerRarity joker_rarity, enum RngSequence key)
{
    // Now determine how many jokers are available based on the rarity
    int jokers_avail_size = get_num_rollable_jokers();

    if (jokers_avail_size == 0)
        return UNDEFINED;

    int matching_joker_ids[jokers_avail_size];
    int fallback_random_idx = rng_get_u32(key) % jokers_avail_size;
    int fallback_random_joker_id = UNDEFINED;
    int match_count = 0;

    BitsetItr itr = bitset_itr_create(&s_rollable_jokers_bitset);

    int i = 0;
    int joker_id = UNDEFINED;
    while ((joker_id = bitset_itr_next(&itr)) != UNDEFINED)
    {
        if (i++ == fallback_random_idx)
            fallback_random_joker_id = joker_id;
        const JokerInfo* info = get_joker_registry_entry(joker_id);
        if (info->rarity == joker_rarity)
        {
            matching_joker_ids[match_count++] = joker_id;
        }
    }

    int selected_joker_id = (match_count > 0) ? matching_joker_ids[rng_get_u32(key) % match_count]
                                              : fallback_random_joker_id;

    return selected_joker_id;
}

Item* joker_object_roll_new(enum RngSequence key)
{
    if (no_rollable_jokers())
        return NULL;

    int joker_id = 0;
#ifdef TEST_JOKER_ID0 // Allow defining an ID for a joker to always appear in shop and be tested
    if (joker_is_rollable(TEST_JOKER_ID0))
    {
        joker_id = TEST_JOKER_ID0;
    }
    else
#endif
#ifdef TEST_JOKER_ID1
        if (joker_is_rollable(TEST_JOKER_ID1))
    {
        joker_id = TEST_JOKER_ID1;
    }
    else
#endif
    {
        joker_id = joker_roll_id(joker_get_random_rarity(key), key);
    }

    // If for some reason only no joker is left, don't make another
    if (joker_id == UNDEFINED)
        return NULL;

    joker_set_rollable(joker_id, false);

    Joker* joker = joker_new(joker_id);
    stake_roll_joker_stickers(joker);
    joker_roll_edition(joker);
#ifdef DEBUG_EDITIONS
    static int s_debug_edition_turn = 0;
    int debug_edition = (s_debug_edition_turn++ % 5) + 1; // 5 means none
    joker_set_edition(joker, EDITION_NONE);
    joker_set_edition(joker, debug_edition <= EDITION_NEGATIVE ? debug_edition : EDITION_NONE);
#endif

    return (Item*)joker_object_new(joker);
}

/**
 * @brief Get a random rarity a Joker will be rolled from
 *
 * @param key to the RNG sequence used
 * @return a random Joker rarity
 */
static inline enum JokerRarity joker_get_random_rarity(enum RngSequence key)
{
    enum JokerRarity joker_rarity = 0;
    int rarity_roll = rng_get_u32(key) % 100;
    if (rarity_roll < COMMON_JOKER_CHANCE)
    {
        joker_rarity = COMMON_JOKER;
    }
    else if (rarity_roll < COMMON_JOKER_CHANCE + UNCOMMON_JOKER_CHANCE)
    {
        joker_rarity = UNCOMMON_JOKER;
    }
    else if (rarity_roll < COMMON_JOKER_CHANCE + UNCOMMON_JOKER_CHANCE + RARE_JOKER_CHANCE)
    {
        joker_rarity = RARE_JOKER;
    }
    else if (rarity_roll < COMMON_JOKER_CHANCE + UNCOMMON_JOKER_CHANCE + RARE_JOKER_CHANCE +
                               LEGENDARY_JOKER_CHANCE)
    {
        joker_rarity = LEGENDARY_JOKER;
    }

    return joker_rarity;
}

/**
 * @brief Copies the tiles of the Joker to the sprite at @p tile_index and draws its edition effect
 *        and its Stake stickers on them.
 */
static void s_joker_load_tiles(const Joker* joker, int tile_index, int pb)
{
    int spritesheet_idx = s_joker_get_spritesheet_idx(joker->id);
    int joker_idx = s_joker_get_sprite_idx_in_sheet(joker->id, spritesheet_idx);
    u32* dest = (u32*)&tile_mem[TILE_MEM_OBJ_CHARBLOCK0_IDX][tile_index];

    memcpy32(
        dest,
        &joker_gfxTiles[spritesheet_idx][joker_idx * TILE_SIZE * JOKER_SPRITE_TILES],
        TILE_SIZE * JOKER_SPRITE_TILES
    );
    edition_draw_tiles(dest, pb, joker->edition);
    s_joker_draw_sticker_badges(tile_index, pb, joker->stickers);
}

void joker_object_set_edition(JokerObject* joker_object, enum Edition edition)
{
    GBAL_RETURN_IF_NULL(joker_object, RET_NONE);
    joker_set_edition(joker_object->joker, edition);

    Sprite* sprite = joker_object_get_sprite(joker_object);
    if (sprite == NULL || joker_object->face_down)
        return; // A face down Joker is redrawn with the edition when it is turned face up

    int tile_index = sprite->obj->attr2 & ATTR2_ID_MASK;
    int spritesheet_idx = s_joker_get_spritesheet_idx(joker_object->joker->id);
    s_joker_load_tiles(joker_object->joker, tile_index, s_joker_spritesheet_pb_map[spritesheet_idx]);
}

void joker_object_set_face_down(JokerObject* joker_object, bool face_down)
{
    GBAL_RETURN_IF_NULL(joker_object, RET_NONE);
    Sprite* sprite = joker_object_get_sprite(joker_object);
    GBAL_RETURN_IF_NULL(sprite, RET_NONE);

    joker_object->face_down = face_down;
    int tile_index = sprite->obj->attr2 & ATTR2_ID_MASK;
    int pb = DECK_PB;
    if (face_down)
    {
        card_copy_deck_back_tiles(&tile_mem[TILE_MEM_OBJ_CHARBLOCK0_IDX][tile_index]);
    }
    else
    {
        int spritesheet_idx = s_joker_get_spritesheet_idx(joker_object->joker->id);
        pb = s_joker_spritesheet_pb_map[spritesheet_idx];
        s_joker_load_tiles(joker_object->joker, tile_index, pb);
    }

    sprite->obj->attr2 = (sprite->obj->attr2 & ~ATTR2_PALBANK_MASK) | ATTR2_PALBANK(pb);
}

void joker_object_set_disabled_look(JokerObject* joker_object, bool disabled)
{
    GBAL_RETURN_IF_NULL(joker_object, RET_NONE);
    Sprite* sprite = joker_object_get_sprite(joker_object);
    GBAL_RETURN_IF_NULL(sprite, RET_NONE);

    // Mosaic only applies to sprites with ATTR0_MOSAIC, the size is shared by all of them
    REG_MOSAIC = MOS_BUILD(0, 0, 2, 2);
    if (disabled)
        sprite->obj->attr0 |= ATTR0_MOSAIC;
    else
        sprite->obj->attr0 &= ~ATTR0_MOSAIC;
}

Sprite* joker_object_get_sprite(JokerObject* joker_object)
{
    if (joker_object == NULL)
        return NULL;
    return sprite_object_get_sprite((SpriteObject*)joker_object);
}

static int s_get_num_spritesheets()
{
    return MAX_NUM_JOKERS_SPRITESHEETS;
}

static int s_joker_get_spritesheet_idx(u8 joker_id)
{
    return JOKER_ID_TO_SPRITE_MAP[joker_id];
}

static int s_joker_get_sprite_idx_in_sheet(u8 joker_id, int spritesheet_idx)
{
    return joker_id - SPRITESHEET_IDX_TO_STARTING_JOKER_ID[JOKER_ID_TO_SPRITE_MAP[joker_id]];
}

static void s_joker_pb_add_sprite_user(int pb)
{
    s_joker_pb_num_sprite_users[pb - JOKER_BASE_PB]++;
}

static void s_joker_pb_remove_sprite_user(int pb)
{
    int num_sprite_users = s_joker_pb_num_sprite_users[pb - JOKER_BASE_PB];
    s_joker_pb_num_sprite_users[pb - JOKER_BASE_PB] = max(0, num_sprite_users - 1);
}

static int s_joker_pb_get_num_sprite_users(int joker_pb)
{
    return s_joker_pb_num_sprite_users[joker_pb - JOKER_BASE_PB];
}

static int s_get_unused_joker_pb()
{
    for (int i = 0; i < NUM_ELEM_IN_ARR(s_joker_pb_num_sprite_users); i++)
    {
        if (s_joker_pb_num_sprite_users[i] == 0)
        {
            return (i + JOKER_BASE_PB);
        }
    }

    return UNDEFINED;
}

// Stake sticker badges drawn into the 32x32 Joker tiles (A9): no sprite and no palette bank of
// their own. A 7x7 square in the bottom left corner of the card with a 5x5 glyph; the two colors
// are the darkest and the lightest of the Joker's own palette bank so it reads on any sheet.
#define STICKER_BADGE_SIZE   7
#define STICKER_BADGE_X0     4  // Card art starts 4 px into the 32x32 sprite
#define STICKER_BADGE_Y0     25 // Bottom of the 32 px high sprite
#define STICKER_BADGE_STRIDE 8

// clang-format off
static const u8 STICKER_GLYPHS[][5] = {
    // Eternal: infinity
    {0b00000, 0b01010, 0b10101, 0b01010, 0b00000},
    // Perishable: hourglass
    {0b11111, 0b01110, 0b00100, 0b01110, 0b11111},
    // Rental: $
    {0b01110, 0b10100, 0b01110, 0b00101, 0b01110},
};
// clang-format on

static int s_color_luma(COLOR color)
{
    // BGR555, weights 2:5:1 are good enough to tell dark from light
    return 2 * (color & 31) + 5 * ((color >> 5) & 31) + ((color >> 10) & 31);
}

static void s_joker_set_tile_pixel(int tile_index, int x, int y, u32 color_idx)
{
    // 1D mapping, 4 tiles per row of the 32x32 sprite, 8 words of 8 4bpp pixels per tile
    u32* word = &((u32*)&tile_mem[TILE_MEM_OBJ_CHARBLOCK0_IDX][tile_index + (y / 8) * 4 + (x / 8)])
                    [y % 8];
    int shift = (x % 8) * 4;
    *word = (*word & ~(0xFu << shift)) | (color_idx << shift);
}

static void s_joker_draw_sticker_badges(int tile_index, int pb, u8 stickers)
{
    if (stickers == 0)
        return;

    u32 dark = 1, light = 1;
    int dark_luma = 0x7FFF, light_luma = -1;
    for (int i = 1; i < PAL_ROW_LEN; i++)
    {
        int luma = s_color_luma(pal_obj_mem[pb * PAL_ROW_LEN + i]);
        if (luma < dark_luma)
        {
            dark_luma = luma;
            dark = i;
        }
        if (luma > light_luma)
        {
            light_luma = luma;
            light = i;
        }
    }

    int badge = 0;
    for (int glyph_idx = 0; glyph_idx < 3; glyph_idx++)
    {
        if (!(stickers & (1 << glyph_idx))) // JOKER_STICKER_ETERNAL, PERISHABLE, RENTAL in order
            continue;

        int x0 = STICKER_BADGE_X0 + badge * STICKER_BADGE_STRIDE;
        for (int y = 0; y < STICKER_BADGE_SIZE; y++)
        {
            for (int x = 0; x < STICKER_BADGE_SIZE; x++)
            {
                bool border = x == 0 || y == 0 || x == STICKER_BADGE_SIZE - 1 ||
                              y == STICKER_BADGE_SIZE - 1;
                bool ink = !border && ((STICKER_GLYPHS[glyph_idx][y - 1] >> (5 - x)) & 1);
                s_joker_set_tile_pixel(
                    tile_index,
                    x0 + x,
                    STICKER_BADGE_Y0 + y,
                    (border || ink) ? dark : light
                );
            }
        }
        badge++;
    }
}

static int s_allocate_pb_if_needed(u8 joker_id)
{
    int joker_spritesheet_idx = s_joker_get_spritesheet_idx(joker_id);
    int joker_pb = s_joker_spritesheet_pb_map[joker_spritesheet_idx];
    if (joker_pb != UNDEFINED)
    {
        // Already allocated
        return joker_pb;
    }

    // Allocate a new palette
    joker_pb = s_get_unused_joker_pb();

    if (joker_pb == UNDEFINED)
    {
        // Ran out of palettes, default to base and pray
        joker_pb = JOKER_BASE_PB;
    }
    else
    {
        s_joker_spritesheet_pb_map[joker_spritesheet_idx] = joker_pb;
        memcpy16(
            &pal_obj_mem[PAL_ROW_LEN * joker_pb],
            joker_gfxPal[joker_spritesheet_idx],
            NUM_ELEM_IN_ARR(joker_gfx0Pal)
        );
    }

    return joker_pb;
}
void joker_economy_on_round_end(bool boss_defeated)
{
    List* jokers = get_jokers_list();

    ListItr itr = list_itr_create(jokers);
    JokerObject* joker_object;
    while ((joker_object = list_itr_next(&itr)))
    {
        Joker* joker = joker_object->joker;
        if (joker_is_perished(joker))
            continue;

        joker->value = joker_economy_add_value(joker->value, joker_economy_self_value_gain(joker->id));

        int others_gain = joker_economy_others_value_gain(joker->id);
        if (others_gain > 0)
        {
            ListItr other_itr = list_itr_create(jokers);
            JokerObject* other;
            while ((other = list_itr_next(&other_itr)))
            {
                if (other->joker != joker)
                    other->joker->value = joker_economy_add_value(other->joker->value, others_gain);
            }
        }

        if (joker->id == ROCKET_JOKER_ID)
            joker->persistent_state = joker_economy_rocket_next(joker->persistent_state, boss_defeated);

        // Batch 4: new targets (Idol, Ancient Joker, To Do List) and per-round counters
        if (joker->id >= BLOODSTONE_JOKER_ID && joker->id <= HIT_THE_ROAD_JOKER_ID)
        {
            JokerEffect* round_end_effect = NULL;
            joker_get_score_effect(joker, NULL, JOKER_EVENT_ON_ROUND_END, &round_end_effect);
        }
    }
}

/*******************************************************************************
 * A8 batch 3: hooks of the Jokers tied to cards, Tarots and the deck
 ******************************************************************************/

// Where Tarots created by Jokers appear
#define JOKER_TAROT_SPAWN_X int2fx(120)
#define JOKER_TAROT_SPAWN_Y int2fx(60)

// Runs @p fn for every owned Joker that works (not Perishable-expired, not disabled by a Boss)
static void joker_for_each_active(void (*fn)(Joker* joker))
{
    ListItr itr = list_itr_create(get_jokers_list());
    JokerObject* joker_object;
    while ((joker_object = list_itr_next(&itr)))
    {
        if (joker_is_perished(joker_object->joker) || blind_effects_is_joker_disabled(joker_object))
            continue;
        fn(joker_object->joker);
    }
}

static void joker_hook_glass_broken(Joker* joker)
{
    if (joker->id == GLASS_JOKER_ID)
        joker->persistent_state++;
}

static void joker_hook_lucky_triggered(Joker* joker)
{
    if (joker->id == LUCKY_CAT_JOKER_ID)
        joker->persistent_state++;
}

static void joker_hook_planet_used(Joker* joker)
{
    if (joker->id == CONSTELLATION_JOKER_ID)
        joker->persistent_state++;
}

static void joker_hook_pack_opened(Joker* joker)
{
    if (joker->id == HALLUCINATION_JOKER_ID &&
        joker_cards_one_in(rng_get_u32(RNG_SEQ_JOKER_HALLUCINATION), HALLUCINATION_CHANCE))
        consumable_create_random_tarot(RNG_SEQ_JOKER_TAROT, JOKER_TAROT_SPAWN_X, JOKER_TAROT_SPAWN_Y);
}

static void joker_hook_blind_selected(Joker* joker)
{
    switch (joker->id)
    {
        case BURGLAR_JOKER_ID:
            g_game_vars.hands = joker_cards_burglar_hands(g_game_vars.hands);
            g_game_vars.discards = 0;
            display_hands();
            display_discards();
            break;
        case CARTOMANCER_JOKER_ID:
            consumable_create_random_tarot(
                RNG_SEQ_JOKER_TAROT,
                JOKER_TAROT_SPAWN_X,
                JOKER_TAROT_SPAWN_Y
            );
            break;
        case MARBLE_JOKER_ID:
            if (deck_get_size() < MAX_DECK_SIZE)
            {
                Card* stone = card_new(SPADES, ACE);
                stone->enhancement = CARD_ENHANCEMENT_STONE;
                deck_push(stone);
            }
            break;
        default:
            break;
    }
}

static void joker_hook_deck_shuffled(Joker* joker)
{
    if (joker->id != CERTIFICATE_JOKER_ID || deck_get_size() >= MAX_DECK_SIZE)
        return;

    Card* card = card_new(
        rng_get_u32(RNG_SEQ_JOKER_CERTIFICATE) % NUM_SUITS,
        rng_get_u32(RNG_SEQ_JOKER_CERTIFICATE) % NUM_RANKS
    );
    card->seal = CARD_SEAL_NONE + 1 + rng_get_u32(RNG_SEQ_JOKER_CERTIFICATE) % (CARD_SEAL_MAX - 1);
    deck_push(card); // The top of the deck is drawn first: the card lands in the hand
}

void joker_on_glass_broken(void)
{
    joker_for_each_active(joker_hook_glass_broken);
}

void joker_on_lucky_triggered(void)
{
    joker_for_each_active(joker_hook_lucky_triggered);
}

void joker_on_consumable_used(bool is_tarot)
{
    if (is_tarot)
        g_game_vars.tarots_used++;
    else
        joker_for_each_active(joker_hook_planet_used);
}

void joker_on_pack_opened(void)
{
    joker_for_each_active(joker_hook_pack_opened);
}

void joker_on_blind_selected(void)
{
    joker_for_each_active(joker_hook_blind_selected);
}

void joker_on_deck_shuffled(void)
{
    joker_for_each_active(joker_hook_deck_shuffled);
}

void joker_on_cards_discarded(int nb_cards, int nb_face_cards, int nb_jacks)
{
    if (nb_cards <= 0)
        return;

    ListItr itr = list_itr_create(get_jokers_list());
    JokerObject* joker_object;
    while ((joker_object = list_itr_next(&itr)))
    {
        Joker* joker = joker_object->joker;
        if (joker_is_perished(joker) || blind_effects_is_joker_disabled(joker_object))
            continue;

        switch (joker->id)
        {
            case GREEN_JOKER_ID:
                joker->persistent_state = joker_batch4_green_discard(joker->persistent_state);
                break;
            case FACELESS_JOKER_ID:
            {
                int money = joker_batch4_faceless_money(nb_face_cards);
                if (money > 0)
                {
                    g_game_vars.money += money;
                    display_money();
                    joker_object_shake(joker_object, UNDEFINED);
                }
                break;
            }
            case HIT_THE_ROAD_JOKER_ID:
                joker->persistent_state += nb_jacks;
                break;
            default:
                break;
        }
    }
}
