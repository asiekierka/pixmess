#include "common.h"
#include "map.h"
#include "tile.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

// TODO: move to misc.c

void mkdir_if_none(char* path)
{
	// I'm an asshat.
	// note, all dirs need the execute flag set to be accessible --GM
#ifdef WIN32
	mkdir(path);
#else
	mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif
}

// PLEASE FREE THE PATH YOU GET WHEN YOU'RE DONE WITH IT. --GM
char* layer_rw_path(map_t *map, s32 x, s32 y)
{
	// TODO: get correct path separator, somehow
	char* path1 = malloc(sizeof(char)*512);
	mkdir_if_none(map->fpath);
	snprintf(path1, 511, "%s%s%d",map->fpath,PATHSEP,y);
	mkdir_if_none(path1);
	snprintf(path1, 511, "%s%s%d%s%d.cnk",map->fpath,PATHSEP,y,PATHSEP,x);
	
	return path1;
}

layer_t* layer_load(map_t *map, s32 x, s32 y)
{
	char* path = layer_rw_path(map,x,y);
	FILE *file = fopen(path,"rb");
	if(file == NULL)
	{
		// TODO: check errno for ENOENT (No such file or directory)
		fprintf(stderr, "ERROR opening file \"%s\" for load\n", path);
		perror("layer_load");
		free(path);
		return NULL;
	}
	free(path);
	
	u32 rawlen = 0;
	u32 cmplen = 0;
	u8* data;
	
	fread(&rawlen, 4, 1, file);
	fread(&cmplen, 4, 1, file);
	if(rawlen == 0 || cmplen == 0)
	{
		fprintf(stderr, "ERROR: premature EOF loading map OR invalid lengths\n");
		perror("layer_load");
		return NULL;
	}
	
	data = (u8*)malloc(cmplen);
	
	if(fread(data, cmplen, 1, file) != 1)
	{
		fprintf(stderr, "ERROR: premature EOF loading map\n");
		perror("layer_load");
		return NULL;
	}
	
	fclose(file);
	
	layer_t *layer = layer_unserialise(data,rawlen,cmplen);
	if(layer != NULL)
	{
		layer->x = x;
		layer->y = y;
	}
	free(data);
	
	return layer;
}

u8 layer_save(map_t *map, layer_t *layer)
{
	int rawlen, cmplen;
	
	u8* data = layer_serialise(layer,&rawlen,&cmplen);
	
	char* path = layer_rw_path(map,layer->x,layer->y);
	FILE *file = fopen(path,"wb");
	if(file == NULL)
	{
		fprintf(stderr, "ERROR opening file \"%s\" for save\n", path);
		perror("layer_save");
		free(path);
		return 1;
	}
	free(path);
	
	u32 rawlen_fp = (u32)rawlen;
	u32 cmplen_fp = (u32)cmplen;
	fwrite(&rawlen_fp, 4, 1, file);
	fwrite(&cmplen_fp, 4, 1, file);
	fwrite(data, cmplen, 1, file);
	
	fclose(file);
	return 0;
}

void map_save(map_t *map)
{
	int i;
	
	for(i = 0; i < map->layer_count; i++)
		if(map->layers[i].data != NULL)
			layer_save(map, map->layers[i].data);
}
