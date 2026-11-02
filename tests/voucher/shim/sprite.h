/**
 * @file sprite.h
 *
 * @brief Host stand-in for include/sprite.h: the SpriteObject fields Shop items inherit.
 *        Sprites themselves are opaque here and their functions are stubbed in the test.
 */
#ifndef TEST_SPRITE_H
#define TEST_SPRITE_H

#include "tonc.h"

typedef struct Sprite Sprite;

typedef struct
{
    Sprite* sprite;
    FIXED tx, ty;
    FIXED x, y;
    FIXED vx, vy;
    FIXED tscale;
    FIXED scale;
    FIXED vscale;
    FIXED trotation;
    FIXED rotation;
    FIXED vrotation;
    bool focused;
} SpriteObject;

#endif // TEST_SPRITE_H
