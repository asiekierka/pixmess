#ifndef _TILE_H_
#define _TILE_H_

#include "common.h"

#define TILE_ALLOWED_BG(a,b) ((((b)>>16)>>(a))&1)
#define TILE_ALLOWED_FG(a,b) (((b)>>(a))&1)

#define TILE_ACTIVE 1
#define TILE_WALKABLE 2
#define TILE_TRANSPARENT 4
#define TILE_OVERLAY 8

void tile_set_name(u8 type, char* name);
void tile_set_flag(u8 type, u32 flag);

inline char* tile_get_name(u8 type);
inline u16 tile_get_preview_char(u8 type);
inline u8 tile_get_preview_color(u8 type);
u16* tile_get_allowed_chars(tile_t tile, u16* length);
inline u32 tile_get_allowed_colors(tile_t tile);

inline tile_t tile_dummy(void);

inline u8 tile_overlay(tile_t tile);
inline u8 tile_walkable(tile_t tile);
inline u8 tile_stackable(u8 type, u8 utype);
inline u8 tile_active(tile_t tile);
inline u8 tile_transparent(tile_t tile);

enum
{
	TILE_DUMMY = 0,
	TILE_FLOOR,
	TILE_WALL,
	TILE_ROOF,
	TILE_WIRE,
	TILE_PNAND,
	TILE_CROSSER,
	TILE_PLATE,

	TILE_TYPES
};

#endif /* _TILE_H_ */
