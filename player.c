#include "common.h"
#include "network.h"
#include "player.h"

u8 player_is_occupied(s32 x, s32 y)
{
	return net_player_is_occupied(x,y);
}
