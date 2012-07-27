#include "interface.h"
#include "common.h"
#include "render.h"
#include "event.h"
#include "misc.h"

u8 isOn;

#define UI_BUTTONS 3

#define COLHL(el,sh) ((el)==(sh)?8:0)

void init_ui(void)
{
	isOn=0;
}

void render_ui(void)
{
	// First, grab mouse stuff.
	int mousex = sfp_event_mousex();
	int mousey = sfp_event_mousey();

	// Prepare some variables.
	u8 whichHL = 0;
	if(inside_rect(mousex,mousey,0,SFP_FIELD_HEIGHT*16,UI_BUTTONS*16,1*16))
		whichHL = (mousex>>4)+1;
	// Now, draw the bar.
	sfp_putc_block_2x(0,SFP_FIELD_HEIGHT,0+COLHL(whichHL,1),4+COLHL(whichHL,1),84);
	sfp_putc_block_2x(1,SFP_FIELD_HEIGHT,1+COLHL(whichHL,2),7+COLHL(whichHL,2),2);
	sfp_putc_block_2x(2,SFP_FIELD_HEIGHT,0+COLHL(whichHL,3),6+COLHL(whichHL,3),67);
}
