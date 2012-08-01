#ifndef _SERVER_H_
#define _SERVER_H_

#include "common.h"

void server_set_tile(s32 x, s32 y, tile_t tile);
void server_set_tile_ext(s32 x, s32 y, u8 uidx, tile_t tile, int sendflag);
void server_alloc_tile_data(s32 x, s32 y, u8 uidx, u16 datalen, int sendflag);
void server_set_tile_data(s32 x, s32 y, u8 uidx, u8 datalen, u16 datapos, u8 *data, int sendflag);
tile_t server_get_tile(s32 x, s32 y);
void server_push_tile(s32 x, s32 y, tile_t tile);
void server_pop_tile(s32 x, s32 y);
u8 server_get_next_update(int *lidx, s32 *x, s32 *y);
void server_set_update(s32 x, s32 y);

#endif /* _SERVER_H */
