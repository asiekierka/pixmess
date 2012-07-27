#include "interface.h"
#include "common.h"
#include "render.h"
#include "event.h"
#include "misc.h"
#include "tile.h"

u8 isOn;
u8 activeUI;
u8 mouse_left_down;
u8 scrollbar_pos;

tile_t *drawing_tile;

#define UI_BUTTONS 3

#define COLHL(el,al,sh) (((el)==(sh))||((al)==(sh))?8:0)

u8 ui_is_occupied(u16 x, u16 y)
{
	if(y>=SFP_FIELD_HEIGHT*16) return 1;
	switch(activeUI)
	{
		case 1:
		case 2:
			if(x<=88 && y>=(SFP_FIELD_HEIGHT*16)-104) return 1;
			break;
		case 3:
			if(x<=80 && y>=(SFP_FIELD_HEIGHT*16)-112) return 1;
			break;
	}
	return 0;
}

void init_ui(void)
{
	isOn=0;
	activeUI=0;
	mouse_left_down=0;
	scrollbar_pos = 0;
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

void render_type_window(void)
{
	// 88x104, 3x4 icons + scroll
	sfp_fill_rect(0,SFP_FIELD_HEIGHT*16-104,88,104,0x000000);
	// Drawing
	u8 i;
	u8 j;
	u8 px;
	u8 py;
	u8 tcol;
	for(i=0;i<12;i++)
	{
		j = i+(scrollbar_pos*3);	
		if(i>=TILE_TYPES) break;
		px = 8+((i%3)*24);
		py = 8+((i/3)*24);
		tcol = tile_get_preview_color(j);
		sfp_putc_2x(px,py+(SFP_FIELD_HEIGHT*16-104),tcol>>4,tcol&15,tile_get_preview_char(j));
	}

	sfp_draw_rect(0,SFP_FIELD_HEIGHT*16-104,88,104,0xCCCCCC);
}
void render_char_window(void)
{
	// 88x104, 3x4 icons + scroll
	sfp_fill_rect(0,SFP_FIELD_HEIGHT*16-104,88,104,0x000000);
	// Drawing
	sfp_draw_rect(0,SFP_FIELD_HEIGHT*16-104,88,104,0xCCCCCC);
}
void render_color_window(void)
{
	// 80x112, 2 4x4 8x8 bars, 16x16 preview
	sfp_fill_rect(0,SFP_FIELD_HEIGHT*16-112,80,112,0x000000);
	// Drawing

	sfp_draw_rect(0,SFP_FIELD_HEIGHT*16-112,80,112,0xCCCCCC);
}

void render_ui(void)
{
	// First, grab mouse stuff.
	int mousex = sfp_event_mousex();
	int mousey = sfp_event_mousey();
	// Prepare some variables.
	u8 whichHL = 0;
	if(inside_rect(mousex,mousey,1*16,SFP_FIELD_HEIGHT*16,UI_BUTTONS*16,1*16))
	{
		whichHL = (mousex>>4);
		if(sfp_event_mouse_button(0))
		{
			if(mouse_left_down==0)
			{
				scrollbar_pos = 0;
				mouse_left_down = 1;
				if(activeUI == (mousex>>4))
					activeUI = 0;
				else
					activeUI = (mousex>>4);
			}
		}
		else mouse_left_down = 0;
	}
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

	// WINDOW
	switch(activeUI)
	{
		case 1: { render_type_window(); break; }
		case 2: { render_char_window(); break; }
		case 3: { render_color_window(); break; }
	}

}
