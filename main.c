/*
split this into several .c files as required.
yeah, welcome to bottom-up development.
hope you enjoy the refactoring.
remember to do it often, or everything will turn to crap!
    --GM
*/

#include "common.h"
#include "map.h"
#include "render.h"

void display (s32 root_x, s32 root_y)
{
	s32 i;
	s32 j;
	s32 rx = root_x;
	s32 ry = root_y;
	tile_t tile;
	for(j=0;j<SFP_SCREEN_HEIGHT;j++)
	{
		for(i=0;i<SFP_SCREEN_WIDTH;i++)
		{
			rx++;
			tile = map_get_tile(rx,ry);
			sfp_putc_block_2x(i,j,(tile.col>>4),(tile.col&15),tile.chr);
		}
		rx = root_x;
		ry++;
	}
}
int main(int argc, char *argv[])
{
	sfp_init_render();

	sfp_render_begin();
	display(0,0);
	sfp_printf_2x(1*8,2*8,0x1F,0,"Hello %s! You are player %i.", "Neo", 42);
	sfp_printf_1x(3*8,7*8,0x07,0,"(You also suck at this game.)");
	sfp_render_end();

	sfp_delay(2000);
	
	return 0;
}

