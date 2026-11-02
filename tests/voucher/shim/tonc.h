/**
 * @file tonc.h
 *
 * @brief Host stand-in for libtonc (GBA): only the scalar types the game headers need.
 */
#ifndef TEST_TONC_H
#define TEST_TONC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef s32 FIXED;

#define INLINE static inline

typedef struct Rect
{
    s16 x0, y0, x1, y1;
} Rect;

#endif // TEST_TONC_H
