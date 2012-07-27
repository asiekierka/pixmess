/*
split this into several .c files as required.
yeah, welcome to bottom-up development.
hope you enjoy the refactoring.
remember to do it often, or everything will turn to crap!
    --GM
*/

#include "common.h"
#include "event.h"
#include "map.h"
#include "player.h"
#include "render.h"

player_t *player;
u8 movement_wait;
u64 frame_counter;

void player_move(s8 dx, s8 dy)
{
	s32 newx = player->x+dx;
	s32 newy = player->y+dy;
	if(tile_walkable(map_get_tile(newx,newy)))
	{
		player->x=newx;
		player->y=newy;
	}
	movement_wait = 2;
}

s32 get_rootx()
{
	return player->x-(SFP_FIELD_WIDTH/2);
}
s32 get_rooty()
{
	return player->y-(SFP_FIELD_HEIGHT/2);
}

void display (player_t *p)
{
	s32 i;
	s32 j;
	s32 rx = p->x;
	s32 orx;
	s32 ry = p->y;
	s32 ory;
	tile_t tile;
	map_layer_set_used_rendered(rx,ry);
	rx -= (SFP_FIELD_WIDTH)/2;
	ry -= (SFP_FIELD_HEIGHT)/2;
	orx = rx;
	ory = ry;
	for(j=0;j<SFP_FIELD_HEIGHT;j++)
	{
		for(i=0;i<SFP_FIELD_WIDTH;i++)
		{
			tile = map_get_tile(rx,ry);
			sfp_putc_block_2x(i,j,(tile.col>>4),(tile.col&15),tile.chr);
			rx++;
		}
		rx = orx;
		ry++;
	}
	// Early player code.
	u32 px = p->x-orx;
	u32 py = p->y-ory;
	if(!tile_overlay(map_get_tile(p->x,p->y))) sfp_putc_block_2x(px,py,(p->col>>4),(p->col&15),p->chr);
	char* name = "Gamemaster";

	u32 pnamex = (px*16)-((strlen(name))*4)+8;
	u32 pnamey = (py*16)-10;
	sfp_printf_1x(pnamex,pnamey,0x0F,0,"%s",name);
}

void mouse_placement()
{
	s32 bx = get_rootx()+((sfp_event_mousex())/16);
	s32 by = get_rooty()+((sfp_event_mousey())/16);
	if(sfp_event_mouse_button(0))
	{
		tile_t tile;
		tile.type = TILE_WALL;
		tile.chr = 1;
		tile.col = 7;
		tile.data = NULL;
		map_set_tile(bx,by,tile);
	}
	if(sfp_event_mouse_button(2))
	{
		tile_t tile;
		tile.type = TILE_FLOOR;
		tile.chr = 0;
		tile.col = 0;
		tile.data = NULL;
		map_set_tile(bx,by,tile);
	}
}

int main(int argc, char *argv[])
{
	if(sfp_init_render())
		return 1;

	map_init();

	movement_wait = 0;

	player = player_get(PLAYER_SELF);

	while(!(sfp_event_key(SFP_KEY_APP_QUIT) || sfp_event_key(SFP_KEY_ESC)))
	{
		sfp_render_begin();
		display(player);
		if(frame_counter<150) sfp_printf_2x(1*8,2*8,0x1F,0,"Hello %s! You are player %i.", "Gamemaster", PLAYER_SELF);
		sfp_printf_1x(sfp_event_mouse_x()-8,sfp_event_mouse_y()-8,14,0,"Mouse");
		sfp_render_end();

		if(movement_wait==0)
		{
			if(sfp_event_key(SFP_KEY_UP))
				player_move(0,-1);
			if(sfp_event_key(SFP_KEY_DOWN))
				player_move(0,1);
			if(sfp_event_key(SFP_KEY_LEFT))
				player_move(-1,0);
			if(sfp_event_key(SFP_KEY_RIGHT))
				player_move(1,0);
		}

		mouse_placement();

		sfp_delay(33); // constant ~30FPS, rule from old 64pixels
		
		if(movement_wait>0) movement_wait--;
		frame_counter++;

		sfp_event_poll();
		sfp_event_tick();
	}
	
	return 0;
}

