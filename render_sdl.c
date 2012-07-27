#include "common.h"

#define I_WANT_INTERNAL_RENDER_STUFF
#include "render.h"

// Rendering code by GreaseMonkey and somewhat asiekierka

SDL_Surface *screen;

void sfp_render_draw_rect(int x, int y, int w, int h, u32 col)
{
	// FIXME: GM, could you please handle this	
}

void sfp_render_fill_rect(int x, int y, int w, int h, u32 col)
{
	// FIXME: GM, could you please handle this	
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
