#ifndef _SERVER_H_
#define _SERVER_H_

#include "common.h"

void server_set_tile(map_t *map, s32 x, s32 y, tile_t tile);
void server_set_tile_ext(map_t *map, s32 x, s32 y, u8 uidx, tile_t tile, int sendflag);
void server_alloc_tile_data(map_t *map, s32 x, s32 y, u8 uidx, u16 datalen, int sendflag);
void server_set_tile_data(map_t *map, s32 x, s32 y, u8 uidx, u8 datalen, u16 datapos, u8 *data, int sendflag);
tile_t server_get_tile(map_t *map, s32 x, s32 y);
void server_push_tile(map_t *map, s32 x, s32 y, tile_t tile);
void server_pop_tile(map_t *map, s32 x, s32 y);
u8 server_get_next_update(map_t *map, int *lidx, s32 *x, s32 *y);
void server_set_update(map_t *map, s32 x, s32 y, u8 tonew);
void server_set_update_n(map_t *map, s32 x, s32 y, u8 tonew);

#endif /* _SERVER_H */
