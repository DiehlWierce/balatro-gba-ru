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
#include "joker.h"
#include "list.h"
#include "random.h"
#include "util.h"

#define PSYCHIC_NB_CARDS      5
#define TOOTH_MONEY_PER_CARD  1
#define HOOK_NB_DISCARDS      2
#define SERPENT_NB_DRAWS      3
#define NEEDLE_NB_HANDS       1
#define MANACLE_HAND_SIZE_MOD 1
#define WHEEL_FACE_DOWN_ODDS  7 // 1 in 7 cards

// Per-round state of the Bosses, reset when their round starts

// The Eye: bit `hand_type` set when the hand type was already played this round
static u16 s_eye_played_hands = 0;
// The Mouth: the only hand type allowed this round, NONE until the first hand
static u8 s_mouth_hand = NONE;
// The Ox: the most played hand when the round started
static u8 s_ox_hand = HIGH_CARD;
// The House: cards are drawn face down until the first hand is played or discarded
static bool s_house_first_hand = false;
// The Fish: cards drawn after a played hand are face down
static bool s_fish_after_play = false;
// Verdant Leaf: every card is debuffed until a Joker is sold
static bool s_leaf_active = false;
// Crimson Heart: the disabled Joker, chosen again after each hand
static const JokerObject* s_heart_joker = NULL;
static bool s_heart_choose = false;
// Cerulean Bell: the card that stays selected
static const Card* s_bell_card = NULL;

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
 * @brief Whether the Boss @p blind debuffs @p card.
 */
static bool boss_debuffs_card(enum BlindType blind, Card* card)
{
    switch (blind)
    {
        case BLIND_TYPE_CLUB:
            return card_has_suit(card, CLUBS);
        case BLIND_TYPE_GOAD:
            return card_has_suit(card, SPADES);
        case BLIND_TYPE_WINDOW:
            return card_has_suit(card, DIAMONDS);
        case BLIND_TYPE_HEAD:
            return card_has_suit(card, HEARTS);
        case BLIND_TYPE_PLANT:
            return card_is_face(card);
        case BLIND_TYPE_PILLAR:
            return card->played_ante == g_game_vars.ante;
        case BLIND_TYPE_LEAF:
            return s_leaf_active;
        default:
            return false;
    }
}

/**
 * @brief The Club, The Goad, The Window, The Head, The Plant, The Pillar, Verdant Leaf:
 *        debuff some cards when they are drawn.
 */
static u32 blind_effect_debuff(enum BlindEvent event, const BlindEventData* data)
{
    enum BlindType blind = g_game_vars.current_blind;

    switch (event)
    {
        case BLIND_EVENT_ON_BLIND_SELECTED:
            s_leaf_active = (blind == BLIND_TYPE_LEAF);
            break;
        case BLIND_EVENT_ON_CARD_DRAWN:
            if (data->card != NULL && boss_debuffs_card(blind, data->card))
            {
                data->card->debuffed = true;
                return BLIND_EFFECT_FLAG_TRIGGERED;
            }
            break;
        case BLIND_EVENT_ON_ROUND_END:
            s_leaf_active = false;
            break;
        default:
            break;
    }

    return BLIND_EFFECT_FLAG_NONE;
}

/**
 * @brief The House, The Wheel, The Mark, The Fish: draw some cards face down.
 *        Face down cards are revealed when played.
 */
static u32 blind_effect_face_down(enum BlindEvent event, const BlindEventData* data)
{
    bool face_down = false;

    switch (event)
    {
        case BLIND_EVENT_ON_BLIND_SELECTED:
            s_house_first_hand = true;
            s_fish_after_play = false;
            return BLIND_EFFECT_FLAG_NONE;
        case BLIND_EVENT_ON_HAND_PLAYED:
            s_house_first_hand = false;
            s_fish_after_play = true;
            return BLIND_EFFECT_FLAG_NONE;
        case BLIND_EVENT_ON_DISCARD:
            s_house_first_hand = false;
            s_fish_after_play = false;
            return BLIND_EFFECT_FLAG_NONE;
        case BLIND_EVENT_ON_CARD_DRAWN:
            break;
        default:
            return BLIND_EFFECT_FLAG_NONE;
    }

    if (data->card == NULL)
        return BLIND_EFFECT_FLAG_NONE;

    switch (g_game_vars.current_blind)
    {
        case BLIND_TYPE_HOUSE:
            face_down = s_house_first_hand;
            break;
        case BLIND_TYPE_WHEEL:
            face_down = rng_get_u32(RNG_SEQ_BOSS_WHEEL) % WHEEL_FACE_DOWN_ODDS == 0;
            break;
        case BLIND_TYPE_MARK:
            face_down = card_is_face(data->card);
            break;
        case BLIND_TYPE_FISH:
            face_down = s_fish_after_play;
            break;
        default:
            break;
    }

    data->card->face_down = face_down;
    return face_down ? BLIND_EFFECT_FLAG_TRIGGERED : BLIND_EFFECT_FLAG_NONE;
}

static void set_owned_jokers_face_down(bool face_down)
{
    ListItr itr = list_itr_create(get_jokers_list());
    JokerObject* joker_object;
    while ((joker_object = list_itr_next(&itr)))
        joker_object_set_face_down(joker_object, face_down);
}

/**
 * @brief Amber Acorn: flips the Jokers face down and shuffles them for the whole round.
 */
static u32 blind_effect_acorn(enum BlindEvent event, const BlindEventData* data)
{
    List* jokers = get_jokers_list();

    if (event == BLIND_EVENT_ON_BLIND_SELECTED)
    {
        // Each step moves a random Joker among the first `remaining` ones to the end
        for (int remaining = list_get_len(jokers); remaining > 1; remaining--)
        {
            int idx = rng_get_u32(RNG_SEQ_BOSS_ACORN) % remaining;
            void* joker_object = list_get_at_idx(jokers, idx);
            list_remove_at_idx(jokers, idx);
            list_push_back(jokers, joker_object);
        }
        set_owned_jokers_face_down(true);
        return BLIND_EFFECT_FLAG_TRIGGERED;
    }

    if (event == BLIND_EVENT_ON_ROUND_END)
        set_owned_jokers_face_down(false);

    return BLIND_EFFECT_FLAG_NONE;
}

static void heart_set_disabled_joker(const JokerObject* joker_object)
{
    if (s_heart_joker != NULL)
        joker_object_set_disabled_look((JokerObject*)s_heart_joker, false);

    s_heart_joker = joker_object;

    if (s_heart_joker != NULL)
        joker_object_set_disabled_look((JokerObject*)s_heart_joker, true);
}

/**
 * @brief Crimson Heart: a random Joker is disabled, chosen again after each hand.
 *        The choice is made in blind_effects_on_draw_complete().
 */
static u32 blind_effect_heart(enum BlindEvent event, const BlindEventData* data)
{
    switch (event)
    {
        case BLIND_EVENT_ON_BLIND_SELECTED:
        case BLIND_EVENT_ON_HAND_PLAYED:
            s_heart_choose = true;
            break;
        case BLIND_EVENT_ON_ROUND_END:
            heart_set_disabled_joker(NULL);
            s_heart_choose = false;
            break;
        default:
            break;
    }

    return BLIND_EFFECT_FLAG_NONE;
}

/**
 * @brief Cerulean Bell: a card is always selected, chosen in blind_effects_on_draw_complete().
 */
static u32 blind_effect_bell(enum BlindEvent event, const BlindEventData* data)
{
    if (event == BLIND_EVENT_ON_BLIND_SELECTED || event == BLIND_EVENT_ON_ROUND_END)
        s_bell_card = NULL;

    return BLIND_EFFECT_FLAG_NONE;
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
    [BLIND_TYPE_HOUSE]   = blind_effect_face_down,
    [BLIND_TYPE_WALL]    = NULL,
    [BLIND_TYPE_WHEEL]   = blind_effect_face_down,
    [BLIND_TYPE_ARM]     = blind_effect_arm,
    [BLIND_TYPE_CLUB]    = blind_effect_debuff,
    [BLIND_TYPE_FISH]    = blind_effect_face_down,
    [BLIND_TYPE_PSYCHIC] = blind_effect_psychic,
    [BLIND_TYPE_GOAD]    = blind_effect_debuff,
    [BLIND_TYPE_WATER]   = blind_effect_water,
    [BLIND_TYPE_WINDOW]  = blind_effect_debuff,
    [BLIND_TYPE_MANACLE] = blind_effect_manacle,
    [BLIND_TYPE_EYE]     = blind_effect_eye,
    [BLIND_TYPE_MOUTH]   = blind_effect_mouth,
    [BLIND_TYPE_PLANT]   = blind_effect_debuff,
    [BLIND_TYPE_SERPENT] = NULL,
    [BLIND_TYPE_PILLAR]  = blind_effect_debuff,
    [BLIND_TYPE_NEEDLE]  = blind_effect_needle,
    [BLIND_TYPE_HEAD]    = blind_effect_debuff,
    [BLIND_TYPE_TOOTH]   = blind_effect_tooth,
    [BLIND_TYPE_FLINT]   = blind_effect_flint,
    [BLIND_TYPE_MARK]    = blind_effect_face_down,

    // Showdown Blinds
    [BLIND_TYPE_ACORN]   = blind_effect_acorn,
    [BLIND_TYPE_LEAF]    = blind_effect_debuff,
    [BLIND_TYPE_VESSEL]  = NULL,
    [BLIND_TYPE_HEART]   = blind_effect_heart,
    [BLIND_TYPE_BELL]    = blind_effect_bell,
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
    [BLIND_TYPE_HOUSE] = {"Дом", {"Первая", "рука", "закрыта", NULL}, "РУКА ?"},
    [BLIND_TYPE_WHEEL] = {"Колесо", {"1 из 7", "карт", "закрыты", NULL}, "1/7 ?"},
    [BLIND_TYPE_CLUB] = {"Трефа", {"Все", "трефы", "откл.", NULL}, "-ТРЕФЫ"},
    [BLIND_TYPE_FISH] = {"Рыба", {"После", "руки", "карты", "закрыты"}, "ДОБОР ?"},
    [BLIND_TYPE_GOAD] = {"Стрекало", {"Все", "пики", "откл.", NULL}, "-ПИКИ"},
    [BLIND_TYPE_WINDOW] = {"Окно", {"Все", "бубны", "откл.", NULL}, "-БУБНЫ"},
    [BLIND_TYPE_PLANT] = {"Растение", {"Все", "картинки", "откл.", NULL}, "-КАРТИН"},
    [BLIND_TYPE_PILLAR] = {"Столп", {"Сыгран.", "в анте", "карты", "откл."}, "-СЫГРАН"},
    [BLIND_TYPE_HEAD] = {"Голова", {"Все", "червы", "откл.", NULL}, "-ЧЕРВЫ"},
    [BLIND_TYPE_MARK] = {"Метка", {"Картинки", "закрыты", NULL, NULL}, "КАРТИН?"},
    [BLIND_TYPE_ACORN] = {"Жёлудь", {"Джокеры", "закрыты", "и", "смешаны"}, "ДЖОК ?"},
    [BLIND_TYPE_LEAF] = {"Лист", {"Карты", "откл. до", "продажи", "джокера"}, "ВСЕ ОТК"},
    [BLIND_TYPE_HEART] = {"Сердце", {"Каждую", "руку", "джокер", "откл."}, "-1 ДЖОК"},
    [BLIND_TYPE_BELL] = {"Колокол", {"Одна", "карта", "всегда", "выбрана"}, "ВЫБОР 1"},
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

    if (!blind_effects_is_implemented(blind) || (int)event < 0 || event >= BLIND_EVENT_MAX ||
        joker_boss_is_disabled())
        return BLIND_EFFECT_FLAG_NONE;

    static const BlindEventData NO_DATA = {.card = NULL, .hand_type = NONE, .nb_cards = 0};

    return BLIND_EFFECT_FUNCS[blind](event, (data != NULL) ? data : &NO_DATA);
}

int blind_effects_get_forced_discards(void)
{
    return (g_game_vars.current_blind == BLIND_TYPE_HOOK && !joker_boss_is_disabled())
               ? HOOK_NB_DISCARDS
               : 0;
}

int blind_effects_get_forced_draws(void)
{
    return (g_game_vars.current_blind == BLIND_TYPE_SERPENT && !joker_boss_is_disabled())
               ? SERPENT_NB_DRAWS
               : 0;
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

static void bell_force_card(void)
{
    CardObject** hand = get_hand_array();
    int nb_held = hand_nb_held_cards();

    // Keep the forced card if it's still in hand
    for (int i = 0; i < nb_held; i++)
    {
        if (hand[i] != NULL && hand[i]->card == s_bell_card)
        {
            if (!card_object_is_selected(hand[i]))
                hand_select_card(i);
            return;
        }
    }

    // Otherwise force a random card that isn't selected yet
    int nb_free = 0;
    for (int i = 0; i < nb_held; i++)
    {
        if (hand[i] != NULL && !card_object_is_selected(hand[i]))
            nb_free++;
    }
    if (nb_free == 0 || hand_get_nb_selected_cards() >= MAX_SELECTION_SIZE)
        return;

    int pick = rng_get_u32(RNG_SEQ_BOSS_BELL) % nb_free;
    for (int i = 0; i < nb_held; i++)
    {
        if (hand[i] == NULL || card_object_is_selected(hand[i]))
            continue;
        if (pick-- == 0)
        {
            hand_select_card(i);
            s_bell_card = hand[i]->card;
            return;
        }
    }
}

void blind_effects_on_draw_complete(void)
{
    enum BlindType blind = g_game_vars.current_blind;

    if (joker_boss_is_disabled()) // Chicot
        return;

    if (blind == BLIND_TYPE_HEART && s_heart_choose)
    {
        s_heart_choose = false;
        List* jokers = get_jokers_list();
        int nb_jokers = list_get_len(jokers);
        const JokerObject* disabled = NULL;
        if (nb_jokers > 0)
            disabled = list_get_at_idx(jokers, rng_get_u32(RNG_SEQ_BOSS_HEART) % nb_jokers);
        heart_set_disabled_joker(disabled);
    }
    else if (blind == BLIND_TYPE_BELL)
    {
        bell_force_card();
    }
}

bool blind_effects_is_card_forced(const CardObject* card_object)
{
    return g_game_vars.current_blind == BLIND_TYPE_BELL && !joker_boss_is_disabled() &&
           card_object != NULL &&
           s_bell_card != NULL && card_object->card == s_bell_card;
}

bool blind_effects_is_joker_disabled(const JokerObject* joker_object)
{
    return g_game_vars.current_blind == BLIND_TYPE_HEART && !joker_boss_is_disabled() &&
           joker_object != NULL &&
           joker_object == s_heart_joker;
}

void blind_effects_on_joker_removed(const JokerObject* joker_object, bool sold)
{
    if (joker_object != NULL && joker_object == s_heart_joker)
        s_heart_joker = NULL; // No need to restore its look, the Joker is going away

    // Verdant Leaf: selling a Joker removes the debuff of every card
    if (sold && s_leaf_active && g_game_vars.current_blind == BLIND_TYPE_LEAF)
    {
        s_leaf_active = false;
        CardObject** hand = get_hand_array();
        for (int i = 0; i <= get_hand_top(); i++)
        {
            if (hand[i] != NULL)
                hand[i]->card->debuffed = false;
        }
        reorder_card_sprites_layers();
    }
}

void blind_effects_reset_card(Card* card)
{
    if (card == NULL)
        return;
    card->debuffed = false;
    card->face_down = false;
}
