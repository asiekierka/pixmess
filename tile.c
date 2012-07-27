#include "common.h"
#include "tile.h"

u8 tile_overlay(u8 type)
{
	switch(type)
	{
		default:
			return 0;
		case TILE_ROOF:
			return 1;
	}
}

u8 tile_walkable(u8 type)
{
	switch(type)
	{
		default:
			return 0;
		case TILE_DUMMY:
		case TILE_FLOOR:
		case TILE_ROOF:
			return 1;
	}
}
