/**
 * @file card_modifiers.c
 *
 * @brief Gameplay effects of card enhancements and seals, see card_modifiers.h.
 */
#include "card_modifiers.h"

#include "consumable.h"
#include "game.h"
#include "game_variables.h"
#include "hand.h"
#include "item.h"
#include "random.h"

#define MULT_CARD_MULT     4
#define GLASS_CARD_XMULT   2
#define GLASS_BREAK_CHANCE 4 // 1 in 4
#define STEEL_XMULT_NUM    3 // X1.5 = 3/2
#define STEEL_XMULT_DEN    2
#define GOLD_CARD_MONEY    3
#define GOLD_SEAL_MONEY    3
#define LUCKY_MULT         20
#define LUCKY_MULT_CHANCE  5 // 1 in 5
#define LUCKY_MONEY        20
#define LUCKY_MONEY_CHANCE 15 // 1 in 15

static inline bool roll_one_in(enum RngSequence key, u32 chance)
{
    return rng_get_u32(key) % chance == 0;
}

CardScoreResult card_modifiers_on_scored(Card* card)
{
    CardScoreResult result = {0};
    if (card == NULL || card->debuffed)
        return result;

    result.chips = card_get_chips(card);
    g_game_vars.chips = u32_protected_add(g_game_vars.chips, result.chips);

    switch (card->enhancement)
    {
        case CARD_ENHANCEMENT_MULT:
            result.mult = MULT_CARD_MULT;
            break;
        case CARD_ENHANCEMENT_LUCKY:
            if (roll_one_in(RNG_SEQ_CARD_LUCKY_MULT, LUCKY_MULT_CHANCE))
                result.mult = LUCKY_MULT;
            if (roll_one_in(RNG_SEQ_CARD_LUCKY_MONEY, LUCKY_MONEY_CHANCE))
                result.money += LUCKY_MONEY;
            result.lucky_triggered = result.mult > 0 || result.money > 0;
            break;
        case CARD_ENHANCEMENT_GLASS:
            result.xmult = true;
            break;
        default:
            break;
    }

    if (card->seal == CARD_SEAL_GOLD)
        result.money += GOLD_SEAL_MONEY;

    if (result.mult > 0)
        g_game_vars.mult = u32_protected_add(g_game_vars.mult, result.mult);
    if (result.xmult)
        g_game_vars.mult = u32_protected_mult(g_game_vars.mult, GLASS_CARD_XMULT);
    if (result.money > 0)
        g_game_vars.money += result.money;

    return result;
}

bool card_modifiers_has_retrigger(const Card* card)
{
    return card != NULL && !card->debuffed && card->seal == CARD_SEAL_RED;
}

bool card_modifiers_on_held(Card* card)
{
    if (card == NULL || card->debuffed || card->enhancement != CARD_ENHANCEMENT_STEEL)
        return false;

    // The mult is an integer: round X1.5 to the nearest (1 -> 2, 3 -> 5) so Steel always counts
    // 64 bits so that neither the product nor the rounding can wrap around near UINT32_MAX
    unsigned long long mult =
        ((unsigned long long)g_game_vars.mult * STEEL_XMULT_NUM + STEEL_XMULT_DEN / 2) /
        STEEL_XMULT_DEN;
    g_game_vars.mult = mult > UINT32_MAX ? UINT32_MAX : (u32)mult;
    return true;
}

bool card_modifiers_breaks_after_scoring(const Card* card)
{
    if (card == NULL || card->debuffed || card->enhancement != CARD_ENHANCEMENT_GLASS)
        return false;

    return roll_one_in(RNG_SEQ_CARD_GLASS, GLASS_BREAK_CHANCE);
}

/**
 * @brief The Planet that levels up @p hand_type, PLANET_TYPE_MAX if none.
 */
static enum PlanetType planet_of_hand(int hand_type)
{
    for (enum PlanetType planet = 0; planet < PLANET_TYPE_MAX; planet++)
    {
        if ((int)planet_get_hand_type(planet) == hand_type)
            return planet;
    }

    // Royal Flush shares Neptune with the Straight Flush
    if (hand_type == ROYAL_FLUSH)
        return PLANET_NEPTUNE;

    return PLANET_TYPE_MAX;
}

static bool blue_seal_create_planet(int last_hand_type, FIXED x, FIXED y)
{
    enum PlanetType planet = planet_of_hand(last_hand_type);
    if (planet == PLANET_TYPE_MAX)
        return false;

    ConsumableObject* consumable = consumable_object_new(CONSUMABLE_TYPE_PLANET, planet);
    if (consumable == NULL)
        return false;

    Item* item = (Item*)consumable;
    if (!item_can_acquire(item))
    {
        // No free consumable slot
        consumable_object_destroy(&consumable);
        return false;
    }

    item->x = x;
    item->y = y;
    item->tx = x;
    item->ty = y;
    item_acquire(item);
    return true;
}

bool card_modifiers_on_round_won(Card* card, int last_hand_type, FIXED x, FIXED y)
{
    if (card == NULL || card->debuffed)
        return false;

    bool triggered = false;

    if (card->enhancement == CARD_ENHANCEMENT_GOLD)
    {
        g_game_vars.money += GOLD_CARD_MONEY;
        triggered = true;
    }

    if (card->seal == CARD_SEAL_BLUE && blue_seal_create_planet(last_hand_type, x, y))
        triggered = true;

    return triggered;
}

bool card_modifiers_on_discarded(Card* card, FIXED x, FIXED y)
{
    if (card == NULL || card->seal != CARD_SEAL_PURPLE)
        return false;

    return consumable_create_random_tarot(RNG_SEQ_TAROT_PURPLE_SEAL, x, y);
}
