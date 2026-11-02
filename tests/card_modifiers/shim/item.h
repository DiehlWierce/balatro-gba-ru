/**
 * @file item.h
 *
 * @brief Host stand-in for include/item.h: the position fields and acquire API used by
 *        consumables. The sprite and graphics code is not part of the host tests.
 */
#ifndef TEST_ITEM_H
#define TEST_ITEM_H

#include "tonc.h"

typedef struct Item
{
    FIXED x, y;
    FIXED tx, ty;
} Item;

typedef struct ItemDescSubtypeInfo
{
    u16 main_color;
    u16 shadow_color;
    const char* name_str;
} ItemDescSubtypeInfo;

bool item_can_acquire(Item* item);
void item_acquire(Item* item);

#endif // TEST_ITEM_H
