#include "map.h"
#include "physics.h"
#include "types.h"

tile_t* handle_physics_tile(tile_t *origTile, s32 x, s32 y, map_t *map)
{
	return NULL;
}

void handle_physics(map_t *map, void (*setBlockFunc)(map_t *, s32, s32, tile_t *, u32))
{
	s32 x, y;
	tile_t *tile;
	u32 underCounter;

	// Loopty loop
	while(map_get_next_update(map,&x,&y))
	{
		printf("Got an update on %d,%d\n",x,y);
		// iterate over all tiles, including underones
		tile = map_get_tile_ref(map,x,y);
		underCounter = 0;

		while(tile != NULL && !tile_active(*tile))
		{
			tile = tile->under;
			underCounter++;
		}

		if(tile != NULL)
		{
			handle_physics_tile(tile,x,y,map);
			(*setBlockFunc)(map,x,y,tile,underCounter);
		}
	}
}
