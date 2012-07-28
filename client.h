#ifndef _CLIENT_H_
#define _CLIENT_H_

void client_set_tile(s32 x, s32 y, tile_t tile);
void client_push_tile(s32 x, s32 y, tile_t tile);
void client_pop_tile(s32 x, s32 y);

#endif /* _CLIENT_H */
