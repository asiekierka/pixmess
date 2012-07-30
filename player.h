#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "common.h"

#define PLAYER_SELF 0 // selfish ID, 0 because.
#define PLAYER_AMOUNT 1024
extern player_t *player;

player_t *player_get(u16 id);
void player_remove(u16 id);

#endif /* _PLAYER_H_ */
