/**
 * @file game_variables.h
 *
 * @brief Host stand-in for include/game_variables.h: the run variables Vouchers change.
 */
#ifndef TEST_GAME_VARIABLES_H
#define TEST_GAME_VARIABLES_H

#include "tonc.h"

typedef struct
{
    u32 owned_vouchers;
    s32 ante;
    s32 shop_slots;
    s32 consumable_slots;
    s32 interest_cap;
    s32 joker_slots;
    s32 hands_per_round;
    s32 hands;
    s32 discards_per_round;
    s32 discards;
    s32 base_hand_size;
    s32 hand_size;
} GameVariables;

extern GameVariables g_game_vars;

#endif // TEST_GAME_VARIABLES_H
