#ifndef _PHYSICS_H_
#define _PHYSICS_H_

#include "common.h"
#include "map.h"
#include "player.h"
#include "tile.h"

enum
{
	HPT_RET_OK = 0,
	HPT_RET_UPDATE_SELF,
	HPT_RET_UPDATE_SELF_AND_NEIGHBORS
};

void register_tiles_physics();
void handle_physics(map_t *map);

#endif /* _PHYSICS_H_ */
