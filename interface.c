#include "interface.h"
#include "common.h"
#include "render.h"
#include "event.h"
#include "misc.h"
#include "tile.h"

u8 isOn;
u8 activeUI;
u8 mouse_left_down;

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

tile_t *ui_get_tile(void)
{
	return drawing_tile;
}

s16 scrollbar_pos;
u8 scrollbar_dragged;
u8 scrollbar_mouse_cooldown;
u16 scrollbar_orig_y;
s16 scrollbar_orig_pos;

u8 ui_can_mouse_button()
{
	return (scrollbar_dragged==0);
}

void render_scrollbar(u16 x, u16 y, u16 h, u16 max)
{
	// Sanity checks
	if(max==0 || h%8>0) return;
	// Basic scrollbar
	u16 i;
	sfp_putc_1x(x,y,8,15,30);
	for(i=8;i<h-8;i+=8)
		sfp_putc_1x(x,y+i,0,8,177);
	sfp_putc_1x(x,y+h-8,8,15,31);

	// The light gray moving part.
	u16 part_pos = scrollbar_pos*(h-32)/max;
	sfp_fill_rect(x,y+8+part_pos,8,16,0xCCCCCC);

	u16 mousex = sfp_event_mousex();
	u16 mousey = sfp_event_mousey();
	u8 lmb = sfp_event_mouse_button(0);

	// Moving the scrollbar.
	if(scrollbar_dragged == 0)
	{
		if(scrollbar_mouse_cooldown == 0)
		{
			if(scrollbar_pos>0 && inside_rect(mousex,mousey,x,y,8,8) && lmb)
				{ scrollbar_pos--; scrollbar_mouse_cooldown = 4; }
			else if(scrollbar_pos<(max-1) && inside_rect(mousex,mousey,x,y+h-8,8,8) && lmb)
				{ scrollbar_pos++, scrollbar_mouse_cooldown = 4; }
			else if(inside_rect(mousex,mousey,x,y+8+part_pos,8,16) && lmb)
				{ scrollbar_dragged = 1; scrollbar_orig_y = mousey; scrollbar_orig_pos = scrollbar_pos; }
		}
		else scrollbar_mouse_cooldown--;
	}
	else if(scrollbar_dragged == 1)
	{
		if(lmb == 0) { scrollbar_dragged = 0; }
		else
		{
			scrollbar_pos=scrollbar_orig_pos+((mousey-scrollbar_orig_y)*max/(h-32));
			if(scrollbar_pos<0) scrollbar_pos=0;
			if(scrollbar_pos>=max) scrollbar_pos=max-1;
		}
	}
}

void render_type_window(void)
{
	u8 lmb = sfp_event_mouse_button(0);

	// 88x104, 3x4 icons + scroll
	sfp_fill_rect(0,SFP_FIELD_HEIGHT*16-104,88,104,0x000000);
	// Drawing
	u8 i,j,tcol,mouse_in;
	u16 px,py;
	for(i=0;i<12;i++)
	{
		j = i+(scrollbar_pos*3);	
		if(j>=TILE_TYPES) break;
		px = 8+((i%3)*24);
		py = 8+((i/3)*24)+(SFP_FIELD_HEIGHT*16-104);
		mouse_in = inside_rect(sfp_event_mousex(),sfp_event_mousey(),px,py,16,16);
		tcol = tile_get_preview_color(j);
		sfp_putc_2x(px,py,tcol>>4,tcol&15,tile_get_preview_char(j));
		sfp_draw_rect(px-1,py-1,18,18,(mouse_in?0xCCCCCC:0x555555));
		if(mouse_in && lmb) drawing_tile->type = j;
	}
	int tmax = ((TILE_TYPES+2/3)-4);
	render_scrollbar(80-1,(SFP_FIELD_HEIGHT*16-104),104,(tmax>0?tmax:0));

	sfp_draw_rect(0,SFP_FIELD_HEIGHT*16-104,88,104,0xCCCCCC);
}
void render_char_window(void)
{
	u8 lmb = sfp_event_mouse_button(0);

	// 88x104, 3x4 icons + scroll
	sfp_fill_rect(0,SFP_FIELD_HEIGHT*16-104,88,104,0x000000);
	// Drawing
	u8 i,tcol,mouse_in;
	u16 j,px,py;
	tcol = drawing_tile->col;
	for(i=0;i<12;i++)
	{
		j = i+(scrollbar_pos*3);	
		if(j>255) break;
		px = 8+((i%3)*24);
		py = 8+((i/3)*24)+(SFP_FIELD_HEIGHT*16-104);
		mouse_in = inside_rect(sfp_event_mousex(),sfp_event_mousey(),px,py,16,16);
		sfp_putc_2x(px,py,tcol>>4,tcol&15,j);
		sfp_draw_rect(px-1,py-1,18,18,(mouse_in?0xCCCCCC:0x555555));
		if(mouse_in && lmb) drawing_tile->chr = j;
	}
	render_scrollbar(80-1,(SFP_FIELD_HEIGHT*16-104),104,(258/3)-4);

	sfp_draw_rect(0,SFP_FIELD_HEIGHT*16-104,88,104,0xCCCCCC);
}
void render_color_window(void)
{
	u8 lmb = sfp_event_mouse_button(0);

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
				if(activeUI == (mousex>>4))
					activeUI = 0;
				else
					activeUI = (mousex>>4);
			}
		}
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
	if(mouse_left_down==0 && sfp_event_mouse_button(0)) mouse_left_down = 1;
	if(mouse_left_down==1 && !sfp_event_mouse_button(0)) mouse_left_down = 0;
}

void init_ui(void)
{
	isOn=0;
	activeUI=0;
	mouse_left_down=0;
	scrollbar_pos = 0;
	scrollbar_mouse_cooldown = 0;
	scrollbar_dragged = 0;
	drawing_tile = (tile_t *)malloc(sizeof(tile_t));
	drawing_tile->type = TILE_WALL;
	drawing_tile->chr = 1;
	drawing_tile->col = 7;
	drawing_tile->data = NULL;
}
