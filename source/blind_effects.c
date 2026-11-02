/**
 * @file blind_effects.c
 *
 * @brief Boss Blind effects, see blind_effects.h.
 */
#include "blind_effects.h"

#include "game_variables.h"
#include "util.h"

/**
 * @brief Handler of every Blind, NULL means no effect.
 *
 * Boss Blinds are listed explicitly so each one gets its handler here when implemented.
 */
// clang-format off
static const BlindEffectFunc BLIND_EFFECT_FUNCS[BLIND_TYPE_MAX] = {
    [BLIND_TYPE_SMALL]   = NULL,
    [BLIND_TYPE_BIG]     = NULL,

    // Boss Blinds
    [BLIND_TYPE_HOOK]    = NULL,
    [BLIND_TYPE_OX]      = NULL,
    [BLIND_TYPE_HOUSE]   = NULL,
    [BLIND_TYPE_WALL]    = NULL,
    [BLIND_TYPE_WHEEL]   = NULL,
    [BLIND_TYPE_ARM]     = NULL,
    [BLIND_TYPE_CLUB]    = NULL,
    [BLIND_TYPE_FISH]    = NULL,
    [BLIND_TYPE_PSYCHIC] = NULL,
    [BLIND_TYPE_GOAD]    = NULL,
    [BLIND_TYPE_WATER]   = NULL,
    [BLIND_TYPE_WINDOW]  = NULL,
    [BLIND_TYPE_MANACLE] = NULL,
    [BLIND_TYPE_EYE]     = NULL,
    [BLIND_TYPE_MOUTH]   = NULL,
    [BLIND_TYPE_PLANT]   = NULL,
    [BLIND_TYPE_SERPENT] = NULL,
    [BLIND_TYPE_PILLAR]  = NULL,
    [BLIND_TYPE_NEEDLE]  = NULL,
    [BLIND_TYPE_HEAD]    = NULL,
    [BLIND_TYPE_TOOTH]   = NULL,
    [BLIND_TYPE_FLINT]   = NULL,
    [BLIND_TYPE_MARK]    = NULL,

    // Showdown Blinds
    [BLIND_TYPE_ACORN]   = NULL,
    [BLIND_TYPE_LEAF]    = NULL,
    [BLIND_TYPE_VESSEL]  = NULL,
    [BLIND_TYPE_HEART]   = NULL,
    [BLIND_TYPE_BELL]    = NULL,
};
// clang-format on

static inline bool blind_type_is_valid(enum BlindType blind)
{
    return (int)blind >= 0 && blind < BLIND_TYPE_MAX;
}

bool blind_effects_is_implemented(enum BlindType blind)
{
    return blind_type_is_valid(blind) && BLIND_EFFECT_FUNCS[blind] != NULL;
}

u32 blind_effects_trigger(enum BlindEvent event, const BlindEventData* data)
{
    enum BlindType blind = g_game_vars.current_blind;

    if (!blind_effects_is_implemented(blind) || (int)event < 0 || event >= BLIND_EVENT_MAX)
        return BLIND_EFFECT_FLAG_NONE;

    static const BlindEventData NO_DATA = {.card = NULL, .hand_type = NONE, .nb_cards = 0};

    return BLIND_EFFECT_FUNCS[blind](event, (data != NULL) ? data : &NO_DATA);
}
