/**
 * @file stake.h
 *
 * @brief Stakes (difficulty levels) of a run.
 *
 * Each Stake also applies the effects of the previous ones. Effects are implemented in:
 * - blind.c: Small Blind reward (Red), score scaling (Green, Purple)
 * - stake.c: -1 discard (Blue), Joker stickers (Black, Orange, Gold)
 */
#ifndef STAKE_H
#define STAKE_H

/**
 * @brief All Stakes, each one also applies the effects of the previous ones
 */
enum Stake
{
    STAKE_WHITE,  // Base difficulty
    STAKE_RED,    // Small Blind gives no reward money
    STAKE_GREEN,  // Required score scales faster for each Ante
    STAKE_BLACK,  // Shop can have Eternal Jokers
    STAKE_BLUE,   // -1 Discard
    STAKE_PURPLE, // Required score scales even faster for each Ante
    STAKE_ORANGE, // Shop can have Perishable Jokers
    STAKE_GOLD,   // Shop can have Rental Jokers
    STAKE_MAX
};

// Max characters of a Stake name and of its rule line in the run setup screen
#define STAKE_NAME_LEN 17
#define STAKE_RULE_LEN 21

// Chance in % for a Shop Joker to get each sticker
#define STAKE_STICKER_CHANCE 30

struct Joker;

/**
 * @brief Russian name of a Stake, e.g. "Белая ставка" (at most STAKE_NAME_LEN characters)
 */
const char* stake_get_name(enum Stake stake);

/**
 * @brief Short rule added by a Stake (at most STAKE_RULE_LEN characters)
 */
const char* stake_get_rule(enum Stake stake);

/**
 * @brief Applies the Stake effects that change the run's base values.
 *        Called by game_start() after deck_apply_start_effects().
 */
void stake_apply_start_effects(void);

/**
 * @brief Rolls the Eternal/Perishable/Rental stickers of a Joker appearing in the Shop.
 */
void stake_roll_joker_stickers(struct Joker* joker);

/**
 * @brief Counts down Perishable Jokers, called once at the end of each round.
 */
void stake_on_round_end(void);

/**
 * @brief Number of owned Rental Jokers, each one costs RENTAL_JOKER_COST at the end of a round.
 */
int stake_get_rental_jokers_count(void);

#endif // STAKE_H
