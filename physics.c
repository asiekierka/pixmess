#include "map.h"
#include "physics.h"
#include "types.h"

int handle_physics_tile(map_t *map, int x, int y, tile_t *tile, u8 uidx)
{
	printf("Attempting update on %i,%i,%i\n",x,y,uidx);
	
	// assuming map != NULL
	int is_server = (map == server_map);
	
	// time to get the neighbors
	static tile_t ntiles[4]; // less waste on the memory
	ntiles[0] = map->f_get_tile(map,x,y-1);
	ntiles[1] = map->f_get_tile(map,x,y+1);
	ntiles[2] = map->f_get_tile(map,x+1,y);
	ntiles[3] = map->f_get_tile(map,x-1,y);

	switch(tile->type)
	{
		case TILE_WIRE:
			// TEST: flip light for serverside
			if(!is_server)
			{
				printf("LOLWIRE\n");
				tile->col++;
				map->f_set_tile_ext(map, x, y, uidx,
					*tile, is_server);
				return HPT_RET_UPDATE_SELF;
			}
			break;
	}
	
	return 0;
}

void handle_physics(map_t *map)
{
	if(map == NULL) return;

	s32 x, y;
	int lidx = 0;
	tile_t *tile;
	u8 uidx;
	u32 changes = 0;
	int is_server = (map == server_map);
	
	// Loopty loop
	while(map_get_next_update(map,&lidx,&x,&y))
	{
		// if(!is_server) printf("Got update on %d,%d",x,y);
		changes++;
		// iterate over all tiles, including underones
		tile = map_get_tile_ref(map,x,y);
		uidx = 0;
		
		while(tile != NULL)
		{
			tile_t new_tile = *tile;
			
			if(tile_active(new_tile))
			{
				int handle_ret = handle_physics_tile(map, x, y, &new_tile, uidx);
				switch(handle_ret)
				{
					default:
						break;
					case HPT_RET_UPDATE_SELF:
						map->f_set_update(map,x,y,1);
						break;
					case HPT_RET_UPDATE_SELF_AND_NEIGHBORS:
						map->f_set_update_n(map,x,y,1);
						break;
				}
				x++;
			}			
			tile = tile->under;
			uidx++;
			
			if(uidx == 0)
			{
				fprintf(stderr, "ERROR: Tile stack overflow!\n");
				break;
			}
		}
	}
	map_switch_masks(map);
}
