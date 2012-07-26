#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdint.h>

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

typedef struct player { s32 x,y; u16 id; u8 col; u16 char; } player_t;
typedef struct tile { u8 type; u16 char; u8 col; void *data; } tile_t;
typedef struct layer { u32 x,y; u16 w,h; tile_t *tiles; } layer_t;

#endif /* _TYPES_H_ */
