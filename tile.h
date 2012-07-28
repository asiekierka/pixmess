#ifndef _TILE_H_
#define _TILE_H_

#include "common.h"

inline char* tile_get_name(u8 type);
inline u16 tile_get_preview_chr(u8 type);
inline u8 tile_get_preview_color(u8 type);

inline u8 tile_overlay(tile_t tile);
inline u8 tile_walkable(tile_t tile);

enum
{
	TILE_DUMMY = 0,
	TILE_FLOOR,
	TILE_WALL,
	TILE_ROOF,
	
	TILE_TYPES
};

#endif /* _TILE_H_ */
