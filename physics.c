#include "map.h"
#include "physics.h"
#include "types.h"

int handle_physics_tile(map_t *map, int x, int y, tile_t *tile, u8 uidx,
	tile_t (*f_get_block)(s32, s32),
	void (*f_set_block)(s32, s32, u8, tile_t, int),
	void (*f_alloc_data)(s32, s32, u8, u16, int),
	void (*f_set_data)(s32, s32, u8, u8, u16, u8 *, int))
{
	printf("Attempting update on %i,%i,%i\n",x,y,uidx);
	
	// assuming map != NULL
	int is_server = (map == server_map);
	
	switch(tile->type)
	{
		case TILE_WIRE:
			// TEST: flip light for clientside
			if(!is_server)
			{
				printf("LOLWIRE\n");
				tile->col ^= 0x08;
				f_set_block(x,y,uidx,*tile,0);
			}
			break;
	}
	
	return 0;
}

void handle_physics(map_t *map,
	tile_t (*f_get_block)(s32, s32),
	void (*f_set_block)(s32, s32, u8, tile_t, int),
	void (*f_alloc_data)(s32, s32, u8, u16, int),
	void (*f_set_data)(s32, s32, u8, u8, u16, u8 *, int))
{
	s32 x, y;
	int lidx = 0;
	tile_t *tile;
	u8 uidx;
	
	// Loopty loop
	while(map_get_next_update(map,&lidx,&x,&y))
	{
		//printf("Got an update on %d,%d\n",x,y);
		// iterate over all tiles, including underones
		tile = map_get_tile_ref(map,x,y);
		uidx = 0;
		
		while(tile != NULL)
		{
			tile_t new_tile = *tile;
			
			if(tile_active(new_tile))
			{
				int handle_ret = handle_physics_tile(map, x, y, &new_tile, uidx,
					f_get_block, f_set_block, f_alloc_data, f_set_data);
				x++;
				
				// TODO: handle some possible return codes here!
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
}
