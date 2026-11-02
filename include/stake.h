/**
 * @file stake.h
 *
 * @brief Stakes (difficulty levels) of a run.
 *
 * Only the list for now: White Stake is the default and the only one with an effect (none).
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

#endif // STAKE_H
