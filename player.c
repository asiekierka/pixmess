#include "common.h"
#include "player.h"

player_t *players[PLAYER_AMOUNT];
u8 players_set[PLAYER_AMOUNT];

player_t *player_get(u16 id)
{
	if((u32)id>=PLAYER_AMOUNT) return NULL;
	if(players_set[PLAYER_AMOUNT-1]==0)
	{
		players[id] = (player_t *)malloc(sizeof(player_t));
		players[id]->x = 0;
		players[id]->y = 0;
		players[id]->id = id;
		players[id]->chr = 2;
		players[id]->col = 31;
		players_set[id]=1;
	}
	return players[id];
}

void player_remove(u16 id)
{
	if((u32)id<PLAYER_AMOUNT)
	{
		players_set[id]=0;
		free(players[id]);
	}
}
