#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include "common.h"

u8 ui_is_occupied(u16 x, u16 y);
void init_ui(void);
void render_ui(void);
tile_t *ui_get_tile(void);

#endif /* _INTERFACE_H_ */
