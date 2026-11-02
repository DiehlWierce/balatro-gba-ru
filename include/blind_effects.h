/**
 * @file blind_effects.h
 *
 * @brief Boss Blind effects, modeled after Joker events and effects.
 *
 * The round calls blind_effects_trigger() at each @ref BlindEvent. The handler of the current
 * Blind (g_game_vars.current_blind) reacts to the event and returns BLIND_EFFECT_FLAG_* flags.
 * Small and Big Blinds, and every Boss without a handler yet, have no effect.
 */
#ifndef BLIND_EFFECTS_H
#define BLIND_EFFECTS_H

#include "blind.h"
#include "card.h"
#include "hand.h"

#include <tonc.h>

/**
 * @brief Moments of a round where a Boss Blind can act
 */
enum BlindEvent
{
    BLIND_EVENT_ON_BLIND_SELECTED, // Round starts, before any card is drawn (e.g. The Needle)
    BLIND_EVENT_ON_HAND_PLAYED,    // Hand played, before scoring (e.g. The Psychic, The Eye)
    BLIND_EVENT_ON_CARD_DRAWN,     // A card is drawn into the hand (e.g. The Wheel, The House)
    BLIND_EVENT_ON_CARD_SCORED,    // A played card has scored
    BLIND_EVENT_ON_DISCARD,        // Selected cards are discarded (e.g. The Serpent)
    BLIND_EVENT_ON_ROUND_END,      // The round is over, won or lost
    BLIND_EVENT_MAX
};

/**
 * @brief Data passed to the Boss handlers, fields not related to the event are NULL / 0
 */
typedef struct BlindEventData
{
    Card* card;              // ON_CARD_DRAWN, ON_CARD_SCORED: the card
    enum HandType hand_type; // ON_HAND_PLAYED: type of the played hand
    int nb_cards;            // ON_HAND_PLAYED, ON_DISCARD: number of cards played or discarded
} BlindEventData;

// Flags returned by blind_effects_trigger(), they can be combined

#define BLIND_EFFECT_FLAG_NONE      0
#define BLIND_EFFECT_FLAG_TRIGGERED (1 << 0) // The Boss acted: give visual feedback on its token
#define BLIND_EFFECT_FLAG_NO_SCORE  (1 << 1) // ON_HAND_PLAYED: the hand scores nothing

/**
 * @brief Boss handler: reacts to @p event and returns BLIND_EFFECT_FLAG_* flags.
 *
 * @param event the event happening in the round
 * @param data details about the event, never NULL
 */
typedef u32 (*BlindEffectFunc)(enum BlindEvent event, const BlindEventData* data);

/**
 * @brief Triggers the effect of the current Blind (g_game_vars.current_blind) for @p event.
 *
 * @param event the event happening in the round
 * @param data details about the event, can be NULL when the event has no data
 *
 * @return BLIND_EFFECT_FLAG_* flags, BLIND_EFFECT_FLAG_NONE if the Blind has no effect
 */
u32 blind_effects_trigger(enum BlindEvent event, const BlindEventData* data);

/**
 * @brief Whether a Blind has an implemented effect.
 *
 * @param blind the Blind to check
 *
 * @return true if @p blind has a handler, false for Small/Big Blinds and unimplemented Bosses
 */
bool blind_effects_is_implemented(enum BlindType blind);

#endif // BLIND_EFFECTS_H
