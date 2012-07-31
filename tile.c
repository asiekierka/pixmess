#include "common.h"
#include "tile.h"

const u8 tile_stack_data[TILE_TYPES*TILE_TYPES] = {
	0, 0, 0, 0, 0, 0, 0,	// Dummy
	1, 0, 0, 0, 0, 0, 0,	// Floor
	1, 1, 0, 0, 0, 0, 0,	// Wall
	1, 1, 1, 0, 1, 1, 1,	// Roof
	1, 1, 1, 0, 1, 1, 0,	// Wire
	1, 1, 1, 0, 1, 0, 0,	// P-NAND
	1, 1, 1, 0, 1, 0, 0	// Crosser
};

inline u8 tile_stackable(u8 type, u8 utype)
{
	if(type==utype) return 0;
	if(utype==TILE_DUMMY) return 1;
	return tile_stack_data[utype+(type*TILE_TYPES)];
}

inline u8 tile_active(tile_t tile) { return 0; }

u16* tile_get_allowed_chars(tile_t tile, u16* length)
{
	u16* chars = NULL;
	*length = 256;
	switch(tile.type)
	{
		case TILE_WIRE: {
			chars = (u16*)malloc(sizeof(u16)*1);
			chars[0] = 197;
			*length = 1;
			break; }
		case TILE_CROSSER: {
			chars = (u16*)malloc(sizeof(u16)*1);
			chars[0] = 206;
			*length = 1;
			break; }
		case TILE_PNAND: {
			chars = (u16*)malloc(sizeof(u16)*4);
			chars[0] = 24;
			chars[1] = 25;
			chars[2] = 26;
			chars[3] = 27;
			*length = 4;
			break; }
	}
	return chars;
}

const u16 tile_preview_data[TILE_TYPES*2] = {
	0,	0,
	176,	8,
	178,	7+(8*16),
	177,	15+(7*16),
	197,	12,
	25,	12,
	206,	11
};

char* names[TILE_TYPES] = {
	"Dummy",
	"Floor",
	"Wall",
	"Roof",
	"Wire",
	"P-NAND Gate",
	"Wire Crosser"
};

inline char* tile_get_name(u8 type)
{
	if(type>=TILE_TYPES) return 0;
	return names[type];
}

inline tile_t tile_dummy(void)
{
	tile_t tile;
	tile.type = TILE_DUMMY;
	tile.chr = 0x20;
	tile.col = 0x07;
	tile.data = NULL;
	return tile;
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
