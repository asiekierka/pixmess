#include "common.h"
#include "map.h"

#define layer_request layer_dummy_request

// TODO these defintions should have a file later on, maybe
void net_report_layer(s32 x, s32 y, u8 position);

layer_t *layers[LAYER_SIZE];
u8 layer_set[LAYER_SIZE];

layer_t *layer_new(void)
{
	layer_t *a = (layer_t *)malloc(sizeof(layer_t));
	a->w = LAYER_WIDTH;
	a->h = LAYER_HEIGHT;
	a->tiles = (tile_t *)malloc(sizeof(tile_t)*LAYER_WIDTH*LAYER_HEIGHT);
	int i;
	for(i=0;i<LAYER_WIDTH*LAYER_HEIGHT;i++)
	{
		a->tiles[i].type = 0;
		a->tiles[i].chr = 0;
		a->tiles[i].col = 0;
	}
	return a;
};

layer_t *layer_dummy_request(s32 x, s32 y)
{
	layer_t *a = layer_new();
	a->x = x; a->y = y;
	return a;
};

layer_t *map_get_layer(s32 x, s32 y)
{
	u8 i;
	// Finding existing layer
	for(i=0;i<LAYER_SIZE;i++) {
		if(layer_set[i]==LAYER_USED && layers[i]->x==x && layers[i]->y==y)
			return layers[i];
	}
	// We didn't find that layer
	for(i=0;i<LAYER_SIZE;i++) {
		if(layer_set[i]==LAYER_UNALLOC)
		{
			layers[i] = (layer_t*)malloc(sizeof(layer_t));
			layer_set[i] = LAYER_UNUSED;
		}
		if(layer_set[i]==LAYER_UNUSED)
			if(layers[i] = layer_request(x,y))
			{
				if(layers[i]->x != x || layers[i]->y != y)
					return NULL;
				layer_set[i] = LAYER_USED;
				net_report_layer(x,y,i);
				return layers[i];
			}
	}
	// We really didn't
	return NULL;
}

void map_layer_set_unused(s32 x, s32 y)
{
	u8 i;
	for(i=0;i<LAYER_SIZE;i++) {
		if(layer_set[i]==LAYER_USED && layers[i]->x==x && layers[i]->y==y)
		{
			layer_set[i] = LAYER_UNUSED;
			return;
		}
	}
}

tile_t layer_get_tile(u8 x, u8 y, layer_t *layer)
{
	return layer->tiles[y*LAYER_WIDTH+x];
}

void layer_set_tile(u8 x, u8 y, tile_t tile, layer_t *layer)
{
	layer->tiles[y*LAYER_WIDTH+x] = tile;
}

tile_t map_get_tile(s32 x, s32 y)
{
	u32 chunk_x = x/LAYER_WIDTH;
	u32 chunk_y = y/LAYER_HEIGHT;
	layer_t *chunk = map_get_layer(chunk_x,chunk_y);
	return layer_get_tile((u8)(x%LAYER_WIDTH),(u8)(y%LAYER_HEIGHT),chunk);
}

void map_set_tile(s32 x, s32 y, tile_t tile)
{
	u32 chunk_x = x/LAYER_WIDTH;
	u32 chunk_y = y/LAYER_HEIGHT;
	layer_t *chunk = map_get_layer(chunk_x,chunk_y);
	layer_set_tile((u8)(x%LAYER_WIDTH),(u8)(y%LAYER_HEIGHT),tile,chunk);
}
