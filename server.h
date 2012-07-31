#ifndef _SERVER_H_
#define _SERVER_H_

#include "common.h"

void server_set_tile(s32 x, s32 y, tile_t tile);
tile_t server_get_tile(s32 x, s32 y);
void server_push_tile(s32 x, s32 y, tile_t tile);
void server_pop_tile(s32 x, s32 y);
u8 server_get_next_update(s32 *x, s32 *y);
void server_set_update(s32 x, s32 y);

#endif /* _SERVER_H */
