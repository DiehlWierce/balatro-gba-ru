#include <assert.h>
#include <card_modifiers.h>
#include <consumable.h>
#include <game_variables.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Host stubs for the game state and the systems card_modifiers.c calls into

GameVariables g_game_vars;

// Scripted RNG: the value rng_get_u32() returns for each sequence, 1 never triggers a 1-in-N roll
static u32 s_rng_next[RNG_SEQ_MAX];

u32 rng_get_u32(enum RngSequence key)
{
    return s_rng_next[key];
}

// Chips of the card, normally from card.c (Bonus and Stone values are not checked here)
static int s_card_chips;

int card_get_chips(Card* card)
{
    (void)card;
    return s_card_chips;
}

// Planet of each hand type, as in consumable.c
static const enum HandType PLANET_HANDS[PLANET_TYPE_MAX] = {
    [PLANET_MERCURY] = PAIR,
    [PLANET_VENUS] = THREE_OF_A_KIND,
    [PLANET_EARTH] = FULL_HOUSE,
    [PLANET_MARS] = FOUR_OF_A_KIND,
    [PLANET_JUPITER] = FLUSH,
    [PLANET_SATURN] = STRAIGHT,
    [PLANET_URANUS] = TWO_PAIR,
    [PLANET_NEPTUNE] = STRAIGHT_FLUSH,
    [PLANET_PLUTO] = HIGH_CARD,
    [PLANET_X] = FIVE_OF_A_KIND,
    [PLANET_CERES] = FLUSH_HOUSE,
    [PLANET_ERIS] = FLUSH_FIVE,
};

enum HandType planet_get_hand_type(enum PlanetType planet)
{
    return PLANET_HANDS[planet];
}

static int s_planets_created;
static int s_tarots_created;
bool consumable_create_random_tarot(enum RngSequence key, FIXED x, FIXED y)
{
    (void)key; (void)x; (void)y;
    s_tarots_created++;
    return true;
}
static int s_last_planet;
static int s_consumables_destroyed;
static bool s_can_acquire;
static int s_items_acquired;

ConsumableObject* consumable_object_new(enum ConsumableType type, int id)
{
    if (type != CONSUMABLE_TYPE_PLANET)
        return NULL;

    s_planets_created++;
    s_last_planet = id;
    return calloc(1, sizeof(ConsumableObject));
}

void consumable_object_destroy(ConsumableObject** consumable_object)
{
    s_consumables_destroyed++;
    free(*consumable_object);
    *consumable_object = NULL;
}

bool item_can_acquire(Item* item)
{
    (void)item;
    return s_can_acquire;
}

void item_acquire(Item* item)
{
    (void)item;
    s_items_acquired++;
}

// Used by util.c for number formatting, which the tests do not cover
char digit_char_to_font_point(char digit_char)
{
    return digit_char;
}

// Helpers

static void reset(void)
{
    memset(&g_game_vars, 0, sizeof(g_game_vars));
    for (int key = 0; key < RNG_SEQ_MAX; key++)
        s_rng_next[key] = 1;
    s_card_chips = 0;
    s_planets_created = 0;
    s_last_planet = -1;
    s_consumables_destroyed = 0;
    s_can_acquire = true;
    s_items_acquired = 0;
}

static Card make(uint8_t enhancement, uint8_t seal)
{
    Card card = {.suit = HEARTS, .rank = KING, .enhancement = enhancement, .seal = seal};
    return card;
}

static Card make_debuffed(uint8_t enhancement, uint8_t seal)
{
    Card card = make(enhancement, seal);
    card.debuffed = true;
    return card;
}

// Tests: scoring

static void test_base_chips_are_added(void)
{
    reset();
    s_card_chips = 10;
    Card card = make(CARD_ENHANCEMENT_NONE, CARD_SEAL_NONE);

    CardScoreResult result = card_modifiers_on_scored(&card);
    assert(result.chips == 10);
    assert(result.mult == 0 && !result.xmult && result.money == 0);
    assert(g_game_vars.chips == 10);
    assert(g_game_vars.mult == 0 && g_game_vars.money == 0);
}

static void test_chips_accumulate_and_saturate(void)
{
    reset();
    s_card_chips = 10;
    g_game_vars.chips = 5;
    Card card = make(CARD_ENHANCEMENT_NONE, CARD_SEAL_NONE);
    card_modifiers_on_scored(&card);
    assert(g_game_vars.chips == 15);

    g_game_vars.chips = UINT32_MAX - 2;
    card_modifiers_on_scored(&card);
    assert(g_game_vars.chips == UINT32_MAX);
}

static void test_mult_card(void)
{
    reset();
    g_game_vars.mult = 10;
    Card card = make(CARD_ENHANCEMENT_MULT, CARD_SEAL_NONE);

    CardScoreResult result = card_modifiers_on_scored(&card);
    assert(result.mult == 4);
    assert(g_game_vars.mult == 14);
}

static void test_glass_card_doubles_mult(void)
{
    reset();
    g_game_vars.mult = 3;
    Card card = make(CARD_ENHANCEMENT_GLASS, CARD_SEAL_NONE);

    CardScoreResult result = card_modifiers_on_scored(&card);
    assert(result.xmult);
    assert(g_game_vars.mult == 6);
}

static void test_glass_card_breaks_one_in_four(void)
{
    reset();
    Card card = make(CARD_ENHANCEMENT_GLASS, CARD_SEAL_NONE);
    for (u32 roll = 0; roll < 12; roll++)
    {
        s_rng_next[RNG_SEQ_CARD_GLASS] = roll;
        assert(card_modifiers_breaks_after_scoring(&card) == (roll % 4 == 0));
    }

    Card mult = make(CARD_ENHANCEMENT_MULT, CARD_SEAL_NONE);
    s_rng_next[RNG_SEQ_CARD_GLASS] = 0;
    assert(!card_modifiers_breaks_after_scoring(&mult)); // Only Glass cards break
}

static void test_lucky_card_mult_one_in_five(void)
{
    reset();
    Card card = make(CARD_ENHANCEMENT_LUCKY, CARD_SEAL_NONE);
    for (u32 roll = 0; roll < 12; roll++)
    {
        g_game_vars.mult = 0;
        s_rng_next[RNG_SEQ_CARD_LUCKY_MULT] = roll;
        CardScoreResult result = card_modifiers_on_scored(&card);
        assert(result.mult == (roll % 5 == 0 ? 20 : 0));
        assert(g_game_vars.mult == (roll % 5 == 0 ? 20 : 0));
    }
}

static void test_lucky_card_money_one_in_fifteen(void)
{
    reset();
    Card card = make(CARD_ENHANCEMENT_LUCKY, CARD_SEAL_NONE);
    for (u32 roll = 0; roll < 31; roll++)
    {
        g_game_vars.money = 0;
        s_rng_next[RNG_SEQ_CARD_LUCKY_MONEY] = roll;
        CardScoreResult result = card_modifiers_on_scored(&card);
        assert(result.money == (roll % 15 == 0 ? 20 : 0));
        assert(g_game_vars.money == (roll % 15 == 0 ? 20 : 0));
    }
}

static void test_lucky_card_rolls_are_independent(void)
{
    reset();
    s_rng_next[RNG_SEQ_CARD_LUCKY_MULT] = 0;
    s_rng_next[RNG_SEQ_CARD_LUCKY_MONEY] = 0;
    Card card = make(CARD_ENHANCEMENT_LUCKY, CARD_SEAL_NONE);

    CardScoreResult result = card_modifiers_on_scored(&card);
    assert(result.mult == 20 && result.money == 20);
}

static void test_gold_seal_pays_when_scored(void)
{
    reset();
    Card card = make(CARD_ENHANCEMENT_NONE, CARD_SEAL_GOLD);

    CardScoreResult result = card_modifiers_on_scored(&card);
    assert(result.money == 3);
    assert(g_game_vars.money == 3);
}

static void test_gold_card_pays_only_when_round_won(void)
{
    reset();
    Card card = make(CARD_ENHANCEMENT_GOLD, CARD_SEAL_NONE);

    CardScoreResult result = card_modifiers_on_scored(&card);
    assert(result.money == 0 && g_game_vars.money == 0);

    assert(card_modifiers_on_round_won(&card, NONE, 0, 0));
    assert(g_game_vars.money == 3);
}

static void test_steel_card_held_rounding(void)
{
    // X1.5 rounded to the nearest integer: 1 -> 2, 2 -> 3, 3 -> 5, 4 -> 6
    reset();
    Card card = make(CARD_ENHANCEMENT_STEEL, CARD_SEAL_NONE);
    const u32 mults[4] = {1, 2, 3, 4};
    const u32 expected[4] = {2, 3, 5, 6};
    for (int i = 0; i < 4; i++)
    {
        g_game_vars.mult = mults[i];
        assert(card_modifiers_on_held(&card));
        assert(g_game_vars.mult == expected[i]);
    }

    g_game_vars.mult = 0;
    assert(card_modifiers_on_held(&card));
    assert(g_game_vars.mult == 0);

    // Near UINT32_MAX the mult saturates instead of wrapping around to 0
    g_game_vars.mult = UINT32_MAX;
    assert(card_modifiers_on_held(&card));
    assert(g_game_vars.mult == UINT32_MAX);
    g_game_vars.mult = UINT32_MAX - 1;
    assert(card_modifiers_on_held(&card));
    assert(g_game_vars.mult == UINT32_MAX);
    g_game_vars.mult = 2863311530u; // 3 * mult / 2 just above UINT32_MAX
    assert(card_modifiers_on_held(&card));
    assert(g_game_vars.mult == UINT32_MAX);
}

static void test_non_steel_card_held_is_inert(void)
{
    reset();
    g_game_vars.mult = 4;
    Card card = make(CARD_ENHANCEMENT_GOLD, CARD_SEAL_NONE);

    assert(!card_modifiers_on_held(&card));
    assert(g_game_vars.mult == 4);
}

static void test_red_seal_retriggers(void)
{
    reset();
    Card red = make(CARD_ENHANCEMENT_NONE, CARD_SEAL_RED);
    Card plain = make(CARD_ENHANCEMENT_NONE, CARD_SEAL_NONE);
    assert(card_modifiers_has_retrigger(&red));
    assert(!card_modifiers_has_retrigger(&plain));
}

static void test_blue_seal_creates_planet_of_last_hand(void)
{
    reset();
    Card card = make(CARD_ENHANCEMENT_NONE, CARD_SEAL_BLUE);

    assert(card_modifiers_on_round_won(&card, PAIR, 0, 0));
    assert(s_planets_created == 1);
    assert(s_last_planet == PLANET_MERCURY);
    assert(s_items_acquired == 1);
}

static void test_blue_seal_royal_flush_gives_neptune(void)
{
    reset();
    Card card = make(CARD_ENHANCEMENT_NONE, CARD_SEAL_BLUE);

    assert(card_modifiers_on_round_won(&card, ROYAL_FLUSH, 0, 0));
    assert(s_last_planet == PLANET_NEPTUNE);
}

static void test_blue_seal_without_planet_or_slot(void)
{
    reset();
    Card card = make(CARD_ENHANCEMENT_NONE, CARD_SEAL_BLUE);

    assert(!card_modifiers_on_round_won(&card, NONE, 0, 0));
    assert(s_planets_created == 0);

    s_can_acquire = false;
    assert(!card_modifiers_on_round_won(&card, PAIR, 0, 0));
    assert(s_consumables_destroyed == 1);
    assert(s_items_acquired == 0);
}

static void test_purple_seal_creates_tarot(void)
{
    reset();
    s_tarots_created = 0;
    Card card = make(CARD_ENHANCEMENT_NONE, CARD_SEAL_PURPLE);
    assert(card_modifiers_on_discarded(&card, 0, 0));
    assert(s_tarots_created == 1);
}

static void test_debuffed_card_is_inert(void)
{
    reset();
    s_card_chips = 10;
    g_game_vars.mult = 7;
    g_game_vars.chips = 3;
    g_game_vars.money = 2;

    Card mult = make_debuffed(CARD_ENHANCEMENT_MULT, CARD_SEAL_GOLD);
    CardScoreResult scored = card_modifiers_on_scored(&mult);
    assert(scored.chips == 0 && scored.mult == 0 && !scored.xmult && scored.money == 0);
    assert(g_game_vars.chips == 3 && g_game_vars.mult == 7 && g_game_vars.money == 2);

    Card glass = make_debuffed(CARD_ENHANCEMENT_GLASS, CARD_SEAL_NONE);
    assert(!card_modifiers_breaks_after_scoring(&glass));
    assert(!card_modifiers_on_scored(&glass).xmult);
    assert(g_game_vars.mult == 7);

    Card steel = make_debuffed(CARD_ENHANCEMENT_STEEL, CARD_SEAL_NONE);
    assert(!card_modifiers_on_held(&steel));
    assert(g_game_vars.mult == 7);

    Card gold = make_debuffed(CARD_ENHANCEMENT_GOLD, CARD_SEAL_NONE);
    assert(!card_modifiers_on_round_won(&gold, NONE, 0, 0));
    assert(g_game_vars.money == 2);

    Card red = make_debuffed(CARD_ENHANCEMENT_NONE, CARD_SEAL_RED);
    assert(!card_modifiers_has_retrigger(&red));

    Card blue = make_debuffed(CARD_ENHANCEMENT_NONE, CARD_SEAL_BLUE);
    assert(!card_modifiers_on_round_won(&blue, PAIR, 0, 0));
    assert(s_planets_created == 0);
}

static void test_null_card_is_inert(void)
{
    reset();
    CardScoreResult scored = card_modifiers_on_scored(NULL);
    assert(scored.chips == 0 && scored.mult == 0 && !scored.xmult && scored.money == 0);
    assert(!card_modifiers_on_held(NULL));
    assert(!card_modifiers_breaks_after_scoring(NULL));
    assert(!card_modifiers_has_retrigger(NULL));
    assert(!card_modifiers_on_round_won(NULL, PAIR, 0, 0));
    assert(!card_modifiers_on_discarded(NULL, 0, 0));
}

// Tests: editions

static void test_foil_adds_chips(void)
{
    reset();
    s_card_chips = 10;
    g_game_vars.mult = 3;
    Card card = make(CARD_ENHANCEMENT_NONE, CARD_SEAL_NONE);
    card.edition = EDITION_FOIL;

    CardScoreResult result = card_modifiers_on_scored(&card);
    assert(result.chips == 60);
    assert(g_game_vars.chips == 60);
    assert(g_game_vars.mult == 3 && result.mult == 0 && !result.xmult_1_5);
}

static void test_holographic_adds_mult(void)
{
    reset();
    s_card_chips = 10;
    g_game_vars.mult = 3;
    Card card = make(CARD_ENHANCEMENT_NONE, CARD_SEAL_NONE);
    card.edition = EDITION_HOLOGRAPHIC;

    CardScoreResult result = card_modifiers_on_scored(&card);
    assert(result.chips == 10 && result.mult == 10);
    assert(g_game_vars.chips == 10 && g_game_vars.mult == 13);
}

static void test_polychrome_multiplies_mult(void)
{
    reset();
    g_game_vars.mult = 10;
    Card card = make(CARD_ENHANCEMENT_NONE, CARD_SEAL_NONE);
    card.edition = EDITION_POLYCHROME;

    CardScoreResult result = card_modifiers_on_scored(&card);
    assert(result.xmult_1_5 && !result.xmult);
    assert(g_game_vars.mult == 15);

    g_game_vars.mult = 1;
    card_modifiers_on_scored(&card);
    assert(g_game_vars.mult == 2); // Rounded to the nearest
}

static void test_edition_scores_after_enhancement(void)
{
    // Glass X2 comes before Holographic +10: (3 * 2) + 10, not (3 + 10) * 2
    reset();
    g_game_vars.mult = 3;
    Card glass = make(CARD_ENHANCEMENT_GLASS, CARD_SEAL_NONE);
    glass.edition = EDITION_HOLOGRAPHIC;
    card_modifiers_on_scored(&glass);
    assert(g_game_vars.mult == 16);

    // Mult card +4 comes before Polychrome X1.5: (6 + 4) * 1.5
    reset();
    g_game_vars.mult = 6;
    Card mult = make(CARD_ENHANCEMENT_MULT, CARD_SEAL_NONE);
    mult.edition = EDITION_POLYCHROME;
    card_modifiers_on_scored(&mult);
    assert(g_game_vars.mult == 15);

    // Bonus card and Foil add up
    reset();
    s_card_chips = 40;
    Card bonus = make(CARD_ENHANCEMENT_BONUS, CARD_SEAL_NONE);
    bonus.edition = EDITION_FOIL;
    assert(card_modifiers_on_scored(&bonus).chips == 90);
}

static void test_negative_card_and_debuffed_edition_are_inert(void)
{
    reset();
    s_card_chips = 10;
    g_game_vars.mult = 5;
    Card negative = make(CARD_ENHANCEMENT_NONE, CARD_SEAL_NONE);
    negative.edition = EDITION_NEGATIVE;
    CardScoreResult result = card_modifiers_on_scored(&negative);
    assert(result.chips == 10 && result.mult == 0 && !result.xmult_1_5);
    assert(g_game_vars.mult == 5);

    reset();
    Card foil = make_debuffed(CARD_ENHANCEMENT_NONE, CARD_SEAL_NONE);
    foil.edition = EDITION_FOIL;
    card_modifiers_on_scored(&foil);
    assert(g_game_vars.chips == 0); // A debuffed card gives nothing, its edition included
}

int main(void)
{
    test_base_chips_are_added();
    test_chips_accumulate_and_saturate();
    test_mult_card();
    test_glass_card_doubles_mult();
    test_glass_card_breaks_one_in_four();
    test_lucky_card_mult_one_in_five();
    test_lucky_card_money_one_in_fifteen();
    test_lucky_card_rolls_are_independent();
    test_gold_seal_pays_when_scored();
    test_gold_card_pays_only_when_round_won();
    test_steel_card_held_rounding();
    test_non_steel_card_held_is_inert();
    test_red_seal_retriggers();
    test_blue_seal_creates_planet_of_last_hand();
    test_blue_seal_royal_flush_gives_neptune();
    test_blue_seal_without_planet_or_slot();
    test_purple_seal_creates_tarot();
    test_debuffed_card_is_inert();
    test_null_card_is_inert();
    test_foil_adds_chips();
    test_holographic_adds_mult();
    test_polychrome_multiplies_mult();
    test_edition_scores_after_enhancement();
    test_negative_card_and_debuffed_edition_are_inert();
    printf("Card Modifiers Tests Passed :)\n");
    return 0;
}
