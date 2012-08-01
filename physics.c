#include "common.h"
#include "map.h"
#include "misc.h"
#include "physics.h"
#include "types.h"

tile_t *find_tile_by_type(tile_t *tile, u8 type)
{
	tile_t *out = tile;
	while(out != NULL && out->type != type)
		out = out->under;

	return out;
}

u8 is_tile_active(tile_t *tile, u8 min_power, u8 dir)
{
	tile_t *out = tile;
	while(out != NULL)
	{
		switch(out->type)
		{
			case TILE_WIRE: {
				if(out->datalen != 4) break;
				u8 max_power = 0;
				if(out->data[dir] > max_power)
					max_power = (out->data[dir] - 1);
				if(out->data[dir^2] > max_power)
					max_power = (out->data[dir^2] - 1);
				if(out->data[dir^3] > max_power)
					max_power = (out->data[dir^3] - 1);
				if(max_power > 0) return max_power;
				break; }
			case TILE_PNAND: {
				if(out->col > 15 && (out->col-24)==dir)
					printf("\nPNAND 15");
					return 15;
				break; }
		}
		out = out->under;
	}
	return 0;
}


int handle_physics_tile(map_t *map, int x, int y, tile_t *tile, u8 uidx)
{
	//printf("Attempting update on %i,%i,%i\n",x,y,uidx);
	
	// assuming map != NULL
	int is_server = (map == server_map);
	int i = 0;
	u8 bg = tile->col>>4;
	u8 fg = tile->col&15;

	// time to get the neighbors
	tile_t *ntiles[4];
	ntiles[0] = map_get_tile_ref(map,x,y-1);
	ntiles[1] = map_get_tile_ref(map,x,y+1);
	ntiles[2] = map_get_tile_ref(map,x+1,y);
	ntiles[3] = map_get_tile_ref(map,x-1,y);

	switch(tile->type)
	{
		case TILE_WIRE: {
			// TEST: flip light for serverside
			if(!is_server) return 0;
			printf("LOLWIRE");
			if(tile->datalen != 4)
			{
				// Set data if it does not exist
				tile->data = malloc(4);
				tile->datalen = 4;
				tile->data[0] = 0;
				tile->data[1] = 0;
				tile->data[2] = 0;
				tile->data[3] = 0;
			}
			u8 changed = 0;
			for(i=0;i<4;i++)
			{
				changed = tile->data[i^1];
				tile->data[i^1] = is_tile_active(ntiles[i],tile->data[i^1], i^1);
				if(tile->data[i^1] != changed) changed++;
			}
			u8 curr_power = MAX(MAX(tile->data[0],tile->data[1]),MAX(tile->data[2],tile->data[3]));
			printf("\nCurrent power: %d\n",curr_power);
			if(curr_power>0 && fg<=7)
			{
				tile->col = (fg&7)+8;
				map->f_set_tile_ext(map, x, y, uidx,
					*tile, is_server);
			} else if(curr_power==0 && fg>7)
			{
				tile->col = (fg&7);
				map->f_set_tile_ext(map, x, y, uidx,
					*tile, is_server);
			}
			if(changed) return HPT_RET_UPDATE_SELF_AND_NEIGHBORS;
			return 0; }
		case TILE_PNAND: {
			if(!is_server) return 0;
			printf("LOLPNAND");
			int j = 0;
			for(i=0;i<4;i++)
			{
				// Check for output direction
				if(i+24 == tile->chr) continue;

				if(find_tile_by_type(ntiles[i],TILE_WALL) ||
				   is_tile_active(ntiles[i], -1, i^1))
				{
					j++;
				}
			}
			if((j==1 || j==2) && fg > 0)
			{
				printf("SET");
				tile->col = (fg<<4);
				map->f_set_tile_ext(map, x, y, uidx,
					*tile, is_server);
				return HPT_RET_UPDATE_SELF_AND_NEIGHBORS;
			}
			else if(!(j==1 || j==2) && bg > 0)
			{
				tile->col = bg;
				map->f_set_tile_ext(map, x, y, uidx,
					*tile, is_server);
				return HPT_RET_UPDATE_SELF_AND_NEIGHBORS;
			}
			return 0; }
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
