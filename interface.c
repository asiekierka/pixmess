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
			if(x<=112 && y>=(SFP_FIELD_HEIGHT*16)-128) return 1;
		case 2:
			if(x<=112 && y>=(SFP_FIELD_HEIGHT*16)-152) return 1;
			break;
		case 3:
			if(x<=208 && y>=(SFP_FIELD_HEIGHT*16)-66) return 1;
			break;
	}
	return 0;
}

tile_t *ui_get_tile(void)
{
	return drawing_tile;
}

int scrollbar_pos = 0;
int scrollbar_pos_save[3];
int scrollbar_dragged;
int scrollbar_mouse_cooldown;
int scrollbar_orig_y;
int scrollbar_orig_pos;

u8 ui_can_mouse_button()
{
	return (scrollbar_dragged==0);
}

void render_scrollbar(u16 x, u16 y, u16 h, u16 max, int mouse_in_parent)
{
	// Sanity checks
	if(max==0 || h%8>0) return;
	// Basic scrollbar
	int i;
	sfp_putc_1x(x,y,8,15,30);
	for(i=8;i<h-8;i+=8)
		sfp_putc_1x(x,y+i,0,8,177);
	sfp_putc_1x(x,y+h-8,8,15,31);

	// The light gray moving part.
	int part_pos = scrollbar_pos*(h-32)/max;
	sfp_fill_rect(x,y+8+part_pos,8,16,0xCCCCCC);

	int mousex = sfp_event_mouse_x();
	int mousey = sfp_event_mouse_y();
	int lmb = sfp_event_mouse_button(0);

	// Moving the scrollbar.
	if(scrollbar_dragged == 0)
	{
		if(scrollbar_mouse_cooldown == 0)
		{
			if(lmb)
			{
				if(scrollbar_pos>0 && inside_rect(mousex,mousey,x,y,8,8))
					{ scrollbar_pos--; scrollbar_mouse_cooldown = 4; }
				else if(scrollbar_pos<(max-1) && inside_rect(mousex,mousey,x,y+h-8,8,8))
					{ scrollbar_pos++, scrollbar_mouse_cooldown = 4; }
				else if(inside_rect(mousex,mousey,x,y+8+part_pos,8,16))
					{ scrollbar_dragged = 1; scrollbar_orig_y = mousey; scrollbar_orig_pos = scrollbar_pos; }
				else if(inside_rect(mousex,mousey,x,y+8,8,h-16)) // Scrolldrag part covered by above
					{ scrollbar_pos = ((mousey-(y+8))*max/(h-32)); }
			}
		}
		else scrollbar_mouse_cooldown--;
		// Scrollwheel
		if(mouse_in_parent && sfp_event_mouse_button_press(3) && scrollbar_pos>0)
			{ scrollbar_pos--; }
		else if(mouse_in_parent && sfp_event_mouse_button_press(4) && scrollbar_pos<(max-1))
			{ scrollbar_pos++; }
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

void render_type_tooltip(u8 id)
{
	int mousex = sfp_event_mouse_x();
	int mousey = sfp_event_mouse_y();

	// Tooltip
	sfp_fill_rect(mousex-1,mousey-10,strlen(tile_get_name(id))*8+2,10,0x000000);
	sfp_printf_1x(mousex,mousey-9,0x07,0,"%s",tile_get_name(id));
}

void verify_tile()
{
	u8 tofix;
	u16 ac_length;
	u16* allowed_chars = tile_get_allowed_chars(*drawing_tile,&ac_length);
	assert(ac_length>0);
	u16 i;
	tofix=1;
	for(i=0;i<ac_length;i++)
		if(allowed_chars[i] == drawing_tile->chr) tofix=0;
	if(tofix==1) drawing_tile->chr = allowed_chars[0];
	tofix=1;
}

void render_type_window(void)
{
	u8 lmb = sfp_event_mouse_button(0);
	int mousex = sfp_event_mouse_x();
	int mousey = sfp_event_mouse_y();

	sfp_fill_rect(0,SFP_FIELD_HEIGHT*16-128,112,128,0x000000);
	// Drawing
	int i,j,tcol,mouse_in;
	int mouse_in_id = -1;
	int px,py;
	for(i=0;i<20;i++)
	{
		j = 1+i+(scrollbar_pos*4);	
		if(j>=TILE_TYPES) break;
		px = 8+((i%4)*24);
		py = 8+((i/4)*24)+(SFP_FIELD_HEIGHT*16-128);
		mouse_in = inside_rect(mousex,mousey,px-2,py-2,20,20);
		tcol = tile_get_preview_color(j);
		sfp_putc_2x(px,py,tcol>>4,tcol&15,tile_get_preview_char(j));
		sfp_draw_rect(px-1,py-1,18,18,(mouse_in?0xCCCCCC:(drawing_tile->type==j?0xAAAAAA:0x555555)));
		if(mouse_in)
		{
			mouse_in_id = j;
			if(lmb)
			{
				drawing_tile->type = j;
				verify_tile();
			}
		}
	}
	mouse_in = inside_rect(sfp_event_mouse_x(),sfp_event_mouse_y(),0,SFP_FIELD_HEIGHT*16-128,112,128);
	int tmax = ((TILE_TYPES+2/4)-4);
	render_scrollbar(104-1,(SFP_FIELD_HEIGHT*16-128),128,(tmax>0?tmax:0),mouse_in);

	sfp_draw_rect(0,SFP_FIELD_HEIGHT*16-128,112,128,0xCCCCCC);

	if(mouse_in_id>=0)
		render_type_tooltip(mouse_in_id);
}
void render_char_window(void)
{
	int lmb = sfp_event_mouse_button(0);

	// 112x152, 4x6 icons + scroll
	sfp_fill_rect(0,SFP_FIELD_HEIGHT*16-152,112,152,0x000000);
	// Drawing
	int i,tcol,mouse_in;
	int j,px,py;
	tcol = drawing_tile->col;
	u16 ac_length;
	u16* allowed_chars = tile_get_allowed_chars(*drawing_tile,&ac_length);
	if(ac_length<(scrollbar_pos*4)) scrollbar_pos = 0;
	for(i=0;i<24;i++)
	{
		if(i+(scrollbar_pos*4)>=ac_length) break;
		if(allowed_chars != NULL)
			j = allowed_chars[i+(scrollbar_pos*4)];
		else
			j = i+(scrollbar_pos*4);
		if(j>255) break;
		px = 8+((i%4)*24);
		py = 8+((i/4)*24)+(SFP_FIELD_HEIGHT*16-152);
		mouse_in = inside_rect(sfp_event_mouse_x(),sfp_event_mouse_y(),px-2,py-2,20,20);
		sfp_putc_2x(px,py,tcol>>4,tcol&15,j);
		sfp_draw_rect(px-1,py-1,18,18,(mouse_in?0xCCCCCC:(drawing_tile->chr==j?0xAAAAAA:0x555555)));
		if(mouse_in && lmb)
		{
			drawing_tile->chr = j;
			verify_tile();	
		}
	}
	
	mouse_in = inside_rect(sfp_event_mouse_x(),sfp_event_mouse_y(),0,SFP_FIELD_HEIGHT*16-152,112,152);
	render_scrollbar(104-1,(SFP_FIELD_HEIGHT*16-152),152,((ac_length+3)/4)-4, mouse_in);

	sfp_draw_rect(0,SFP_FIELD_HEIGHT*16-152,112,152,0xCCCCCC);
}
void render_color_window(void)
{
	int lmb = sfp_event_mouse_button(0);
	int mousex = sfp_event_mouse_x();
	int mousey = sfp_event_mouse_y();

	sfp_fill_rect(0,SFP_FIELD_HEIGHT*16-66,208,66,0x000000);
	// Drawing
	int i;
	sfp_printf_1x(8,SFP_FIELD_HEIGHT*16-32,0x0F,0,"Background");
	sfp_printf_1x(8,SFP_FIELD_HEIGHT*16-60,0x0F,0,"Foreground");
	u8 tfg = drawing_tile->col&15;
	u8 tbg = drawing_tile->col>>4;
	for(i=0;i<16;i++)
	{
		sfp_fill_rect(8+(i*12)+1,SFP_FIELD_HEIGHT*16-20+1,10,10,sfp_get_palette(i));
		sfp_fill_rect(8+(i*12)+1,SFP_FIELD_HEIGHT*16-48+1,10,10,sfp_get_palette(i));
		if(i==0 || sfp_get_palette(i)==0x000000)
		{
			sfp_draw_rect(8,SFP_FIELD_HEIGHT*16-20,12,12,0x555555);
			sfp_draw_rect(8,SFP_FIELD_HEIGHT*16-48,12,12,0x555555);
		}
		if(i==tfg)
			sfp_draw_rect(8+(i*12),SFP_FIELD_HEIGHT*16-48,12,12,0xCCCCCC);
		if(i==tbg)
			sfp_draw_rect(8+(i*12),SFP_FIELD_HEIGHT*16-20,12,12,0xCCCCCC);
		if(inside_rect(mousex,mousey,8+(i*12),SFP_FIELD_HEIGHT*16-48,12,12) && lmb)
			tfg = i;
		if(inside_rect(mousex,mousey,8+(i*12),SFP_FIELD_HEIGHT*16-20,12,12) && lmb)
			tbg = i;
	}
	drawing_tile->col = (tbg<<4)|tfg;
	verify_tile();
	sfp_draw_rect(0,SFP_FIELD_HEIGHT*16-66,208,66,0xCCCCCC);
}

void render_ui(void)
{
	// First, grab mouse stuff.
	int mousex = sfp_event_mouse_x();
	int mousey = sfp_event_mouse_y();

	sfp_fill_rect(0,SFP_FIELD_HEIGHT*16,SFP_SCREEN_REAL_WIDTH,16,0x000000);

	// Prepare some variables.
	u8 whichHL = 0;
	if(inside_rect(mousex,mousey,1*16,SFP_FIELD_HEIGHT*16,UI_BUTTONS*16,1*16))
	{
		whichHL = (mousex>>4);
		if(sfp_event_mouse_button(0))
		{
			if(mouse_left_down==0)
			{
				if(activeUI >= 1 && activeUI <= 3)
					scrollbar_pos_save[activeUI-1] = scrollbar_pos;
				
				if(activeUI == (mousex>>4))
					activeUI = 0;
				else
					activeUI = (mousex>>4);
				
				if(activeUI >= 1 && activeUI <= 3)
					scrollbar_pos = scrollbar_pos_save[activeUI-1];
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

	// Tooltip
	if(inside_rect(mousex,mousey,5*16,SFP_FIELD_HEIGHT*16,16,16))
		render_type_tooltip(drawing_tile->type);
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
