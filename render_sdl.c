#include "common.h"

#define I_WANT_INTERNAL_RENDER_STUFF
#include "render.h"

// Rendering code by GreaseMonkey and somewhat asiekierka

SDL_Surface *screen;

void sfp_render_draw_rect(int x, int y, int w, int h, u32 col)
{
	int i;
	
	// sanity check: w/h MUST BE POSITIVE
	if(w <= 0 || h <= 0)
		return;
	
	// get x,y TLC/BRC
	int x1 = x, x2 = x+w-1;
	int y1 = y, y2 = y+h-1;
	
	// don't attempt anything if we can't draw anything
	if(x2 < 0 || x1 >= screen->w || y2 < 0 || y1 >= screen->h)
		return;
	
	// set some flags
	int do_l = (x1 >= 0);
	int do_t = (y1 >= 0);
	int do_r = (x2 < screen->w);
	int do_b = (y2 < screen->h);
	
	if(!do_l) x1 = 1;
	if(!do_t) y1 = 0;
	if(!do_r) x2 = screen->w-2;
	if(!do_b) y2 = screen->h-1;
	
	// save on some redundancy
	do_r = do_r && x1 != x2;
	do_b = do_b && y1 != y2;
	
	// get our TLC pointer
	uint32_t *vb = (uint32_t *)(screen->pixels + y1*screen->pitch + x1*4);
	
	// now render each line!
	if(do_t)
	{
		uint32_t *v = vb+1;
		for(i = x1+1; i < x2; i++)
			*(v++) = col;
	}
	
	if(do_b)
	{
		uint32_t *v = (uint32_t *)(screen->pixels + y2*screen->pitch + (x1+1)*4);
		for(i = x1+1; i < x2; i++)
			*(v++) = col;
	}
	
	if(do_l)
	{
		uint32_t *v = vb;
		for(i = y1; i <= y2; i++)
		{
			*v = col;
			v = (uint32_t *)(((uint8_t *)v) + screen->pitch);
		}
	}
	
	if(do_r)
	{
		uint32_t *v = (uint32_t *)(screen->pixels + y1*screen->pitch + x2*4);
		for(i = y1; i <= y2; i++)
		{
			*v = col;
			v = (uint32_t *)(((uint8_t *)v) + screen->pitch);
		}
	}
	
}

void sfp_render_fill_rect(int x, int y, int w, int h, u32 col)
{
	// clamp x and y TLC
	if(x < 0)
	{
		w += x;
		x = 0;
	}
	
	if(y < 0)
	{
		h += y;
		y = 0;
	}
	
	// clamp w and h BRC
	if(x+w > screen->w)
		w = screen->w-x;
	if(y+h > screen->h)
		h = screen->h-y;
	
	// don't attempt anything if we can't draw anything
	if(w <= 0 || h <= 0)
		return;
	
	// get TLC pointer
	uint32_t *v = (uint32_t *)(screen->pixels + y*screen->pitch + x*4);
	
	// draw!
	int offs = screen->pitch - w*4;
	for(; h > 0; h--)
	{
		int i;
		
		// oh how i wish for a STOSD... -O2 might provide it here --GM
		for(i = 0; i < w; i++)
			*(v++) = col;
		
		v = (uint32_t *)(((uint8_t *)v) + offs);
	}
}

int sfp_sdl_error(char *ref)
{
	fprintf(stderr, "%s: %s\n", ref, SDL_GetError());
	return 1;
}

int sfp_render_init_video()
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE))
		return sfp_sdl_error("SDL_Init"); // asie this would be a better idea --GM
	SDL_WM_SetCaption("c64pixels v0.-1", NULL);
	
	screen = SDL_SetVideoMode(
		SFP_SCREEN_WIDTH*16,
		SFP_SCREEN_HEIGHT*16,
		32,
		SDL_SWSURFACE | SDL_DOUBLEBUF);
	if(screen == NULL)
		return sfp_sdl_error("SDL_SetVideoMode");
	
	return 0;
}

void sfp_render_render_begin()
{
	SDL_LockSurface(screen);
}

void sfp_render_render_end()
{
	SDL_UnlockSurface(screen);
	SDL_Flip(screen);
}

void sfp_render_putc_2x(int x, int y, u32 bg, u32 fg, u8 *p)
{
	// calculate screen pointer
	u32 *v = (uint32_t *)(screen->pixels + screen->pitch*y + 4*x);
	u32 *v2 = (uint32_t *)(((uint8_t *)v) + screen->pitch);
	
	// render!
	int minx = -x, miny = -y;
	int maxx = screen->w-x, maxy = screen->h-y;
	int i,j;
	int pitchoffs = screen->pitch*2 - 4*2*8;
	for(i = 0; i < 8; i++)
	{
		if(i >= maxy)
			break;
		
		u8 c = *(p++);
		
		if(i < miny)
		{
			v += 16;
			v2 += 16;
		} else {
			for(j = 0; j < 8; j++)
			{
				if(j < minx || j >= maxx)
				{
					v += 2;
					v2 += 2;
				} else if(c&128) {
					*(v++) = fg;
					*(v++) = fg;
					*(v2++) = fg;
					*(v2++) = fg;
				} else {
					*(v++) = bg;
					*(v++) = bg;
					*(v2++) = bg;
					*(v2++) = bg;
				}
			
				c <<= 1;
			}
		}
		
		v = (u32 *)(((u8 *)v) + pitchoffs);
		v2 = (u32 *)(((u8 *)v2) + pitchoffs);
	}
}

void sfp_render_putc_1x(int x, int y, u32 bg, u32 fg, u8 *p)
{
	// calculate screen pointer
	u32 *v = (u32 *)(screen->pixels + screen->pitch*y + 4*x);
	
	// render!
	int minx = -x, miny = -y;
	int maxx = screen->w-x, maxy = screen->h-y;
	int i,j;
	int pitchoffs = screen->pitch - 4*8;
	for(i = 0; i < 8; i++)
	{
		if(i >= maxy)
			break;
		
		u8 c = *(p++);
		
		if(i < miny)
			v += 8;
		else
		for(j = 0; j < 8; j++)
		{
			if(j >= maxx)
				break;
			
			if(j < minx || j >= maxx)
				v++;
			else
				*(v++) = ((c&128) ? fg : bg);
			
			c <<= 1;
		}
		
		v = (u32 *)(((u8 *)v) + pitchoffs);
	}
}
