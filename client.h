#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "common.h"

void client_set_tile(s32 x, s32 y, tile_t tile);
tile_t client_get_tile(s32 x, s32 y);
void client_push_tile(s32 x, s32 y, tile_t tile);
void client_pop_tile(s32 x, s32 y);
u8 client_get_next_update(s32 *x, s32 *y);
void client_set_update(s32 x, s32 y);

#endif /* _CLIENT_H */
