#include "physics.h"

void handle_physics(map_t *map)
{
	s32 x, y;
	tile_t tile;
	// Loopty loop
	while(map_get_next_update(&x,&y))
	{
		printf("Got an update on %d,%d\n",x,y);
		tile = map_get_tile(map,x,y);
		if(tile_active(tile))
		{
			printf("Seems active\n");
		}
	}
}
