#include "common.h"
#include "map.h"
#include "network.h"

// TODO these defintions should have a file later on, maybe

layer_t *layers[LAYER_SIZE];
u8 layer_set[LAYER_SIZE];

void layer_free(layer_t *layer)
{
	int i;
	tile_t *t, *t2;
	
	for(i = 0; i < layer->w*layer->h; i++)
	{
		// free "under" stuff
		for(t = layer->tiles[i].under; t != NULL; t = t2)
		{
			t2 = t->under;
			if(t->data != NULL)
				free(t->data);
			free(t);
		}
		
		t = &(layer->tiles[i]);
		
		if(t->data != NULL)
			free(t->data);
	}
	
	free(layer);
}

layer_t *layer_new(void)
{
	layer_t *a = (layer_t *)malloc(sizeof(layer_t));
	a->w = LAYER_WIDTH;
	a->h = LAYER_HEIGHT;
	a->tiles = (tile_t *)malloc(sizeof(tile_t)*LAYER_WIDTH*LAYER_HEIGHT);
	int i;
	for(i=0;i<LAYER_WIDTH*LAYER_HEIGHT;i++)
	{
		int randomizer = rand()%64;
		if(randomizer == 42 || randomizer == 24)
		{
			a->tiles[i].type = TILE_WALL;
			// Hacks to skip empties.
			a->tiles[i].chr = 32;
			while(a->tiles[i].chr == 32)
				a->tiles[i].chr = (u16)(rand()%254)+1;
			a->tiles[i].col = (1+(rand()%15))+((rand()%8)<<4);
		}
		else if(randomizer == 17)
		{
			a->tiles[i].type = TILE_ROOF;
			// Hacks to skip empties.
			a->tiles[i].chr = 32;
			while(a->tiles[i].chr == 32)
				a->tiles[i].chr = (u16)(rand()%254)+1;
			a->tiles[i].col = 240+(rand()%15);
		}
		else
		{
			a->tiles[i].type = TILE_FLOOR;
			a->tiles[i].chr = 0;
			a->tiles[i].col = 0;
		}
		a->tiles[i].under = NULL;
		a->tiles[i].data = NULL;
	}
	return a;
};

layer_t *layer_dummy_request(s32 x, s32 y)
{
	layer_t *a = layer_new();
	a->x = x; a->y = y;
	return a;
};

void map_init(void)
{
	u8 i;
	for(i=0;i<LAYER_SIZE;i++)
		layer_set[i]=LAYER_UNALLOC;
}

void layer_unload(int i)
{
	net_report_unlayer(layers[i]->x,layers[i]->y,i);
	layer_free(layers[i]);
	layer_set[i] = LAYER_UNALLOC;
}

layer_t *map_get_existing_layer(s32 x, s32 y)
{
	u8 i;
	// Finding existing layer
	for(i=0;i<LAYER_SIZE;i++) {
		if(layer_set[i]==LAYER_USED && layers[i]->x==x && layers[i]->y==y)
			return layers[i];
	}
	return NULL;
}

layer_t *map_get_empty_layer(s32 x, s32 y)
{
	u8 i;
	// Create empty layer
	for(i=0;i<LAYER_SIZE;i++) {
		if(layer_set[i]==LAYER_UNUSED)
			layer_unload(i);
		if(layer_set[i]==LAYER_REQUESTED)
			layer_unload(i);
		if(layer_set[i]==LAYER_UNALLOC)
		{
			net_report_layer(x,y,i);
			
			// TODO: factor this code out --GM
			layers[i] = layer_dummy_request(x,y);
			
			if(layers[i] == NULL)
			{
				layer_set[i] = LAYER_REQUESTED;
			} else {
				if(layers[i]->x != x || layers[i]->y != y)
					return NULL;
				layer_set[i] = LAYER_USED;
				return layers[i];
			}
		}
	}
	return NULL;
}

void map_layer_set_unused(s32 x, s32 y)
{
	u8 i;
	for(i=0;i<LAYER_SIZE;i++) {
		if(layer_set[i]!=LAYER_UNALLOC && layers[i]->x==x && layers[i]->y==y)
		{
			layer_set[i] = LAYER_UNUSED;
			return;
		}
	}
}

void map_layer_set_used(s32 x, s32 y)
{
	u8 i;
	for(i=0;i<LAYER_SIZE;i++) {
		if(layer_set[i]!=LAYER_UNALLOC && layers[i]->x==x && layers[i]->y==y)
		{
			layer_set[i] = LAYER_USED;
			return;
		}
	}
}

void map_layer_set_unused_all(void)
{
	u8 i;
	for(i=0;i<LAYER_SIZE;i++) 
		if(layer_set[i]!=LAYER_UNALLOC)
			layer_set[i] = LAYER_UNUSED;
}

void map_layer_set_used_rendered(s32 topx, s32 topy)
{
	s32 chunk_x = topx/LAYER_WIDTH;
	s32 chunk_y = topy/LAYER_HEIGHT;
	map_layer_set_unused_all();
	map_layer_set_used(chunk_x,chunk_y);
	map_layer_set_used(chunk_x+1,chunk_y);
	map_layer_set_used(chunk_x,chunk_y+1);
	map_layer_set_used(chunk_x+1,chunk_y+1);
}

tile_t *layer_get_tile_ref(u8 x, u8 y, layer_t *layer)
{
	return &(layer->tiles[y*LAYER_WIDTH+x]);
}

tile_t layer_get_tile(u8 x, u8 y, layer_t *layer)
{
	return *layer_get_tile_ref(x, y, layer);
}

void layer_set_tile(u8 x, u8 y, tile_t tile, layer_t *layer)
{
	// NOTE: preserve "under" reference!
	tile_t *mt = layer_get_tile_ref(x, y, layer);
	tile_t *under = mt->under;
	*mt = tile;
	mt->under = under;
}

void layer_push_tile(u8 x, u8 y, tile_t tile, layer_t *layer)
{
	// get tile reference
	tile_t *old = layer_get_tile_ref(x, y, layer);
	
	// if we're not stacking, just use layer_set_tile
	if(old->type == TILE_DUMMY || old->type == tile.type)
		return layer_set_tile(x, y, tile, layer);

	// check if we can stack, otherwise pop and try again.
	if(!tile_stackable(tile.type,old->type))
	{
		layer_pop_tile(x,y,layer);
		return layer_push_tile(x,y,tile,layer);
	}

	// duplicate the top tile if necessary
	tile_t *new = malloc(sizeof(tile_t));
	
	// if the malloc fails, front-dismount the unicycle gracefully
	// (in other words, drop everything and leave)
	if(new == NULL)
	{
		fprintf(stderr, "FATAL: COULD NOT ALLOCATE MEMORY FOR NEW TILE!\n");
		perror("layer_push_tile");
		exit(44); // 4 to indicate death, 44 to indicate we're DYING HORRIBLY
	}
	
	// ok, copy old tile across to, *ahem*, "new"
	memcpy(new, old, sizeof(tile_t));
	
	// now copy "tile" across to, *ahem*, "old"
	memcpy(old, &tile, sizeof(tile_t));
	
	// set the "under" field
	old->under = new;
}

void layer_pop_tile(u8 x, u8 y, layer_t *layer)
{
	tile_t *t = layer_get_tile_ref(x, y, layer);
	tile_t *under = t->under;
	if(under == NULL)
	{
		t->type = TILE_DUMMY;
		t->chr = 0x20;
		t->col = 0x07;
		if(t->data != NULL)
			free(t->data);
		t->data = NULL;
	} else {
		memcpy(t, under, sizeof(tile_t));
		free(under);
	}
}

tile_t map_get_tile(s32 x, s32 y)
{
	s32 chunk_x = x/LAYER_WIDTH;
	s32 chunk_y = y/LAYER_HEIGHT;
	layer_t *chunk = map_get_existing_layer(chunk_x,chunk_y);
	if(chunk == NULL) return tile_dummy();
	return layer_get_tile(absmod(x,LAYER_WIDTH),absmod(y,LAYER_HEIGHT),chunk);
}

void map_set_tile(s32 x, s32 y, tile_t tile)
{
	s32 chunk_x = x/LAYER_WIDTH;
	s32 chunk_y = y/LAYER_HEIGHT;
	layer_t *chunk = map_get_existing_layer(chunk_x,chunk_y);
	if(chunk == NULL) return;
	layer_set_tile(absmod(x,LAYER_WIDTH),absmod(y,LAYER_HEIGHT),tile,chunk);
}

void map_push_tile(s32 x, s32 y, tile_t tile)
{
	s32 chunk_x = x/LAYER_WIDTH;
	s32 chunk_y = y/LAYER_HEIGHT;
	layer_t *chunk = map_get_existing_layer(chunk_x,chunk_y);
	if(chunk == NULL) return;
	layer_push_tile(absmod(x,LAYER_WIDTH),absmod(y,LAYER_HEIGHT),tile,chunk);
}

void map_pop_tile(s32 x, s32 y)
{
	s32 chunk_x = x/LAYER_WIDTH;
	s32 chunk_y = y/LAYER_HEIGHT;
	layer_t *chunk = map_get_existing_layer(chunk_x,chunk_y);
	if(chunk == NULL) return;
	layer_pop_tile(absmod(x,LAYER_WIDTH),absmod(y,LAYER_HEIGHT),chunk);
}

