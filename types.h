#ifndef _SFP_TYPES_H_
#define _SFP_TYPES_H_

#include <stdint.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

// Mental note: chr, not char
typedef struct player { s32 x,y; u16 id; u8 col; u16 chr; } player_t;
typedef struct tile { u8 type; u16 chr; u8 col; u8 *data; } tile_t;
typedef struct layer { s32 x,y; u16 w,h; tile_t *tiles; } layer_t;

enum { LAYER_UNALLOC = 0, LAYER_UNUSED = 1, LAYER_USED = 2 };

#endif /* _SFP_TYPES_H_ */
