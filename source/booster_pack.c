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
#include "joker_batch6.h"
#include "game_variables.h"
#include "graphic_utils.h"
#include "item.h"
#include "pool.h"
#include "random.h"
#include "spectral_rules.h"
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
        {
            deck_push(card_object->card);
            joker_on_card_added(); // Hologram
        }
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

static Item* spectral_pack_roll_choice(int idx);

/**
 * @brief The card of a choice can't be rolled again while the pack is open (without Showman).
 */
static Item* pack_lock(Item* choice)
{
    if (choice != NULL)
        consumable_lock_unique((ConsumableObject*)choice);
    return choice;
}

static Item* arcana_pack_roll_choice(int idx)
{
    // Omen Globe: a choice may be a Spectral card
    if (voucher_is_owned(VOUCHER_OMEN_GLOBE) &&
        pack_choice_is_spectral(true, rng_get_u32(RNG_SEQ_PACK_OMEN_GLOBE)))
        return spectral_pack_roll_choice(idx);

    // The Soul may replace a choice, 0.3%
    if (spectral_pack_rolls_soul(rng_get_u32(RNG_SEQ_PACK_SOUL)))
        return pack_lock((Item*)consumable_object_new(CONSUMABLE_TYPE_SPECTRAL, SPECTRAL_SOUL));

    enum TarotType tarot = tarot_roll(RNG_SEQ_PACK_TAROT);
    if (tarot == TAROT_TYPE_MAX)
        return NULL;
    return pack_lock((Item*)consumable_object_new(CONSUMABLE_TYPE_TAROT, tarot));
}

static bool consumable_pack_can_pick(Item* choice)
{
    return item_can_acquire(choice);
}

static void arcana_pack_pick(Item** choice)
{
    ConsumableObject* consumable = (ConsumableObject*)(*choice);

    // The Tarot goes to the consumables row where it is, to be used later on the cards of a round
    // (the Shop doesn't let a choice be taken without a free slot)
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

#pragma region Celestial and Spectral Packs

static Item* celestial_pack_roll_choice(int idx)
{
    enum PlanetType planet = PLANET_TYPE_MAX;

    // Telescope: the first choice is the Planet of the most played poker hand
    if (idx == 0 && voucher_is_owned(VOUCHER_TELESCOPE))
        planet = planet_for_hand(most_played_hand(g_game_vars.nb_played_hands));

    // Black Hole may replace a choice, 0.3%
    if (planet == PLANET_TYPE_MAX && spectral_pack_rolls_black_hole(rng_get_u32(RNG_SEQ_PACK_SPECTRAL)))
        return pack_lock(
            (Item*)consumable_object_new(CONSUMABLE_TYPE_SPECTRAL, SPECTRAL_BLACK_HOLE)
        );

    if (planet == PLANET_TYPE_MAX)
        planet = planet_roll(RNG_SEQ_PACK_PLANET);
    if (planet == PLANET_TYPE_MAX)
        return NULL;

    return pack_lock((Item*)consumable_object_new(CONSUMABLE_TYPE_PLANET, planet));
}

static Item* spectral_pack_roll_choice(int idx)
{
    (void)idx;
    enum SpectralType spectral;
    if (spectral_pack_rolls_black_hole(rng_get_u32(RNG_SEQ_PACK_SPECTRAL)))
        spectral = SPECTRAL_BLACK_HOLE;
    else if (spectral_pack_rolls_soul(rng_get_u32(RNG_SEQ_PACK_SOUL)))
        spectral = SPECTRAL_SOUL;
    else
        spectral = spectral_roll(RNG_SEQ_PACK_SPECTRAL);
    if (spectral == SPECTRAL_TYPE_MAX)
        return NULL;

    return pack_lock((Item*)consumable_object_new(CONSUMABLE_TYPE_SPECTRAL, spectral));
}

#pragma endregion

#pragma region Buffoon Pack

static Item* buffoon_pack_roll_choice(int idx)
{
    (void)idx;
    // Same roll as the Shop: rarity, edition and stickers; the Joker id is locked until it is
    // taken or disposed of (without Showman)
    return item_roll_new(ITEM_TYPE_JOKER, RNG_SEQ_PACK_CONTENTS);
}

static void buffoon_pack_pick(Item** choice)
{
    // The Shop doesn't let a choice be taken without a free Joker slot
    if (item_can_acquire(*choice))
        item_acquire(*choice);
    else
        item_dispose(choice);

    *choice = NULL;
}

static bool buffoon_pack_can_pick(Item* choice)
{
    return item_can_acquire(choice);
}

static void buffoon_pack_discard(Item** choice)
{
    item_dispose(choice); // Sets *choice to NULL, the Joker can be rolled again
}

#pragma endregion

// Weights are in hundredths: Standard, Arcana and Celestial 4 / 2 / 0.5, Buffoon 1.2 / 0.6 / 0.15,
// Spectral 0.6 / 0.3 / 0.07 for Normal / Jumbo / Mega
#define PACK_STD(_name, _size, _price, _choices, _picks, _weight)         \
    {                                                                       \
        .name = _name, .price = _price, .nb_choices = _choices,             \
        .nb_picks = _picks, .weight = _weight, .kind = PACK_KIND_STANDARD,  \
        .size = _size,                                                      \
        .desc_what = "игральных карт и добавьте в колоду",                  \
        .roll_choice = standard_pack_roll_choice,                           \
        .pick = standard_pack_pick, .can_pick = NULL,                       \
        .discard = standard_pack_discard,                                   \
        .body_r = 41, .body_g = 173, .body_b = 255,                         \
    }
#define PACK_ARC(_name, _size, _price, _choices, _picks, _weight)           \
    {                                                                       \
        .name = _name, .price = _price, .nb_choices = _choices,             \
        .nb_picks = _picks, .weight = _weight, .kind = PACK_KIND_ARCANA,    \
        .size = _size,                                                      \
        .desc_what = "карт Таро и возьмите в слот",                         \
        .roll_choice = arcana_pack_roll_choice,                             \
        .pick = arcana_pack_pick, .can_pick = consumable_pack_can_pick,     \
        .discard = arcana_pack_discard,                                     \
        .body_r = 137, .body_g = 105, .body_b = 230,                        \
    }
#define PACK_CEL(_name, _size, _price, _choices, _picks, _weight)           \
    {                                                                       \
        .name = _name, .price = _price, .nb_choices = _choices,             \
        .nb_picks = _picks, .weight = _weight, .kind = PACK_KIND_CELESTIAL, \
        .size = _size,                                                      \
        .desc_what = "карт планет и возьмите в слот",                       \
        .roll_choice = celestial_pack_roll_choice,                          \
        .pick = arcana_pack_pick, .can_pick = consumable_pack_can_pick,     \
        .discard = arcana_pack_discard,                                     \
        .body_r = 35, .body_g = 89, .body_b = 85,                           \
    }
#define PACK_SPE(_name, _size, _price, _choices, _picks, _weight)           \
    {                                                                       \
        .name = _name, .price = _price, .nb_choices = _choices,             \
        .nb_picks = _picks, .weight = _weight, .kind = PACK_KIND_SPECTRAL,  \
        .size = _size,                                                      \
        .desc_what = "спектральных карт и возьмите в слот",                 \
        .roll_choice = spectral_pack_roll_choice,                           \
        .pick = arcana_pack_pick, .can_pick = consumable_pack_can_pick,     \
        .discard = arcana_pack_discard,                                     \
        .body_r = 60, .body_g = 70, .body_b = 170,                          \
    }
#define PACK_BUF(_name, _size, _price, _choices, _picks, _weight)           \
    {                                                                       \
        .name = _name, .price = _price, .nb_choices = _choices,             \
        .nb_picks = _picks, .weight = _weight, .kind = PACK_KIND_BUFFOON,   \
        .size = _size,                                                      \
        .desc_what = "джокеров и возьмите в ряд",                           \
        .roll_choice = buffoon_pack_roll_choice,                            \
        .pick = buffoon_pack_pick, .can_pick = buffoon_pack_can_pick,       \
        .discard = buffoon_pack_discard,                                    \
        .body_r = 240, .body_g = 52, .body_b = 100,                         \
    }

// clang-format off
static const PackInfo PACK_INFOS[PACK_TYPE_MAX] = {
    [PACK_TYPE_STANDARD]       = PACK_STD("Стандартный бустер", PACK_SIZE_NORMAL, 4, 3, 1, 400),
    [PACK_TYPE_ARCANA]         = PACK_ARC("Бустер Аркана",      PACK_SIZE_NORMAL, 4, 3, 1, 400),
    [PACK_TYPE_CELESTIAL]      = PACK_CEL("Небесный бустер",    PACK_SIZE_NORMAL, 4, 3, 1, 400),
    [PACK_TYPE_SPECTRAL]       = PACK_SPE("Спектр. бустер",     PACK_SIZE_NORMAL, 4, 2, 1, 60),
    [PACK_TYPE_BUFFOON]        = PACK_BUF("Бустер Шутов",       PACK_SIZE_NORMAL, 4, 2, 1, 120),
    [PACK_TYPE_JUMBO_STANDARD] = PACK_STD("Джамбо Стандарт",    PACK_SIZE_JUMBO,  6, 5, 1, 200),
    [PACK_TYPE_JUMBO_ARCANA]   = PACK_ARC("Джамбо Аркана",      PACK_SIZE_JUMBO,  6, 5, 1, 200),
    [PACK_TYPE_JUMBO_CELESTIAL]= PACK_CEL("Джамбо Небесный",    PACK_SIZE_JUMBO,  6, 5, 1, 200),
    [PACK_TYPE_JUMBO_SPECTRAL] = PACK_SPE("Джамбо Спектр.",     PACK_SIZE_JUMBO,  6, 4, 1, 30),
    [PACK_TYPE_JUMBO_BUFFOON]  = PACK_BUF("Джамбо Шутов",       PACK_SIZE_JUMBO,  6, 4, 1, 60),
    [PACK_TYPE_MEGA_STANDARD]  = PACK_STD("Мега Стандарт",      PACK_SIZE_MEGA,   8, 5, 2, 50),
    [PACK_TYPE_MEGA_ARCANA]    = PACK_ARC("Мега Аркана",        PACK_SIZE_MEGA,   8, 5, 2, 50),
    [PACK_TYPE_MEGA_CELESTIAL] = PACK_CEL("Мега Небесный",      PACK_SIZE_MEGA,   8, 5, 2, 50),
    [PACK_TYPE_MEGA_SPECTRAL]  = PACK_SPE("Мега Спектр.",       PACK_SIZE_MEGA,   8, 4, 2, 7),
    [PACK_TYPE_MEGA_BUFFOON]   = PACK_BUF("Мега Шутов",         PACK_SIZE_MEGA,   8, 4, 2, 15),
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
    if (info == NULL || info->roll_choice == NULL || info->pick == NULL || info->discard == NULL)
        return false;
    return BOOSTER_BUFFOON_ROLLABLE || info->kind != PACK_KIND_BUFFOON;
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

    // Size marks between the top band and the card: none for Normal, 1 for Jumbo, 2 for Mega
    for (int i = 0; i < (int)info->size; i++)
    {
        int left = 16 - (int)info->size * 2 + i * 5;
        pack_fill_rect(tiles, left, 7, left + 1, 7, white);
    }

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

#ifdef DEBUG_GIVE_TAROT
// The emulator pokes a PackType here to force the Packs of the Shop (-1: roll them normally)
volatile int g_debug_force_pack = -1;
#endif

Item* booster_pack_roll_new(enum RngSequence key)
{
#ifdef DEBUG_GIVE_TAROT
    if (g_debug_force_pack >= 0 && g_debug_force_pack < PACK_TYPE_MAX)
        return (Item*)booster_pack_new(g_debug_force_pack);
#endif

    int total_weight = 0;
    for (enum PackType type = 0; type < PACK_TYPE_MAX; type++)
    {
        if (booster_pack_is_rollable(type))
            total_weight += PACK_INFOS[type].weight;
    }
    if (total_weight == 0)
        return NULL;

    int pick = rng_get_u32(key) % total_weight;
    for (enum PackType type = 0; type < PACK_TYPE_MAX; type++)
    {
        if (!booster_pack_is_rollable(type))
            continue;
        pick -= PACK_INFOS[type].weight;
        if (pick < 0)
            return (Item*)booster_pack_new(type);
    }
    return NULL;
}

int booster_pack_get_buy_price(Item* item)
{
    GBAL_RETURN_IF_NULL(item, UNDEFINED);
    const PackInfo* info = booster_pack_get_info(((BoosterPackObject*)item)->pack_type);
    GBAL_RETURN_IF_NULL(info, UNDEFINED);
    if (info->kind == PACK_KIND_CELESTIAL)
        return joker_batch6_astronomer_price(info->price, is_joker_owned(ASTRONOMER_JOKER_ID));
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
                      "%d" TTE_BLACK_TAG " %s",
        info->nb_picks,
        info->nb_choices,
        info->desc_what
    );

    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}
