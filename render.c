#include "common.h"

#define I_WANT_INTERNAL_RENDER_STUFF
#include "render.h"
#include "render_data.h"

// This code is insanely DRY.

u8 palette[16*3];
u8 font[32768*8];

u8 render_initialized = 0;

u32 sfp_get_palette(u8 id)
{
	return palette[id*3+2] | (palette[id*3+1]<<8) | (palette[id*3]<<16);
}

int sfp_init_render()
{
	// Load defaults.
	memset(font, 0, 32768*8);
	memcpy(font, font_cga, 256*8);
	memcpy(palette, palette_cga, 16*3);
	
	// Now call the real deal.
	if(sfp_render_init_video())
		return 1;

	render_initialized = 255;
	
	return 0;
}

void sfp_render_begin()
{
	// safeguard! idk why
	if(!render_initialized)
		if(sfp_init_render())
		{
			// apparently this is a bad thing,
			// but you SHOULD have called sfp_init_render beforehand.
			// --GM
			fprintf(stderr, "FATAL: sfp_render_begin safeguard failed!\n");
			abort();
		}

	sfp_render_render_begin();
}
void sfp_render_end() { sfp_render_render_end(); }

void sfp_draw_rect(int x, int y, int w, int h, u32 col)
{
	sfp_render_draw_rect(x,y,w,h,col);
}
void sfp_fill_rect(int x, int y, int w, int h, u32 col)
{
	sfp_render_fill_rect(x,y,w,h,col);
}

#define SFP_DEFINE_PUTC(putctype,putcsize) \
void sfp_putc_##putctype(int x, int y, u8 bg, u8 fg, u16 chr) \
{ \
	u8 *fgp = &palette[fg*3]; \
	u8 *bgp = &palette[bg*3]; \
	u32 realfg = fgp[2] | (fgp[1] << 8) | (fgp[0] << 16); \
	u32 realbg = bgp[2] | (bgp[1] << 8) | (bgp[0] << 16); \
	u8 *fptr = &font[(chr & 0x7FFF)*8]; \
	if(chr>=32768) printf("TRANSPARENCY!!! %d\n",chr); \
	sfp_render_putc_##putctype(x, y, realbg, realfg, fptr, (((chr&32768)&&(bg==0))?1:0)); \
} \
\
void sfp_putc_block_##putctype(int x, int y, u8 bg, u8 fg, u16 chr) \
{ sfp_putc_##putctype(x*8*(putcsize),y*8*(putcsize),bg,fg,chr); }

SFP_DEFINE_PUTC(2x,2);
SFP_DEFINE_PUTC(1x,1);

// cutting down on redundancy... at least in the source code --GM
#define SFP_DEFINE_PRINTF(putctype,putcsize) \
void sfp_printf_##putctype(int x, int y, int col, int flags, char *fmt, ...) \
{ \
	/* sanity checks, also skip unnecessary parsing / printing! */ \
	if(y < 0 || y >= SFP_SCREEN_REAL_HEIGHT || x >= SFP_SCREEN_REAL_WIDTH) \
		return; \
	 \
	char buf[512]; \
	int i,slen; \
	 \
	buf[0] = '\0'; \
	 \
	/* varargs vsnprintf crap */ \
	va_list argp; \
	va_start(argp, fmt); \
	vsnprintf(buf, sizeof(buf)-1, fmt, argp); \
	va_end(argp); \
	 \
	/* print char by char */ \
	slen = strlen(buf); \
	for(i = 0; i < slen; i++) \
	{ \
		if(x >= SFP_SCREEN_REAL_WIDTH) \
			break; \
		 \
		if(x >= 0) \
			sfp_putc_##putctype(x, y, (col>>4), (col&15), buf[i] | ((flags&SFP_TRANSPARENCY)?32768:0)); \
		 \
		x+=(8*(putcsize)); \
	} \
}

SFP_DEFINE_PRINTF(2x,2);
SFP_DEFINE_PRINTF(1x,1);
