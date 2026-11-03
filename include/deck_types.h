/**
 * @file deck_types.h
 *
 * @brief Functions related to the different types of Decks.
 */
#ifndef DECK_TYPES_H
#define DECK_TYPES_H

#include "deck_rules.h"
#include "graphic_utils.h"

#define DECK_NAME_LENGTH 14 // 13 chars + '\0'
#define DECK_DESC_WIDTH  DECK_NAME_LENGTH
#define DECK_DESC_HEIGHT 5

/**
 * @brief Prints the Deck name string to the screen at the given position.
 *
 * @param deck Decktype we want the name of.
 * @param pos screen position in pixels to print the name at.
 */
void print_deck_name(enum DeckType deck, BG_POINT pos);

/**
 * @brief Prints the Deck description string to the screen at the given position.
 *
 * @param deck DeckType we want the description of.
 * @param pos screen position in pixels of the top-left corner of the rectangle
 *             we'll print the Deck description into
 */
void print_deck_description(enum DeckType deck, BG_POINT pos);

/**
 * @brief Applies the effects of a Deck at the start of a run.
 *
 * Called once by game_start(), after game_init() has reset the run's base values
 * (g_game_vars.hands_per_round, discards_per_round, base_hand_size, joker_slots, money...) and
 * before the first round. Effects change these base values, not the current round's ones.
 *
 * @param deck DeckType chosen for the run.
 */
void deck_apply_start_effects(enum DeckType deck);

#endif // DECK_TYPES_H
