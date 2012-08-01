#include "common.h"
#include "client.h"
#include "map.h"
#include "network.h"

void client_load_chunk(map_t *map, s32 x, s32 y)
{
	s32 px = divneg(x,LAYER_WIDTH);
	s32 py = divneg(y,LAYER_HEIGHT);
	if(!map_get_existing_layer(map, px,py))
		map_get_net_layer(map, px,py);
}

void client_set_tile(map_t *map, s32 x, s32 y, tile_t tile)
{
	client_load_chunk(map, x, y);
	net_block_set(x, y, tile);
}

void client_set_tile_ext(map_t *map, s32 x, s32 y, u8 uidx, tile_t tile, int sendflag)
{
	client_load_chunk(map, x, y);
	if(sendflag)
		/* TODO! */;
	else
		map_set_tile_ext(map, x, y, uidx, tile);
}

void client_alloc_tile_data(map_t *map, s32 x, s32 y, u8 uidx, u16 datalen, int sendflag)
{
	client_load_chunk(map, x, y);
	if(sendflag)
		/* TODO! */;
	else
		map_alloc_tile_data(map, x, y, uidx, datalen);
}

void client_set_tile_data(map_t *map, s32 x, s32 y, u8 uidx, u8 datalen, u16 datapos, u8 *data, int sendflag)
{
	client_load_chunk(map, x, y);
	if(sendflag)
		/* TODO! */;
	else
		map_set_tile_data(map, x, y, uidx, datalen, datapos, data);
}

tile_t client_get_tile(map_t *map, s32 x, s32 y)
{
	client_load_chunk(map, x, y);
	return map_get_tile(map, x, y);
}

void client_push_tile(map_t *map, s32 x, s32 y, tile_t tile)
{
	client_load_chunk(map, x, y);
	net_block_push(x, y, tile);
}

void client_pop_tile(map_t *map, s32 x, s32 y)
{
	client_load_chunk(map, x, y);
	net_block_pop(x, y);
}

u8 client_get_next_update(map_t *map, int *lidx, s32 *x, s32 *y)
{
	return map_get_next_update(client_map, lidx, x, y);
}

void client_set_update(map_t *map, s32 x, s32 y)
{
	client_load_chunk(map, x, y);
	map_set_update(client_map, x,y);
}
