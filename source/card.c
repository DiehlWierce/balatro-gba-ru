#include "card.h"

#include "edition_gfx.h"
#include "game.h"
#include "game_variables.h"
#include "graphic_utils.h"
#include "item.h"
#include "util.h"
#include "voucher.h"

#include <maxmod.h>
#include <stdlib.h>

// Audio
#include "pool.h"
#include "soundbank.h"

// Card Sprites and Palettes
#include "deck_big_gfx.h"
#include "deck_gfx.h"
#include "decks_face_down_gfx.h"
#include "card_rarity_pal_gfx.h"
#include "high_contrast_deck_pal_gfx.h"

// Card sprites lookup table. First index is the suit, second index is the rank. The value is the
// tile index.
const static u16 CARD_SPRITE_LUT[NUM_SUITS][NUM_RANKS] = {
    {0,   16,  32,  48,  64,  80,  96,  112, 128, 144, 160, 176, 192},
    {208, 224, 240, 256, 272, 288, 304, 320, 336, 352, 368, 384, 400},
    {416, 432, 448, 464, 480, 496, 512, 528, 544, 560, 576, 592, 608},
    {624, 640, 656, 672, 688, 704, 720, 736, 752, 768, 784, 800, 816}
};
// Deck sprites lookup table. Index is the deck Id. The value is the tile index.
const static u16 DECK_SPRITE_LUT[DECK_TYPE_MAX] = {0, 16, 32, 48, 64, 80};

// Playing cards in the Shop (Magic Trick): their sprites use the layers that the hand uses in a
// round, from the end so that the cards of an opened Booster Pack (layers 0 to 4) don't clash
#define SHOP_CARDS_MAX               MAX_SHOP_ITEMS
#define SHOP_CARDS_FIRST_LAYER       (MAX_HAND_SIZE - SHOP_CARDS_MAX)
#define SHOP_CARD_ENHANCEMENT_CHANCE 40 // %, Illusion, same as Standard Packs
#define SHOP_CARD_SEAL_CHANCE        20 // %

static CardObject* s_shop_cards[SHOP_CARDS_MAX] = {NULL};

static const char* const RANK_NAMES[NUM_RANKS] = {
    "Двойка", "Тройка", "Четвёрка", "Пятёрка", "Шестёрка", "Семёрка", "Восьмёрка", "Девятка",
    "Десятка", "Валет", "Дама", "Король", "Туз",
};
static const char* const SUIT_NAMES[NUM_SUITS] = {"бубен", "треф", "червей", "пик"};

static bool s_high_contrast = DEFAULT_HIGH_CONTRAST;
static bool s_more_readable = DEFAULT_MORE_READABLE;

void card_init()
{
    GRIT_CPY(&pal_obj_mem[DECK_PB * PAL_ROW_LEN], decks_face_down_gfxPal);
}

void set_cards_high_contrast(bool enable)
{
    s_high_contrast = enable;
    if (s_high_contrast)
    {
        GRIT_CPY(&pal_obj_mem[CARD_PB * PAL_ROW_LEN], high_contrast_deck_pal_gfxPal);
    }
    else
    {
        GRIT_CPY(&pal_obj_mem[CARD_PB * PAL_ROW_LEN], deck_gfxPal);
    }
}

void set_cards_more_readable(bool enable)
{
    s_more_readable = enable;
}

bool get_cards_high_contrast(void)
{
    return s_high_contrast;
}

bool get_cards_more_readable(void)
{
    return s_more_readable;
}

// Card methods
Card* card_new(u8 suit, u8 rank)
{
    Card* card = POOL_GET(Card);

    card->suit = suit;
    card->rank = rank;
    card->enhancement = CARD_ENHANCEMENT_NONE;
    card->seal = CARD_SEAL_NONE;
    card->debuffed = false;
    card->face_down = false;
    card->edition = EDITION_NONE;
    card->played_ante = 0;
    card->bonus_chips = 0;

    return card;
}

void card_destroy(Card** card)
{
    POOL_FREE(Card, *card);
    *card = NULL;
}

u8 card_get_value(Card* card)
{
    if (!card_has_rank(card))
    {
        return 0; // Stone cards have no rank
    }
    else if (card->rank == JACK || card->rank == QUEEN || card->rank == KING)
    {
        return 10; // Face cards are worth 10
    }
    else if (card->rank == ACE)
    {
        return 11; // Ace is worth 11
    }
    else
    {
        return card->rank + RANK_OFFSET; // 2-10 are worth their rank + RANK_OFFSET
    }

    return 0; // Should never reach here, but just in case
}

#define BONUS_CARD_CHIPS 30
#define STONE_CARD_CHIPS 50

int card_get_chips(Card* card)
{
    if (card->enhancement == CARD_ENHANCEMENT_STONE)
        return STONE_CARD_CHIPS + card->bonus_chips;
    if (card->enhancement == CARD_ENHANCEMENT_BONUS)
        return card_get_value(card) + BONUS_CARD_CHIPS + card->bonus_chips;
    return card_get_value(card) + card->bonus_chips;
}

// clang-format off
static const char* const ENHANCEMENT_NAMES[CARD_ENHANCEMENT_MAX] = {
    [CARD_ENHANCEMENT_NONE]  = NULL,
    [CARD_ENHANCEMENT_BONUS] = "Бонусная",
    [CARD_ENHANCEMENT_MULT]  = "Множ.",
    [CARD_ENHANCEMENT_WILD]  = "Дикая",
    [CARD_ENHANCEMENT_GLASS] = "Стеклянная",
    [CARD_ENHANCEMENT_STEEL] = "Стальная",
    [CARD_ENHANCEMENT_STONE] = "Каменная",
    [CARD_ENHANCEMENT_GOLD]  = "Золотая",
    [CARD_ENHANCEMENT_LUCKY] = "Счастливая",
};

static const char* const SEAL_NAMES[CARD_SEAL_MAX] = {
    [CARD_SEAL_NONE]   = NULL,
    [CARD_SEAL_GOLD]   = "Золотая печать",
    [CARD_SEAL_RED]    = "Красная печать",
    [CARD_SEAL_BLUE]   = "Синяя печать",
    [CARD_SEAL_PURPLE] = "Фиолет. печать",
};
// clang-format on

const char* card_get_enhancement_name(const Card* card)
{
    return card->enhancement < CARD_ENHANCEMENT_MAX ? ENHANCEMENT_NAMES[card->enhancement] : NULL;
}

const char* card_get_seal_name(const Card* card)
{
    return card->seal < CARD_SEAL_MAX ? SEAL_NAMES[card->seal] : NULL;
}

const char* card_get_edition_name(const Card* card)
{
    return edition_get_name(card->edition);
}

#pragma region Card modifiers drawing
/*
 * Enhancements and seals are drawn into the card's own copy of the tiles, with the colours of the
 * card palette (CARD_PB): no extra palette bank and no extra sprite. Enhancements tint the white
 * face of the card with a 1-pixel dither of a palette colour, Stone cards get a grey face without
 * rank or suit, seals are a small disc in the bottom-left corner. The colours are looked up by
 * RGB in the current card palette, so the high contrast palette uses its closest colours.
 */

#define CARD_TILES_WORDS (CARD_SPRITE_TILES * TILE_SIZE) // u32 words of a 32x32 4bpp sprite
#define CARD_SPRITE_PX   32
#define CARD_FACE_COLOR  1 // White face of the card in deck_gfx.png
#define CARD_FACE_LEFT   5
#define CARD_FACE_RIGHT  26
#define CARD_FACE_TOP    1
#define CARD_FACE_BOTTOM 30
#define SEAL_LEFT        5
#define SEAL_TOP         23
#define SEAL_SIZE        6

typedef struct
{
    u8 r, g, b;
} Rgb;

static inline int card_px_get(const u32* tiles, int x, int y)
{
    int tile = (y / 8) * 4 + (x / 8);
    u32 word = tiles[tile * TILE_SIZE + (y % 8)];
    return (word >> ((x % 8) * 4)) & 0xF;
}

static inline void card_px_set(u32* tiles, int x, int y, int color)
{
    int tile = (y / 8) * 4 + (x / 8);
    u32* word = &tiles[tile * TILE_SIZE + (y % 8)];
    int shift = (x % 8) * 4;
    *word = (*word & ~(0xFu << shift)) | ((u32)color << shift);
}

/**
 * @brief Index of the card palette colour closest to @p c, ignoring the transparent index 0.
 */
static int card_palette_closest(Rgb c)
{
    int best = 1;
    int best_dist = 0x7FFFFFFF;
    for (int i = 1; i < PAL_ROW_LEN; i++)
    {
        COLOR pal = pal_obj_mem[CARD_PB * PAL_ROW_LEN + i];
        int dr = ((pal & 0x1F) << 3) - c.r;
        int dg = (((pal >> 5) & 0x1F) << 3) - c.g;
        int db = (((pal >> 10) & 0x1F) << 3) - c.b;
        int dist = dr * dr + dg * dg + db * db;
        if (dist < best_dist)
        {
            best_dist = dist;
            best = i;
        }
    }
    return best;
}

int card_palette_find_color(u8 r, u8 g, u8 b)
{
    return card_palette_closest((Rgb){r, g, b});
}

// clang-format off
// Tint of each enhancement, dithered with the white face
static const Rgb ENHANCEMENT_TINTS[CARD_ENHANCEMENT_MAX] = {
    [CARD_ENHANCEMENT_BONUS] = {41,  173, 255},
    [CARD_ENHANCEMENT_MULT]  = {254, 95,  85 },
    [CARD_ENHANCEMENT_GLASS] = {192, 200, 214},
    [CARD_ENHANCEMENT_STEEL] = {192, 200, 214},
    [CARD_ENHANCEMENT_GOLD]  = {255, 163, 0  },
    [CARD_ENHANCEMENT_LUCKY] = {35,  89,  85 },
};

// Wild cards: one tint per quarter of the card
static const Rgb WILD_TINTS[4] = {
    {240, 107, 63 },
    {41,  173, 255},
    {35,  89,  85 },
    {240, 52,  100},
};

static const Rgb SEAL_COLORS[CARD_SEAL_MAX] = {
    [CARD_SEAL_GOLD]   = {255, 163, 0  },
    [CARD_SEAL_RED]    = {240, 52,  100},
    [CARD_SEAL_BLUE]   = {41,  173, 255},
    [CARD_SEAL_PURPLE] = {137, 105, 230},
};

// 6x6 disc: 'o' outline, 'x' seal colour, 'w' shine
static const char* const SEAL_SHAPE[SEAL_SIZE] = {
    " oooo ",
    "oxxxxo",
    "oxwxxo",
    "oxxxxo",
    "oxxxxo",
    " oooo ",
};
// clang-format on

static void card_draw_stone(u32* tiles)
{
    int stone = card_palette_closest((Rgb){192, 200, 214});
    int speck_dark = card_palette_closest((Rgb){55, 70, 73});
    int speck_light = card_palette_closest((Rgb){230, 230, 230});

    for (int y = CARD_FACE_TOP + 1; y < CARD_FACE_BOTTOM; y++)
    {
        for (int x = CARD_FACE_LEFT + 1; x < CARD_FACE_RIGHT; x++)
        {
            int color = stone;
            // Irregular speckles (a linear pattern would look like stripes)
            int h = (x * x * 5 + y * y * 3 + x * y) % 13;
            if (h == 0)
                color = speck_dark;
            else if (h == 7 || h == 11)
                color = speck_light;
            card_px_set(tiles, x, y, color);
        }
    }
}

static inline bool card_enhancement_px_tinted(enum CardEnhancement enhancement, int x, int y)
{
    switch (enhancement)
    {
        case CARD_ENHANCEMENT_GLASS:
            return ((x + y) % 6) < 2; // Diagonal shine streaks
        case CARD_ENHANCEMENT_STEEL:
            return ((x + y) & 1) || (y & 1); // Mostly grey
        default:
            return (x + y) & 1; // Checkerboard dither
    }
}

static void card_draw_enhancement(u32* tiles, enum CardEnhancement enhancement)
{
    if (enhancement == CARD_ENHANCEMENT_STONE)
    {
        card_draw_stone(tiles);
        return;
    }

    int tints[4];
    for (int i = 0; i < 4; i++)
    {
        tints[i] = card_palette_closest(
            enhancement == CARD_ENHANCEMENT_WILD ? WILD_TINTS[i] : ENHANCEMENT_TINTS[enhancement]
        );
    }

    for (int y = 0; y < CARD_SPRITE_PX; y++)
    {
        for (int x = 0; x < CARD_SPRITE_PX; x++)
        {
            if (card_px_get(tiles, x, y) != CARD_FACE_COLOR ||
                !card_enhancement_px_tinted(enhancement, x, y))
                continue;

            int quarter = (y >= CARD_SPRITE_PX / 2) * 2 + (x >= CARD_SPRITE_PX / 2);
            card_px_set(tiles, x, y, tints[quarter]);
        }
    }
}

static void card_draw_seal(u32* tiles, enum CardSeal seal)
{
    int fill = card_palette_closest(SEAL_COLORS[seal]);
    int outline = card_palette_closest((Rgb){60, 67, 104});
    int shine = card_palette_closest((Rgb){255, 255, 255});

    for (int dy = 0; dy < SEAL_SIZE; dy++)
    {
        for (int dx = 0; dx < SEAL_SIZE; dx++)
        {
            char c = SEAL_SHAPE[dy][dx];
            if (c == ' ')
                continue;
            int color = (c == 'o') ? outline : (c == 'w') ? shine : fill;
            card_px_set(tiles, SEAL_LEFT + dx, SEAL_TOP + dy, color);
        }
    }
}

/**
 * @brief Copies the card tiles to @p dest (VRAM) with the enhancement and seal drawn on them.
 */
static void card_copy_tiles(const Card* card, u32* dest, const unsigned int* src)
{
    if (card->enhancement == CARD_ENHANCEMENT_NONE && card->seal == CARD_SEAL_NONE &&
        card->edition == EDITION_NONE)
    {
        memcpy32(dest, src, CARD_TILES_WORDS);
        return;
    }

    u32 tiles[CARD_TILES_WORDS];
    memcpy32(tiles, src, CARD_TILES_WORDS);

    if (card->enhancement < CARD_ENHANCEMENT_MAX && card->enhancement != CARD_ENHANCEMENT_NONE)
        card_draw_enhancement(tiles, card->enhancement);
    if (card->edition != EDITION_NONE && card->edition < EDITION_MAX)
        edition_draw_tiles(tiles, CARD_PB, card->edition);
    if (card->seal < CARD_SEAL_MAX && card->seal != CARD_SEAL_NONE)
        card_draw_seal(tiles, card->seal);

    memcpy32(dest, tiles, CARD_TILES_WORDS);
}
#pragma endregion

// CardObject methods
CardObject* card_object_new(Card* card)
{
    CardObject* card_object = POOL_GET(CardObject);
    sprite_object_init((SpriteObject*)card_object);
    card_object->card = card;

    card_object->type = ITEM_TYPE_PLAYING_CARD;
    card_object->selected = false;

    return card_object;
}

void card_object_destroy(CardObject** card_object)
{
    if (*card_object == NULL)
        return;
    sprite_object_destroy((SpriteObject*)(*card_object));
    POOL_FREE(CardObject, *card_object);
    *card_object = NULL;
}

/**
 * @brief Colors of a debuffed card: the face becomes grey and every colored pixel dark grey,
 *        so the card stays readable but looks disabled. Indices of `deck_gfx.png`, also valid
 *        for the high contrast palette (1 and 12 white, 2 light grey, 9 dark grey).
 */
static const u8 DEBUFFED_CARD_COLOR_LUT[16] = {
    0,
    2,
    2,
    9,
    9,
    9,
    9,
    7,
    9,
    9,
    9,
    9,
    2,
    9,
    9,
    15,
};

/**
 * @brief Recolors the 4bpp tiles of a card sprite with DEBUFFED_CARD_COLOR_LUT, in place.
 */
static void card_tiles_apply_debuffed_colors(u32* tiles, int nb_words)
{
    for (int w = 0; w < nb_words; w++)
    {
        u32 word = tiles[w];
        u32 recolored = 0;
        for (int px = 0; px < 8; px++)
        {
            u32 color = (word >> (px * 4)) & 0xF;
            recolored |= (u32)DEBUFFED_CARD_COLOR_LUT[color] << (px * 4);
        }
        tiles[w] = recolored;
    }
}

void card_object_set_sprite(CardObject* card_object, s16 layer)
{
    // Free the current sprite first: the new one uses the same OAM slot
    sprite_object_set_sprite((SpriteObject*)card_object, NULL);

    int tile_index = sprite_get_tid(CARD_SPRITE, layer);
    Card* card = card_object->card;
    int pb = CARD_PB;

    if (card->face_down)
    {
        // Face down cards show the back of the run's Deck
        card_copy_deck_back_tiles(&tile_mem[TILE_MEM_OBJ_CHARBLOCK0_IDX][tile_index]);
        pb = DECK_PB;
    }
    else
    {
        const unsigned int* card_tiles = s_more_readable ? deck_big_gfxTiles : deck_gfxTiles;
        card_copy_tiles(
            card,
            (u32*)&tile_mem[TILE_MEM_OBJ_CHARBLOCK0_IDX][tile_index],
            &card_tiles[CARD_SPRITE_LUT[card->suit][card->rank] * TILE_SIZE]
        );
        if (card->debuffed)
        {
            card_tiles_apply_debuffed_colors(
                (u32*)&tile_mem[TILE_MEM_OBJ_CHARBLOCK0_IDX][tile_index],
                TILE_SIZE * CARD_SPRITE_TILES
            );
        }
    }

    Sprite* sprite = sprite_new(
        ATTR0_SQUARE | ATTR0_4BPP | ATTR0_AFF,
        ATTR1_SIZE_32,
        tile_index,
        pb,
        sprite_get_starting_layer(CARD_SPRITE) + layer
    );
    sprite_object_set_sprite((SpriteObject*)card_object, sprite);
}

void card_object_set_sprite_face_down(CardObject* card_object, enum DeckType deck, s16 layer)
{
    // Free the current sprite first: the new one uses the same OAM slot
    sprite_object_set_sprite((SpriteObject*)card_object, NULL);

    int tile_index = sprite_get_tid(DECK_SPRITE, layer);
    memcpy32(
        &tile_mem[TILE_MEM_OBJ_CHARBLOCK0_IDX][tile_index],
        &decks_face_down_gfxTiles[DECK_SPRITE_LUT[deck] * TILE_SIZE],
        TILE_SIZE * CARD_SPRITE_TILES
    );
    Sprite* sprite = sprite_new(
        ATTR0_SQUARE | ATTR0_4BPP | ATTR0_AFF,
        ATTR1_SIZE_32,
        tile_index,
        DECK_PB,
        sprite_get_starting_layer(DECK_SPRITE) + layer
    );
    sprite_object_set_sprite((SpriteObject*)card_object, sprite);
}

void card_copy_deck_back_tiles(void* dst_tiles)
{
    memcpy32(
        dst_tiles,
        &decks_face_down_gfxTiles[DECK_SPRITE_LUT[g_game_vars.deck] * TILE_SIZE],
        TILE_SIZE * CARD_SPRITE_TILES
    );
}

void card_object_shake(CardObject* card_object, mm_word sound_id)
{
    sprite_object_shake((SpriteObject*)card_object, sound_id);
}

void card_object_set_selected(CardObject* card_object, bool selected)
{
    if (card_object == NULL)
        return;
    card_object->selected = selected;
}

bool card_object_is_selected(CardObject* card_object)
{
    if (card_object == NULL)
        return false;
    return card_object->selected;
}

Sprite* card_object_get_sprite(CardObject* card_object)
{
    if (card_object == NULL)
        return NULL;
    return sprite_object_get_sprite((SpriteObject*)card_object);
}

int card_object_get_buy_price(Item* card_object)
{
    GBAL_RETURN_IF_NULL(card_object, UNDEFINED);
    GBAL_RETURN_IF_ASSERT_FAILS(card_object->type == ITEM_TYPE_PLAYING_CARD, UNDEFINED);

    return 1;
}

void card_object_dispose(Item** card_object_item)
{
    GBAL_RETURN_IF_NULL(card_object_item, RET_NONE);
    GBAL_RETURN_IF_NULL(*card_object_item, RET_NONE);
    GBAL_RETURN_IF_ASSERT_FAILS((*card_object_item)->type == ITEM_TYPE_PLAYING_CARD, RET_NONE);

    CardObject* card_object = (CardObject*)(*card_object_item);

    for (int i = 0; i < SHOP_CARDS_MAX; i++)
    {
        if (s_shop_cards[i] == card_object)
            s_shop_cards[i] = NULL;
    }

    // A card that was not bought is lost, the bought ones were given to the deck (card == NULL)
    if (card_object->card != NULL)
        card_destroy(&card_object->card);
    card_object_destroy(&card_object);
    *card_object_item = NULL;
}

#pragma region Shop playing cards (Magic Trick, Illusion)

Item* card_object_roll_new(enum RngSequence key)
{
    int slot = UNDEFINED;
    for (int i = 0; i < SHOP_CARDS_MAX; i++)
    {
        if (s_shop_cards[i] == NULL)
        {
            slot = i;
            break;
        }
    }
    if (slot == UNDEFINED)
        return NULL;

    u8 suit = rng_get_u32(key) % NUM_SUITS;
    u8 rank = rng_get_u32(key) % NUM_RANKS;
    Card* card = card_new(suit, rank);

    // Illusion: the cards may have an enhancement and a seal
    if (voucher_is_owned(VOUCHER_ILLUSION))
    {
        if (rng_get_u32(RNG_SEQ_SHOP_CARD_MODS) % 100 < SHOP_CARD_ENHANCEMENT_CHANCE)
        {
            card->enhancement = CARD_ENHANCEMENT_BONUS +
                                rng_get_u32(RNG_SEQ_SHOP_CARD_MODS) % (CARD_ENHANCEMENT_MAX - 1);
        }
        if (rng_get_u32(RNG_SEQ_SHOP_CARD_MODS) % 100 < SHOP_CARD_SEAL_CHANCE)
        {
            card->seal = CARD_SEAL_GOLD +
                         rng_get_u32(RNG_SEQ_SHOP_CARD_MODS) % (CARD_SEAL_MAX - 1);
        }
    }

    CardObject* card_object = card_object_new(card);
    card_object_set_sprite(card_object, SHOP_CARDS_FIRST_LAYER + slot);
    s_shop_cards[slot] = card_object;
    return (Item*)card_object;
}

const char* card_object_get_name(Item* item)
{
    GBAL_RETURN_IF_NULL(item, ITEM_NAME_UNDEFINED);
    const Card* card = ((CardObject*)item)->card;
    GBAL_RETURN_IF_NULL(card, ITEM_NAME_UNDEFINED);

    // Names must stay in one line of the description header (up to 16 characters)
    if (card->enhancement == CARD_ENHANCEMENT_STONE)
        return "Каменная карта";

    static char name[24];
    snprintf(name, sizeof(name), "%s %s", RANK_NAMES[card->rank], SUIT_NAMES[card->suit]);
    return name;
}

ItemDescSubtypeInfo card_object_get_subtype_info(Item* item)
{
    ItemDescSubtypeInfo subtype_info = ITEM_SUBTYPE_INFO_DEFAULT;
    GBAL_RETURN_IF_NULL(item, subtype_info);

    subtype_info.main_color = card_rarity_pal_gfxPal[1];
    subtype_info.shadow_color = card_rarity_pal_gfxPal[2];
    subtype_info.name_str = "Карта";
    return subtype_info;
}

bool card_object_can_acquire(Item* item)
{
    GBAL_RETURN_IF_NULL(item, false);
    return deck_get_size() < MAX_DECK_SIZE;
}

void card_object_acquire(Item* item)
{
    GBAL_RETURN_IF_NULL(item, RET_NONE);
    CardObject* card_object = (CardObject*)item;
    GBAL_RETURN_IF_NULL(card_object->card, RET_NONE);

    // The card joins the deck and the sprite flies away like a redeemed Voucher
    deck_push(card_object->card);
    card_object->card = NULL;
    display_deck_size_max();
    item_start_discard_animation(item);
}

int card_object_print_description(Item* item, Rect dest_rect)
{
    GBAL_RETURN_IF_NULL(item, 0);
    const Card* card = ((CardObject*)item)->card;
    GBAL_RETURN_IF_NULL(card, 0);

    const char* enhancement = card_get_enhancement_name(card);
    const char* seal = card_get_seal_name(card);

    char desc[120];
    snprintf(
        desc,
        sizeof(desc),
        TTE_BLACK_TAG "Карта добавляется в вашу колоду%s" TTE_YELLOW_TAG "%s%s%s",
        (enhancement != NULL || seal != NULL) ? ": " : "",
        enhancement != NULL ? enhancement : "",
        enhancement != NULL && seal != NULL ? ", " : "",
        seal != NULL ? seal : ""
    );

    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

#pragma endregion
