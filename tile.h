#ifndef _TILE_H_
#define _TILE_H_

#include "common.h"

#define TILE_ALLOWED_BG(a,b) ((((b)>>16)>>(a))&1)
#define TILE_ALLOWED_FG(a,b) (((b)>>(a))&1)
#define TILE_COLOR(fg,bg) ((fg)|((bg)<<4))

#define TILE_MAX_TYPES 256

#define TILE_ACTIVE 1
#define TILE_WALKABLE 2
#define TILE_TRANSPARENT 4
#define TILE_OVERLAY 8

tileinfo_t *tileinfo_new(u8 type, char* name);
void tileinfo_free(tileinfo_t* ti);
void tileinfo_set_stackable(u8 type, u8 type2);
void tileinfo_set_stackable_va(u8 type, u8 amount, ...);
void tileinfo_set_flag(u8 type, u32 flag);
void tileinfo_clear_flag(u8 type, u32 flag);
void tileinfo_set_allowed_colors(u8 type, u32 ac);
void tileinfo_set_allowed_chars(u8 type, u16 len, const u16* ac);
void tileinfo_set_preview_data(u8 type, u16 preview_char, u8 preview_color);

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
	TILE_PLATE
};

void register_tiles();

#endif /* _TILE_H_ */
