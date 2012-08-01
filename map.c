#include "client.h"
#include "common.h"
#include "map.h"
#include "network.h"
#include "server.h"

// TODO these defintions should have a file later on, maybe

map_t *client_map;
map_t *server_map;

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
	
	free(layer->updmask);
	free(layer->tiles);
	free(layer);
}

layer_t *layer_new(int w, int h, int template)
{
	if(w < 1 || h < 1 || w > 255 || h > 255)
	{
		fprintf(stderr, "ERROR: %i x %i are not valid layer dimensions\n", w, h);
		return NULL;
	}
	
	layer_t *a = (layer_t *)malloc(sizeof(layer_t));
	a->w = w;
	a->h = h;
	a->updmask = malloc(((w+31)/32)*h*4);
	a->new_updmask = malloc(((w+31)/32)*h*4);
	memset(a->updmask,0xFF,((w+31)/32)*h*4); // FIXME: check for widths not divisible by 32
	memset(a->new_updmask,0x00,((w+31)/32)*h*4);
	
	a->tiles = (tile_t *)malloc(sizeof(tile_t)*w*h);
	int i;
	
	switch(template)
	{
		default:
			fprintf(stderr,"EDOOFUS: invalid template; going with LAYER_TEMPLATE_EMPTY\n");
		case LAYER_TEMPLATE_EMPTY:
			for(i=0;i<w*h;i++)
			{
				a->tiles[i].type = TILE_FLOOR;
				a->tiles[i].chr = 0x20;
				a->tiles[i].col = 0x07;
				a->tiles[i].under = NULL;
				a->tiles[i].data = NULL;
				a->tiles[i].datalen = 0;
			};
			break;
		case LAYER_TEMPLATE_CLASSIC:
			for(i=0;i<w*h;i++)
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
					a->tiles[i].chr = 0x20;
					a->tiles[i].col = 0x07;
				}
				a->tiles[i].under = NULL;
				a->tiles[i].data = NULL;
				a->tiles[i].datalen = 0;
			}
			break;
	}
	return a;
};

u8 *layer_serialise(layer_t *layer, int *rawlen, int *cmplen)
{
	int i;
	u32 i1;
	u16 s1;
	
	// create a temp file
#ifdef WIN32
	FILE *fp = fopen("windows.sucks.tmp","wb");
#else
	FILE *fp = tmpfile();
#endif
	
	if(fp == NULL)
		return NULL;
	
	// write header
	i1 = 0x1A571ECE; fwrite(&i1, 4, 1, fp);
	s1 = LAYER_VERSION; fwrite(&s1, 2, 1, fp);
	fputc(layer->w, fp);
	fputc(layer->h, fp);
	
	// encode stream
	for(i = 0; i < layer->w*layer->h; i++)
	{
		tile_t *t = &(layer->tiles[i]);
		while(t != NULL)
		{
			u8 flags = 0;
			if(t->chr > 0xFF)
				flags |= MAP_FLAG_U16CHAR;
			if(t->under != NULL)
				flags |= MAP_FLAG_UNDER;
			if(t->data != NULL)
				flags |= MAP_FLAG_DATA;
			if(t->datalen > 0xFF)
				flags |= MAP_FLAG_EXT_DATALEN;
			
			// flags & type
			fputc(flags,fp);
			fputc(t->type,fp);
			
			// chr + col
			fputc(t->chr&0xFF, fp);
			if(flags&MAP_FLAG_U16CHAR)
				fputc(t->chr>>8, fp);
			fputc(t->col, fp);
			
			// data
			if(flags&MAP_FLAG_DATA)
			{
				fputc(t->datalen&0xFF, fp);
				if(flags&MAP_FLAG_EXT_DATALEN)
					fputc(t->datalen>>8, fp);
				fwrite(t->data, t->datalen, 1, fp);
			}
			// next tile
			t = t->under;
		}
	}
	
	// calculate size
	// (i personally doubt you'd get a >=2GB layer --GM)
	*rawlen = (int)ftell(fp);
	if(fseek(fp, 0, SEEK_SET) != 0)
	{
		fprintf(stderr, "ERROR: could not seek to start of temp file\n");
		perror("layer_serialise");
		fclose(fp);
		return NULL;
	}
	
	// construct buffer
	u8 *buf_raw = malloc(*rawlen);
	if(buf_raw == NULL)
	{
		fprintf(stderr, "FATAL: COULD NOT ALLOCATE\n");
		perror("layer_serialise");
		abort();
	}
	
	// very rare case of me checking the return value of fread --GM
#ifdef WIN32
	fread(buf_raw, *rawlen, 1, fp);
#else
	if(fread(buf_raw, *rawlen, 1, fp) != 1)
	{
		fprintf(stderr, "FATAL: YOUR OS SUCKS\n");
		perror("layer_serialise");
		abort(); // die badly - this should NEVER happen!
	}
#endif
	
	// construct compress buffer
	// length calculation is based on the max size of a "store" block
	// plus overhead, plus a little bit more breathing room too.
	*cmplen = (u32)((((u64)*rawlen)*(u64)32005)/(u64)32000)+1024;
	u8 *buf_cmp = malloc(*cmplen);
	
	// compress it!
	uLongf cmplen_zlib = *cmplen;
	int zerr = compress(buf_cmp, &cmplen_zlib, buf_raw, *rawlen);
	if(zerr != Z_OK)
	{
		fprintf(stderr, "ERROR: could not compress layer - %i\n", zerr);
		fclose(fp);
		free(buf_raw);
		free(buf_cmp);
		return NULL;
	}
	
	// set correct compression length;
	*cmplen = cmplen_zlib;
	
	// tidy up and return
	fclose(fp);
	free(buf_raw);
	
	return buf_cmp;
}

layer_t *layer_unserialise(u8 *buf_cmp, int rawlen, int cmplen)
{
	int i;
	
	// allocate buffer
	u8 *buf_raw = malloc(rawlen);
	if(buf_raw == NULL)
	{
		fprintf(stderr, "FATAL: COULD NOT ALLOCATE MEMORY FOR RAW BUFFER\n");
		perror("layer_unserialise");
		abort();
	}
	
	// decompress
	uLongf rawlen_zlib = (uLongf)rawlen;
	uLongf cmplen_zlib = (uLongf)cmplen;
	int zerr = uncompress((Bytef *)buf_raw, &rawlen_zlib, buf_cmp, cmplen_zlib);
	if(zerr != Z_OK)
	{
		fprintf(stderr, "ERROR: could not uncompress layer - %i\n", zerr);
		free(buf_raw);
		return NULL;
	}
	u8 *v = buf_raw;
	
	// load the header
	if(*(u32 *)v != (u32)0x1A571ECE)
	{
		fprintf(stderr, "ERROR: incorrect magic number %08X\n", *(u32 *)v);
		free(buf_raw);
		return NULL;
	}
	v += 4;
	
	int version = *(u16 *)v;
	v += 2;
	
	if(version < LAYER_LOWEST_VERSION || version > LAYER_VERSION)
	{
		fprintf(stderr, "ERROR: layer version %i unsupported, current version is %i\n",
			version, LAYER_VERSION);
		free(buf_raw);
		return NULL;
	}
	
	int w = *v++;
	int h = *v++;
	
	if(w == 0 || h == 0)
	{
		fprintf(stderr, "ERROR: %i x %i are not valid layer dimensions\n", w, h);
		free(buf_raw);
		return NULL;
	}
	
	// allocate layer
	layer_t *layer = layer_new(w, h, LAYER_TEMPLATE_EMPTY);
	if(layer == NULL)
	{
		fprintf(stderr, "EDOOFUS: layer_new given wrong parameters and crapped out :(\n");
		free(buf_raw);
		return NULL;
	}
	
	// decode stream
	for(i = 0; i < w*h; i++)
	{
		tile_t *t = &(layer->tiles[i]);
		
		while(t != NULL)
		{
			u8 flags = (*(v++));
			t->type = (*(v++));
			
			//printf("%i type = %02X%s\n", i, t->type, (extflag ? " EXTENDED" : ""));
			
			if(flags&MAP_FLAG_U16CHAR)
			{
				t->chr = *(u16 *)v;
				v += 2;
			} else {
				t->chr = *(v++);
			}
			
			t->col = *(v++);
			t->data = NULL;
			t->datalen = 0;
			if(flags&MAP_FLAG_DATA)
			{
				t->datalen = *(v++);
				if(flags&MAP_FLAG_EXT_DATALEN)
					t->datalen |= ((*(v++))<<8);
				
				//printf("chr=%04X col=%02X datalen=%02X\n", t->chr, t->col, t->datalen);
				if(t->datalen != 0)
				{
					t->data = malloc(t->datalen);
					if(t->data == NULL)
					{
						fprintf(stderr, "FATAL: COULD NOT ALLOCATE DATA\n");
						perror("layer_unserialise");
						abort();
					}
					
					memcpy(t->data, v, t->datalen);
					v += t->datalen;
				}
			}
				
			t->under = NULL;
			if(flags&MAP_FLAG_UNDER)
			{
				t->under = malloc(sizeof(tile_t));
				if(t->under == NULL)
				{
					fprintf(stderr, "FATAL: COULD NOT ALLOCATE UNDER\n");
					perror("layer_unserialise");
					abort();
				}
			}
			t = t->under;
		}
	}
	
	free(buf_raw);
	return layer;
}

layer_t *layer_dummy_request(s32 x, s32 y)
{
	layer_t *a = layer_new(LAYER_WIDTH, LAYER_HEIGHT, LAYER_TEMPLATE_CLASSIC);
	a->x = x; a->y = y;
	return a;
};

u8 layer_get_next_update(layer_t *layer, u32 *ux, u32 *uy)
{
	u32 i;
	u32 j;
	u8 k;
	u32 p;
	
	p = 0;
	for(i=*uy;i<layer->h;i++)
	{
		for(j=0;j<layer->w;j+=32)
		{
			if(layer->updmask[p] != 0)
			{
				for(k=0;k<32;k++)
				{
					if((((layer->updmask[p])>>k)&1)==1)
					{
						*ux=j+k; *uy=i;
						layer->updmask[p] &= ~(1<<k);
						return 1;
					}
				}
			}
			p++;
		}	
	}
	return 0;
};

void layer_switch_masks(layer_t *layer)
{
	// why free 30 times a second? ^3^
	u32 *temp = layer->updmask;
	layer->updmask = layer->new_updmask;
	layer->new_updmask = temp;	
	memset(layer->new_updmask,0x00,((layer->w+31)/32)*layer->h*4);
};

void layer_set_update(layer_t *layer, u32 ux, u32 uy, u8 tonew)
{
	u32 i = uy*((layer->w+31)/32) + (ux/32);
	u8 k = ux & 31;
	if(tonew)
		layer->new_updmask[i] |= 1<<k;
	else
		layer->updmask[i] |= 1<<k;
};

map_t *map_new(u32 layercount)
{
	u32 i;
	u32 j;

	map_t *map = malloc(sizeof(map_t) + sizeof(layerinfo_t)*layercount);
	if(map == NULL) return NULL;

	map->layer_count = layercount;

	for(i=0;i<layercount; i++)
	{
		map->layers[i].refcount = 0;
		map->layers[i].data = NULL;
	}
	map->layer_cmpbuf = NULL;
	return map;
}

void layer_unload(map_t *map, int i)
{
	map->layers[i].refcount = 0;
	
	if(map->layers[i].data == NULL)
		return;
	
	net_layer_release(map->layers[i].data->x,map->layers[i].data->y);
	layer_save(map, map->layers[i].data);
	layer_free(map->layers[i].data);
	map->layers[i].data = NULL;
}

int map_find_unused_layer(map_t *map)
{
	u16 i;
	for(i=0;i<map->layer_count;i++)
		if(map->layers[i].data == NULL)
			return i;

	for(i=0;i<map->layer_count;i++)
		if(map->layers[i].refcount == 0)
		{
			layer_unload(map, i);
			return i;
		}

	return -1;
}

int map_find_good_layer(map_t *map, s32 x, s32 y)
{
	u16 i;
	
	for(i=0;i<map->layer_count;i++)
		if(map->layers[i].x == x && map->layers[i].y == y)
			return i;
	
	return map_find_unused_layer(map);
}

layer_t *map_get_new_layer(map_t *map, s32 x, s32 y)
{
	int i = map_find_unused_layer(map);
	
	if(i>=0)
	{
		map->layers[i].data = layer_new(LAYER_WIDTH, LAYER_HEIGHT,
			LAYER_TEMPLATE_CLASSIC);
			
		if(map->layers[i].data!=NULL)
		{
			map->layers[i].x = x;
			map->layers[i].y = y;
			map->layers[i].refcount = 1;
			
		}
		return map->layers[i].data;
	}
	return NULL;
}

layer_t *map_get_existing_layer(map_t *map, s32 x, s32 y)
{
	int i;
	// Finding existing layer
	for(i=0;i<map->layer_count;i++) {
		if(map->layers[i].data != NULL
				&& map->layers[i].x==x
				&& map->layers[i].y==y)
			return map->layers[i].data;
	}
	return NULL;
}

layer_t *map_get_net_layer(map_t *map, s32 x, s32 y)
{
	int i;
	
	// Check if this layer is already requested
	for(i=0;i<map->layer_count;i++)
		if(map->layers[i].refcount != 0)
			if(map->layers[i].x == x && map->layers[i].y == y)
				return NULL;
	
	// Create empty layer
	i = map_find_good_layer(map, x, y);
	if(i == -1)
	{
		fprintf(stderr,"ERROR: Could not find unused layer!\n");
		fprintf(stderr,"TODO: handle this more gracefully!\n");
		abort();
	}
	if(i>=0)
	{
		// ensure we're not trying to grab a layer we have
		if(map->layers[i].refcount != 0)
			return NULL;
		
		map->layers[i].data = net_layer_request(x,y);
		map->layers[i].x = x;
		map->layers[i].y = y;
		
		if(map->layers[i].data == NULL)
		{
			map->layers[i].refcount = 1;
			return NULL;
		} else {
			// i guess this is a safety check? --GM
			if(map->layers[i].x != x || map->layers[i].y != y)
			{
				fprintf(stderr, "EDOOFUS: layer pos incorrect in map_get_net_layer!\n");
				return NULL;
			}
			
			map->layers[i].refcount = 1;
			return map->layers[i].data;
		}
	}
	return NULL;
}

layer_t *map_get_file_layer(map_t *map, s32 x, s32 y)
{
	int i = map_find_unused_layer(map);

	if(i>=0)
	{
		map->layers[i].data = layer_load(map,x,y);
		if(map->layers[i].data != NULL)
		{
			map->layers[i].x = x;
			map->layers[i].y = y;
			map->layers[i].refcount = 1;
		}
		return map->layers[i].data;
	}
	return NULL;
}

void map_layer_set_unused(map_t *map, s32 x, s32 y)
{
	int i;
	for(i=0;i<map->layer_count;i++) {
		if(map->layers[i].data != NULL
			&& map->layers[i].x==x && map->layers[i].y==y)
		{
			map->layers[i].refcount = 0;
			return;
		}
	}
}

void map_layer_set_used(map_t *map, s32 x, s32 y)
{
	int i;
	for(i=0;i<map->layer_count;i++) {
		if(map->layers[i].data != NULL
			&& map->layers[i].x==x && map->layers[i].y==y)
		{
			map->layers[i].refcount = 1;
			return;
		}
	}
}

void map_layer_set_unused_all(map_t *map)
{
	int i;
	for(i=0;i<map->layer_count;i++) 
		if(map->layers[i].data != NULL)
			map->layers[i].refcount = 0;
}

void map_layer_set_used_rendered(map_t *map, s32 topx, s32 topy)
{
	s32 chunk_x = divneg(topx,LAYER_WIDTH);
	s32 chunk_y = divneg(topy,LAYER_HEIGHT);
	map_layer_set_unused_all(map);
	map_layer_set_used(map,chunk_x,chunk_y);
	map_layer_set_used(map,chunk_x+1,chunk_y);
	map_layer_set_used(map,chunk_x,chunk_y+1);
	map_layer_set_used(map,chunk_x+1,chunk_y+1);
}

tile_t *layer_get_tile_ref(u8 x, u8 y, layer_t *layer)
{
	return &(layer->tiles[y*layer->w+x]);
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

void layer_set_tile_ext(u8 x, u8 y, u8 uidx, tile_t tile, layer_t *layer)
{
	// NOTE: preserve "under" reference!
	tile_t *mt = layer_get_tile_ref(x, y, layer);
	
	// dig down
	while(uidx-- > 0)
	{
		mt = mt->under;
		if(mt == NULL)
		{
			fprintf(stderr, "ERROR: uidx too large!\n");
			return;
		}
	}
	
	tile_t *under = mt->under;
	*mt = tile;
	mt->under = under;
}

void layer_set_tile_data(u8 x, u8 y, u8 uidx, u8 datalen, u8 datapos, u8 *data, layer_t *layer)
{
	tile_t *mt = layer_get_tile_ref(x, y, layer);
	
	// dig down
	while(uidx-- > 0)
	{
		mt = mt->under;
		if(mt == NULL)
		{
			fprintf(stderr, "ERROR: uidx too large!\n");
			return;
		}
	}
	
	// do a pointer check
	if(mt->data != NULL)
	{
		fprintf(stderr, "ERROR: tile data to write to is NULL!\n");
		return;
	}
	
	// do a range check
	if(datalen == 0)
	{
		fprintf(stderr, "WARNING: data length to write is 0!\n");
		return;
	}
	
	if((s32)datapos + (s32)datalen > (s32)mt->datalen)
	{
		fprintf(stderr, "WARNING: tile data out of range!\n");
		int xdatalen = (int)((s32)mt->datalen - (s32)datapos);
		if(xdatalen <= 0)
			return;
		
		datalen = (u32)xdatalen;
	}
	
	// copy!
	memcpy(&mt->data[datapos], data, datalen);
}

void layer_alloc_tile_data(u8 x, u8 y, u8 uidx, u16 datalen, layer_t *layer)
{
	tile_t *mt = layer_get_tile_ref(x, y, layer);
	
	// dig down
	while(uidx-- > 0)
	{
		mt = mt->under;
		if(mt == NULL)
		{
			fprintf(stderr, "ERROR: uidx too large!\n");
			return;
		}
	}
	
	// allocate
	mt->data = realloc(mt->data, (int)datalen);
	mt->datalen = datalen;
	
	// clear data
	memset(mt->data, 0, (int)datalen);
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
		abort(); // DIE HORRIBLY
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

tile_t map_get_tile(map_t *map, s32 x, s32 y)
{
	s32 chunk_x = divneg(x,LAYER_WIDTH);
	s32 chunk_y = divneg(y,LAYER_HEIGHT);
	layer_t *chunk = map_get_existing_layer(map,chunk_x,chunk_y);
	if(chunk == NULL) return tile_dummy();
	return layer_get_tile(absmod(x,chunk->w),absmod(y,chunk->h),chunk);
}

tile_t *map_get_tile_ref(map_t *map, s32 x, s32 y)
{
	s32 chunk_x = divneg(x,LAYER_WIDTH);
	s32 chunk_y = divneg(y,LAYER_HEIGHT);
	layer_t *chunk = map_get_existing_layer(map,chunk_x,chunk_y);
	if(chunk == NULL) return NULL;
	return layer_get_tile_ref(absmod(x,chunk->w),absmod(y,chunk->h),chunk);
}

void map_set_tile(map_t *map, s32 x, s32 y, tile_t tile)
{
	s32 chunk_x = divneg(x,LAYER_WIDTH);
	s32 chunk_y = divneg(y,LAYER_HEIGHT);
	layer_t *chunk = map_get_existing_layer(map,chunk_x,chunk_y);
	if(chunk == NULL) return;
	layer_set_tile(absmod(x,chunk->w),absmod(y,chunk->h),tile,chunk);
}

void map_set_tile_ext(map_t *map, s32 x, s32 y, u8 uidx, tile_t tile)
{
	s32 chunk_x = divneg(x,LAYER_WIDTH);
	s32 chunk_y = divneg(y,LAYER_HEIGHT);
	layer_t *chunk = map_get_existing_layer(map,chunk_x,chunk_y);
	if(chunk == NULL) return;
	layer_set_tile_ext(absmod(x,chunk->w),absmod(y,chunk->h),uidx,tile,chunk);
}

void map_set_tile_data(map_t *map, s32 x, s32 y, u8 uidx, u8 datalen, u8 datapos, u8 *data)
{
	s32 chunk_x = divneg(x,LAYER_WIDTH);
	s32 chunk_y = divneg(y,LAYER_HEIGHT);
	layer_t *chunk = map_get_existing_layer(map,chunk_x,chunk_y);
	if(chunk == NULL) return;
	layer_set_tile_data(absmod(x,chunk->w),absmod(y,chunk->h),uidx,datalen,datapos,data,chunk);
}

void map_alloc_tile_data(map_t *map, s32 x, s32 y, u8 uidx, u16 datalen)
{
	s32 chunk_x = divneg(x,LAYER_WIDTH);
	s32 chunk_y = divneg(y,LAYER_HEIGHT);
	layer_t *chunk = map_get_existing_layer(map,chunk_x,chunk_y);
	if(chunk == NULL) return;
	layer_alloc_tile_data(absmod(x,chunk->w),absmod(y,chunk->h),uidx,datalen,chunk);
}

void map_push_tile(map_t *map, s32 x, s32 y, tile_t tile)
{
	s32 chunk_x = divneg(x,LAYER_WIDTH);
	s32 chunk_y = divneg(y,LAYER_HEIGHT);
	layer_t *chunk = map_get_existing_layer(map,chunk_x,chunk_y);
	if(chunk == NULL) return;
	layer_push_tile(absmod(x,chunk->w),absmod(y,chunk->h),tile,chunk);
}

void map_pop_tile(map_t *map, s32 x, s32 y)
{
	s32 chunk_x = divneg(x,LAYER_WIDTH);
	s32 chunk_y = divneg(y,LAYER_HEIGHT);
	layer_t *chunk = map_get_existing_layer(map,chunk_x,chunk_y);
	if(chunk == NULL) return;
	layer_pop_tile(absmod(x,chunk->w),absmod(y,chunk->h),chunk);
}

void map_switch_masks(map_t *map)
{
	int i;	
	for(i=0;i<map->layer_count;i++)
		if(map->layers[i].data != NULL)
			layer_switch_masks(map->layers[i].data);
}

u8 map_get_next_update(map_t *map, int *lidx, s32 *x, s32 *y)
{
	int i;
	u32 lx, ly;
	lx = 0;
	ly = 0;

	// Finding all existing layers
	for(i=*lidx;i<map->layer_count;i++) {
		if(map->layers[i].data != NULL && layer_get_next_update(map->layers[i].data,&lx,&ly))
		{
			*x = map->layers[i].x*LAYER_WIDTH  + lx;
			*y = map->layers[i].y*LAYER_HEIGHT + ly;
			*lidx = i;
			return 1;
		}
	}

	return 0;
}

void map_set_update(map_t *map, s32 x, s32 y, u8 tonew)
{
	s32 chunk_x = divneg(x,LAYER_WIDTH);
	s32 chunk_y = divneg(y,LAYER_HEIGHT);
	layer_t *chunk = map_get_existing_layer(map,chunk_x,chunk_y);
	if(chunk == NULL) return;
	layer_set_update(chunk, absmod(x,chunk->w),absmod(y,chunk->h),tonew);
}

void map_init(void)
{
	client_map = map_new(LAYER_SIZE_CLIENT);
	server_map = map_new(LAYER_SIZE_SERVER);
	
	client_map->f_get_tile = client_get_tile;
	client_map->f_set_tile = client_set_tile;
	client_map->f_push_tile = client_push_tile;
	client_map->f_pop_tile = client_pop_tile;
	client_map->f_set_tile_ext = client_set_tile_ext;
	client_map->f_alloc_tile_data = client_alloc_tile_data;
	client_map->f_set_tile_data = client_set_tile_data;
	client_map->f_get_next_update = client_get_next_update;
	client_map->f_set_update = client_set_update;
	client_map->f_set_update_n = client_set_update_n;

	server_map->f_get_tile = server_get_tile;
	server_map->f_set_tile = server_set_tile;
	server_map->f_push_tile = server_push_tile;
	server_map->f_pop_tile = server_pop_tile;
	server_map->f_set_tile_ext = server_set_tile_ext;
	server_map->f_alloc_tile_data = server_alloc_tile_data;
	server_map->f_set_tile_data = server_set_tile_data;
	server_map->f_get_next_update = server_get_next_update;
	server_map->f_set_update = server_set_update;
	server_map->f_set_update_n = server_set_update_n;

	client_map->fpath = "xmap/";
	server_map->fpath = "svmap/";
}
