/**
 * @file card_modifiers.h
 *
 * @brief Gameplay effects of card enhancements and seals (see enum CardEnhancement, CardSeal).
 *
 * The round calls these functions at the matching moments of the scoring; they apply the effects
 * to g_game_vars and return what happened so the round can show it.
 */
#ifndef CARD_MODIFIERS_H
#define CARD_MODIFIERS_H

#include "card.h"

#include <tonc.h>

/**
 * @brief What a scored card gave, for display. Fields are 0 when the effect didn't happen.
 */
typedef struct CardScoreResult
{
    int chips;            // Chips added (rank value, Bonus, Stone, Foil)
    int mult;             // Mult added (Mult card, Lucky card, Holographic)
    bool xmult;           // Mult multiplied by 2 (Glass card)
    bool xmult_1_5;       // Mult multiplied by 1.5 (Polychrome)
    int money;            // Money earned (Gold seal, Lucky card)
    bool lucky_triggered; // A Lucky card won its mult or its money (Lucky Cat)
} CardScoreResult;

/**
 * @brief Multiplies an integer mult by the fraction @p num / @p den (X1.5 = 3 / 2), rounded to the
 *        nearest (halves up) and saturated at UINT32_MAX. The one rounding rule of every X1.5 in
 *        the game (Steel, Polychrome, Observatory): the mult is an integer on this port, so
 *        3 x 1.5 is 5 everywhere, not 4 in one place and 5 in another.
 */
u32 card_modifiers_apply_xmult(u32 mult, u32 num, u32 den);

/**
 * @brief Applies the effects of a played card when it scores: chips, enhancement and seal.
 *
 * @param card the scoring card
 *
 * @return what the card gave
 */
CardScoreResult card_modifiers_on_scored(Card* card);

/**
 * @brief Whether the card is retriggered once by its Red seal after scoring.
 */
bool card_modifiers_has_retrigger(const Card* card);

/**
 * @brief Applies the effects of a card held in hand while a hand is scored (Steel: X1.5 mult).
 *
 * @return true if the card had an effect
 */
bool card_modifiers_on_held(Card* card);

/**
 * @brief Whether a scored card breaks after the hand (Glass: 1 in 4 chance).
 *        Rolls the chance, call it once per scored card.
 */
bool card_modifiers_breaks_after_scoring(const Card* card);

/**
 * @brief Applies the effects of a card held in hand when the round is won:
 *        Gold card gives $3, Blue seal creates the Planet of the last played hand.
 *
 * @param card the held card
 * @param last_hand_type the type of the last played hand
 * @param x, y screen position of the card, where a created Planet appears
 *
 * @return true if the card had an effect
 */
bool card_modifiers_on_round_won(Card* card, int last_hand_type, FIXED x, FIXED y);

/**
 * @brief Applies the effects of a discarded card (Purple seal: creates a Tarot).
 *
 * @param x, y screen position of the card, where a created Tarot appears
 *
 * @return true if the card had an effect
 */
bool card_modifiers_on_discarded(Card* card, FIXED x, FIXED y);

#endif // CARD_MODIFIERS_H
