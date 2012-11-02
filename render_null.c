#include "common.h"

#define I_WANT_INTERNAL_RENDER_STUFF
#include "render.h"

void sfp_render_draw_rect(int x, int y, int w, int h, u32 col)
{
	#ifdef DEBUG_RENDER
	printf("sfp_render_draw_rect(%d,%d,%d,%d,%d)\n",x,y,w,h,col);	
	#endif
}

void sfp_render_fill_rect(int x, int y, int w, int h, u32 col)
{
	#ifdef DEBUG_RENDER
	printf("sfp_render_fill_rect(%d,%d,%d,%d,%d)\n",x,y,w,h,col);	
	#endif
}

int sfp_render_init_video()
{
	return 0;
}

void sfp_render_render_begin()
{
}

void sfp_render_render_end()
{
}

// NOTE: THESE VERSIONS REQUIRE THE FOLLOWING CONDITIONS:
// - the whole box must be in range.
// - ... i think that's it.
inline void sfp_render_putc_2x_fast(int x, int y, u32 bg, u32 fg, u8 *p)
{
	#ifdef DEBUG_RENDER
	printf("sfp_render_putc_2x_fast(%d,%d,%d,%d,*p)\n",x,y,bg,fg);	
	#endif
}

inline void sfp_render_putc_1x_fast(int x, int y, u32 bg, u32 fg, u8 *p)
{
	#ifdef DEBUG_RENDER
	printf("sfp_render_putc_1x_fast(%d,%d,%d,%d,*p)\n",x,y,bg,fg);	
	#endif
}

// NORMAL VERSIONS.
void sfp_render_putc_2x(int x, int y, u32 bg, u32 fg, u8 *p)
{
	#ifdef DEBUG_RENDER
	printf("sfp_render_putc_2x(%d,%d,%d,%d,*p)\n",x,y,bg,fg);	
	#endif
}

void sfp_render_putc_1x(int x, int y, u32 bg, u32 fg, u8 *p)
{
	#ifdef DEBUG_RENDER
	printf("sfp_render_putc_1x(%d,%d,%d,%d,*p)\n",x,y,bg,fg);
	#endif
}
