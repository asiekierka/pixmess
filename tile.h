#ifndef _TILE_H_
#define _TILE_H_

#include "common.h"

inline char* tile_get_name(u8 type);
inline u16 tile_get_preview_char(u8 type);
inline u8 tile_get_preview_color(u8 type);

inline tile_t tile_dummy(void);

inline u8 tile_overlay(tile_t tile);
inline u8 tile_walkable(tile_t tile);
inline u8 tile_stackable(u8 type, u8 utype);
inline u8 tile_active(tile_t tile);
enum
{
	TILE_DUMMY = 0,
	TILE_FLOOR,
	TILE_WALL,
	TILE_ROOF,
	
	TILE_TYPES
};

#endif /* _TILE_H_ */
