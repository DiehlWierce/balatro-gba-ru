/**
 * @file spectral_rules.h
 *
 * @brief Spectral cards: the list, what each one targets, which ones can appear and the pure rules
 *        that change or create Cards, plus the Shop odds that Vouchers change (Tarot Merchant /
 *        Tycoon, Omen Globe, Telescope).
 *
 * Plain C without libtonc so it can be tested on the host (tests/spectral_rules).
 */
#ifndef SPECTRAL_RULES_H
#define SPECTRAL_RULES_H

#include "edition_rules.h"
#include "hand_type.h"
#include "tarot_rules.h"

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Spectral cards, in the order of their sprites in graphics/spectral_gfx.png
 */
enum SpectralType
{
    SPECTRAL_FAMILIAR,   // Destroys 1 random card in hand, adds 3 random enhanced face cards
    SPECTRAL_GRIM,       // Destroys 1 random card in hand, adds 2 random enhanced Aces
    SPECTRAL_INCANTATION, // Destroys 1 random card in hand, adds 4 random enhanced numbered cards
    SPECTRAL_TALISMAN,   // Gold seal on 1 card
    SPECTRAL_AURA,       // random edition (Foil, Holographic, Polychrome) on 1 card
    SPECTRAL_WRAITH,     // Creates a random Rare Joker, money becomes $0
    SPECTRAL_SIGIL,      // Every card in hand becomes one random suit
    SPECTRAL_OUIJA,      // Every card in hand becomes one random rank, -1 hand size
    SPECTRAL_ECTOPLASM,  // Negative on a random Joker without edition, -1 hand size
    SPECTRAL_IMMOLATE,   // Destroys 5 random cards in hand, +$20
    SPECTRAL_ANKH,       // Copies a random Joker, destroys the others
    SPECTRAL_DEJA_VU,    // Red seal on 1 card
    SPECTRAL_HEX,        // Polychrome on a random Joker without edition, destroys the others
    SPECTRAL_TRANCE,     // Blue seal on 1 card
    SPECTRAL_MEDIUM,     // Purple seal on 1 card
    SPECTRAL_CRYPTID,    // 2 copies of 1 card are added to the hand
    SPECTRAL_SOUL,       // Needs Legendary Jokers: not implemented
    SPECTRAL_BLACK_HOLE, // Every poker hand gains a level
    SPECTRAL_TYPE_MAX
};

/**
 * @brief 1 since the Editions exist (edition_rules.h) and consumable.c applies them.
 *        Aura, Ectoplasm and Hex don't appear and can't be used while it is 0.
 */
#ifndef SPECTRAL_EDITIONS_AVAILABLE
#define SPECTRAL_EDITIONS_AVAILABLE 1
#endif

#define SPECTRAL_MAX_TARGETS         1
#define SPECTRAL_IMMOLATE_NB_CARDS   5
#define SPECTRAL_IMMOLATE_MONEY      20
#define SPECTRAL_CRYPTID_NB_COPIES   2
#define SPECTRAL_OUIJA_HAND_SIZE_MIN 1
#define SPECTRAL_MAX_CREATED_CARDS   4 // Incantation
#define SPECTRAL_BLACK_HOLE_PERCENT_X10 3 // 0.3% per pack choice, as in Balatro

/**
 * @brief Where a Spectral card acts.
 */
enum SpectralScope
{
    SPECTRAL_SCOPE_ANYWHERE,      // Jokers and money, usable in the Shop and in a round
    SPECTRAL_SCOPE_SELECTED_CARD, // The card selected in the hand, needs a round
    SPECTRAL_SCOPE_HAND,          // Cards of the hand chosen at random or all of them, needs a round
};

static inline enum SpectralScope spectral_get_scope(enum SpectralType spectral)
{
    switch (spectral)
    {
        case SPECTRAL_TALISMAN:
        case SPECTRAL_AURA:
        case SPECTRAL_DEJA_VU:
        case SPECTRAL_TRANCE:
        case SPECTRAL_MEDIUM:
        case SPECTRAL_CRYPTID:
            return SPECTRAL_SCOPE_SELECTED_CARD;
        case SPECTRAL_FAMILIAR:
        case SPECTRAL_GRIM:
        case SPECTRAL_INCANTATION:
        case SPECTRAL_SIGIL:
        case SPECTRAL_OUIJA:
        case SPECTRAL_IMMOLATE:
            return SPECTRAL_SCOPE_HAND;
        default:
            return SPECTRAL_SCOPE_ANYWHERE;
    }
}

/**
 * @brief Whether the Spectral card needs Editions to work.
 */
static inline bool spectral_needs_editions(enum SpectralType spectral)
{
    return spectral == SPECTRAL_AURA || spectral == SPECTRAL_ECTOPLASM || spectral == SPECTRAL_HEX;
}

/**
 * @brief Whether the Spectral card works at all (it can be created and used).
 */
static inline bool spectral_is_implemented(enum SpectralType spectral)
{
    if (spectral < 0 || spectral >= SPECTRAL_TYPE_MAX || spectral == SPECTRAL_SOUL)
        return false;
    return SPECTRAL_EDITIONS_AVAILABLE || !spectral_needs_editions(spectral);
}

/**
 * @brief Whether the Spectral card is in the common pool of Spectral Packs and Omen Globe.
 *        Black Hole has its own small chance and The Soul is not implemented.
 */
static inline bool spectral_is_in_common_pool(enum SpectralType spectral)
{
    return spectral_is_implemented(spectral) && spectral != SPECTRAL_BLACK_HOLE;
}

/**
 * @brief Number of Spectral cards in the common pool.
 */
static inline int spectral_common_pool_size(void)
{
    int nb = 0;
    for (int spectral = 0; spectral < SPECTRAL_TYPE_MAX; spectral++)
        nb += spectral_is_in_common_pool(spectral);
    return nb;
}

/**
 * @brief The @p nth (from 0) Spectral card of the common pool, SPECTRAL_TYPE_MAX if out of range.
 */
static inline enum SpectralType spectral_common_pool_get(int nth)
{
    for (int spectral = 0; spectral < SPECTRAL_TYPE_MAX; spectral++)
    {
        if (spectral_is_in_common_pool(spectral) && nth-- == 0)
            return spectral;
    }
    return SPECTRAL_TYPE_MAX;
}

/**
 * @brief The seal a Spectral card gives, CARD_SEAL_NONE if it doesn't give one.
 */
static inline enum CardSeal spectral_get_seal(enum SpectralType spectral)
{
    switch (spectral)
    {
        case SPECTRAL_TALISMAN:
            return CARD_SEAL_GOLD;
        case SPECTRAL_DEJA_VU:
            return CARD_SEAL_RED;
        case SPECTRAL_TRANCE:
            return CARD_SEAL_BLUE;
        case SPECTRAL_MEDIUM:
            return CARD_SEAL_PURPLE;
        default:
            return CARD_SEAL_NONE;
    }
}

/**
 * @brief Cards a Spectral card adds to the hand: Familiar 3, Grim 2, Incantation 4, Cryptid 2.
 */
static inline int spectral_get_nb_created_cards(enum SpectralType spectral)
{
    switch (spectral)
    {
        case SPECTRAL_FAMILIAR:
            return 3;
        case SPECTRAL_GRIM:
            return 2;
        case SPECTRAL_INCANTATION:
            return 4;
        case SPECTRAL_CRYPTID:
            return SPECTRAL_CRYPTID_NB_COPIES;
        default:
            return 0;
    }
}

/**
 * @brief Cards destroyed in the hand: Familiar, Grim and Incantation 1, Immolate 5.
 */
static inline int spectral_get_nb_destroyed_cards(enum SpectralType spectral)
{
    switch (spectral)
    {
        case SPECTRAL_FAMILIAR:
        case SPECTRAL_GRIM:
        case SPECTRAL_INCANTATION:
            return 1;
        case SPECTRAL_IMMOLATE:
            return SPECTRAL_IMMOLATE_NB_CARDS;
        default:
            return 0;
    }
}

/**
 * @brief A random enhancement from the 8 real ones, @p r any random number.
 */
static inline enum CardEnhancement spectral_random_enhancement(uint32_t r)
{
    return (enum CardEnhancement)(CARD_ENHANCEMENT_BONUS + r % (CARD_ENHANCEMENT_MAX - 1));
}

/**
 * @brief Builds a card created by Familiar (face card), Grim (Ace) or Incantation (2 to 10),
 *        always enhanced. Stone cards would have no rank, so they are replaced by Bonus.
 *
 * @param r_rank, r_suit, r_enhancement random numbers
 */
static inline Card spectral_make_card(
    enum SpectralType spectral,
    uint32_t r_rank,
    uint32_t r_suit,
    uint32_t r_enhancement
)
{
    Card card = {0};
    switch (spectral)
    {
        case SPECTRAL_FAMILIAR:
            card.rank = JACK + r_rank % 3;
            break;
        case SPECTRAL_GRIM:
            card.rank = ACE;
            break;
        default:
            card.rank = TWO + r_rank % 9;
            break;
    }
    card.suit = r_suit % NUM_SUITS;
    card.enhancement = spectral_random_enhancement(r_enhancement);
    return card;
}

/**
 * @brief Cryptid: @p dst becomes a copy of @p src (suit, rank, enhancement, seal, Hiker chips).
 *        Runtime flags are not copied.
 */
static inline Card spectral_make_copy(const Card* src)
{
    Card card = {0};
    card_copy_from(&card, src);
    return card;
}

/**
 * @brief Gives a seal to a card, replacing the one it had.
 */
static inline void card_set_seal(Card* card, enum CardSeal seal)
{
    if (seal > CARD_SEAL_NONE && seal < CARD_SEAL_MAX)
        card->seal = seal;
}

/**
 * @brief Sets the rank of a card. Stone cards have no rank and are left as they are (Ouija).
 */
static inline void card_set_rank(Card* card, uint8_t rank)
{
    if (rank < NUM_RANKS && card->enhancement != CARD_ENHANCEMENT_STONE)
        card->rank = rank;
}

/**
 * @brief Hand size after Ouija / Ectoplasm: one less, never below SPECTRAL_OUIJA_HAND_SIZE_MIN.
 */
static inline int spectral_hand_size_minus_one(int hand_size)
{
    return hand_size > SPECTRAL_OUIJA_HAND_SIZE_MIN ? hand_size - 1 : SPECTRAL_OUIJA_HAND_SIZE_MIN;
}

/**
 * @brief Edition given by Aura, weighted like the Shop without Negative: Foil 20, Holographic 14,
 *        Polychrome 3 (out of 37).
 */
static inline enum Edition spectral_aura_edition(uint32_t roll)
{
    uint32_t value = roll % (EDITION_CHANCE_FOIL + EDITION_CHANCE_HOLOGRAPHIC + EDITION_CHANCE_POLYCHROME);
    if (value < EDITION_CHANCE_FOIL)
        return EDITION_FOIL;
    if (value < EDITION_CHANCE_FOIL + EDITION_CHANCE_HOLOGRAPHIC)
        return EDITION_HOLOGRAPHIC;
    return EDITION_POLYCHROME;
}

/**
 * @brief Whether Black Hole replaces a Pack choice, @p r any random number (0.3%).
 */
static inline bool spectral_pack_rolls_black_hole(uint32_t r)
{
    return r % 1000 < SPECTRAL_BLACK_HOLE_PERCENT_X10;
}

/**
 * @brief Picks @p nb distinct indexes among 0..n-1 (a partial Fisher-Yates shuffle).
 *
 * @param n number of candidates, at most 32
 * @param nb how many to pick, clamped to n
 * @param next gives a random number, called nb times
 * @param ctx passed to @p next
 * @param out receives the picked indexes
 *
 * @return how many were picked
 */
static inline int spectral_pick_distinct(
    int n,
    int nb,
    uint32_t (*next)(void* ctx),
    void* ctx,
    int out[]
)
{
    int idx[32];
    if (n > 32)
        n = 32;
    if (nb > n)
        nb = n;
    for (int i = 0; i < n; i++)
        idx[i] = i;
    for (int i = 0; i < nb; i++)
    {
        int j = i + (int)(next(ctx) % (uint32_t)(n - i));
        int tmp = idx[i];
        idx[i] = idx[j];
        idx[j] = tmp;
        out[i] = idx[i];
    }
    return nb;
}

// Shop odds

/**
 * @brief Omen Globe: chance in percent that an Arcana Pack choice is a Spectral card.
 */
#define OMEN_GLOBE_SPECTRAL_PERCENT 20

static inline bool pack_choice_is_spectral(bool omen_globe_owned, uint32_t r)
{
    return omen_globe_owned && r % 100 < OMEN_GLOBE_SPECTRAL_PERCENT;
}

/**
 * @brief The most played poker hand, for the Telescope: ties go to the weakest hand, as in the
 *        original game. A Royal Flush counts as a Straight Flush. HIGH_CARD if nothing was played.
 *
 * @param nb_played nb_played_hands, indexed by `hand_type - 1`
 */
static inline enum HandType most_played_hand(const uint32_t nb_played[HAND_TYPE_MAX])
{
    enum HandType best = HIGH_CARD;
    uint32_t best_count = 0;
    for (int hand_type = HIGH_CARD; hand_type <= HAND_TYPE_MAX; hand_type++)
    {
        uint32_t count = nb_played[hand_type - 1];
        if (hand_type == STRAIGHT_FLUSH)
            count += nb_played[ROYAL_FLUSH - 1];
        if (hand_type == ROYAL_FLUSH)
            continue;
        if (count > best_count)
        {
            best = (enum HandType)hand_type;
            best_count = count;
        }
    }
    return best;
}

/**
 * @brief Weight of Tarots in the Shop relative to Planets, in tenths (Tarot Merchant x2.4,
 *        Tarot Tycoon x8, as in the original game).
 */
#define TAROT_SHOP_WEIGHT_X10          40
#define TAROT_MERCHANT_SHOP_WEIGHT_X10 96
#define TAROT_TYCOON_SHOP_WEIGHT_X10   320

/**
 * @brief Whether a Shop consumable slot holds a Tarot rather than a Planet.
 *
 * With equal weights it is a coin flip (the rolls of a run without Vouchers don't change).
 *
 * @param tarot_weight_x10, planet_weight_x10 weights in tenths
 * @param r random number
 */
static inline bool shop_consumable_is_tarot(int tarot_weight_x10, int planet_weight_x10, uint32_t r)
{
    if (tarot_weight_x10 == planet_weight_x10)
        return r % 2 == 0;
    return r % (uint32_t)(tarot_weight_x10 + planet_weight_x10) < (uint32_t)tarot_weight_x10;
}

#endif // SPECTRAL_RULES_H
