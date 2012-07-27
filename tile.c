#include "common.h"
#include "tile.h"

const u16 tile_preview_data[TILE_TYPES*2] = {
	0,	0,
	176,	8,
	178,	7+(8*16),
	177,	15+(7*16)
};

char* names[TILE_TYPES] = {
	"Dummy",
	"Floor",
	"Wall",
	"Roof"
};

inline char* tile_get_name(u8 type)
{
	if(type>=TILE_TYPES) return 0;
	return names[type];
}

inline u16 tile_get_preview_char(u8 type)
{
	if(type>=TILE_TYPES) return 0;
	return tile_preview_data[type*2];
}

inline u8 tile_get_preview_color(u8 type)
{
	if(type>=TILE_TYPES) return 0;
	return (u8)tile_preview_data[type*2+1];
}

inline u8 tile_overlay(tile_t tile)
{
	switch(tile.type)
	{
		default:
			return 0;
		case TILE_ROOF:
			return 1;
	}
}

inline u8 tile_walkable(tile_t tile)
{
	switch(tile.type)
	{
		default:
			return 0;
		case TILE_DUMMY:
		case TILE_FLOOR:
		case TILE_ROOF:
			return 1;
	}
}
