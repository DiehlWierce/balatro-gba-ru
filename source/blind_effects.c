/**
 * @file blind_effects.c
 *
 * @brief Boss Blind effects, see blind_effects.h.
 *
 * The score requirement of The Wall and Violet Vessel is part of the Blind data in blind.c.
 */
#include "blind_effects.h"

#include "game.h"
#include "game_variables.h"
#include "hand.h"
#include "util.h"

#define PSYCHIC_NB_CARDS      5
#define TOOTH_MONEY_PER_CARD  1
#define HOOK_NB_DISCARDS      2
#define SERPENT_NB_DRAWS      3
#define NEEDLE_NB_HANDS       1
#define MANACLE_HAND_SIZE_MOD 1

// Per-round state of the Bosses, reset when their round starts

// The Eye: bit `hand_type` set when the hand type was already played this round
static u16 s_eye_played_hands = 0;
// The Mouth: the only hand type allowed this round, NONE until the first hand
static u8 s_mouth_hand = NONE;
// The Ox: the most played hand when the round started
static u8 s_ox_hand = HIGH_CARD;

/**
 * @brief The most played poker hand of the run, High Card if none was played.
 *        Ties go to the lowest hand, like the Game Over screen.
 */
static enum HandType most_played_hand(void)
{
    enum HandType most_played = HIGH_CARD;
    for (enum HandType hand_type = PAIR; hand_type <= HAND_TYPE_MAX; hand_type++)
    {
        if (g_game_vars.nb_played_hands[hand_type - 1] >
            g_game_vars.nb_played_hands[most_played - 1])
            most_played = hand_type;
    }
    return most_played;
}

static u32 blind_effect_needle(enum BlindEvent event, const BlindEventData* data)
{
    if (event != BLIND_EVENT_ON_BLIND_SELECTED)
        return BLIND_EFFECT_FLAG_NONE;

    g_game_vars.hands = NEEDLE_NB_HANDS;
    display_hands();
    return BLIND_EFFECT_FLAG_TRIGGERED;
}

static u32 blind_effect_water(enum BlindEvent event, const BlindEventData* data)
{
    if (event != BLIND_EVENT_ON_BLIND_SELECTED)
        return BLIND_EFFECT_FLAG_NONE;

    g_game_vars.discards = 0;
    display_discards();
    return BLIND_EFFECT_FLAG_TRIGGERED;
}

static u32 blind_effect_manacle(enum BlindEvent event, const BlindEventData* data)
{
    // The hand size is restored from g_game_vars.base_hand_size at the end of the round
    if (event != BLIND_EVENT_ON_BLIND_SELECTED || g_game_vars.hand_size <= 1)
        return BLIND_EFFECT_FLAG_NONE;

    g_game_vars.hand_size -= MANACLE_HAND_SIZE_MOD;
    return BLIND_EFFECT_FLAG_TRIGGERED;
}

static u32 blind_effect_psychic(enum BlindEvent event, const BlindEventData* data)
{
    if (event == BLIND_EVENT_ON_HAND_PLAYED && data->nb_cards != PSYCHIC_NB_CARDS)
        return BLIND_EFFECT_FLAG_TRIGGERED | BLIND_EFFECT_FLAG_NO_SCORE;

    return BLIND_EFFECT_FLAG_NONE;
}

static u32 blind_effect_eye(enum BlindEvent event, const BlindEventData* data)
{
    if (event == BLIND_EVENT_ON_BLIND_SELECTED)
    {
        s_eye_played_hands = 0;
    }
    else if (event == BLIND_EVENT_ON_HAND_PLAYED)
    {
        u16 hand_bit = 1 << data->hand_type;
        if (s_eye_played_hands & hand_bit)
            return BLIND_EFFECT_FLAG_TRIGGERED | BLIND_EFFECT_FLAG_NO_SCORE;

        s_eye_played_hands |= hand_bit;
    }

    return BLIND_EFFECT_FLAG_NONE;
}

static u32 blind_effect_mouth(enum BlindEvent event, const BlindEventData* data)
{
    if (event == BLIND_EVENT_ON_BLIND_SELECTED)
    {
        s_mouth_hand = NONE;
    }
    else if (event == BLIND_EVENT_ON_HAND_PLAYED)
    {
        if (s_mouth_hand == NONE)
            s_mouth_hand = data->hand_type;
        else if (s_mouth_hand != data->hand_type)
            return BLIND_EFFECT_FLAG_TRIGGERED | BLIND_EFFECT_FLAG_NO_SCORE;
    }

    return BLIND_EFFECT_FLAG_NONE;
}

static u32 blind_effect_tooth(enum BlindEvent event, const BlindEventData* data)
{
    if (event != BLIND_EVENT_ON_HAND_PLAYED || data->nb_cards <= 0)
        return BLIND_EFFECT_FLAG_NONE;

    g_game_vars.money -= data->nb_cards * TOOTH_MONEY_PER_CARD;
    display_money();
    return BLIND_EFFECT_FLAG_TRIGGERED;
}

static u32 blind_effect_flint(enum BlindEvent event, const BlindEventData* data)
{
    if (event != BLIND_EVENT_ON_HAND_PLAYED)
        return BLIND_EFFECT_FLAG_NONE;

    // Halved and rounded like in Balatro, the mult stays at least 1
    g_game_vars.chips = (g_game_vars.chips + 1) / 2;
    g_game_vars.mult = (g_game_vars.mult + 1) / 2;
    if (g_game_vars.mult < 1)
        g_game_vars.mult = 1;

    display_chips();
    display_mult();
    return BLIND_EFFECT_FLAG_TRIGGERED;
}

static u32 blind_effect_ox(enum BlindEvent event, const BlindEventData* data)
{
    if (event == BLIND_EVENT_ON_BLIND_SELECTED)
    {
        s_ox_hand = most_played_hand();
    }
    else if (event == BLIND_EVENT_ON_HAND_PLAYED && data->hand_type == s_ox_hand)
    {
        g_game_vars.money = 0;
        display_money();
        return BLIND_EFFECT_FLAG_TRIGGERED;
    }

    return BLIND_EFFECT_FLAG_NONE;
}

static u32 blind_effect_arm(enum BlindEvent event, const BlindEventData* data)
{
    if (event != BLIND_EVENT_ON_HAND_PLAYED || data->hand_type <= NONE ||
        data->hand_type > HAND_TYPE_MAX)
        return BLIND_EFFECT_FLAG_NONE;

    u16* level = &g_game_vars.hand_levels[data->hand_type - 1];
    if (*level <= DEFAULT_HAND_LEVEL)
        return BLIND_EFFECT_FLAG_NONE;

    (*level)--;
    return BLIND_EFFECT_FLAG_TRIGGERED;
}

/**
 * @brief Handler of every Blind, NULL means no effect.
 *
 * The Hook and The Serpent act through blind_effects_get_forced_discards() and
 * blind_effects_get_forced_draws(), The Wall and Violet Vessel through their score requirement.
 */
// clang-format off
static const BlindEffectFunc BLIND_EFFECT_FUNCS[BLIND_TYPE_MAX] = {
    [BLIND_TYPE_SMALL]   = NULL,
    [BLIND_TYPE_BIG]     = NULL,

    // Boss Blinds
    [BLIND_TYPE_HOOK]    = NULL,
    [BLIND_TYPE_OX]      = blind_effect_ox,
    [BLIND_TYPE_HOUSE]   = NULL,
    [BLIND_TYPE_WALL]    = NULL,
    [BLIND_TYPE_WHEEL]   = NULL,
    [BLIND_TYPE_ARM]     = blind_effect_arm,
    [BLIND_TYPE_CLUB]    = NULL,
    [BLIND_TYPE_FISH]    = NULL,
    [BLIND_TYPE_PSYCHIC] = blind_effect_psychic,
    [BLIND_TYPE_GOAD]    = NULL,
    [BLIND_TYPE_WATER]   = blind_effect_water,
    [BLIND_TYPE_WINDOW]  = NULL,
    [BLIND_TYPE_MANACLE] = blind_effect_manacle,
    [BLIND_TYPE_EYE]     = blind_effect_eye,
    [BLIND_TYPE_MOUTH]   = blind_effect_mouth,
    [BLIND_TYPE_PLANT]   = NULL,
    [BLIND_TYPE_SERPENT] = NULL,
    [BLIND_TYPE_PILLAR]  = NULL,
    [BLIND_TYPE_NEEDLE]  = blind_effect_needle,
    [BLIND_TYPE_HEAD]    = NULL,
    [BLIND_TYPE_TOOTH]   = blind_effect_tooth,
    [BLIND_TYPE_FLINT]   = blind_effect_flint,
    [BLIND_TYPE_MARK]    = NULL,

    // Showdown Blinds
    [BLIND_TYPE_ACORN]   = NULL,
    [BLIND_TYPE_LEAF]    = NULL,
    [BLIND_TYPE_VESSEL]  = NULL,
    [BLIND_TYPE_HEART]   = NULL,
    [BLIND_TYPE_BELL]    = NULL,
};
// clang-format on

typedef struct BlindCaption
{
    const char* name;
    const char* lines[BLIND_CAPTION_MAX_LINES];
    const char* short_caption;
} BlindCaption;

// Marks the line replaced by the name of the most played hand (The Ox)
#define CAPTION_OX_HAND ""

/**
 * @brief Russian name and rule of each Boss with an implemented effect.
 *        Lines are at most BLIND_CAPTION_LINE_LEN characters, short captions
 *        BLIND_SHORT_CAPTION_LEN characters (one byte per character in CP1251).
 */
// clang-format off
static const BlindCaption BLIND_CAPTIONS[BLIND_TYPE_MAX] = {
    [BLIND_TYPE_HOOK]    = {"Крюк",    {"После",   "руки",    "сброс 2", "карт"   }, "СБРОС 2"},
    [BLIND_TYPE_OX]      = {"Бык",     {"Сыграть", CAPTION_OX_HAND, "деньги",  "= $0"   }, NULL     },
    [BLIND_TYPE_WALL]    = {"Стена",   {"Огромный","блайнд",  "цель x4", NULL     }, "ЦЕЛЬ X4"},
    [BLIND_TYPE_ARM]     = {"Длань",   {"Уровень", "руки -1", NULL,      NULL     }, "УРОВ -1"},
    [BLIND_TYPE_PSYCHIC] = {"Медиум",  {"Играть",  "ровно",   "5 карт",  NULL     }, "РОВНО 5"},
    [BLIND_TYPE_WATER]   = {"Вода",    {"Без",     "сбросов", NULL,      NULL     }, "0 СБРОС"},
    [BLIND_TYPE_MANACLE] = {"Кандалы", {"-1 карта","в руке",  NULL,      NULL     }, "-1 РАЗМ"},
    [BLIND_TYPE_EYE]     = {"Глаз",    {"Каждая",  "комбин.", "только",  "1 раз"  }, "НЕ ПОВТ"},
    [BLIND_TYPE_MOUTH]   = {"Рот",     {"Только",  "одна",    "комбин.", "в раунд"}, "1 КОМБ."},
    [BLIND_TYPE_SERPENT] = {"Змей",    {"Добор",   "всегда",  "3 карты", NULL     }, "ДОБОР 3"},
    [BLIND_TYPE_NEEDLE]  = {"Игла",    {"Только",  "1 рука",  NULL,      NULL     }, "1 РУКА" },
    [BLIND_TYPE_TOOTH]   = {"Зуб",     {"-$1 за",  "каждую",  "карту",   NULL     }, "ЦЕНА $1"},
    [BLIND_TYPE_FLINT]   = {"Кремень", {"Базовые", "фишки и", "множ.",   "пополам"}, "БАЗА /2"},
    [BLIND_TYPE_VESSEL]  = {"Сосуд",   {"Очень",   "огромный","блайнд",  "цель x6"}, "ЦЕЛЬ X6"},
};
// clang-format on

static inline bool blind_type_is_valid(enum BlindType blind)
{
    return (int)blind >= 0 && blind < BLIND_TYPE_MAX;
}

bool blind_effects_is_implemented(enum BlindType blind)
{
    return blind_type_is_valid(blind) && BLIND_EFFECT_FUNCS[blind] != NULL;
}

u32 blind_effects_trigger(enum BlindEvent event, const BlindEventData* data)
{
    enum BlindType blind = g_game_vars.current_blind;

    if (!blind_effects_is_implemented(blind) || (int)event < 0 || event >= BLIND_EVENT_MAX)
        return BLIND_EFFECT_FLAG_NONE;

    static const BlindEventData NO_DATA = {.card = NULL, .hand_type = NONE, .nb_cards = 0};

    return BLIND_EFFECT_FUNCS[blind](event, (data != NULL) ? data : &NO_DATA);
}

int blind_effects_get_forced_discards(void)
{
    return (g_game_vars.current_blind == BLIND_TYPE_HOOK) ? HOOK_NB_DISCARDS : 0;
}

int blind_effects_get_forced_draws(void)
{
    return (g_game_vars.current_blind == BLIND_TYPE_SERPENT) ? SERPENT_NB_DRAWS : 0;
}

const char* blind_effects_get_name(enum BlindType blind)
{
    return blind_type_is_valid(blind) ? BLIND_CAPTIONS[blind].name : NULL;
}

const char* blind_effects_get_caption_line(enum BlindType blind, int line)
{
    if (!blind_type_is_valid(blind) || line < 0 || line >= BLIND_CAPTION_MAX_LINES)
        return NULL;

    const char* text = BLIND_CAPTIONS[blind].lines[line];
    if (blind == BLIND_TYPE_OX && text != NULL && text[0] == '\0')
        return get_hand_type_name(most_played_hand());

    return text;
}

const char* blind_effects_get_short_caption(enum BlindType blind)
{
    if (!blind_type_is_valid(blind))
        return NULL;

    // The Ox shows the hand that sets money to $0, chosen when its round started
    if (blind == BLIND_TYPE_OX)
        return get_hand_type_name(s_ox_hand);

    return BLIND_CAPTIONS[blind].short_caption;
}
