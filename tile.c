#include "common.h"
#include "tile.h"

// When adding a new tile type, modify those tables below v

// Vertical axis is the block being put.
// Horizontal axis is the one below it.
const u8 tile_stack_data[TILE_TYPES*TILE_TYPES] = {
//	Du Fl Wa Ro Wi P- Cr Pl (( On/What ))
	0, 0, 0, 0, 0, 0, 0, 0,	// Dummy
	1, 0, 0, 0, 0, 0, 0, 0,	// Floor
	1, 1, 0, 0, 0, 0, 0, 0,	// Wall
	1, 1, 1, 0, 1, 1, 1, 1,	// Roof
	1, 1, 1, 0, 0, 0, 0, 0,	// Wire
	1, 1, 1, 0, 0, 0, 0, 0,	// P-NAND
	1, 1, 1, 0, 0, 0, 0, 0,	// Crosser
	1, 1, 1, 0, 0, 0, 0, 0	// Plate
};

const u16 tile_preview_data[TILE_TYPES*2] = {
	0,	0,
	176,	8,
	178,	7+(8*16),
	177,	15+(7*16),
	197,	12,
	25,	12,
	206,	11,
	22,	7
};

char* names[TILE_TYPES] = {
	"Dummy",
	"Floor",
	"Wall",
	"Roof",
	"Wire",
	"P-NAND Gate",
	"Wire Crosser",
	"Plate"
};

u8 tile_active(tile_t tile)
{
	switch(tile.type)
	{
		default:
			return 0;
		case TILE_WIRE:
		case TILE_PNAND:
		case TILE_CROSSER:
		case TILE_PLATE:
			return 1;
	}
}

u8 tile_transparent(tile_t tile)
{
	switch(tile.type)
	{
		default:
			return 0;
		case TILE_ROOF:
		case TILE_WIRE:
		case TILE_CROSSER:
		case TILE_PLATE:
			return 1;
	}
}

inline u32 tile_get_allowed_colors(tile_t tile)
{
	switch(tile.type)
	{
		case TILE_WIRE:
			return 0x000100FE;
		case TILE_PNAND:
			return 0x0001FFFE;
		default:
			return 0xFFFFFFFF;
	}
}

u16* tile_get_allowed_chars(tile_t tile, u16* length)
{
	int i;
	static u16 chars[32768];
	*length = 0;
	switch(tile.type)
	{
		case TILE_WIRE: {
			chars[0] = 197;
			*length = 1;
			break; }
		case TILE_CROSSER: {
			chars[0] = 206;
			*length = 1;
			break; }
		case TILE_PNAND: {
			chars[0] = 24;
			chars[1] = 25;
			chars[2] = 26;
			chars[3] = 27;
			*length = 4;
			break; }
		case TILE_PLATE: {
			// Magical values!
			chars[0] = 254;
			chars[1] = 7;
			chars[2] = 8;
			chars[3] = 9;
			chars[4] = 10;
			chars[5] = 15;
			chars[6] = 16;
			chars[7] = 17;
			chars[8] = 22;
			chars[9] = 30;
			chars[10] = 31;
			chars[11] = 174;
			chars[12] = 175;
			chars[13] = 240;
			chars[14] = 244;
			chars[15] = 245;
			chars[16] = 247;
			chars[17] = 4;
			*length = 18;
			break; }
		default: {
			for(i = 0; i < 256; i++)
				chars[i] = i;
			*length = 256;
			break;
		}
	}
	return chars;
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
		case TILE_WIRE:
		case TILE_PLATE:
			return 1;
	}
}

// The below functions do not need modifications for new blocks IN MOST CASES.

inline u8 tile_stackable(u8 type, u8 utype)
{
	if(type==utype) return 0;
	if(utype==TILE_DUMMY) return 1;
	return tile_stack_data[utype+(type*TILE_TYPES)];
}

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
