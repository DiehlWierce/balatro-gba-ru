/**
 * @file game_variables.h
 *
 * @brief Host stand-in for include/game_variables.h: only the round score fields the modifiers
 *        write to.
 */
#ifndef TEST_GAME_VARIABLES_H
#define TEST_GAME_VARIABLES_H

#include "tonc.h"

typedef struct
{
    u32 chips;
    u32 mult;
    s32 money;
} GameVariables;

extern GameVariables g_game_vars;

#endif // TEST_GAME_VARIABLES_H
