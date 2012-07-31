#include "common.h"
#include "server.h"
#include "map.h"

// TODO DRY this out a bit --GM

void server_load_chunk(s32 x, s32 y)
{
	s32 px = divneg(x,LAYER_WIDTH);
	s32 py = divneg(y,LAYER_HEIGHT);
	if(!map_get_existing_layer(server_map,px,py))
		map_get_net_layer(server_map,px,py);
}

void server_set_tile(s32 x, s32 y, tile_t tile)
{
	server_load_chunk(x,y);
	map_set_tile(server_map,x,y,tile);
}

tile_t server_get_tile(s32 x, s32 y)
{
	server_load_chunk(x,y);
	return map_get_tile(server_map,x,y);
}

void server_push_tile(s32 x, s32 y, tile_t tile)
{
	server_load_chunk(x,y);
	map_push_tile(server_map,x,y,tile);
}

void server_pop_tile(s32 x, s32 y)
{
	server_load_chunk(x,y);
	map_pop_tile(server_map,x,y);
}

u8 server_get_next_update(s32 *x, s32 *y)
{
	return map_get_next_update(server_map,x,y);
}

void server_set_update(s32 x, s32 y)
{
	server_load_chunk(x,y);
	map_set_update(server_map,x,y);
}

