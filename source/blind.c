/**
 * @file blind.c
 * @brief Implementation of functions relative to the behaviour and graphics of Blinds.
 */

#include "ante_table.h"
#include "deck_rules.h"
#include "blind.h"

#include "blind_gfx.h"
#include "game.h"
#include "game/common_ui.h"
#include "graphic_utils.h"
#include "hand.h"
#include "joker.h"
#include "list.h"
#include "random.h"
#include "stdbool.h"
#include "util.h"

#include <stdlib.h>
#include <tonc.h>

#define BLIND_SPRITE_COPY_SIZE (BLIND_SPRITE_TILES * TILE_SIZE)

#define BLIND_TOKENS_PER_SPRITESHEET 2
#define BLIND_TOKEN_PALETTE_SIZE     8

#define SMALL_BLIND_REWARD    3
#define BIG_BLIND_REWARD      4
#define BOSS_BLIND_REWARD     5
#define SHOWDOWN_BLIND_REWARD 8

static const unsigned int* blind_gfxTiles[] = {
#define DEF_BLIND_GFX(idx) blind_gfx##idx##Tiles,
#include "../include/def_blind_gfx_table.h"
#undef DEF_BLIND_GFX
};

static const unsigned short* blind_gfxPal[] = {
#define DEF_BLIND_GFX(idx) blind_gfx##idx##Pal,
#include "../include/def_blind_gfx_table.h"
#undef DEF_BLIND_GFX
};

// Bitfields storing blinds we have yet to beat during the current run
static List s_unbeaten_boss_blinds = LIST_DEFAULT;
static List s_unbeaten_showdown_blinds = LIST_DEFAULT;

// The base requirement per ante lives in ante_table.h (antes 0-12, so Endless mode fits in a u32).
// Green and Purple Stakes use the faster scaling tables, like in Balatro.
static inline u32 ante_base_requirement(int ante)
{
    int scaling = 0;
    if (g_game_vars.stake >= STAKE_PURPLE)
        scaling = 2;
    else if (g_game_vars.stake >= STAKE_GREEN)
        scaling = 1;
    return ante_table_base(scaling, ante);
}


/**
 * @brief One Blind struct for each BlindType value, indexed by BlindType, filled when the lists of
 *         unbeaten Blinds are built. The score requirement multipliers are in blind_rules.h
 *         (The Wall x4 and Violet Vessel x6 effects are their score requirement).
 */
static Blind s_blind_type_map[BLIND_TYPE_MAX];

void blind_init()
{
    // Set up the palette used by normal blind tokens.
    // We will never need to edit it ever again.
    memcpy16(&pal_obj_bank[NORMAL_BLIND_PB], blind_gfxPal[0], NUM_ELEM_IN_ARR(blind_gfx0Pal));

    return;
}

u32 blind_get_requirement(enum BlindType type, int ante)
{
    // Ensure ante is within valid range
    if (ante < 0 || ante > ANTE_LAST)
        ante = 0;

    u32 multiplier = blind_rules_req_mult_fx8(type);
    if (type == BLIND_TYPE_WALL || type == BLIND_TYPE_VESSEL)
    {
        // Chicot: the Boss is disabled, so it asks the plain Boss requirement (X2)
        if (joker_boss_is_disabled())
            multiplier = (u32)FIX_ONE * joker_batch6_chicot_requirement_mult(2);
    }

    // 64 bits and saturated: ante 12 Boss Blinds are in the billions
    return ante_table_requirement(
        ante_base_requirement(ante),
        multiplier * deck_rules_blind_scale(g_game_vars.deck)
    );
}

int blind_get_reward(enum BlindType type)
{
    switch (type)
    {
        case BLIND_TYPE_SMALL:
            // Red Stake and above: the Small Blind gives no reward money
            return (g_game_vars.stake >= STAKE_RED) ? 0 : SMALL_BLIND_REWARD;
        case BLIND_TYPE_BIG:
            return BIG_BLIND_REWARD;
        case BLIND_TYPE_BOSS ...(BLIND_TYPE_SHOWDOWN - 1):
            return BOSS_BLIND_REWARD;
        default:
            return SHOWDOWN_BLIND_REWARD;
    }
}

/**
 * @brief Fill Lists of unbeaten Boss and Showdown Blinds
 *
 *         By keeping track of what Blind we have beaten or not, we can ensure that until we've
 *         beaten all the Blinds in a single Run, we won't encounter the same one twice.
 *
 * This must be called at the beginning of a run.
 *
 * @param showdown toggle between the List for Showdown and regular Boss Blinds
 */
static void init_unbeaten_blinds_list(bool showdown)
{
    List* p_unbeaten_blinds = showdown ? &s_unbeaten_showdown_blinds : &s_unbeaten_boss_blinds;

    // empty the list just to be sure
    list_clear(p_unbeaten_blinds);

    int lower_blind = showdown ? BLIND_TYPE_SHOWDOWN : BLIND_TYPE_BOSS;
    int upper_blind = showdown ? BLIND_TYPE_MAX - 1 : BLIND_TYPE_SHOWDOWN - 1;

    for (int i = lower_blind; i <= upper_blind; i++)
    {
        s_blind_type_map[i].type = i;
        list_push_back(p_unbeaten_blinds, &s_blind_type_map[i]);
    }
}

void init_unbeaten_blinds_lists(void)
{
    init_unbeaten_blinds_list(false);
    init_unbeaten_blinds_list(true);
}

/**
 * @brief Types of the unbeaten Blinds of a list, returns how many there are
 */
static int unbeaten_blinds_types(List* p_unbeaten_blinds, u8* types)
{
    int nb_types = 0;
    ListItr itr = list_itr_create(p_unbeaten_blinds);
    Blind* blind = NULL;
    while ((blind = list_itr_next(&itr)))
        types[nb_types++] = blind->type;
    return nb_types;
}

enum BlindType roll_blind_type(bool showdown, int ante)
{
    List* p_unbeaten_blinds = showdown ? &s_unbeaten_showdown_blinds : &s_unbeaten_boss_blinds;
    u8 types[BLIND_TYPE_MAX];

    // Roll among the unbeaten Blinds that can show up at this ante
    int nb_types = unbeaten_blinds_types(p_unbeaten_blinds, types);
    int nb_eligible = 0;
    blind_rules_pick(types, nb_types, ante, 0, &nb_eligible);

    // Fill the list with all blinds if none can be rolled
    // (happens on startup or if we have beaten all the blinds of this ante)
    if (nb_eligible == 0)
    {
        init_unbeaten_blinds_list(showdown);
        nb_types = unbeaten_blinds_types(p_unbeaten_blinds, types);
        blind_rules_pick(types, nb_types, ante, 0, &nb_eligible);
    }

    // Never happens: every ante has Blinds of its kind. Falls back on the first of the list.
    if (nb_eligible == 0)
        return showdown ? BLIND_TYPE_SHOWDOWN : BLIND_TYPE_BOSS;

    return (enum BlindType)blind_rules_pick(
        types,
        nb_types,
        ante,
        rng_get_u32(RNG_SEQ_BLIND),
        NULL
    );
}

void set_blind_beaten(enum BlindType type)
{
    bool showdown = (type >= BLIND_TYPE_SHOWDOWN);
    List* p_unbeaten_blinds = showdown ? &s_unbeaten_showdown_blinds : &s_unbeaten_boss_blinds;

    // find the beaten blind idx in the list
    int beaten_idx = 0;
    Blind* beaten_blind = NULL;
    ListItr itr = list_itr_create(p_unbeaten_blinds);

    while ((beaten_blind = list_itr_next(&itr)))
    {
        if (beaten_blind->type == type)
        {
            break;
        }
        beaten_idx++;
    }

    if (beaten_idx < list_get_len(p_unbeaten_blinds))
    {
        list_remove_at_idx(p_unbeaten_blinds, beaten_idx);
    }
}

static u32 get_blind_pb(enum BlindType type)
{
    return (type <= BLIND_TYPE_BIG) ? NORMAL_BLIND_PB : BOSS_BLIND_PB;
}

static u32 get_blind_spritesheet_idx(enum BlindType type)
{
    // See comment below in blind_get_color why we differenciate before/after the Mark
    return (type < BLIND_TYPE_MARK)
             ? type / BLIND_TOKENS_PER_SPRITESHEET
             : BLIND_TYPE_MARK / BLIND_TOKENS_PER_SPRITESHEET + type - BLIND_TYPE_MARK;
}

u16 blind_get_color(enum BlindType type, enum BlindColorIndex index)
{
    // Do a little translation of palette idx -> custom array idx
    // All blinds before the Mark are arranged in pairs with their palettte split in two
    // | XX |  1 |  2 |  3 |  4 |  5 |  6 |  7 | -> first sprite
    // | XX |  9 | 10 | 11 | 12 | 13 | 14 | 15 | -> second sprite
    // so we need to offset the color indices by 8 for blinds with an odd type
    // From the Mark onwards, all sprites are alone in their spritesheet, so no need to offset
    u32 color_idx = index + ((type < BLIND_TYPE_MARK)
                                 ? BLIND_TOKEN_PALETTE_SIZE * (type % BLIND_TOKENS_PER_SPRITESHEET)
                                 : 0);
    return blind_gfxPal[get_blind_spritesheet_idx(type)][color_idx];
}

void apply_blind_colors(enum BlindType type)
{
    // keep track of active boss blind spritesheet to copy colors only when changing
    static u32 active_boss_spritesheet = BLIND_TYPE_MAX;
    u32 new_spritesheet = get_blind_spritesheet_idx(type);

    // No need to update normal blind palette
    if (type < BLIND_TYPE_BOSS || active_boss_spritesheet == new_spritesheet)
    {
        return;
    }

    active_boss_spritesheet = new_spritesheet;

    // Just copy the palette as is to the sprite palette bank
    memcpy16(
        &pal_obj_bank[BOSS_BLIND_PB],
        blind_gfxPal[active_boss_spritesheet],
        NUM_ELEM_IN_ARR(blind_gfx0Pal)
    );
}

void apply_blind_tiles(enum BlindType type, enum BlindTokenLayers layer)
{
    int tile_index = sprite_get_tid(BLIND_TOKEN_SPRITE, layer);
    u32 spritesheet_idx = get_blind_spritesheet_idx(type);
    u32 sprite_idx = (type < BLIND_TYPE_MARK) ? type % BLIND_TOKENS_PER_SPRITESHEET : 0;
    memcpy32(
        &tile_mem[TILE_MEM_OBJ_CHARBLOCK0_IDX][tile_index],
        &blind_gfxTiles[spritesheet_idx][sprite_idx * BLIND_SPRITE_COPY_SIZE],
        BLIND_SPRITE_COPY_SIZE
    );

    apply_blind_colors(type);
}

Sprite* blind_token_new(enum BlindType type, int x, int y, enum BlindTokenLayers layer)
{
    apply_blind_tiles(type, layer);

    Sprite* sprite = sprite_new(
        ATTR0_SQUARE | ATTR0_4BPP,
        ATTR1_SIZE_32x32,
        sprite_get_tid(BLIND_TOKEN_SPRITE, layer),
        get_blind_pb(type),
        sprite_get_starting_layer(BLIND_TOKEN_SPRITE) + layer
    );
    sprite_position(sprite, x, y);

    return sprite;
}
