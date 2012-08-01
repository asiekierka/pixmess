#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "common.h"

void client_set_tile(map_t *map, s32 x, s32 y, tile_t tile);
void client_set_tile_ext(map_t *map, s32 x, s32 y, u8 uidx, tile_t tile, int sendflag);
void client_alloc_tile_data(map_t *map, s32 x, s32 y, u8 uidx, u16 datalen, int sendflag);
void client_set_tile_data(map_t *map, s32 x, s32 y, u8 uidx, u8 datalen, u16 datapos, u8 *data, int sendflag);
tile_t client_get_tile(map_t *map, s32 x, s32 y);
void client_push_tile(map_t *map, s32 x, s32 y, tile_t tile);
void client_pop_tile(map_t *map, s32 x, s32 y);
u8 client_get_next_update(map_t *map, int *lidx, s32 *x, s32 *y);
void client_set_update(map_t *map, s32 x, s32 y, u8 tonew);
void client_set_update_n(map_t *map, s32 x, s32 y, u8 tonew);

#endif /* _CLIENT_H */
