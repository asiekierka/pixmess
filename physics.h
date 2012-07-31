#ifndef _PHYSICS_H_
#define _PHYSICS_H_

#include "common.h"
#include "map.h"
#include "tile.h"

void handle_physics(map_t *map, void (*setBlockFunc)(s32, s32, tile_t));

#endif /* _PHYSICS_H_ */
