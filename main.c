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

void display (player_t *p)
{
	s32 i;
	s32 j;
	s32 rx = p->x;
	s32 orx;
	s32 ry = p->y;
	tile_t tile;
	map_layer_set_used_rendered(rx,ry);
	rx -= (SFP_SCREEN_WIDTH)/2;
	ry -= (SFP_SCREEN_HEIGHT)/2;
	orx = rx;
	for(j=0;j<SFP_SCREEN_HEIGHT;j++)
	{
		for(i=0;i<SFP_SCREEN_WIDTH;i++)
		{
			rx++;
			tile = map_get_tile(rx,ry);
			sfp_putc_block_2x(i,j,(tile.col>>4),(tile.col&15),tile.chr);
		}
		rx = orx;
		ry++;
	}
	// Early player code.
	sfp_putc_block_2x((SFP_SCREEN_WIDTH)/2,(SFP_SCREEN_HEIGHT)/2,(p->col>>4),(p->col&15),p->chr);
	char* name = "Gamemaster";

	u32 pnx = (((SFP_SCREEN_WIDTH)/2)*16)-((strlen(name))*4)+8;
	u32 pny = (((SFP_SCREEN_HEIGHT)/2)*16)-10;
	sfp_printf_1x(pnx,pny,0x0F,0,"%s",name);
}

int main(int argc, char *argv[])
{
	if(sfp_init_render())
		return 1;

	player = player_get(PLAYER_SELF);

	while(!(sfp_event_key(SFP_KEY_APP_QUIT) || sfp_event_key(SFP_KEY_ESC)))
	{
		sfp_render_begin();
		display(player);
		sfp_printf_2x(1*8,2*8,0x1F,0,"Hello %s! You are player %i.", "Gamemaster", PLAYER_SELF);
		sfp_putc_1x(sfp_event_mouse_x()-8,sfp_event_mouse_y()-8,1,14,0x02);
		sfp_render_end();
	
		sfp_delay(20);
		
		sfp_event_poll();
	}
	
	return 0;
}

