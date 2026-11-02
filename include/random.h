/**
 * @file random.h
 *
 * @brief Common functions to handle RNG manipulation. Using this interface has three goals:
 * 1. Make RNG repeatable so that people can share seeds and saves, and end up with the same run
 * 2. Prevent the players from engaging in save-scumming since the pseudo-RNG is deterministic
 * 3. Accomodate for base-36 seeds, so that we can freely chose one with alpha-numeric characters in
 * the seed screen input
 */
#ifndef RANDOM_H
#define RANDOM_H

#include <tonc.h>

/**
 * @brief Keys to different independent RNG sequences
 */
enum RngSequence
{
    RNG_SEQ_CARD_SHUFFLE,
    RNG_SEQ_BLIND,
    RNG_SEQ_SHOP_ITEMS,
    RNG_SEQ_SKIP_TAGS,

    // Each Joker/Tag with a random effect has its own independent RNG sequence
    RNG_SEQ_JOKER_MISPRINT,
    RNG_SEQ_JOKER_RESERVED_PARKING,
    RNG_SEQ_JOKER_BUSINESS_CARD,
    RNG_SEQ_TAG_TOP_UP,

    // Shop: type of each top row item (Joker or Planet), then which Planet
    RNG_SEQ_SHOP_ITEM_TYPE,
    RNG_SEQ_SHOP_PLANETS,

    // For non-gameplay related things such as sound effects or visual effects, so as to not
    // interfere with important stuff like Shop rolls or Joker effects.
    RNG_SEQ_MISC,

    // Added after RNG_SEQ_MISC so the states of the sequences above stay the same for a given seed
    RNG_SEQ_BOSS_HOOK,
    RNG_SEQ_STAKE_STICKERS, // Eternal/Perishable/Rental stickers of Shop Jokers
    RNG_SEQ_BOSS_WHEEL,     // The Wheel: 1 in 7 cards drawn face down
    RNG_SEQ_BOSS_ACORN,     // Amber Acorn: Jokers shuffle
    RNG_SEQ_BOSS_HEART,     // Crimson Heart: disabled Joker
    RNG_SEQ_BOSS_BELL,      // Cerulean Bell: forced card
    RNG_SEQ_SHOP_VOUCHERS,

    // Card enhancements and Booster Packs
    RNG_SEQ_CARD_LUCKY_MULT,
    RNG_SEQ_CARD_LUCKY_MONEY,
    RNG_SEQ_CARD_GLASS,
    RNG_SEQ_PACK_CONTENTS,
    RNG_SEQ_SHOP_PACKS, // Type of the Booster Packs for sale in the Shop

    // Tarots
    RNG_SEQ_SHOP_CONSUMABLE_TYPE, // Planet or Tarot in a Shop slot of consumables
    RNG_SEQ_SHOP_TAROT,           // Which Tarot is for sale in the Shop
    RNG_SEQ_TAROT_CREATE,         // Planets of the High Priestess, Tarots of the Emperor
    RNG_SEQ_TAROT_PURPLE_SEAL,    // Tarot created by a discarded card with a Purple seal

    RNG_SEQ_MAX
};

/**
 * @brief Information to track and restore RNG state
 */
typedef struct
{
    /** Initial seed */
    u32 seed;
    /** Individual states for independent rng sequences */
    u32 states[RNG_SEQ_MAX];
} RngInfo;

/**
 * @brief Starts counting CPU cycles, which will be used by rng_shuffle_seed to
 *         generate a more random seed. To be called once on game start.
 */
void rng_init(void);

/**
 * @brief Accumulates CPU cycles for RNG seed generation, call exaclty once per frame.
 */
void rng_update(void);

/**
 * @brief Set the rng seed to the chosen value, and reset the step counter to 0.
 *         The seed will be capped at `MAX_BASE36` for compatibility with the Seed Input screen
 *         used to choose a seed for seeded runs.
 *
 * @param seed the new RNG seed
 */
void rng_set_seed(u32 seed);

/**
 * @brief Sets a new randomized seed for the RNG using a source of entropy.
 *
 * This needs to be called at least once before generating anything random but not on the very
 * first cycle because it uses a CPU timer for entropy.
 *
 * This would also be used to generate the new seed when starting a new non-player-seeded run
 *
 * @note `rng_init` needs to have been called, so that the CPU profiling can provide entropy.
 * @sa rng_init
 */
void rng_shuffle_seed(void);

/**
 * @brief Get the next "randomly" generated number in the sequence corresponding to the given type.
 *
 * @param key key to the RNG sequence we need to pull a random number from
 *
 * @return u32
 */
u32 rng_get_u32(enum RngSequence key);

/**
 * @brief Restore RNG info struct in the GameVariables. Sets the `seed` and restores the state for
 *         each independent sequence
 *
 * @param info RngInfo struct applied
 *
 * @note Consistency of the RNG after reloading the RngInfo struct has yet to be tested properly
 */
void rng_restore(RngInfo info);

#endif // RANDOM_H
