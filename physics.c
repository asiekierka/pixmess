#include "common.h"
#include "map.h"
#include "misc.h"
#include "physics.h"
#include "types.h"

// y-1, y+1, x+1, x-1
const u16 wirium_chars[16] = {
	197, // 0
	179, // N
	179, // S
	179, // NS
	196, // E
	192, // EN
	218, // ES
	195, // ENS
	196, // W
	217, // WN
	191, // WS
	180, // WNS
	196, // WE
	193, // WEN
	194, // WES
	197  // WENS
};

pblocklist_t *blocklist;

void add_tile(s32 x, s32 y, u8 uidx, tile_t *tile, u8 copy)
{
	tile_t *tile2 = tile;
	if(copy>0)
	{
		tile2 = (tile_t *)malloc(sizeof(tile_t));
		memcpy(tile2,tile,sizeof(tile_t));
	}
	pblocklist_t *old_list = blocklist;
	blocklist = (pblocklist_t*)malloc(sizeof(pblocklist_t));
	blocklist->x = x;
	blocklist->y = y;
	blocklist->uidx = uidx;
	blocklist->tile = tile2;
	blocklist->next = old_list;
}

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
				if(out->datalen != 2) break;
				u8 max_power = (out->data[0] - 1);
				if((dir<4 && out->data[1] == (dir^1)) || max_power>31) return 0;
				return max_power;
				break; }
			case TILE_PNAND: {
				if(out->col > 15 && ((dir>3) || (out->chr-24)==dir))
				{
					return 15;
				}
				break; }
		}
		out = out->under;
	}
	return 0;
}

u8 can_tile_active(tile_t *tile)
{
	tile_t *out = tile;
	while(out != NULL)
	{
		if(tile_active(*tile)) return 1;
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
			if(tile->datalen != 2)
			{
				if(tile->datalen > 0) free(tile->data);
				// Set data if it does not exist
				tile->data = malloc(2);
				tile->datalen = 2;
				tile->data[0] = 0;
				tile->data[1] = 4;
			}
			u8 changed = 0;
			u8 curr_power = 0;
			u16 char_old = tile->chr;
			u8 old_dir = tile->data[1];
			tile->data[1] = 4;
			u8 char_sel = 0;
			for(i=0;i<4;i++)
			{
				//if((old_dir^1)==i) continue;
				changed = is_tile_active(ntiles[i],curr_power,i^1);
				if(can_tile_active(ntiles[i])) char_sel |= 1<<i;
				if(changed > curr_power)
				{
					curr_power = changed;
					tile->data[1] = i^1;
				}
			}
			tile->chr = wirium_chars[char_sel];
			tile->data[0] = curr_power;
			if(curr_power>0 && ((fg&7)+8)!=fg)
			{
				tile->col = (fg&7)+8;
				add_tile(x,y,uidx,tile,1);
				return HPT_RET_UPDATE_SELF_AND_NEIGHBORS;
			} else if(curr_power==0 && (fg&7)!=fg)
			{
				tile->col = (fg&7);
				add_tile(x,y,uidx,tile,1);
				return HPT_RET_UPDATE_SELF_AND_NEIGHBORS;
			} else if(tile->chr != char_old)
			{
				add_tile(x,y,uidx,tile,1);
				return HPT_RET_UPDATE_SELF;
			}
			return 0; }
		case TILE_PNAND: {
			if(!is_server) return 0;
			int j = 0;
			for(i=0;i<4;i++)
			{
				// Check for output direction
				if(i+24 == tile->chr) continue;
				printf("%d %d: Checking position %d\n",x,y,i);

				if(find_tile_by_type(ntiles[i],TILE_WALL) ||
				   is_tile_active(ntiles[i], 0, 4)>0)
				{
					printf("%d %d: I PNANDed at position %d\n",x,y,i);
					j++;
				}
			}
			if((j==1 || j==2) && fg > 0)
			{
				tile->col = (fg<<4);
				add_tile(x,y,uidx,tile,1);
				return HPT_RET_UPDATE_SELF_AND_NEIGHBORS;
			}
			else if(!(j==1 || j==2) && bg > 0)
			{
				tile->col = bg;
				add_tile(x,y,uidx,tile,1);
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

	tile_t *next_tile;
	pblocklist_t *old_list;
	while(blocklist != NULL)
	{
		map->f_set_tile_ext(map, blocklist->x, blocklist->y, blocklist->uidx,
			*blocklist->tile, is_server);
		old_list = blocklist;
		blocklist = blocklist->next;
		free(old_list->tile);
		free(old_list);
	}
	map_switch_masks(map);
}
