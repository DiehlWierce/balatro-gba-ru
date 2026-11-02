/**
 * @file hand_levels_screen.h
 *
 * @brief "Уровни рук" screen: level, chips, mult and times played of every poker hand.
 *        Opened with SELECT while selecting cards in a round, closed with SELECT or B.
 */
#ifndef GAME_HAND_LEVELS_SCREEN_H
#define GAME_HAND_LEVELS_SCREEN_H

#include <stdbool.h>

/**
 * @brief Opens/closes the screen and handles its input. To be called first in the round update.
 *
 * @return true while the screen is open: the round must not process anything else this frame
 */
bool hand_levels_screen_update(void);

/**
 * @brief Whether the screen is currently open.
 */
bool hand_levels_screen_is_open(void);

#endif // GAME_HAND_LEVELS_SCREEN_H
