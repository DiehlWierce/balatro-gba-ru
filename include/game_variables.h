/**
 * @file game_variables.h
 *
 * @brief Game global game variables struct definition
 */
#ifndef GAME_VARIABLES_H
#define GAME_VARIABLES_H

#include "blind.h"
#include "hand.h"
#include "random.h"
#include "stake.h"

#include <tonc.h>

#define GAME_SPEED_MIN 1
#define GAME_SPEED_MAX 4

// Volume is stored from 0 to 5 but is an increment of 20 so 0 to 100 will be displayed
#define VOLUME_OPTION_MIN       0
#define VOLUME_OPTION_MAX       5
#define VOLUME_OPTION_INCREMENT 20

#define DEFAULT_GAME_SPEED   1
#define DEFAULT_MUSIC_VOLUME VOLUME_OPTION_MAX
#define DEFAULT_SOUND_VOLUME VOLUME_OPTION_MAX

// Default per-round hands and discards of a run, before Deck/Voucher effects
#define MAX_HANDS    4
#define MAX_DISCARDS 3

#define DEFAULT_HAND_SIZE 8

// Default run limits, changed by Decks, Vouchers and Stakes
#define DEFAULT_JOKER_SLOTS       5 // Must not exceed MAX_JOKERS_HELD_SIZE (game.h)
#define DEFAULT_CONSUMABLE_SLOTS  2
#define DEFAULT_SHOP_SLOTS        2 // Must not exceed MAX_SHOP_ITEMS (game.h)
#define DEFAULT_INTEREST_CAP      5 // Maximum interest in $ at the end of a round
#define DEFAULT_MONEY_PER_HAND    1 // $ per remaining hand at the end of a round
#define DEFAULT_MONEY_PER_DISCARD 0 // $ per remaining discard at the end of a round

#define DEFAULT_HAND_LEVEL 1

/**
 * @brief A central location for all game variables.
 *
 * **NOTE**: This is currently WIP and will be populated with a refactor effort.
 * NOT ALL VARIABLES ARE LOCATED HERE YET
 */
typedef struct
{
    // Internal variables

    s32 timer; // This might already exist in libtonc but idk so i'm just making my own
    RngInfo rng_info;

    // Variables visible by the player

    // Round variables
    u32 score;
    u32 chips;
    u32 mult;
    s32 hands;
    s32 discards;

    // Run variables
    s32 money;
    s32 hand_size;
    s32 ante;
    s32 round;
    // The ante 8 Boss was beaten: show the win screen (with Endless mode offer) after the cash out
    bool win_pending;
    s32 deck;
    // Indexed by `hand_type - 1`: HAND_TYPE_MAX is the last hand type (FLUSH_FIVE), not a count
    u32 nb_played_hands[HAND_TYPE_MAX];

    // Run limits: base values reset by game_init() and changed by Deck, Voucher and Stake effects
    s32 hands_per_round;    // Hands given at the start of each round
    s32 discards_per_round; // Discards given at the start of each round
    s32 base_hand_size;     // Hand size restored at the end of each round
    s32 joker_slots;
    s32 consumable_slots;
    s32 shop_slots;        // Number of items in the top row of the Shop
    s32 interest_cap;      // Maximum interest in $ at the end of a round
    s32 money_per_hand;    // $ per remaining hand at the end of a round
    s32 money_per_discard; // $ per remaining discard at the end of a round
    enum Stake stake;

    // Level of each poker hand, indexed by `hand_type - 1` like nb_played_hands
    u16 hand_levels[HAND_TYPE_MAX];

    // Bit `i` is set when the Voucher with index `i` is owned (32 Vouchers in the base game)
    u32 owned_vouchers;

    // Hidden variables

    u32 best_hand_score;
    u32 nb_skipped_rounds;
    u32 nb_unused_discards;

    // Blind variables

    enum BlindType current_blind;
    enum BlindType next_boss_blind;
    enum BlindState blinds_states[NUM_BLINDS_PER_ANTE];

    Sprite* playing_blind_token;
    Sprite* round_end_blind_token;

    // Options variables

    // BY DEFAULT IS SET TO 1, but if changed to 2 or more, should speed up all (or most) of the
    // game aspects that should be sped up by speed, as in the original game.
    u8 game_speed;
    u8 music_volume;
    u8 sound_volume;
} GameVariables;

extern GameVariables g_game_vars;

#endif // GAME_VARIABLES_H
