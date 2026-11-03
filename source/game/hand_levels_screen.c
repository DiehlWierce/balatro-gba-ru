/**
 * @file hand_levels_screen.c
 *
 * @brief "Уровни рук" screen, see hand_levels_screen.h.
 *
 * While open, the sprites and the main background (UI panels) are hidden and the whole text
 * layer is used. The text layer map is saved before and restored after, so the round's texts
 * don't need to be reprinted.
 */
#include "game/hand_levels_screen.h"

#include "game.h"
#include "game_variables.h"
#include "graphic_utils.h"
#include "hand.h"

#include <stdio.h>
#include <string.h>
#include <tonc.h>

#define TEXT_LAYER_MAP_ENTRIES (32 * 32)

#define TITLE_Y  8
#define HEADER_Y 24
#define FIRST_Y  40
#define HINT_Y   144

// Hands from the strongest to the weakest, Royal Flush shares the Straight Flush level
static const enum HandType LISTED_HANDS[] = {
    FLUSH_FIVE,
    FLUSH_HOUSE,
    FIVE_OF_A_KIND,
    STRAIGHT_FLUSH,
    FOUR_OF_A_KIND,
    FULL_HOUSE,
    FLUSH,
    STRAIGHT,
    THREE_OF_A_KIND,
    TWO_PAIR,
    PAIR,
    HIGH_CARD,
};

#define SCREEN_KEYS (KEY_SELECT | KEY_B)

static EWRAM_BSS SCR_ENTRY s_saved_text_map[TEXT_LAYER_MAP_ENTRIES];
static bool s_open = false;
// After closing, the keys are swallowed until released so the round doesn't get them
static bool s_wait_keys_release = false;

bool hand_levels_screen_is_open(void)
{
    return s_open;
}

/**
 * @brief Secret hands are listed once they have been played or leveled up, like in the original
 */
static bool hand_is_listed(enum HandType hand_type)
{
    bool secret =
        hand_type == FIVE_OF_A_KIND || hand_type == FLUSH_HOUSE || hand_type == FLUSH_FIVE;
    return !secret || g_game_vars.nb_played_hands[hand_type - 1] > 0 ||
           hand_get_level(hand_type) > HAND_LEVEL_MIN;
}

static void print_screen(void)
{
    tte_printf("#{P:%d,%d; cx:0x%X000}УРОВНИ РУК", 80, TITLE_Y, TTE_YELLOW_PB);
    tte_printf("#{P:%d,%d; cx:0x%X000}РУКА     УР   ФИШ МНОЖ  СЫГР", 0, HEADER_Y, TTE_WHITE_PB);

    int y = FIRST_Y;
    for (size_t i = 0; i < sizeof(LISTED_HANDS) / sizeof(LISTED_HANDS[0]); i++)
    {
        enum HandType hand_type = LISTED_HANDS[i];
        if (!hand_is_listed(hand_type))
            continue;

        HandLevelValues values = hand_get_values(hand_type);
        u32 nb_played = g_game_vars.nb_played_hands[hand_type - 1];
        if (hand_type == STRAIGHT_FLUSH)
            nb_played += g_game_vars.nb_played_hands[ROYAL_FLUSH - 1];

        // 28 of 30 columns (the last one must not touch the screen edge): name 8, level 3,
        // chips 6 (blue), " x" and mult 6 (red), played 5
        tte_printf(
            "#{P:%d,%d; cx:0x%X000}%-8s#{cx:0x%X000}%3d#{cx:0x%X000}%6lu#{cx:0x%X000}"
            " x%-4lu#{cx:0x%X000}%5lu",
            0,
            y,
            TTE_WHITE_PB,
            get_hand_type_name(hand_type),
            TTE_YELLOW_PB,
            hand_get_level(hand_type),
            TTE_BLUE_PB,
            (unsigned long)values.chips,
            TTE_RED_PB,
            (unsigned long)values.mult,
            TTE_WHITE_PB,
            (unsigned long)nb_played
        );
        y += TILE_SIZE;
    }

    tte_printf("#{P:%d,%d; cx:0x%X000}SELECT/B: назад", 60, HINT_Y, TTE_WHITE_PB);
}

static void open_screen(void)
{
    s_open = true;

    memcpy16(s_saved_text_map, se_mem[TTE_SBB], TEXT_LAYER_MAP_ENTRIES);
    tte_erase_screen();

    REG_DISPCNT &= ~(DCNT_BG1 | DCNT_OBJ);
    print_screen();
}

static void close_screen(void)
{
    s_open = false;

    memcpy16(se_mem[TTE_SBB], s_saved_text_map, TEXT_LAYER_MAP_ENTRIES);
    REG_DISPCNT |= DCNT_BG1 | DCNT_OBJ;
}

bool hand_levels_screen_update(void)
{
    if (s_wait_keys_release)
    {
        if (key_is_down(SCREEN_KEYS))
            return true;
        s_wait_keys_release = false;
    }

    if (!s_open)
    {
        if (key_hit(KEY_SELECT) && get_hand_state() == HAND_SELECT)
        {
            open_screen();
            return true;
        }
        return false;
    }

    if (key_hit(SCREEN_KEYS))
    {
        close_screen();
        s_wait_keys_release = true;
    }

    // Don't let the closing key press reach the round this frame
    return true;
}
