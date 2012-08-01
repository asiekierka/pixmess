#include "common.h"
#include "client.h"
#include "map.h"
#include "network.h"

void client_load_chunk(s32 x, s32 y)
{
	s32 px = divneg(x,LAYER_WIDTH);
	s32 py = divneg(y,LAYER_HEIGHT);
	if(!map_get_existing_layer(client_map, px,py))
		map_get_net_layer(client_map, px,py);
}

void client_set_tile(s32 x, s32 y, tile_t tile)
{
	client_load_chunk(x,y);
	//map_set_tile(client_map, x,y,tile);
	net_block_set(x, y, tile);
}

void client_set_tile_ext_local(s32 x, s32 y, u8 uidx, tile_t tile)
{
	client_load_chunk(x,y);
	map_set_tile_ext(client_map, x, y, uidx, tile);
}

void client_alloc_tile_data_local(s32 x, s32 y, u8 uidx, u16 datalen)
{
	client_load_chunk(x,y);
	map_alloc_tile_data(client_map, x, y, uidx, datalen);
}

void client_set_tile_data_local(s32 x, s32 y, u8 uidx, u8 datalen, u16 datapos, u8 *data)
{
	client_load_chunk(x,y);
	map_set_tile_data(client_map, x, y, uidx, datalen, datapos, data);
}

tile_t client_get_tile(s32 x, s32 y)
{
	client_load_chunk(x,y);
	return map_get_tile(client_map, x,y);
}

void client_push_tile(s32 x, s32 y, tile_t tile)
{
	client_load_chunk(x,y);
	//map_push_tile(client_map, x,y,tile);
	net_block_push(x, y, tile);
}

void client_pop_tile(s32 x, s32 y)
{
	client_load_chunk(x,y);
	//map_pop_tile(client_map, x,y);
	net_block_pop(x, y);
}

u8 client_get_next_update(int *lidx, s32 *x, s32 *y)
{
	return map_get_next_update(client_map, lidx, x, y);
}

void client_set_update(s32 x, s32 y)
{
	client_load_chunk(x,y);
	map_set_update(client_map, x,y);
}
