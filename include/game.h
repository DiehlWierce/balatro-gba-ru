#ifndef GAME_H
#define GAME_H

#include "joker_economy.h"
#include "game/common_ui.h"
#include "game_variables.h"
#include "graphic_utils.h"
#include "item.h"

#include <tonc.h>

#define MAX_DECK_SIZE 80 // Standard Packs add cards to the 52 of the starting Deck
#define FRAMES(x)     (((x) + (g_game_vars.game_speed) - 1) / (g_game_vars.game_speed))

// Capacities for arrays and screen layouts. The limits during a run are
// g_game_vars.joker_slots and g_game_vars.shop_slots.
#define MAX_JOKERS_HELD_SIZE 7 // This doesn't account for negatives right now.
#define MAX_SHOP_ITEMS       4

// The maximum interest is g_game_vars.interest_cap
#define INTEREST_PER_5 1

// Input bindings

#define KEY_CONFIRM          KEY_A // Not using tonc's KEY_ACCEPT because it includes SELECT
#define KEY_CARD_SELECT      KEY_CONFIRM
#define KEY_CARDS_DESELECT   KEY_CANCEL // From tonc
#define KEY_PEEK_DECK        KEY_L      // TODO: Not implemented, remove?
#define KEY_PAUSE_GAME       KEY_START  // TODO: Not implemented, remove?
#define KEY_SHOW_DESCRIPTION KEY_B
#define KEY_SELL             KEY_L
#define KEY_TAB_LEFT         KEY_L
#define KEY_TAB_RIGHT        KEY_R
#define KEY_BOSS_REROLL      KEY_R // Blind Select, with Director's Cut or Retcon

// Using these buttons for play and discard hand to match the position of the on-screen buttons
#define KEY_PLAY_HAND KEY_L
// Same value as KEY_SELL - okay because they are activated on different rows

#define KEY_DISCARD_HAND KEY_R

struct List;
typedef struct List List;

// Utility functions for other files
typedef struct CardObject CardObject;
typedef struct Card Card;
typedef struct JokerObject JokerObject;

// Enum value names in ../include/def_state_info_table.h
enum GameState
{
#define DEF_STATE_INFO(stateEnum, on_init, on_update, on_exit) stateEnum,
#include "def_state_info_table.h"
#undef DEF_STATE_INFO
    GAME_STATE_MAX,
    GAME_STATE_UNDEFINED
};

// Game functions
void game_init(void);

/**
 * @brief Called when exiting the Game Over screen (both win or lose) to reset game variables
 *         and start a fresh new run.
 *
 * WARNING: This function is currently only meant to be called from the "GAME_OVER" state
 * and shouldn't be called from other states, otherwise some data such as shop jokers
 * may not be properly reset.
 */
void game_reset(void);

void game_update(void);
void game_change_state(enum GameState new_game_state);
enum GameState game_get_state(void);

bool is_joker_owned(int joker_id);

/**
 * @brief Sums the effects of the owned Jokers on the run limits (see joker_economy.h)
 */
JokerRunBonus game_get_joker_bonus(void);
int game_get_hands_per_round(void);
int game_get_discards_per_round(void);
int game_get_hand_size(void);

/**
 * @brief Whether the player can pay @p price, Credit Card allows going into debt
 */
bool game_can_afford(int price);
bool joker_object_can_acquire(Item* item);
bool card_is_face(Card* card);
void add_joker(JokerObject* joker_object);
void remove_owned_joker(int owned_joker_idx);
List* get_jokers_list(void);
List* get_expired_jokers_list(void);

/**
 * @brief Starts the discard animation for the item, destroying it when it ends
 * @param item The discarded item
 */
void item_start_discard_animation(Item* item);

int deck_get_size(void);

/**
 * @brief Cards the player owns in a round: draw pile, hand, played and discarded cards.
 */
int game_get_total_cards(void);

/**
 * @brief Number of cards with @p enhancement in the whole deck: draw pile, hand, played cards
 *        and discard pile (Steel Joker, Stone Joker)
 */
int game_count_deck_enhancement(u8 enhancement);

/**
 * @brief Number of cards of @p rank in the whole deck, same cards as game_count_deck_enhancement
 *        (Cloud 9). Stone cards have no rank.
 */
int game_count_deck_rank(u8 rank);
int get_deck_top(void);
void deck_push(Card* card);
Card* deck_pop(void);
void deck_shuffle(void);
int get_num_discards_remaining(void);
int get_num_hands_remaining(void);

void display_deck_size_max(void);
void display_chips(void);
void display_mult(void);
void display_money(void);
void display_ante(void);

// joker specific functions
bool is_shortcut_joker_active(void);
int get_straight_and_flush_size(void);

void game_start(void);

/**
 * @brief Shows or hides the sprite with the back of the run's Deck, drawn over the red Deck baked
 *        into the backgrounds (bottom right corner). It lives from game_start() to game_reset().
 */
void game_deck_sprite_set_hidden(bool hidden);

void display_round(void);
void display_hands(void);
void display_discards(void);
void display_temp_score(u32 value);
void erase_temp_score(void);
void display_score(u32 value);

#endif // GAME_H
