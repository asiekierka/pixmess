#ifndef _TILE_H_
#define _TILE_H_

#include "common.h"

inline char* tile_get_name(u8 type);
inline u16 tile_get_preview_chr(u8 type);
inline u8 tile_get_preview_color(u8 type);

inline u8 tile_overlay(tile_t tile);
inline u8 tile_walkable(tile_t tile);

#define TILE_TYPES 4

enum { TILE_DUMMY = 0, TILE_FLOOR = 1, TILE_WALL = 2, TILE_ROOF = 3 };

#endif /* _TILE_H_ */
