#ifndef _RENDER_H_
#define _RENDER_H_

// screen size measured in 2x chars
#define SFP_SCREEN_WIDTH 40
#define SFP_SCREEN_HEIGHT 25

#define SFP_SCREEN_REAL_WIDTH (SFP_SCREEN_WIDTH*16)
#define SFP_SCREEN_REAL_HEIGHT (SFP_SCREEN_HEIGHT*16)

#ifdef I_WANT_INTERNAL_RENDER_STUFF
#include <SDL.h>

void sfp_render_putc_2x(int x, int y, u32 bg, u32 fg, u8 *p);
void sfp_render_putc_1x(int x, int y, u32 bg, u32 fg, u8 *p);
void sfp_render_init_video();
void sfp_render_render_begin();
void sfp_render_render_end();
#endif

void sfp_render_begin();
void sfp_render_end();

#define SFP_SIZED_DEFINES(putctype) \
void sfp_putc_##putcsize(int x, int y, u8 bg, u8 fg, u16 chr); \
void sfp_putc_block_##putcsize(int x, int y, u8 bg, u8 fg, u16 chr); \
void sfp_printf_##putcsize(int x, int y, int col, int flags, char *fmt, ...);

SFP_SIZED_DEFINES(2x);
SFP_SIZED_DEFINES(1x);

#endif /* _RENDER_H_ */
