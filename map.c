#include "common.h"
#include "map.h"
#include "network.h"

// TODO these defintions should have a file later on, maybe

layer_t *layers[LAYER_SIZE];
u8 layer_set[LAYER_SIZE];
s32 layer_x[LAYER_SIZE];
s32 layer_y[LAYER_SIZE];

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
					a->tiles[i].chr = 0x120; // TODO: reset back to 0x20 --GM
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
	FILE *fp = tmpfile();
	
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
			int extflag = (t->chr > 0xFF || t->under != NULL || t->data != NULL);
			
			// type + extflag
			fputc(extflag ? (t->type|0x80) : (t->type&~0x80)
				,fp);
			
			// chr + col
			fputc(t->chr&0xFF, fp);
			if(extflag)
				fputc(t->chr>>8, fp);
			fputc(t->col, fp);
			
			// data
			if(extflag)
			{
				if(t->data == NULL)
				{
					fputc(0x00, fp);
				} else {
					fputc(t->datalen, fp);
					fwrite(t->data, t->datalen, 1, fp);
				}
				
				if(t->under == NULL)
					fputc(0x00, fp);
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
	if(fread(buf_raw, *rawlen, 1, fp) != 1)
	{
		fprintf(stderr, "FATAL: YOUR OS SUCKS\n");
		perror("layer_serialise");
		abort(); // die badly - this should NEVER happen!
	}
	
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
		fprintf(stderr, "ERROR: incorrect magic number\n");
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
			int extflag = (*v) & 0x80;
			
			t->type = (*(v++)) & 0x7F;
			
			//printf("%i type = %02X%s\n", i, t->type, (extflag ? " EXTENDED" : ""));
			
			if(extflag)
			{
				t->chr = *(u16 *)v;
				v += 2;
				t->col = *(v++);
				t->datalen = *(v++);
				t->data = NULL;
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
				
				//printf("%02X\n", *v);
				if(*v == 0x80)
				{
					// TODO: handle this more gracefully rather than just crashing
					fprintf(stderr, "ERROR: value 0x80 is reserved in type chain\n");
					fprintf(stderr, "don't know how to handle correctly -- ABORTING!\n");
					abort();
				}
				
				if(*v != 0x00)
				{
					t->under = malloc(sizeof(tile_t));
					if(t->under == NULL)
					{
						fprintf(stderr, "FATAL: COULD NOT ALLOCATE UNDER\n");
						perror("layer_unserialise");
						abort();
					}
					t = t->under;
				} else {
					v++;
					t->under = NULL;
					t = NULL;
				}
			} else {
				t->chr = *(v++);
				t->col = *(v++);
				t->data = NULL;
				t->under = NULL;
				t->datalen = 0;
				
				t = NULL;
			}
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

void map_init(void)
{
	u8 i;
	for(i=0;i<LAYER_SIZE;i++)
		layer_set[i]=LAYER_UNALLOC;
}

void layer_unload(int i)
{
	layer_set[i] = LAYER_UNALLOC;
	if(layers[i] == NULL)
		return;
	
	net_layer_release(layers[i]->x,layers[i]->y,i);
	layer_free(layers[i]);
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
	
	// Check if this layer is already requested
	for(i=0;i<LAYER_SIZE;i++)
		if(layer_set[i]==LAYER_REQUESTED)
			if(layer_x[i] == x && layer_y[i] == y)
				return NULL;
	
	// Create empty layer
	for(i=0;i<LAYER_SIZE;i++)
	{
		if(layer_set[i]==LAYER_UNUSED)
			layer_unload(i);
		if(layer_set[i]==LAYER_UNALLOC)
		{
			layers[i] = net_layer_request(x,y,i);
			layer_x[i] = x;
			layer_y[i] = y;
			
			if(layers[i] == NULL)
			{
				layer_set[i] = LAYER_REQUESTED;
				return NULL;
			} else {
				if(layer_x[i] != x || layer_y[i] != y)
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
		if(layers[i] != NULL && layer_x[i]==x && layer_y[i]==y)
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
		if(layers[i] != NULL && layer_x[i]==x && layer_y[i]==y)
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
		if(layers[i] != NULL)
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

