#ifndef _PHYSICS_H_
#define _PHYSICS_H_

#include "common.h"
#include "map.h"
#include "tile.h"

void handle_physics(map_t *map,
	tile_t (*f_get_block)(s32, s32),
	void (*f_set_block)(s32, s32, u8, tile_t, int),
	void (*f_alloc_data)(s32, s32, u8, u16, int),
	void (*f_set_data)(s32, s32, u8, u8, u16, u8 *, int));

#endif /* _PHYSICS_H_ */
