#include "common.h"
#include "tile.h"

tileinfo_t *tiles[TILE_MAX_TYPES];

tileinfo_t *tileinfo_new(u8 type, char* name)
{
	tileinfo_t *ti = malloc(sizeof(tileinfo_t));
	ti->name = name;
	ti->stack_data = malloc(sizeof(u8)*256);
	ti->flags = 0;
	ti->preview_char = 63; // '?'
	ti->preview_color = TILE_COLOR(12,10); // green on red
	ti->ach_len = 256;
	ti->allowed_chars = malloc(sizeof(u16)*256);
	ti->allowed_colors = 0xFFFFFFFF;

	int i;
	for(i = 0; i < 256; i++)
	{
		ti->allowed_chars[i] = i;
 		ti->stack_data[i] = 0;
	}

	tiles[type] = ti;
	return ti;
}

void tileinfo_free(tileinfo_t* ti)
{
	free(ti->stack_data);
	free(ti->name);
	free(ti->allowed_chars);
	free(ti);
}

void tileinfo_set_stackable(u8 type, u8 type2)
{
	if(tiles[type])
		tiles[type]->stack_data[type2] = 1;
}

u8 tileinfo_is_set(u8 type)
{
	return tiles[type]?1:0;
}

void tileinfo_set_stackable_va(u8 type, u8 amount, ...)
{
	va_list args;
	va_start(args, amount);
	int i;
	for(i=1;i<amount;i++)
		tileinfo_set_stackable(type, va_arg(args, int));
	va_end(args);
}

u16 tileinfo_types()
{
	u16 i;
	for(i=0;i<256;i++)
		if(!tiles[i]) return i;
	return 256;
}

tileinfo_t* tileinfo_get(u8 type)
{
	return tiles[type];
}

void tileinfo_set_flag(u8 type, u32 flag)
{
	if(tiles[type])
		tiles[type]->flags |= flag;
}

void tileinfo_clear_flag(u8 type, u32 flag)
{
	if(tiles[type])
		tiles[type]->flags &= ~flag;
}

inline u8 tile_overlay(tile_t tile)
{
	if(tiles[tile.type])
		return (tiles[tile.type]->flags & TILE_OVERLAY)?1:0;
	return 0;
}

inline u8 tile_walkable(tile_t tile)
{
	if(tiles[tile.type])
		return (tiles[tile.type]->flags & TILE_WALKABLE)?1:0;
	return 0;
}


inline u8 tile_active(tile_t tile)
{
	if(tiles[tile.type])
		return (tiles[tile.type]->flags & TILE_ACTIVE)?1:0;
	return 0;
}

inline u8 tile_transparent(tile_t tile)
{
	if(tiles[tile.type])
		return (tiles[tile.type]->flags & TILE_TRANSPARENT)?1:0;
	return 0;
}

void tileinfo_set_allowed_colors(u8 type, u32 allowed_colors)
{
	if(tiles[type])
		tiles[type]->allowed_colors = allowed_colors;
}

void tileinfo_set_allowed_chars(u8 type, u16 len, const u16* allowed_chars)
{
	if(tiles[type])
	{
		if(tiles[type]->allowed_chars != NULL) free(tiles[type]->allowed_chars);
		tiles[type]->allowed_chars = malloc(sizeof(u16)*len);
		memcpy(tiles[type]->allowed_chars,allowed_chars,sizeof(u16)*len);
		tiles[type]->ach_len = len;
	}	
}

inline u32 tile_get_allowed_colors(tile_t tile)
{
	if(tiles[tile.type])
		return tiles[tile.type]->allowed_colors;
	return 0;
}

u16* tile_get_allowed_chars(tile_t tile, u16* length)
{
	*length = 0;
	if(!tiles[tile.type]) return NULL;
	*length = tiles[tile.type]->ach_len;
	return tiles[tile.type]->allowed_chars;
}

inline u8 tile_stackable(u8 type, u8 utype)
{
	if(!tiles[type]) return 0;
	return tiles[type]->stack_data[utype];
}

inline char* tile_get_name(u8 type)
{
	if(tiles[type]) return tiles[type]->name;
	return "Undefined";
}

inline tile_t tile_dummy(void)
{
	tile_t tile;
	tile.type = 0;
	tile.chr = 0x20;
	tile.col = 0x07;
	tile.data = NULL;
	return tile;
}

inline u16 tile_get_preview_char(u8 type)
{
	if(!tiles[type]) return 0;
	return tiles[type]->preview_char;
}

inline u8 tile_get_preview_color(u8 type)
{
	if(!tiles[type]) return 0;
	return tiles[type]->preview_color;
}

void tileinfo_set_preview_data(u8 type, u16 pc, u8 pcol)
{
	if(!tiles[type]) return;
	tiles[type]->preview_char = pc;
	tiles[type]->preview_color = pcol;
}

