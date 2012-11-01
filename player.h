#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "common.h"

#define PLAYER_SELF 1023
#define PLAYER_NONE 65535
#define PLAYER_AMOUNT 1024

extern player_t *player;

u8 player_is_occupied(s32 x, s32 y);
player_t *player_get(u16 id);
void player_new(u16 id);
void player_set(u16 id, player_t *player);
void player_remove(u16 id);

#endif /* _PLAYER_H_ */
