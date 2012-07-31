#include "map.h"
#include "physics.h"
#include "types.h"

tile_t* handle_physics_tile(tile_t *origTile, s32 x, s32 y, map_t *map);

void handle_physics(map_t *map, void (*setBlockFunc)(s32, s32, tile_t))
{
	s32 x, y;
	tile_t *tile;
	tile_t *tile2;

	// Loopty loop
	while(map_get_next_update(map,&x,&y))
	{
		printf("Got an update on %d,%d\n",x,y);
		// iterate over all tiles, including underones
		tile = (tile_t *)malloc(sizeof(tile_t));
		while(tile != NULL && !tile_active(*tile))
		{
			tile = (tile_t *)malloc(sizeof(tile_t));
			*tile = map_get_tile(map,x,y);
		}
		if(tile != NULL)
		{
			// handle physics on tile
			tile2 = handle_physics_tile(tile,x,y,map);
			if(tile2 != NULL)
			{
				(*setBlockFunc)(x,y,*tile);
				free(tile2);
			}
			free(tile);
		}
	}
}

tile_t* handle_physics_tile(tile_t *origTile, s32 x, s32 y, map_t *map)
{
	return NULL;
}
