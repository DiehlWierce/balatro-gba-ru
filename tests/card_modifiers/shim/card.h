/**
 * @file card.h
 *
 * @brief Host stand-in for include/card.h: the pure card rules plus the functions the
 *        modifiers read. card.c itself needs the GBA sprites and sound bank.
 */
#ifndef TEST_CARD_H
#define TEST_CARD_H

#include "card_rules.h"
#include "tonc.h"

int card_get_chips(Card* card);

#endif // TEST_CARD_H
