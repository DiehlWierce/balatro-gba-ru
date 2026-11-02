#include <assert.h>
#include <stdio.h>
#include <tarot_rules.h>

static Card make(uint8_t rank, uint8_t suit, uint8_t enhancement, uint8_t seal)
{
    Card card = {.suit = suit, .rank = rank, .enhancement = enhancement, .seal = seal};
    return card;
}

static void test_suit_tarots(void)
{
    assert(tarot_get_suit(TAROT_STAR) == DIAMONDS);
    assert(tarot_get_suit(TAROT_MOON) == CLUBS);
    assert(tarot_get_suit(TAROT_SUN) == HEARTS);
    assert(tarot_get_suit(TAROT_WORLD) == SPADES);
    assert(tarot_get_suit(TAROT_DEATH) == -1);

    Card card = make(FIVE, CLUBS, CARD_ENHANCEMENT_BONUS, CARD_SEAL_RED);
    card_set_suit(&card, HEARTS);
    assert(card.suit == HEARTS && card.rank == FIVE);
    assert(card.enhancement == CARD_ENHANCEMENT_BONUS && card.seal == CARD_SEAL_RED);

    // Stone cards have no suit
    Card stone = make(ACE, CLUBS, CARD_ENHANCEMENT_STONE, CARD_SEAL_NONE);
    card_set_suit(&stone, SPADES);
    assert(stone.suit == CLUBS);

    card_set_suit(&card, NUM_SUITS); // Invalid suit is ignored
    assert(card.suit == HEARTS);
}

static void test_strength(void)
{
    Card card = make(TWO, SPADES, CARD_ENHANCEMENT_NONE, CARD_SEAL_NONE);
    card_rank_up(&card);
    assert(card.rank == THREE);

    card.rank = KING;
    card_rank_up(&card);
    assert(card.rank == ACE);
    card_rank_up(&card);
    assert(card.rank == TWO); // Ace wraps around to Two

    Card stone = make(TEN, SPADES, CARD_ENHANCEMENT_STONE, CARD_SEAL_NONE);
    card_rank_up(&stone);
    assert(stone.rank == TEN);
}

static void test_death(void)
{
    Card dst = make(TWO, CLUBS, CARD_ENHANCEMENT_NONE, CARD_SEAL_NONE);
    dst.debuffed = true;
    dst.played_ante = 3;
    Card src = make(QUEEN, HEARTS, CARD_ENHANCEMENT_GLASS, CARD_SEAL_GOLD);

    card_copy_from(&dst, &src);
    assert(dst.suit == HEARTS && dst.rank == QUEEN);
    assert(dst.enhancement == CARD_ENHANCEMENT_GLASS && dst.seal == CARD_SEAL_GOLD);
    assert(dst.debuffed && dst.played_ante == 3); // Runtime flags are kept
    assert(src.suit == HEARTS && src.rank == QUEEN);
}

static void test_enhancement_tarots(void)
{
    assert(tarot_get_enhancement(TAROT_MAGICIAN) == CARD_ENHANCEMENT_LUCKY);
    assert(tarot_get_enhancement(TAROT_EMPRESS) == CARD_ENHANCEMENT_MULT);
    assert(tarot_get_enhancement(TAROT_HIEROPHANT) == CARD_ENHANCEMENT_BONUS);
    assert(tarot_get_enhancement(TAROT_LOVERS) == CARD_ENHANCEMENT_WILD);
    assert(tarot_get_enhancement(TAROT_CHARIOT) == CARD_ENHANCEMENT_STEEL);
    assert(tarot_get_enhancement(TAROT_JUSTICE) == CARD_ENHANCEMENT_GLASS);
    assert(tarot_get_enhancement(TAROT_DEVIL) == CARD_ENHANCEMENT_GOLD);
    assert(tarot_get_enhancement(TAROT_TOWER) == CARD_ENHANCEMENT_STONE);
    assert(tarot_get_enhancement(TAROT_FOOL) == CARD_ENHANCEMENT_NONE);

    // Every enhancement but none is given by exactly one Tarot
    for (int enhancement = CARD_ENHANCEMENT_BONUS; enhancement < CARD_ENHANCEMENT_MAX; enhancement++)
    {
        int count = 0;
        for (int tarot = 0; tarot < TAROT_TYPE_MAX; tarot++)
            count += tarot_get_enhancement(tarot) == enhancement;
        assert(count == 1);
    }
}

static void test_target_ranges(void)
{
    int min, max;
    tarot_get_target_range(TAROT_FOOL, &min, &max);
    assert(min == 0 && max == 0);
    tarot_get_target_range(TAROT_DEATH, &min, &max);
    assert(min == 2 && max == 2);
    tarot_get_target_range(TAROT_STAR, &min, &max);
    assert(min == 1 && max == 3);
    tarot_get_target_range(TAROT_HANGED_MAN, &min, &max);
    assert(min == 1 && max == 2);
    tarot_get_target_range(TAROT_TOWER, &min, &max);
    assert(min == 1 && max == 1);

    for (int tarot = 0; tarot < TAROT_TYPE_MAX; tarot++)
    {
        tarot_get_target_range(tarot, &min, &max);
        assert(min <= max && max <= TAROT_MAX_TARGETS);
        // A Tarot either targets cards or creates/gives something, not both
        assert(!(max > 0 && tarot_creates_consumables(tarot)));
    }
}

static void test_money(void)
{
    assert(tarot_hermit_gain(0) == 0);
    assert(tarot_hermit_gain(-5) == 0);
    assert(tarot_hermit_gain(7) == 7);
    assert(tarot_hermit_gain(20) == 20);
    assert(tarot_hermit_gain(35) == 20);

    assert(tarot_temperance_gain(0) == 0);
    assert(tarot_temperance_gain(12) == 12);
    assert(tarot_temperance_gain(80) == 50);
}

static void test_rollable(void)
{
    int count = 0;
    for (int tarot = 0; tarot < TAROT_TYPE_MAX; tarot++)
        count += tarot_is_rollable(tarot);
    assert(count == TAROT_TYPE_MAX - 1);
    assert(tarot_is_rollable(TAROT_JUDGEMENT));
    assert(!tarot_is_rollable(TAROT_WHEEL));
    assert(!tarot_is_rollable(TAROT_TYPE_MAX));
}

int main(void)
{
    test_suit_tarots();
    test_strength();
    test_death();
    test_enhancement_tarots();
    test_target_ranges();
    test_money();
    test_rollable();
    printf("tarot_rules: all tests passed\n");
    return 0;
}
