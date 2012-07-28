#include "common.h"
#include "client.h"
#include "map.h"

void client_set_tile(s32 x, s32 y, tile_t tile)
{
	map_set_tile(x,y,tile);
}

void client_push_tile(s32 x, s32 y, tile_t tile)
{
	map_push_tile(x,y,tile);
}

void client_pop_tile(s32 x, s32 y)
{
	map_pop_tile(x,y);
}
