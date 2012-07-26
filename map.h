#ifndef _MAP_H_
#define _MAP_H_

#include "common.h"

#define LAYER_SIZE 64 // 32-255, no promises for lower, will crash on higher
#define LAYER_WIDTH 64
#define LAYER_HEIGHT 64

layer_t *layer_new(void);
layer_t *layer_get(s32 x, s32 y);
void layer_set_unused(s32 x, s32 y);

#endif /* _MAP_H_ */
