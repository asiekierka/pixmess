#ifndef _SFP_RENDER_H_
#define _SFP_RENDER_H_

// screen size measured in 2x chars
#define SFP_SCREEN_WIDTH 40
#define SFP_SCREEN_HEIGHT 25

#define SFP_FIELD_WIDTH SFP_SCREEN_WIDTH
#define SFP_FIELD_HEIGHT (SFP_SCREEN_HEIGHT-1)

#define SFP_SCREEN_REAL_WIDTH (SFP_SCREEN_WIDTH*16)
#define SFP_SCREEN_REAL_HEIGHT (SFP_SCREEN_HEIGHT*16)

// flags
#define SFP_TRANSPARENCY 1

u32 sfp_get_palette(u8 id);

void sfp_draw_rect(int x, int y, int w, int h, u32 col);
void sfp_fill_rect(int x, int y, int w, int h, u32 col);

int sfp_init_render();
void sfp_render_begin();
void sfp_render_end();

// unrolled as gcc was still complaining --GM
void sfp_putc_1x(int x, int y, u8 bg, u8 fg, u16 chr);
void sfp_putc_block_1x(int x, int y, u8 bg, u8 fg, u16 chr);
void sfp_printf_1x(int x, int y, int col, int flags, char *fmt, ...);
void sfp_putc_2x(int x, int y, u8 bg, u8 fg, u16 chr);
void sfp_putc_block_2x(int x, int y, u8 bg, u8 fg, u16 chr);
void sfp_printf_2x(int x, int y, int col, int flags, char *fmt, ...);

//
// INTERNAL RENDER STUFF FOLLOWS.
//

#ifdef I_WANT_INTERNAL_RENDER_STUFF

void sfp_render_draw_rect(int x, int y, int w, int h, u32 col);
void sfp_render_fill_rect(int x, int y, int w, int h, u32 col);

void sfp_render_putc_2x(int x, int y, u32 bg, u32 fg, u8 *p, u8 transparency);
void sfp_render_putc_1x(int x, int y, u32 bg, u32 fg, u8 *p, u8 transparency);

int sfp_render_init_video();
void sfp_render_render_begin();
void sfp_render_render_end();
#endif

#endif /* _SFP_RENDER_H_ */
