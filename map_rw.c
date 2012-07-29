#include "common.h"
#include "map.h"
#include "tile.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

extern int errno;

// TODO: move to misc.c

void mkdir_if_none(char* path)
{
	// I'm an asshat.
	mkdir(path,S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
}

char* layer_rw_path(s32 x, s32 y)
{
	char* path1 = malloc(sizeof(char)*64);
	sprintf(path1,"%d,%d.cnk",x,y);	
	return path1;
}

layer_t* layer_load(s32 x, s32 y)
{
	char* path = layer_rw_path(x,y);
	FILE *file = fopen(path,"rb");
	if(file == NULL)
		return NULL;
	u32 cmplen;
	u32 rawlen;
	u8* data;
	// Query size
	fseek(file,0L,SEEK_END);
	cmplen = ftell(file);
	fseek(file,0L,SEEK_SET);

	rawlen = MAP_MAX_SIZE;
	data = (u8*)malloc(cmplen);

	if(!fread(data,1,cmplen,file))
		return NULL;

	fclose(file);
	layer_t *layer = layer_unserialise(data,rawlen,cmplen);

	return layer;
}
u8 layer_save(layer_t *layer)
{
	// Generate path.
	char* path = layer_rw_path(layer->x,layer->y);
	int rawlen, cmplen;
	u8* data = layer_serialise(layer,&rawlen,&cmplen);
	FILE *file = fopen(path,"wb");
	fwrite(&data,1,cmplen,file);
	fclose(file);
	return 0;
}
