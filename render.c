#include "common.h"

#define I_WANT_INTERNAL_RENDER_STUFF
#include "render.h"
#include "render_data.h"

// This code is insanely DRY.

u8 palette[16*3];
u8 font[256*8];

u8 initialized;

void sfp_init_render()
{
	// Load defaults.
	memcpy(font, font_cga, 256*8);
	memcpy(palette, palette_cga, 16*3);

	// Now call the real deal.
	sfp_render_init_video();

	initialized = 255;
}

void sfp_render_begin()
{
	// safeguard! idk why
	if(!initialized) sfp_init_render();

	sfp_render_render_begin();
}
void sfp_render_end() { sfp_render_render_end(); }

#define SFP_DEFINE_PUTC(putctype,putcsize) \
void sfp_putc_##putctype(int x, int y, u8 bg, u8 fg, u16 chr) \
{ \
	u8 *fgp = &palette[fg*3]; \
	u8 *bgp = &palette[bg*3]; \
	u32 realfg = fgp[2] | (fgp[1] << 8) | (fgp[0] << 16); \
	u32 realbg = bgp[2] | (bgp[1] << 8) | (bgp[0] << 16); \
	u8 *fptr = &font[chr*8]; \
	sfp_render_putc_##putctype(x, y, realbg, realfg, fptr); \
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
			sfp_putc_##putctype(x, y, (col>>4), (col&15), buf[i]); \
		 \
		x+=(8*(putcsize)); \
	} \
}

SFP_DEFINE_PRINTF(2x,2);
SFP_DEFINE_PRINTF(1x,1);
