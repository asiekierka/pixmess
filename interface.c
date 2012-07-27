#include "interface.h"
#include "common.h"
#include "render.h"
#include "event.h"
#include "misc.h"

u8 isOn;
u8 activeUI;

tile_t *drawing_tile;

#define UI_BUTTONS 3

#define COLHL(el,al,sh) (((el)==(sh))||((al)==(sh))?8:0)

void init_ui(void)
{
	isOn=0;
	activeUI=0;
	drawing_tile = (tile_t *)malloc(sizeof(tile_t));
	drawing_tile->type = TILE_WALL;
	drawing_tile->chr = 1;
	drawing_tile->col = 7;
	drawing_tile->data = NULL;
}

tile_t *ui_get_tile(void)
{
	return drawing_tile;
}

void render_ui(void)
{
	// First, grab mouse stuff.
	int mousex = sfp_event_mousex();
	int mousey = sfp_event_mousey();

	// Prepare some variables.
	u8 whichHL = 0;
	if(inside_rect(mousex,mousey,1*16,SFP_FIELD_HEIGHT*16,UI_BUTTONS*16,1*16))
		whichHL = (mousex>>4);

	// Now, draw the bar.

	// INACTIVE ELEMENTS
	sfp_putc_block_2x(0,SFP_FIELD_HEIGHT,0,8,179);
	sfp_putc_block_2x(4,SFP_FIELD_HEIGHT,0,8,179);
	sfp_putc_block_2x(6,SFP_FIELD_HEIGHT,0,8,179);
	sfp_putc_block_2x(5,SFP_FIELD_HEIGHT,(drawing_tile->col>>4),(drawing_tile->col&15),drawing_tile->chr);

	// ACTIVE ELEMENTS - Wundericons
	sfp_putc_block_2x(1,SFP_FIELD_HEIGHT,0+COLHL(whichHL,activeUI,1),4+COLHL(whichHL,activeUI,1),84);
	sfp_putc_block_2x(2,SFP_FIELD_HEIGHT,1+COLHL(whichHL,activeUI,2),7+COLHL(whichHL,activeUI,2),2);
	sfp_putc_block_2x(3,SFP_FIELD_HEIGHT,0+COLHL(whichHL,activeUI,3),6+COLHL(whichHL,activeUI,3),67);

}
