#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdint.h>

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

// Mental note: chr, not char
typedef struct player { s32 x,y; u16 id; u8 col; u16 chr; } player_t;
typedef struct tile { u8 type; u16 chr; u8 col; u8 *data; } tile_t;
typedef struct layer { u32 x,y; u16 w,h; tile_t *tiles; } layer_t;

enum { LAYER_UNALLOC = 0, LAYER_UNUSED = 1, LAYER_USED = 2 };

enum { TILE_DUMMY = 0, TILE_FLOOR = 1, TILE_WALL = 2 };

#define PLAYER_SELF 0 // selfish ID, 0 because.
#define PLAYER_AMOUNT 256

#endif /* _TYPES_H_ */
