/**
 * @file game.h
 *
 * @brief Host stand-in for include/game.h: the run limits and the HUD updates Vouchers call.
 */
#ifndef TEST_GAME_H
#define TEST_GAME_H

#include "game_variables.h"

#define MAX_JOKERS_HELD_SIZE 7
#define MAX_SHOP_ITEMS       4

void display_ante(void);
void display_hands(void);
void display_discards(void);

#endif // TEST_GAME_H
