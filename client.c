#include "common.h"
#include "client.h"
#include "map.h"
#include "network.h"

void client_load_chunk(s32 x, s32 y)
{
	s32 px = divneg(x,LAYER_WIDTH);
	s32 py = divneg(y,LAYER_HEIGHT);
	if(!map_get_existing_layer(&client_map, px,py))
		map_get_net_layer(&client_map, px,py);
}

void client_set_tile(s32 x, s32 y, tile_t tile)
{
	client_load_chunk(x,y);
	//map_set_tile(&client_map, x,y,tile);
	net_block_set(x, y, tile);
}

tile_t client_get_tile(s32 x, s32 y)
{
	client_load_chunk(x,y);
	return map_get_tile(&client_map, x,y);
}
void client_push_tile(s32 x, s32 y, tile_t tile)
{
	client_load_chunk(x,y);
	//map_push_tile(&client_map, x,y,tile);
	net_block_push(x, y, tile);
}

void client_pop_tile(s32 x, s32 y)
{
	client_load_chunk(x,y);
	//map_pop_tile(&client_map, x,y);
	net_block_pop(x, y);
}
