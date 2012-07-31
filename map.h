#ifndef _SFP_MAP_H_
#define _SFP_MAP_H_

#include "common.h"

#define MAP_MAX_SIZE 262144 // 256KB should be enough for anyone.
#define MAP_RW_DIR "pm-map"
#define DIR_DELIM "/"

#define LAYER_VERSION 1
#define LAYER_LOWEST_VERSION 1

void map_init(void);
void layer_free(layer_t *layer);
layer_t *layer_new(int w, int h, int template);

layer_t *map_get_new_layer(map_t *map, s32 x, s32 y);
layer_t *map_get_net_layer(map_t *map, s32 x, s32 y);
layer_t *map_get_existing_layer(map_t *map, s32 x, s32 y);
layer_t *map_get_file_layer(map_t *map, s32 x, s32 y);

int map_find_unused_layer(map_t *map);
int map_find_good_layer(map_t *map, s32 x, s32 y);

void map_layer_set_unused(map_t *map, s32 x, s32 y);
void map_layer_set_used(map_t *map, s32 x, s32 y);
tile_t map_get_tile(map_t *map, s32 x, s32 y);
void map_set_tile(map_t *map, s32 x, s32 y, tile_t tile);
void map_push_tile(map_t *map, s32 x, s32 y, tile_t tile);
void map_pop_tile(map_t *map, s32 x, s32 y);

tile_t *layer_get_tile_ref(u8 x, u8 y, layer_t *layer);
tile_t layer_get_tile(u8 x, u8 y, layer_t *layer);
void layer_set_tile(u8 x, u8 y, tile_t tile, layer_t *layer);
void layer_push_tile(u8 x, u8 y, tile_t tile, layer_t *layer);
void layer_pop_tile(u8 x, u8 y, layer_t *layer);
layer_t *layer_dummy_request(s32 x, s32 y);

u8 *layer_serialise(layer_t *layer, int *rawlen, int *cmplen);
layer_t *layer_unserialise(u8 *buf_cmp, int rawlen, int cmplen);

void map_layer_set_used_rendered(map_t *map, s32 x, s32 y);

u8 layer_save(map_t *map, layer_t *layer);
layer_t* layer_load(map_t *map, s32 x, s32 y);
void map_save(map_t *map);

u8 layer_get_next_update(layer_t *layer, u32 *ux, u32 *uy);
void layer_set_update(layer_t *layer, u32 ux, u32 uy);
void map_set_update(map_t *map, s32 x, s32 y);

extern map_t *client_map;
extern map_t *server_map;

enum
{
	LAYER_TEMPLATE_EMPTY,
	LAYER_TEMPLATE_CLASSIC,
};

#endif /* _SFP_MAP_H_ */
