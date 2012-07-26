#ifndef _MAP_H_
#define _MAP_H_

#include "common.h"

#define LAYER_SIZE 64 // 32-255, no promises for lower, will crash on higher
#define LAYER_WIDTH  64
#define LAYER_HEIGHT 64

layer_t *layer_new(void);
layer_t *map_get_layer(s32 x, s32 y);
void map_layer_set_unused(s32 x, s32 y);
tile_t map_get_tile(s32 x, s32 y);
tile_t layer_get_tile(u8 x, u8 y, layer_t *layer);
void layer_set_tile(u8 x, u8 y, tile_t tile, layer_t *layer);
tile_t map_get_tile(s32 x, s32 y);
void map_set_tile(s32 x, s32 y, tile_t tile);

#endif /* _MAP_H_ */
