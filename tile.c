#include "common.h"
#include "tile.h"

u8 tile_walkable(u8 type)
{
	switch(type)
	{
		default:
			return 0;
		case TILE_DUMMY:
		case TILE_FLOOR:
			return 1;
	}
}
