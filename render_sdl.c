#include "common.h"
#include "render.h"

// Rendering code by GreaseMonkey

SDL_Surface *screen;

int sfp_sdl_error(char *ref)
{
	fprintf(stderr, "%s: %s\n", ref, SDL_GetError());
	return 1;
}

void sfp_render_init_video()
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE))
		exit(sfp_sdl_error("SDL_Init")); // Asie, don't. They say if you do this, you'll die.	
	SDL_WM_SetCaption("c64pixels v0.-1", NULL);
	
	screen = SDL_SetVideoMode(
		SFP_SCREEN_WIDTH*16,
		SFP_SCREEN_HEIGHT*16,
		32,
		SDL_SWSURFACE | SDL_DOUBLEBUF);
	if(screen == NULL)
		exit(sfp_sdl_error("SDL_SetVideoMode"));
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
	int i,j;
	int pitchoffs = screen->pitch*2 - 4*2*8;
	for(i = 0; i < 8; i++)
	{
		u8 c = *(p++);
		
		for(j = 0; j < 8; j++)
		{
			if(c&128)
			{
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
		
		v = (u32 *)(((u8 *)v) + pitchoffs);
		v2 = (u32 *)(((u8 *)v2) + pitchoffs);
	}
}

void sfp_render_putc_1x(int x, int y, u32 bg, u32 fg, u8 *p)
{
	// calculate screen pointer
	u32 *v = (u32 *)(screen->pixels + screen->pitch*y + 4*x);
	
	// render!
	int i,j;
	int pitchoffs = screen->pitch - 4*8;
	for(i = 0; i < 8; i++)
	{
		u8 c = *(p++);
		
		for(j = 0; j < 8; j++)
		{
			*(v++) = ((c&128) ? fg : bg);
			c <<= 1;
		}
		
		v = (u32 *)(((u8 *)v) + pitchoffs);
	}
}
