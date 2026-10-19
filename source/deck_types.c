/**
 * @file deck_types.c
 *
 * @brief Implementation of functions related to the different types of Decks.
 */
#include "deck_types.h"

#include "game.h"

#include <tonc.h>

#define TTE_COLOR_TEXT_FORMAT "#{cx:0x%X000}%s"

/**
 * @brief List of DeckType names, all formatted to be centered in a string of 13 characters.
 */
static const char DECK_NAMES[DECK_TYPE_MAX][DECK_NAME_LENGTH] = {
    "   Красная   ",
    "    Синяя    ",
    "   Желтая    ",
    "   Зелёная   ",
    "   Чёрная    ",
    "  Расписная  "
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

#define SEGS(...) (const DeckDescSegment[]){__VA_ARGS__, {0, NULL}}

static const DeckDescSegment* const DECK_DESCRIPTIONS[DECK_TYPE_MAX][5] = {
    {
        NULL,
        SEGS({TTE_RED_PB, "+1 "}, {TTE_BLACK_PB, "сброс"}),
        SEGS({TTE_BLACK_PB, "каждый раунд"}),
        NULL,
        NULL,
    },
    {
        NULL,
        SEGS({TTE_BLUE_PB, "+1 "}, {TTE_BLACK_PB, "рука"}),
        SEGS({TTE_BLACK_PB, "каждый раунд"}),
        NULL,
        NULL,
    },
    {
        NULL,
        SEGS({TTE_BLACK_PB, "Старт с"}),
        SEGS({TTE_YELLOW_PB, "лишними $10"}),
        NULL,
        NULL,
    },
    {
        SEGS({TTE_BLACK_PB, "Конец раунда:"}),
        NULL,
        SEGS({TTE_BLACK_PB, "-"}, {TTE_YELLOW_PB, "$2 "}, {TTE_BLACK_PB, "за "}, {TTE_RED_PB, "руку"}),
        SEGS({TTE_BLACK_PB, "-"}, {TTE_YELLOW_PB, "$1 "}, {TTE_BLACK_PB, "за "}, {TTE_BLUE_PB, "сброс"}),
        SEGS({TTE_BLACK_PB, "-Нет "}, {TTE_YELLOW_PB, "процента"}),
    },
    {
        SEGS({TTE_YELLOW_PB, "+1 "}, {TTE_BLACK_PB, "слот Джок."}),
        NULL,
        SEGS({TTE_BLUE_PB, "-1 "}, {TTE_BLACK_PB, "рука"}),
        SEGS({TTE_BLACK_PB, "каждый раунд"}),
        NULL,
    },
    {
        NULL,
        SEGS({TTE_YELLOW_PB, "+2 "}, {TTE_BLACK_PB, "к размеру"}),
        SEGS({TTE_BLACK_PB, "руки"}),
        SEGS({TTE_RED_PB, "-1 "}, {TTE_BLACK_PB, "слот Джок."}),
        NULL,
    },
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
