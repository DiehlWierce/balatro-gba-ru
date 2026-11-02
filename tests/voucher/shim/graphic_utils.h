/**
 * @file graphic_utils.h
 *
 * @brief Host stand-in for include/graphic_utils.h (GBA drawing, not used by the host tests).
 */
#ifndef TEST_GRAPHIC_UTILS_H
#define TEST_GRAPHIC_UTILS_H

#include "tonc.h"

// Text tags copied from include/graphic_utils.h
#define TTE_GREEN_TAG "#{cx:0x6000}"
#define TTE_PURPLE_TAG "#{cx:0x7000}"
#define TTE_DARK_GREEN_TAG "#{cx:0x8000}"
#define TTE_DARK_BLUE_TAG "#{cx:0x9000}"
#define TTE_BLACK_TAG "#{cx:0xA000}"
#define TTE_YELLOW_TAG "#{cx:0xB000}"
#define TTE_BLUE_TAG "#{cx:0xC000}"
#define TTE_RED_TAG "#{cx:0xD000}"
#define TTE_WHITE_TAG "#{cx:0xE000}"
#define TTE_DIAMOND_TAG TTE_YELLOW_TAG "Diamond "
#define TTE_HEART_TAG TTE_RED_TAG "Hearts "
#define TTE_SPADE_TAG TTE_DARK_BLUE_TAG "Spade "
#define TTE_CLUB_TAG TTE_DARK_GREEN_TAG "Club "

#endif // TEST_GRAPHIC_UTILS_H
