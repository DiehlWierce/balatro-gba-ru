#include "joker.h"

#include "blind_effects.h"
#include "card.h"
#include "consumable.h"
#include "edition_gfx.h"
#include "hand.h"
#include "game/round.h"
#include "game/common_ui.h"
#include "game_variables.h"
#include "graphic_utils.h"
#include "item.h"
#include "joker_batch4.h"
#include "joker_cards.h"
#include "layout.h"
#include "mgba_logger.h"
#include "pool.h"
#include "random.h"
#include "shop_economy.h"
#include "skip_tag.h"
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
#define MAX_NUM_JOKERS_SPRITESHEETS 120

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

// Run state of the Jokers (reset with the rollable Jokers when a run starts)
static bool s_gros_michel_extinct = false; // A Gros Michel was destroyed: Cavendish can show up
static bool s_luchador_disabled = false;   // A Luchador was sold during the current Boss
static int s_deck_start_size = 0;          // Cards in the Deck when the run started (Erosion)
EWRAM_BSS static u32 s_round_start_hands[HAND_TYPE_MAX]; // nb_played_hands when the round began
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
    74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88,
    89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
    100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113,
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
    94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108,
    109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123,
    124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
    135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148
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
static int joker_count_deck_cards(void);

// Raises the sell value of the owned consumables by dollars. consumable.c (G-3) provides it; weak so
// the build works before it exists
void consumables_add_sell_value(int dollars) __attribute__((weak));

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
    if (joker == NULL)
        return NULL;
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

    return shop_economy_joker_buy_price(joker->value, joker->stickers & JOKER_STICKER_RENTAL);
}

int joker_get_sell_value(const Joker* joker)
{
    GBAL_RETURN_IF_NULL(joker, UNDEFINED);

    // Rental Jokers are bought for RENTAL_JOKER_BUY_PRICE: they sell for the minimum, $1
    return shop_economy_joker_sell_value(joker->value, joker->stickers & JOKER_STICKER_RENTAL);
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

    // The VRAM tiles and the OAM slots only exist for MAX_JOKER_OBJECTS layers: no layer, no object
    s16 layer = UNDEFINED;
    for (s16 i = 0; i < MAX_JOKER_OBJECTS; i++)
    {
        if (!s_used_layers[i])
        {
            layer = i;
            break;
        }
    }
    if (layer == UNDEFINED)
        return NULL;

    JokerObject* joker_object = POOL_GET(JokerObject);
    if (joker_object == NULL)
        return NULL;

    s_used_layers[layer] = true; // Mark this layer as used
    sprite_object_init((SpriteObject*)joker_object);

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

    // Called when a run starts
    s_gros_michel_extinct = false;
    s_luchador_disabled = false;
    s_deck_start_size = 0;
    memset(s_round_start_hands, 0, sizeof(s_round_start_hands));
}

int joker_roll_id(enum JokerRarity joker_rarity, enum RngSequence key)
{
    // Now determine how many jokers are available based on the rarity
    int jokers_avail_size = get_num_rollable_jokers();

    if (jokers_avail_size == 0)
        return UNDEFINED;

    // Joker IDs are below MAX_DEFINABLE_JOKERS (150) so they fit in a byte
    u8 matching_joker_ids[jokers_avail_size];
    u8 fallback_joker_ids[jokers_avail_size];
    int match_count = 0;
    int fallback_count = 0;
    u32 fallback_roll = rng_get_u32(key);

    BitsetItr itr = bitset_itr_create(&s_rollable_jokers_bitset);

    int joker_id = UNDEFINED;
    while ((joker_id = bitset_itr_next(&itr)) != UNDEFINED)
    {
        // Cavendish only after a Gros Michel went extinct, and then no more Gros Michel
        if (!joker_batch5_can_roll(joker_id, s_gros_michel_extinct))
            continue;

        const JokerInfo* info = get_joker_registry_entry(joker_id);
        if (info->rarity == joker_rarity)
            matching_joker_ids[match_count++] = joker_id;
        else if (info->rarity != LEGENDARY_JOKER) // Legendary Jokers only come from The Soul
            fallback_joker_ids[fallback_count++] = joker_id;
    }

    if (match_count > 0)
        return matching_joker_ids[rng_get_u32(key) % match_count];
    if (fallback_count > 0)
        return fallback_joker_ids[fallback_roll % fallback_count];
    return UNDEFINED;
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

    if (joker_batch7_showman_locks_roll(is_joker_owned(SHOWMAN_JOKER_ID)))
        joker_set_rollable(joker_id, false); // Showman: Jokers can repeat

    Joker* joker = joker_new(joker_id);
    if (joker == NULL)
    {
        joker_set_rollable(joker_id, true);
        return NULL;
    }
    // Judgement gives a Joker without Stake stickers, it can still get an edition
    if (key != RNG_SEQ_TAROT_JUDGEMENT)
        stake_roll_joker_stickers(joker);
    joker_roll_edition(joker);
#ifdef DEBUG_EDITIONS
    static int s_debug_edition_turn = 0;
    int debug_edition = (s_debug_edition_turn++ % 5) + 1; // 5 means none
    joker_set_edition(joker, EDITION_NONE);
    joker_set_edition(joker, debug_edition <= EDITION_NEGATIVE ? debug_edition : EDITION_NONE);
#endif

    JokerObject* joker_object = joker_object_new(joker);
    if (joker_object == NULL) // No free sprite layer
    {
        joker_set_rollable(joker_id, true);
        joker_destroy(&joker);
    }
    return (Item*)joker_object;
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

    s_luchador_disabled = false; // The Boss is over: the next one works again

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
                other->joker->value = joker_economy_add_value(other->joker->value, others_gain);

            // Gift Card also raises the sell value of the consumables (+$1 each)
            if (consumables_add_sell_value != NULL)
                consumables_add_sell_value(others_gain / 2);
        }

        if (joker->id == INVISIBLE_JOKER_ID)
            joker->persistent_state = joker_batch7_invisible_next(joker->persistent_state);

        if (joker->id == CAMPFIRE_JOKER_ID)
            joker->persistent_state = joker_batch7_campfire_after_round(joker->persistent_state, boss_defeated);

        if (joker->id == ROCKET_JOKER_ID)
            joker->persistent_state = joker_economy_rocket_next(joker->persistent_state, boss_defeated);

        // Batch 4: new targets (Idol, Ancient Joker, To Do List) and per-round counters
        if (joker->id >= BLOODSTONE_JOKER_ID && joker->id <= HIT_THE_ROAD_JOKER_ID)
        {
            JokerEffect* round_end_effect = NULL;
            joker_get_score_effect(joker, NULL, JOKER_EVENT_ON_ROUND_END, &round_end_effect);
        }

        // Batch 5: new targets (Castle, Mail-In Rebate), per-round counters, food that is eaten
        // up or destroyed by chance (Gros Michel, Cavendish, Popcorn, Turtle Bean)
        if (joker->id >= FIRST_BATCH5_JOKER_ID && joker->id <= LAST_BATCH5_JOKER_ID)
        {
            JokerEffect* round_end_effect = NULL;
            u32 flags =
                joker_get_score_effect(joker, NULL, JOKER_EVENT_ON_ROUND_END, &round_end_effect);
            if ((flags & JOKER_EFFECT_FLAG_EXPIRE) && round_end_effect != NULL &&
                round_end_effect->expire && joker_object_expire(joker_object) &&
                joker->id == GROS_MICHEL_JOKER_ID)
                s_gros_michel_extinct = true;
        }
    }
}

/*******************************************************************************
 * A8 batch 3: hooks of the Jokers tied to cards, Tarots and the deck
 ******************************************************************************/


// Where Tarots created by Jokers appear
#define JOKER_TAROT_SPAWN_X int2fx(120)
#define JOKER_TAROT_SPAWN_Y int2fx(60)

// The Joker whose ability a hook runs: the copied Joker for a Blueprint or a Brainstorm, the Joker
// itself for the others. Only hooks that don't keep a counter ask for it (the copy has no state).
static Joker* joker_hook_source(Joker* joker)
{
    if (joker->id != BLUEPRINT_JOKER_ID && joker->id != BRAINSTORM_JOKER_ID)
        return joker;
    Joker* target = joker_get_copy_target(joker);
    return target != NULL ? target : joker;
}

// Runs @p fn for every owned Joker that works (not Perishable-expired, not disabled by a Boss)
static void joker_for_each_active(void (*fn)(Joker* joker, Joker* source))
{
    ListItr itr = list_itr_create(get_jokers_list());
    JokerObject* joker_object;
    while ((joker_object = list_itr_next(&itr)))
    {
        if (joker_is_perished(joker_object->joker) || blind_effects_is_joker_disabled(joker_object))
            continue;
        fn(joker_object->joker, joker_hook_source(joker_object->joker));
    }
}

static void joker_hook_glass_broken(Joker* joker, Joker* source)
{
    if (joker->id == GLASS_JOKER_ID)
        joker->persistent_state++;
}

static void joker_hook_lucky_triggered(Joker* joker, Joker* source)
{
    if (joker->id == LUCKY_CAT_JOKER_ID)
        joker->persistent_state++;
}

static void joker_hook_planet_used(Joker* joker, Joker* source)
{
    if (joker->id == CONSTELLATION_JOKER_ID)
        joker->persistent_state++;
}

static void joker_hook_pack_opened(Joker* joker, Joker* source)
{
    if (joker->id == HALLUCINATION_JOKER_ID &&
        joker_cards_one_in(rng_get_u32(RNG_SEQ_JOKER_HALLUCINATION), HALLUCINATION_CHANCE))
        consumable_create_random_tarot(RNG_SEQ_JOKER_TAROT, JOKER_TAROT_SPAWN_X, JOKER_TAROT_SPAWN_Y);
}

static void joker_hook_blind_selected(Joker* joker, Joker* source)
{
    // Burglar and Marble Joker change the run for good: no Blueprint copy
    switch (joker->id)
    {
        case BURGLAR_JOKER_ID:
            g_game_vars.hands = joker_cards_burglar_hands(g_game_vars.hands);
            g_game_vars.discards = 0;
            display_hands();
            display_discards();
            break;
        case MARBLE_JOKER_ID:
            if (deck_get_size() < MAX_DECK_SIZE)
            {
                Card* stone = card_new(SPADES, ACE);
                stone->enhancement = CARD_ENHANCEMENT_STONE;
                deck_push(stone);
                joker_on_card_added();
            }
            break;
        default:
            break;
    }

    // Cartomancer and Riff-Raff can be copied by Blueprint and Brainstorm
    switch (source->id)
    {
        case CARTOMANCER_JOKER_ID:
            consumable_create_random_tarot(
                RNG_SEQ_JOKER_TAROT,
                JOKER_TAROT_SPAWN_X,
                JOKER_TAROT_SPAWN_Y
            );
            break;
        case RIFF_RAFF_JOKER_ID:
            for (int n = joker_batch7_riff_raff_count(g_game_vars.joker_slots - jokers_get_used_slots());
                 n > 0;
                 n--)
            {
                if (!consumable_create_joker(
                        COMMON_JOKER,
                        RNG_SEQ_JOKER_RIFF_RAFF,
                        JOKER_TAROT_SPAWN_X,
                        JOKER_TAROT_SPAWN_Y
                    ))
                    break;
            }
            break;
        default:
            break;
    }
}

static void joker_hook_deck_shuffled(Joker* joker, Joker* source)
{
    if (joker->id != CERTIFICATE_JOKER_ID || deck_get_size() >= MAX_DECK_SIZE)
        return;

    Card* card = card_new(
        rng_get_u32(RNG_SEQ_JOKER_CERTIFICATE) % NUM_SUITS,
        rng_get_u32(RNG_SEQ_JOKER_CERTIFICATE) % NUM_RANKS
    );
    card->seal = CARD_SEAL_NONE + 1 + rng_get_u32(RNG_SEQ_JOKER_CERTIFICATE) % (CARD_SEAL_MAX - 1);
    joker_on_card_added();
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

// Ceremonial Dagger: destroys the Joker to its right and gains twice its sell value as mult
static void joker_dagger_on_blind_selected(void)
{
    List* jokers = get_jokers_list();
    for (int i = 0; i < list_get_len(jokers) - 1; i++)
    {
        JokerObject* dagger = list_get_at_idx(jokers, i);
        if (dagger->joker->id != CEREMONIAL_DAGGER_JOKER_ID || joker_is_perished(dagger->joker) ||
            blind_effects_is_joker_disabled(dagger))
            continue;

        JokerObject* victim = list_get_at_idx(jokers, i + 1);
        if (!joker_can_be_sold(victim->joker))
            continue;

        dagger->joker->persistent_state = joker_batch5_dagger_add(
            dagger->joker->persistent_state,
            joker_get_sell_value(victim->joker)
        );
        joker_object_shake(dagger, UNDEFINED);
        remove_owned_joker(i + 1);
        item_start_discard_animation((Item*)victim);
    }
}

void joker_on_card_destroyed(Card* card)
{
    if (card == NULL || !card_is_face(card))
        return;

    ListItr itr = list_itr_create(get_jokers_list());
    JokerObject* joker_object;
    while ((joker_object = list_itr_next(&itr)))
    {
        Joker* joker = joker_object->joker;
        if (joker->id != CANIO_JOKER_ID || joker_is_perished(joker) ||
            blind_effects_is_joker_disabled(joker_object))
            continue;
        joker->persistent_state++;
        joker_object_shake(joker_object, UNDEFINED);
    }
}

void joker_on_shop_exit(void)
{
    ListItr itr = list_itr_create(get_jokers_list());
    JokerObject* joker_object;
    while ((joker_object = list_itr_next(&itr)))
    {
        Joker* joker = joker_object->joker;
        if (joker_hook_source(joker)->id != PERKEO_JOKER_ID || joker_is_perished(joker) ||
            blind_effects_is_joker_disabled(joker_object))
            continue;
        consumable_copy_random_owned(RNG_SEQ_JOKER_PERKEO);
        joker_object_shake(joker_object, UNDEFINED);
    }
}

bool joker_boss_is_disabled(void)
{
    if (s_luchador_disabled)
        return true;

    ListItr itr = list_itr_create(get_jokers_list());
    JokerObject* joker_object;
    while ((joker_object = list_itr_next(&itr)))
    {
        if (joker_object->joker->id == CHICOT_JOKER_ID && !joker_is_perished(joker_object->joker))
            return true;
    }
    return false;
}

static void joker_add_to_all(u8 joker_id)
{
    ListItr itr = list_itr_create(get_jokers_list());
    JokerObject* joker_object;
    while ((joker_object = list_itr_next(&itr)))
    {
        Joker* joker = joker_object->joker;
        if (joker->id != joker_id || joker_is_perished(joker) ||
            blind_effects_is_joker_disabled(joker_object))
            continue;
        joker->persistent_state++;
        joker_object_shake(joker_object, UNDEFINED);
    }
}

void joker_on_shop_reroll(void)
{
    joker_add_to_all(FLASH_CARD_JOKER_ID);
}

void joker_on_pack_skipped(void)
{
    joker_add_to_all(RED_CARD_JOKER_ID);
}

void joker_on_first_discard(enum HandType hand_type, bool first_discard)
{
    enum HandType target = joker_batch7_burnt_target(hand_type, first_discard);
    if (target == NONE)
        return;

    ListItr itr = list_itr_create(get_jokers_list());
    JokerObject* joker_object;
    while ((joker_object = list_itr_next(&itr)))
    {
        Joker* joker = joker_object->joker;
        if (joker_hook_source(joker)->id != BURNT_JOKER_ID || joker_is_perished(joker) ||
            blind_effects_is_joker_disabled(joker_object))
            continue;
        hand_level_up(target, 1);
        joker_object_shake(joker_object, UNDEFINED);
    }
}

void joker_on_boss_triggered(void)
{
    ListItr itr = list_itr_create(get_jokers_list());
    JokerObject* joker_object;
    while ((joker_object = list_itr_next(&itr)))
    {
        Joker* joker = joker_object->joker;
        if (joker_hook_source(joker)->id != MATADOR_JOKER_ID || joker_is_perished(joker) ||
            blind_effects_is_joker_disabled(joker_object))
            continue;
        g_game_vars.money += joker_batch8_matador_money(true);
        display_money();
        joker_object_shake(joker_object, UNDEFINED);
    }
}

int joker_mime_extra_passes(void)
{
    int nb_mimes = 0;
    ListItr itr = list_itr_create(get_jokers_list());
    JokerObject* joker_object;
    while ((joker_object = list_itr_next(&itr)))
    {
        Joker* joker = joker_object->joker;
        if (joker_hook_source(joker)->id == MIME_JOKER_ID && !joker_is_perished(joker) &&
            !blind_effects_is_joker_disabled(joker_object))
        {
            nb_mimes++;
            joker_object_shake(joker_object, UNDEFINED);
        }
    }
    return joker_batch8_mime_passes(nb_mimes) - 1;
}

void joker_on_card_added(void)
{
    joker_add_to_all(HOLOGRAM_JOKER_ID);
}

void joker_on_planet_used(int planet)
{
    g_game_vars.planets_used_mask = joker_batch7_planet_mask_add(g_game_vars.planets_used_mask, planet);
}

void joker_on_card_sold(void)
{
    joker_add_to_all(CAMPFIRE_JOKER_ID); // Campfire
}

// Madness: X0.5 per Small or Big Blind, destroys a random other Joker. Every Madness triggers.
static void joker_madness_on_blind_selected(void)
{
    if (!joker_batch7_madness_triggers(g_game_vars.current_blind >= BLIND_TYPE_BOSS))
        return;

    // Collect the Madness first: destroying a Joker changes the list under our feet
    JokerObject* madnesses[MAX_ACTIVE_JOKERS];
    int nb_madnesses = 0;
    ListItr itr = list_itr_create(get_jokers_list());
    JokerObject* joker_object;
    while ((joker_object = list_itr_next(&itr)) && nb_madnesses < MAX_ACTIVE_JOKERS)
    {
        if (joker_object->joker->id == MADNESS_JOKER_ID && !joker_is_perished(joker_object->joker) &&
            !blind_effects_is_joker_disabled(joker_object))
            madnesses[nb_madnesses++] = joker_object;
    }

    List* jokers = get_jokers_list();
    for (int m = 0; m < nb_madnesses; m++)
    {
        JokerObject* madness = madnesses[m];
        if (madness == NULL)
            continue; // Destroyed by a Madness before it

        madness->joker->persistent_state++;
        joker_object_shake(madness, UNDEFINED);

        int nb_candidates = 0;
        for (int j = 0; j < list_get_len(jokers); j++)
        {
            JokerObject* other = list_get_at_idx(jokers, j);
            if (other != madness && joker_can_be_sold(other->joker))
                nb_candidates++;
        }
        int pick = joker_batch7_madness_pick(rng_get_u32(RNG_SEQ_JOKER_MADNESS), nb_candidates);
        if (pick < 0)
            continue;
        for (int j = 0; j < list_get_len(jokers); j++)
        {
            JokerObject* victim = list_get_at_idx(jokers, j);
            if (victim == madness || !joker_can_be_sold(victim->joker))
                continue;
            if (pick-- == 0)
            {
                for (int k = m + 1; k < nb_madnesses; k++)
                {
                    if (madnesses[k] == victim)
                        madnesses[k] = NULL;
                }
                remove_owned_joker(j);
                item_start_discard_animation((Item*)victim);
                break;
            }
        }
    }
}

void joker_on_joker_sold(Joker* joker)
{
    // Diet Cola: selling it gives a Double Tag
    if (joker->id == DIET_COLA_JOKER_ID && !joker_is_perished(joker))
    {
        SkipTag* tag = skip_tag_new(SKIP_TAG_TYPE_DOUBLE);
        add_skip_tag(&tag);
    }

    // Invisible Joker: after enough rounds a random other Joker is duplicated
    if (joker->id == INVISIBLE_JOKER_ID && !joker_is_perished(joker) &&
        joker_batch7_invisible_ready(joker->persistent_state))
        consumable_copy_random_joker(joker, RNG_SEQ_JOKER_INVISIBLE, JOKER_TAROT_SPAWN_X, JOKER_TAROT_SPAWN_Y);

    if (joker->id == LUCHADOR_JOKER_ID && !joker_is_perished(joker) &&
        joker_batch6_luchador_disables(
            game_get_state() == GAME_STATE_ROUND,
            g_game_vars.current_blind >= BLIND_TYPE_BOSS
        ))
        s_luchador_disabled = true;
}

bool joker_mr_bones_saves(u32 score, u32 requirement)
{
    if (!joker_batch6_mr_bones_saves(score, requirement))
        return false;

    ListItr itr = list_itr_create(get_jokers_list());
    JokerObject* joker_object;
    while ((joker_object = list_itr_next(&itr)))
    {
        Joker* joker = joker_object->joker;
        if (joker->id != MR_BONES_JOKER_ID || joker_is_perished(joker) ||
            blind_effects_is_joker_disabled(joker_object))
            continue;
        // An Eternal Mr. Bones can't be consumed, so it can't save the run either
        if (!joker_object_expire(joker_object))
            continue;
        return true;
    }
    return false;
}

void joker_on_blind_selected(void)
{
    s_luchador_disabled = false;
    if (s_deck_start_size == 0)
        s_deck_start_size = joker_count_deck_cards();
    // Card Sharp: the hands played in the run so far, to tell the hands of this round apart
    memcpy(s_round_start_hands, g_game_vars.nb_played_hands, sizeof(s_round_start_hands));
    joker_for_each_active(joker_hook_blind_selected);
    joker_dagger_on_blind_selected();
    joker_madness_on_blind_selected();
}

void joker_on_deck_shuffled(void)
{
    joker_for_each_active(joker_hook_deck_shuffled);
}

static int joker_count_deck_cards(void)
{
    int total = 0;
    for (int e = 0; e < CARD_ENHANCEMENT_MAX; e++)
        total += game_count_deck_enhancement(e);
    return total;
}

int joker_deck_start_size(void)
{
    // Normally taken when the first Blind is selected, before any card is added or destroyed
    return s_deck_start_size > 0 ? s_deck_start_size : joker_count_deck_cards();
}

const u32* joker_round_start_hands(void)
{
    return s_round_start_hands;
}

bool joker_object_expire(JokerObject* joker_object)
{
    if (joker_object == NULL || !joker_can_be_sold(joker_object->joker))
        return false;

    List* expired = get_expired_jokers_list();
    ListItr itr = list_itr_create(expired);
    JokerObject* other;
    while ((other = list_itr_next(&itr)))
    {
        if (other == joker_object)
            return false;
    }

    joker_object_shake(joker_object, UNDEFINED);
    list_push_back(expired, joker_object);
    return true;
}

bool joker_on_cards_discarded(const DiscardCounts* discarded)
{
    bool destroy_cards = false;
    if (discarded->nb_cards <= 0)
        return false;

    ListItr itr = list_itr_create(get_jokers_list());
    JokerObject* joker_object;
    while ((joker_object = list_itr_next(&itr)))
    {
        Joker* joker = joker_object->joker;
        if (joker_is_perished(joker) || blind_effects_is_joker_disabled(joker_object))
            continue;

        int money = 0;

        // Faceless Joker, Mail-In Rebate and Trading Card can be copied by Blueprint/Brainstorm
        const Joker* source = joker_hook_source(joker);
        switch (source->id)
        {
            case FACELESS_JOKER_ID:
                money = joker_batch4_faceless_money(discarded->nb_face_cards);
                break;
            case MAIL_IN_REBATE_JOKER_ID:
                money = joker_batch5_mail_in_money(discarded->ranks[source->persistent_state % NUM_RANKS]);
                break;
            case TRADING_CARD_JOKER_ID:
                if (joker_batch5_trading_card_hit(round_get_discards_used(), discarded->nb_cards))
                {
                    money = TRADING_CARD_MONEY;
                    destroy_cards = true;
                }
                break;
            default:
                break;
        }

        switch (joker->id)
        {
            case GREEN_JOKER_ID:
                joker->persistent_state = joker_batch4_green_discard(joker->persistent_state);
                break;
            case HIT_THE_ROAD_JOKER_ID:
                joker->persistent_state += discarded->nb_jacks;
                break;
            case YORICK_JOKER_ID:
            {
                int before = joker_batch6_yorick_percent(joker->persistent_state);
                joker->persistent_state += discarded->nb_cards;
                if (joker_batch6_yorick_percent(joker->persistent_state) != before)
                    joker_object_shake(joker_object, UNDEFINED);
                break;
            }
            case RAMEN_JOKER_ID:
                joker->persistent_state =
                    joker_batch5_ramen_discard(joker->persistent_state, discarded->nb_cards);
                if (joker_batch5_ramen_gone(joker->persistent_state))
                    joker_object_expire(joker_object);
                else
                    joker_object_shake(joker_object, UNDEFINED);
                break;
            case CASTLE_JOKER_ID:
            {
                int cards = discarded->suits[joker_batch5_castle_suit(joker->persistent_state)];
                if (cards > 0)
                {
                    joker->persistent_state =
                        joker_batch5_castle_discard(joker->persistent_state, cards);
                    joker_object_shake(joker_object, UNDEFINED);
                }
                break;
            }
            default:
                break;
        }

        if (money > 0)
        {
            g_game_vars.money += money;
            display_money();
            joker_object_shake(joker_object, UNDEFINED);
        }
    }
    return destroy_cards;
}
