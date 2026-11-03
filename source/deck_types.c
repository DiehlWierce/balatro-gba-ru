/**
 * @file deck_types.c
 *
 * @brief Implementation of functions related to the different types of Decks.
 */
#include "deck_types.h"

#include "consumable.h"
#include "game.h"
#include "spectral_rules.h"
#include "tarot_rules.h"
#include "voucher.h"

#include <string.h>
#include <tonc.h>

#define TTE_COLOR_TEXT_FORMAT "#{cx:0x%X000}%s"

/**
 * @brief List of DeckType names, all formatted to be centered in a string of 13 characters.
 */
static const char DECK_NAMES[DECK_TYPE_MAX][DECK_NAME_LENGTH] = {
    "   Красная   ",
    "    Синяя    ",
    "   Жёлтая    ",
    "   Зелёная   ",
    "   Чёрная    ",
    "  Расписная  ",
    "  Волшебная  ",
    " Туманность  ",
    " Призрачная  ",
    " Заброшенная ",
    "  Клетчатая  ",
    "   Зодиак    ",
    " Плазменная  ",
    "  Хаотичная  "
};

void print_deck_name(enum DeckType deck, BG_POINT pos)
{
    tte_printf("#{P:%d,%d; cx:0x%X000}%s", pos.x, pos.y, TTE_WHITE_PB, DECK_NAMES[deck]);
}

/**
 * @brief Description lines of every Deck: 5 lines of 13 characters, each made of colored segments.
 */
typedef struct
{
    int color_pb;
    const char* text;
} DeckDescSegment;

#define SEGS(...)             \
    (const DeckDescSegment[]) \
    {                         \
        __VA_ARGS__,          \
        {                     \
            0, NULL           \
        }                     \
    }

static const DeckDescSegment* const DECK_DESCRIPTIONS[DECK_TYPE_MAX][5] = {
    {
     NULL, SEGS({TTE_RED_PB, "+1 "}, {TTE_BLACK_PB, "сброс"}),
     SEGS({TTE_BLACK_PB, "каждый раунд"}),
     NULL, NULL,
     },
    {
     NULL, SEGS({TTE_BLUE_PB, "+1 "}, {TTE_BLACK_PB, "рука"}),
     SEGS({TTE_BLACK_PB, "каждый раунд"}),
     NULL, NULL,
     },
    {
     NULL, SEGS({TTE_BLACK_PB, "Старт с"}),
     SEGS({TTE_YELLOW_PB, "лишними $10"}),
     NULL, NULL,
     },
    {
     SEGS({TTE_BLACK_PB, "Конец раунда:"}),
     NULL, SEGS({TTE_BLACK_PB, "-"}, {TTE_YELLOW_PB, "$2 "}, {TTE_BLACK_PB, "за "}, {TTE_BLUE_PB, "руку"}),
     SEGS({TTE_BLACK_PB, "-"}, {TTE_YELLOW_PB, "$1 "}, {TTE_BLACK_PB, "за "}, {TTE_RED_PB, "сброс"}),
     SEGS({TTE_BLACK_PB, "-Нет "}, {TTE_YELLOW_PB, "процента"}),
     },
    {
     SEGS({TTE_YELLOW_PB, "+1 "}, {TTE_BLACK_PB, "слот Джок."}),
     NULL, SEGS({TTE_BLUE_PB, "-1 "}, {TTE_BLACK_PB, "рука"}),
     SEGS({TTE_BLACK_PB, "каждый раунд"}),
     NULL, },
    {
     NULL, SEGS({TTE_YELLOW_PB, "+2 "}, {TTE_BLACK_PB, "к размеру"}),
     SEGS({TTE_BLACK_PB, "руки"}),
     SEGS({TTE_RED_PB, "-1 "}, {TTE_BLACK_PB, "слот Джок."}),
     NULL, },
    // Magic
    {
     NULL, SEGS({TTE_BLACK_PB, "Старт с"}),
     SEGS({TTE_YELLOW_PB, "Хруст. шаром"}),
     SEGS({TTE_BLACK_PB, "и 2 "}, {TTE_YELLOW_PB, "Дураками"}),
     NULL, },
    // Nebula
    {
     NULL, SEGS({TTE_BLACK_PB, "Старт с"}),
     SEGS({TTE_YELLOW_PB, "Телескопом"}),
     SEGS({TTE_RED_PB, "-1 "}, {TTE_BLACK_PB, "слот"}),
     SEGS({TTE_BLACK_PB, "расходников"}), },
    // Ghost
    {
     SEGS({TTE_BLACK_PB, "Спектр. карты"}),
     SEGS({TTE_BLACK_PB, "в лавке"}),
     NULL, SEGS({TTE_BLACK_PB, "Старт с"}),
     SEGS({TTE_YELLOW_PB, "Порчей"}), },
    // Abandoned
    {
     NULL, SEGS({TTE_BLACK_PB, "В колоде нет"}),
     SEGS({TTE_RED_PB, "картинок"}),
     SEGS({TTE_BLACK_PB, "(40 карт)"}),
     NULL, },
    // Checkered
    {
     NULL, SEGS({TTE_BLACK_PB, "26 пик и"}),
     SEGS({TTE_RED_PB, "26 червей"}),
     SEGS({TTE_BLACK_PB, "в колоде"}),
     NULL, },
    // Zodiac
    {
     SEGS({TTE_BLACK_PB, "Старт с"}),
     SEGS({TTE_YELLOW_PB, "Торг. Таро,"}),
     SEGS({TTE_YELLOW_PB, "Торг. планет"}),
     SEGS({TTE_BLACK_PB, "и "}, {TTE_YELLOW_PB, "Затоваркой"}),
     NULL, },
    // Plasma
    {
     SEGS({TTE_BLACK_PB, "Фишки и "}, {TTE_RED_PB, "множ."}),
     SEGS({TTE_BLACK_PB, "уравниваются"}),
     NULL, SEGS({TTE_BLACK_PB, "Цель блайнда"}),
     SEGS({TTE_YELLOW_PB, "X2"}), },
    // Erratic
    {
     NULL, SEGS({TTE_BLACK_PB, "Ранги и масти"}),
     SEGS({TTE_BLACK_PB, "всех карт"}),
     SEGS({TTE_YELLOW_PB, "случайные"}),
     NULL, },
};

void print_deck_description(enum DeckType deck, BG_POINT pos)
{
    for (int line = 0; line < 5; line++)
    {
        int y = pos.y + line * TILE_SIZE;
        int used = 0;
        const DeckDescSegment* seg = DECK_DESCRIPTIONS[deck][line];

        // Start from a blank line of 13 characters, then print the colored segments
        tte_printf("#{P:%d,%d}" TTE_COLOR_TEXT_FORMAT, pos.x, y, TTE_BLACK_PB, "             ");

        if (seg == NULL)
            continue;

        int total = 0;
        for (const DeckDescSegment* s = seg; s->text != NULL; s++)
            total += (int)strlen(s->text);

        int x = pos.x + ((13 - total) / 2) * TTE_CHAR_SIZE;
        for (const DeckDescSegment* s = seg; s->text != NULL; s++)
        {
            tte_printf("#{P:%d,%d}" TTE_COLOR_TEXT_FORMAT, x, y, s->color_pb, s->text);
            used = (int)strlen(s->text);
            x += used * TTE_CHAR_SIZE;
        }
    }
}

void deck_apply_start_effects(enum DeckType deck)
{
    switch (deck)
    {
        case DECK_TYPE_RED:
            g_game_vars.discards_per_round += 1;
            break;

        case DECK_TYPE_BLUE:
            g_game_vars.hands_per_round += 1;
            break;

        case DECK_TYPE_YELLOW:
            g_game_vars.money += 10;
            break;

        case DECK_TYPE_GREEN:
            g_game_vars.money_per_hand = 2;
            g_game_vars.money_per_discard = 1;
            g_game_vars.interest_cap = 0;
            break;

        case DECK_TYPE_BLACK:
            g_game_vars.joker_slots += 1;
            g_game_vars.hands_per_round -= 1;
            break;

        case DECK_TYPE_PAINTED:
            g_game_vars.base_hand_size += 2;
            g_game_vars.joker_slots -= 1;
            break;

        case DECK_TYPE_MAGIC:
            voucher_redeem(VOUCHER_CRYSTAL_BALL);
            consumable_give_at_start(CONSUMABLE_TYPE_TAROT, TAROT_FOOL);
            consumable_give_at_start(CONSUMABLE_TYPE_TAROT, TAROT_FOOL);
            break;

        case DECK_TYPE_NEBULA:
            voucher_redeem(VOUCHER_TELESCOPE);
            g_game_vars.consumable_slots -= 1;
            break;

        case DECK_TYPE_GHOST:
            consumable_give_at_start(CONSUMABLE_TYPE_SPECTRAL, SPECTRAL_HEX);
            break;

        case DECK_TYPE_ZODIAC:
            voucher_redeem(VOUCHER_TAROT_MERCHANT);
            voucher_redeem(VOUCHER_PLANET_MERCHANT);
            voucher_redeem(VOUCHER_OVERSTOCK);
            break;

        // Abandoned, Checkered and Erratic change the starting cards (deck_rules_build() in
        // game_start()), Plasma the scoring (round.c) and the Blind sizes (blind.c)
        default:
            break;
    }
}
