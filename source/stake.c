/**
 * @file stake.c
 *
 * @brief Stake effects that are not part of the Blinds, see stake.h.
 */
#include "stake.h"

#include "game.h"
#include "game_variables.h"
#include "joker.h"
#include "joker_batch6.h"
#include "list.h"
#include "random.h"

#define SELTZER_JOKER_ID 51 // Expires by itself, so it can't be Eternal

// clang-format off
static const char* const STAKE_NAMES[STAKE_MAX] = {
    [STAKE_WHITE]  = "Белая ставка",
    [STAKE_RED]    = "Красная ставка",
    [STAKE_GREEN]  = "Зелёная ставка",
    [STAKE_BLACK]  = "Чёрная ставка",
    [STAKE_BLUE]   = "Синяя ставка",
    [STAKE_PURPLE] = "Фиолетовая ставка",
    [STAKE_ORANGE] = "Оранжевая ставка",
    [STAKE_GOLD]   = "Золотая ставка",
};

static const char* const STAKE_RULES[STAKE_MAX] = {
    [STAKE_WHITE]  = "Обычная игра",
    [STAKE_RED]    = "Малый блайнд без $",
    [STAKE_GREEN]  = "Цели растут быстрее",
    [STAKE_BLACK]  = "Вечные джокеры",
    [STAKE_BLUE]   = "-1 сброс",
    [STAKE_PURPLE] = "Цели ещё быстрее",
    [STAKE_ORANGE] = "Скоропорт. джокеры",
    [STAKE_GOLD]   = "Джокеры в аренду",
};
// clang-format on

static inline bool stake_is_valid(enum Stake stake)
{
    return (int)stake >= 0 && stake < STAKE_MAX;
}

const char* stake_get_name(enum Stake stake)
{
    return stake_is_valid(stake) ? STAKE_NAMES[stake] : "";
}

const char* stake_get_rule(enum Stake stake)
{
    return stake_is_valid(stake) ? STAKE_RULES[stake] : "";
}

/**
 * @brief Blue Stake: -1 discard every round.
 *        Kept on its own since this Stake changes in Balatro 1.1.
 */
static void stake_apply_blue(void)
{
    if (g_game_vars.discards_per_round > 0)
        g_game_vars.discards_per_round--;
}

void stake_apply_start_effects(void)
{
    if (g_game_vars.stake >= STAKE_BLUE)
        stake_apply_blue();
}

void stake_roll_joker_stickers(struct Joker* joker)
{
    if (joker == NULL)
        return;

    joker->stickers = 0;
    joker->perish_rounds = 0;

    if (g_game_vars.stake < STAKE_BLACK)
        return;

    // One roll for Eternal or Perishable (they can't be combined), one roll for Rental
    u32 roll = rng_get_u32(RNG_SEQ_STAKE_STICKERS) % 100;
    if (roll < STAKE_STICKER_CHANCE && joker->id != SELTZER_JOKER_ID &&
        joker->id != MR_BONES_JOKER_ID)
    {
        joker->stickers |= JOKER_STICKER_ETERNAL;
    }
    else if (g_game_vars.stake >= STAKE_ORANGE && roll >= STAKE_STICKER_CHANCE &&
             roll < 2 * STAKE_STICKER_CHANCE)
    {
        joker->stickers |= JOKER_STICKER_PERISHABLE;
        joker->perish_rounds = PERISHABLE_JOKER_ROUNDS;
    }

    if (g_game_vars.stake >= STAKE_GOLD &&
        rng_get_u32(RNG_SEQ_STAKE_STICKERS) % 100 < STAKE_STICKER_CHANCE)
    {
        joker->stickers |= JOKER_STICKER_RENTAL;
    }
}

void stake_on_round_end(void)
{
    ListItr itr = list_itr_create(get_jokers_list());
    JokerObject* joker_object;
    while ((joker_object = list_itr_next(&itr)))
    {
        Joker* joker = joker_object->joker;
        if ((joker->stickers & JOKER_STICKER_PERISHABLE) && joker->perish_rounds > 0)
            joker->perish_rounds--;
    }
    game_refresh_joker_rules(); // A Joker that just perished stops working
}

int stake_get_rental_jokers_count(void)
{
    int count = 0;
    ListItr itr = list_itr_create(get_jokers_list());
    JokerObject* joker_object;
    while ((joker_object = list_itr_next(&itr)))
    {
        if (joker_object->joker->stickers & JOKER_STICKER_RENTAL)
            count++;
    }
    return count;
}
