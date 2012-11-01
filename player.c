#include "common.h"
#include "player.h"

player_t *players[PLAYER_AMOUNT];
u8 players_set[PLAYER_AMOUNT];

player_t *player_get(u16 id)
{
	if((u32)id>=PLAYER_AMOUNT) return NULL;
	return players[id];
}

void player_new(u16 id)
{
	if((u32)id>=PLAYER_AMOUNT) return;
	players[id] = (player_t *)malloc(sizeof(player_t));
	players[id]->x = 0;
	players[id]->y = 0;
	players[id]->id = id;
	players[id]->chr = 2;
	players[id]->col = 31;
	players_set[id]=1;
}

void player_set(u16 id, player_t *player)
{
	if((u32)id<PLAYER_AMOUNT)
	{
		players_set[id]=1;
		players[id]=player;
	}
}

void player_remove(u16 id)
{
	if((u32)id<PLAYER_AMOUNT)
	{
		players_set[id]=0;
		free(players[id]);
	}
}
