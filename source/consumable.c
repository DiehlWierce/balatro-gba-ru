/**
 * @file consumable.c
 *
 * @brief Consumable cards: Planets for now, see consumable.h.
 */
#include "consumable.h"

#include "game.h"
#include "game/common_ui.h"
#include "game/round.h"
#include "game_variables.h"
#include "graphic_utils.h"
#include "hand.h"
#include "joker.h"
#include "layout.h"
#include "pool.h"
#include "soundbank.h"
#include "sprite.h"
#include "util.h"

// Tiles and palettes
#include "card_rarity_pal_gfx.h"
#include "planet_gfx.h"
#include "tarot_gfx.h"

#include <stdio.h>
#include <tonc.h>

// Owned consumables panel (pixels): one card centered, two cards side by side overlapping
#define CONSUMABLES_PANEL_CENTER_X 192
#define CONSUMABLES_SPACING_X      16

#define PLANET_SUBTYPE_PAL_IDX 9 // Planet colors in card_rarity_pal_gfx.png
#define TAROT_SUBTYPE_PAL_IDX  7 // Tarot colors in card_rarity_pal_gfx.png

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

typedef struct
{
    const char* name;
    const char* desc; // With color tags, see TTE_*_TAG
} TarotInfo;

#define Y_ TTE_YELLOW_TAG
#define B_ TTE_BLACK_TAG

// clang-format off
static const TarotInfo TAROT_INFO[TAROT_TYPE_MAX] = {
    [TAROT_FOOL]           = {"Дурак",           B_ "Создаёт последнюю использованную Планету или Таро (кроме Дурака)"},
    [TAROT_MAGICIAN]       = {"Маг",             B_ "До 2 выбранных карт получают улучшение: " Y_ "Счастливая"},
    [TAROT_HIGH_PRIESTESS] = {"Жрица",           B_ "Создаёт до 2 Планет (нужны свободные слоты)"},
    [TAROT_EMPRESS]        = {"Императрица",     B_ "До 2 выбранных карт получают улучшение: " Y_ "Множитель"},
    [TAROT_EMPEROR]        = {"Император",       B_ "Создаёт до 2 карт Таро (нужны свободные слоты)"},
    [TAROT_HIEROPHANT]     = {"Иерофант",        B_ "До 2 выбранных карт получают улучшение: " Y_ "Бонусная"},
    [TAROT_LOVERS]         = {"Влюблённые",      B_ "1 выбранная карта получает улучшение: " Y_ "Дикая"},
    [TAROT_CHARIOT]        = {"Колесница",       B_ "1 выбранная карта получает улучшение: " Y_ "Стальная"},
    [TAROT_JUSTICE]        = {"Справедливость",  B_ "1 выбранная карта получает улучшение: " Y_ "Стеклянная"},
    [TAROT_HERMIT]         = {"Отшельник",       B_ "Удваивает ваши деньги (максимум " Y_ "+$20" B_ ")"},
    [TAROT_WHEEL]          = {"Колесо фортуны",  B_ "Нужны издания, пока не реализовано"},
    [TAROT_STRENGTH]       = {"Сила",            B_ "Повышает ранг до 2 выбранных карт на 1"},
    [TAROT_HANGED_MAN]     = {"Повешенный",      B_ "Уничтожает до 2 выбранных карт"},
    [TAROT_DEATH]          = {"Смерть",          B_ "Выберите 2 карты: левая станет копией правой"},
    [TAROT_TEMPERANCE]     = {"Умеренность",     B_ "Даёт деньги, равные цене продажи всех джокеров (максимум " Y_ "+$50" B_ ")"},
    [TAROT_DEVIL]          = {"Дьявол",          B_ "1 выбранная карта получает улучшение: " Y_ "Золотая"},
    [TAROT_TOWER]          = {"Башня",           B_ "1 выбранная карта получает улучшение: " Y_ "Каменная"},
    [TAROT_STAR]           = {"Звезда",          B_ "До 3 выбранных карт меняют масть на " Y_ "Бубны"},
    [TAROT_MOON]           = {"Луна",            B_ "До 3 выбранных карт меняют масть на " Y_ "Трефы"},
    [TAROT_SUN]            = {"Солнце",          B_ "До 3 выбранных карт меняют масть на " Y_ "Червы"},
    [TAROT_JUDGEMENT]      = {"Суд",             B_ "Создаёт случайного джокера (нужен свободный слот)"},
    [TAROT_WORLD]          = {"Мир",             B_ "До 3 выбранных карт меняют масть на " Y_ "Пики"},
};
// clang-format on

#undef Y_
#undef B_

static List s_owned_consumables;
// The last Planet or Tarot (except The Fool) that was used, for The Fool
static u8 s_last_used_type = CONSUMABLE_TYPE_MAX;
static u8 s_last_used_id = 0;
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

static inline bool tarot_is_valid(int tarot)
{
    return tarot >= 0 && tarot < TAROT_TYPE_MAX;
}

static inline bool is_tarot(const ConsumableObject* consumable_object)
{
    return consumable_object->consumable_type == CONSUMABLE_TYPE_TAROT &&
           tarot_is_valid(consumable_object->id);
}

void consumables_forget_last_used(void)
{
    s_last_used_type = CONSUMABLE_TYPE_MAX;
    s_last_used_id = 0;
}

void consumables_init(void)
{
    s_owned_consumables = list_init();
    consumables_forget_last_used();
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
    consumables_forget_last_used();
}

List* get_consumables_list(void)
{
    return &s_owned_consumables;
}

#ifdef DEBUG_GIVE_TAROT
// The emulator pokes a TarotType here to get that Tarot in the owned row
volatile int g_debug_give_tarot = -1;
#endif

void consumables_update_loop(const Item* skipped_item)
{
#ifdef DEBUG_GIVE_TAROT
    if (g_debug_give_tarot >= 0)
    {
        if (g_debug_give_tarot < TAROT_TYPE_MAX)
        {
            consumable_create_owned(
                CONSUMABLE_TYPE_TAROT,
                g_debug_give_tarot,
                int2fx(CONSUMABLES_PANEL_CENTER_X),
                int2fx(HELD_JOKERS_POS.y)
            );
        }
        g_debug_give_tarot = -1;
    }
#endif

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
    s_last_used_type = CONSUMABLE_TYPE_PLANET;
    s_last_used_id = planet;

    enum HandType hand_type = planet_get_hand_type(planet);
    hand_level_up(hand_type, 1);
    hand_show_level_up(hand_type);
}

// Tarots

enum TarotType tarot_roll(enum RngSequence key)
{
    int nb_rollable = 0;
    for (int tarot = 0; tarot < TAROT_TYPE_MAX; tarot++)
        nb_rollable += tarot_is_rollable(tarot);

    int pick = rng_get_u32(key) % nb_rollable;
    for (int tarot = 0; tarot < TAROT_TYPE_MAX; tarot++)
    {
        if (tarot_is_rollable(tarot) && pick-- == 0)
            return tarot;
    }

    return TAROT_TYPE_MAX;
}

/**
 * @brief Selected cards of the hand, in the array order (the leftmost card on the screen has the
 *        highest index). Only during a round, when the hand can be changed.
 *
 * @return the number of selected cards, at most TAROT_MAX_TARGETS + 1
 */
static int tarot_get_selected_cards(CardObject* out[TAROT_MAX_TARGETS + 1])
{
    if (game_get_state() != GAME_STATE_ROUND || get_hand_state() != HAND_SELECT)
        return 0;

    int nb_selected = 0;
    CardObject** hand = get_hand_array();
    for (int i = 0; i <= get_hand_top(); i++)
    {
        if (hand[i] != NULL && card_object_is_selected(hand[i]))
        {
            if (nb_selected > TAROT_MAX_TARGETS)
                return nb_selected + 1;
            out[nb_selected++] = hand[i];
        }
    }
    return nb_selected;
}

bool tarot_can_use_now(enum TarotType tarot)
{
    if (!tarot_is_valid(tarot) || !tarot_is_rollable(tarot))
        return false;

    int min, max;
    tarot_get_target_range(tarot, &min, &max);
    if (max > 0)
    {
        CardObject* selected[TAROT_MAX_TARGETS + 1];
        int nb_selected = tarot_get_selected_cards(selected);
        if (nb_selected < min || nb_selected > max)
            return false;

        // Never empty the hand: the round couldn't go on
        if (tarot == TAROT_HANGED_MAN && nb_selected >= hand_nb_held_cards())
            return false;
        return true;
    }

    if (tarot == TAROT_FOOL)
        return s_last_used_type != CONSUMABLE_TYPE_MAX;

    if (tarot == TAROT_JUDGEMENT)
        return list_get_len(get_jokers_list()) < g_game_vars.joker_slots;

    return true;
}

bool consumable_create_owned(enum ConsumableType type, int id, FIXED x, FIXED y)
{
    ConsumableObject* consumable = consumable_object_new(type, id);
    if (consumable == NULL)
        return false;

    Item* item = (Item*)consumable;
    if (!item_can_acquire(item))
    {
        // No free consumable slot
        consumable_object_destroy(&consumable);
        return false;
    }

    item->x = x;
    item->y = y;
    item->tx = x;
    item->ty = y;
    item_acquire(item);
    return true;
}

bool consumable_create_random_tarot(enum RngSequence key, FIXED x, FIXED y)
{
    if (list_get_len(&s_owned_consumables) >= g_game_vars.consumable_slots)
        return false;

    enum TarotType tarot = tarot_roll(key);
    return tarot != TAROT_TYPE_MAX && consumable_create_owned(CONSUMABLE_TYPE_TAROT, tarot, x, y);
}

/**
 * @brief Applies a Tarot. The Tarot was already taken out of the owned row, so its slot is free.
 *
 * @param x, y where the Tarot is, consumables created by it appear there
 */
static void tarot_use(enum TarotType tarot, FIXED x, FIXED y)
{
    CardObject* selected[TAROT_MAX_TARGETS + 1];
    int nb_selected = tarot_get_selected_cards(selected);
    int min, max;
    tarot_get_target_range(tarot, &min, &max);
    if (max == 0)
        nb_selected = 0;

    if (tarot == TAROT_FOOL)
    {
        consumable_create_owned(s_last_used_type, s_last_used_id, x, y);
    }
    else if (tarot == TAROT_HIGH_PRIESTESS || tarot == TAROT_EMPEROR)
    {
        for (int i = 0; i < TAROT_CREATE_MAX; i++)
        {
            if (list_get_len(&s_owned_consumables) >= g_game_vars.consumable_slots)
                break;

            if (tarot == TAROT_EMPEROR)
            {
                consumable_create_random_tarot(RNG_SEQ_TAROT_CREATE, x, y);
            }
            else
            {
                enum PlanetType planet = planet_roll(RNG_SEQ_TAROT_CREATE);
                if (planet != PLANET_TYPE_MAX)
                    consumable_create_owned(CONSUMABLE_TYPE_PLANET, planet, x, y);
            }
        }
    }
    else if (tarot == TAROT_JUDGEMENT)
    {
        Item* joker = item_roll_new(ITEM_TYPE_JOKER, RNG_SEQ_TAROT_JUDGEMENT);
        if (joker != NULL)
        {
            joker->x = x;
            joker->y = y;
            joker->tx = x;
            joker->ty = y;
            item_acquire(joker);
        }
    }
    else if (tarot == TAROT_HERMIT)
    {
        g_game_vars.money += tarot_hermit_gain(g_game_vars.money);
        display_money();
    }
    else if (tarot == TAROT_TEMPERANCE)
    {
        int total = 0;
        ListItr itr = list_itr_create(get_jokers_list());
        Item* joker;
        while ((joker = list_itr_next(&itr)))
            total += joker_object_get_sell_price(joker);

        g_game_vars.money += tarot_temperance_gain(total);
        display_money();
    }
    else if (tarot == TAROT_HANGED_MAN)
    {
        hand_destroy_selected_cards();
        round_fix_hand_selection();
    }
    else if (nb_selected > 0)
    {
        // Pointers to the cards: the hand is sorted again afterwards
        Card* cards[TAROT_MAX_TARGETS + 1];
        for (int i = 0; i < nb_selected; i++)
        {
            cards[i] = selected[i]->card;
            card_object_shake(selected[i], SFX_CARD_SELECT);
        }

        if (tarot == TAROT_DEATH)
        {
            // selected[] goes from the card at the bottom of the array (right on the screen) up
            // to the left one: the left card becomes a copy of the right card
            card_copy_from(cards[nb_selected - 1], cards[0]);
        }
        else
        {
            for (int i = 0; i < nb_selected; i++)
            {
                if (tarot == TAROT_STRENGTH)
                    card_rank_up(cards[i]);
                else if (tarot_get_suit(tarot) >= 0)
                    card_set_suit(cards[i], tarot_get_suit(tarot));
                else if (tarot_get_enhancement(tarot) != CARD_ENHANCEMENT_NONE)
                    cards[i]->enhancement = tarot_get_enhancement(tarot);
            }
        }

        hand_deselect_all_cards();
        sort_cards(); // Sorts again after rank and suit changes, and redraws the card sprites
        compute_hand_value_info();
    }

    if (tarot != TAROT_FOOL)
    {
        s_last_used_type = CONSUMABLE_TYPE_TAROT;
        s_last_used_id = tarot;
    }
}

// ConsumableObject

ConsumableObject* consumable_object_new(enum ConsumableType type, int id)
{
    bool valid = (type == CONSUMABLE_TYPE_PLANET && planet_is_valid(id)) ||
                 (type == CONSUMABLE_TYPE_TAROT && tarot_is_valid(id));
    if (!valid)
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

    // Planets and Tarots share one palette bank, see docs/ru/PALETTES.md
    memcpy16(&pal_obj_mem[PAL_ROW_LEN * CONSUMABLE_PB], planet_gfxPal, PAL_ROW_LEN);

    const unsigned int* gfx_tiles =
        (type == CONSUMABLE_TYPE_TAROT) ? tarot_gfxTiles : planet_gfxTiles;
    int tile_index = sprite_get_tid(CONSUMABLE_SPRITE, layer);
    memcpy32(
        &tile_mem[TILE_MEM_OBJ_CHARBLOCK0_IDX][tile_index],
        &gfx_tiles[id * TILE_SIZE * CONSUMABLE_SPRITE_TILES],
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

    if (!consumable_object->is_owned)
        return false;
    // Planets can always be used
    if (is_planet(consumable_object))
        return true;
    return is_tarot(consumable_object) && tarot_can_use_now(consumable_object->id);
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

    if (is_tarot(consumable_object))
        tarot_use(consumable_object->id, consumable_object->x, consumable_object->y);
    else
        planet_use(consumable_object->id);
    joker_on_consumable_used(is_tarot(consumable_object));

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
    // The Shop sells as many Tarots as Planets
    if (key == RNG_SEQ_SHOP_PLANETS && rng_get_u32(RNG_SEQ_SHOP_CONSUMABLE_TYPE) % 2 == 0)
    {
        enum TarotType tarot = tarot_roll(RNG_SEQ_SHOP_TAROT);
        if (tarot != TAROT_TYPE_MAX)
            return (Item*)consumable_object_new(CONSUMABLE_TYPE_TAROT, tarot);
    }

    enum PlanetType planet = planet_roll(key);
    if (planet == PLANET_TYPE_MAX)
        return NULL;

    return (Item*)consumable_object_new(CONSUMABLE_TYPE_PLANET, planet);
}

int consumable_object_get_buy_price(Item* item)
{
    GBAL_RETURN_IF_NULL(item, UNDEFINED);
    return ((ConsumableObject*)item)->consumable_type == CONSUMABLE_TYPE_TAROT ? TAROT_BUY_PRICE
                                                                               : PLANET_BUY_PRICE;
}

int consumable_object_get_sell_price(Item* item)
{
    GBAL_RETURN_IF_NULL(item, UNDEFINED);
    return consumable_object_get_buy_price(item) / 2;
}

const char* consumable_object_get_name(Item* item)
{
    GBAL_RETURN_IF_NULL(item, ITEM_NAME_UNDEFINED);
    ConsumableObject* consumable_object = (ConsumableObject*)item;
    if (is_tarot(consumable_object))
        return TAROT_INFO[consumable_object->id].name;
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
    else if (is_tarot((ConsumableObject*)item))
    {
        subtype_info.main_color = card_rarity_pal_gfxPal[TAROT_SUBTYPE_PAL_IDX];
        subtype_info.shadow_color = card_rarity_pal_gfxPal[TAROT_SUBTYPE_PAL_IDX + 1];
        subtype_info.name_str = "Таро";
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
    if (is_tarot(consumable_object))
    {
        return tte_printf_justified_in_rect(
            TAROT_INFO[consumable_object->id].desc,
            dest_rect,
            JUSTIFY_CENTER,
            SCREEN_LEFT,
            true
        );
    }
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
