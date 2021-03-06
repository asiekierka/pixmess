#include "common.h"
#include "map.h"
#include "misc.h"
#include "physics.h"
#include "types.h"

// DIRECTION CHEATSHEET: y-1, y+1, x+1, x-1

/* CHAR SEL EXPLANATION:
   The character selection code, 1<<dir, sets one of the directions. This
   is the table for the resulting character. */
 
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

#define DATA_FRAGILE 1
#define DATA_NOT_FRAGILE 0

#define SELF_ADD_TILE add_tile(x,y,uidx,tile,DATA_FRAGILE)
#define OPPOSITE(a) ((a)^1)
#define CHR_PNAND(a) ((a)-24)
#define PNAND_CHR(a) ((a)+24)
#define MAXIMUM_POWER 15

void add_tile(s32 x, s32 y, u8 uidx, tile_t *tile, u8 copy)
{
	tile_t *tile2 = tile;
	if(copy>0)
	{
		int tile_size = sizeof(*tile);
		tile2 = (tile_t *)malloc(tile_size);
		memcpy(tile2,tile,tile_size);
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

u8 is_tile_active_wire(tile_t *out, u8 min_power, u8 dir)
{
	if(out->datalen != 2) return 0;
	u8 out_power = (out->data[0] - 1);
	if((dir<4 && out->data[1] == (OPPOSITE(dir))) || out_power>MAXIMUM_POWER) return 0;
	return out_power;
}

u8 is_tile_active_pnand(tile_t *out, u8 min_power, u8 dir)
{
	if(out->col > 15 && ((dir > 3) || (CHR_PNAND(out->chr)==dir)))
		return MAXIMUM_POWER;
	return 0;
}

u8 is_tile_active_crosser(tile_t *out, u8 min_power, u8 dir)
{
	if(out->datalen != 4) return 0;
	u8 offset = dir & 2;
	u8 out_dir = out->data[offset+1];
	u8 out_power = out->data[offset];
	if((dir<4 && dir == OPPOSITE(out_dir)) || out_power > MAXIMUM_POWER) return 0;
	return out_power;
}

u8 is_tile_active_plate(tile_t *out, u8 min_power, u8 dir)
{
	if(out->datalen != 1) return 0;
	return out->data[0]>0?MAXIMUM_POWER:0;
}

u8 is_tile_active(tile_t *tile, u8 min_power, u8 dir)
{
	tile_t *out = tile;
	while(out != NULL)
	{
		if(tile_active(*out))
		{
			tileinfo_t *ti = tileinfo_get(out->type);
			if(ti->f_is_active)
				return ti->f_is_active(out,min_power,dir);
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

int handle_physics_tile_wire(map_t *map, s32 x, s32 y, tile_t *tile, u8 uidx, tile_t **ntiles, u8 is_server)
{
	int dir = 0;
	u8 bg = tile->col>>4;
	u8 fg = tile->col&15;

	if(!is_server) return 0;

	if(tile->datalen != 2) // Fix the data, if corrupted.
	{
		if(tile->datalen > 0 || tile->data != NULL) free(tile->data);
		// Set data if it does not exist
		tile->data = malloc(2);
		tile->datalen = 2;
		tile->data[0] = 0;
		tile->data[1] = 4;
	}
	u8 neighbour_power = 0;
	u8 curr_power = 0;
	u16 char_old = tile->chr; // Used to check for updates.
	u8 old_dir = tile->data[1];
	tile->data[1] = 4;
	u8 old_power = tile->data[0];
	u8 char_sel = 0; // Selects the character that the wire will end up using.

	for(dir=0;dir<4;dir++)
	{
		neighbour_power = is_tile_active(ntiles[dir],curr_power,OPPOSITE(dir));
		if(can_tile_active(ntiles[dir])) char_sel |= 1<<dir;
		if(neighbour_power >= curr_power)
		{
			curr_power = neighbour_power;
			tile->data[1] = OPPOSITE(dir);
		}
	}

	tile->chr = wirium_chars[char_sel];
	tile->data[0] = curr_power;

	if(curr_power>0 && ((fg&7)+8)!=fg)
	{
		tile->col = (fg&7)+8;
		SELF_ADD_TILE;
		return HPT_RET_UPDATE_SELF_AND_NEIGHBORS;
	} else if(curr_power==0 && (fg&7)!=fg)
	{
		tile->col = (fg&7);
		SELF_ADD_TILE;
		return HPT_RET_UPDATE_SELF_AND_NEIGHBORS;
	} else if(tile->chr != char_old)
		SELF_ADD_TILE;

	if(old_dir != tile->data[1] || old_power != tile->data[0] || tile->chr != char_old)
		return HPT_RET_UPDATE_SELF_AND_NEIGHBORS;
	return 0;
}

int handle_physics_tile_pnand(map_t *map, s32 x, s32 y, tile_t *tile, u8 uidx, tile_t **ntiles, u8 is_server)
{
	if(!is_server) return 0;

	u8 bg = tile->col>>4;
	u8 fg = tile->col&15;

	int dir = 0;
	int active_sides = 0;
	for(dir=0;dir<4;dir++)
	{
		if(PNAND_CHR(dir) == tile->chr) continue;

		if((!can_tile_active(ntiles[dir]) && find_tile_by_type(ntiles[dir],TILE_WALL)) ||
		   is_tile_active(ntiles[dir], 0, OPPOSITE(dir))>0)
			active_sides++;
	}

	if((active_sides==1 || active_sides==2) && fg > 0)
	{
		tile->col = (fg<<4);
		SELF_ADD_TILE;
		return HPT_RET_UPDATE_SELF_AND_NEIGHBORS;
	}
	else if(!(active_sides==1 || active_sides==2) && bg > 0)
	{
		tile->col = bg;
		SELF_ADD_TILE;
		return HPT_RET_UPDATE_SELF_AND_NEIGHBORS;
	}
	return 0;
}

int handle_physics_tile_crosser(map_t *map, s32 x, s32 y, tile_t *tile, u8 uidx, tile_t **ntiles, u8 is_server)
{
	if(!is_server) return 0;
	if(tile->datalen != 4) // Fix the data, if corrupted.
	{
		if(tile->datalen > 0 || tile->data != NULL) free(tile->data);
		// Set data if it does not exist
		tile->data = malloc(4);
		tile->datalen = 4;
		tile->data[1] = 4;
		tile->data[3] = 4;
	}
	u8 neighbour_power = 0;

	u8 old_d1 = tile->data[1];
	u8 old_d3 = tile->data[3];
	tile->data[0] = 0;
	tile->data[1] = 4;
	tile->data[2] = 0;
	tile->data[3] = 4;

	u8 changed = 0;
	u8 dir = 0;
	for(dir=0;dir<4;dir++)
	{
		u8 offset = dir & 2;
		neighbour_power = is_tile_active(ntiles[dir],tile->data[offset],OPPOSITE(dir));
		if(neighbour_power > tile->data[offset])
		{
			tile->data[offset] = neighbour_power;
			tile->data[offset+1] = OPPOSITE(dir);
		}
	}
	if(old_d1 != tile->data[1] || old_d3 != tile->data[3])
	{
		SELF_ADD_TILE;
		return HPT_RET_UPDATE_SELF_AND_NEIGHBORS;
	}
	return 0;
}

int handle_physics_tile_plate(map_t *map, s32 x, s32 y, tile_t *tile, u8 uidx, tile_t **ntiles, u8 is_server)
{
	if(!is_server) return 0;

	if(tile->datalen != 1) // Fix the data, if corrupted.
	{
		if(tile->datalen > 0 || tile->data != NULL) free(tile->data);
		tile->data = malloc(1); tile->datalen = 1;
		tile->data[0] = 0;
	}

	u8 prev_plate = tile->data[0];
	u8 new_plate = player_is_occupied(x,y);
	if(new_plate != prev_plate)
	{
		tile->data[0] = new_plate;
		SELF_ADD_TILE;
		return HPT_RET_UPDATE_SELF_AND_NEIGHBORS;
	}
	return 0;
}

int handle_physics_tile(map_t *map, s32 x, s32 y, tile_t *tile_old, u8 uidx)
{
	if(map == NULL || tile_old == NULL) return -1;
	u8 is_server = (map == server_map);

	// Make local copy of tile.
	tile_t *tile = malloc(sizeof(*tile_old));
	memcpy(tile,tile_old,sizeof(*tile_old));

	tile->data = malloc(sizeof(u8)*tile->datalen);
	memcpy(tile->data,tile_old->data,sizeof(u8)*tile->datalen);

	// time to get the neighbours
	tile_t *ntiles[4];
	ntiles[0] = map_get_tile_ref(map,x,y-1);
	ntiles[1] = map_get_tile_ref(map,x,y+1);
	ntiles[2] = map_get_tile_ref(map,x+1,y);
	ntiles[3] = map_get_tile_ref(map,x-1,y);

	tileinfo_t *ti = tileinfo_get(tile->type);
	
	if(ti->f_handle_physics)
		return ti->f_handle_physics(map,x,y,tile,uidx,ntiles,is_server);

	return 0;
}

void handle_physics(map_t *map)
{
	if(map == NULL) return;

	s32 x, y;
	int lidx = 0;
	tile_t *tile = NULL;
	u8 uidx;
	u32 changes = 0;
	int is_server = (map == server_map);
	// Loopty loop
	while(map_get_next_update(map,&lidx,&x,&y))
	{
		changes++;
		// iterate over all tiles, including underones
		tile = map_get_tile_ref(map,x,y);
		uidx = 0;
		
		while(tile != NULL)
		{
			if(tile_active(*tile))
			{
				int handle_ret = handle_physics_tile(map, x, y, tile, uidx);
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

void register_tiles_physics()
{
	tileinfo_t *ti;

	ti = tileinfo_get(TILE_WIRE);
	ti->f_is_active = &is_tile_active_wire;
	ti->f_handle_physics = &handle_physics_tile_wire;

	ti = tileinfo_get(TILE_PNAND);
	ti->f_is_active = &is_tile_active_pnand;
	ti->f_handle_physics = &handle_physics_tile_pnand;

	ti = tileinfo_get(TILE_CROSSER);
	ti->f_is_active = &is_tile_active_crosser;
	ti->f_handle_physics = &handle_physics_tile_crosser;

	ti = tileinfo_get(TILE_PLATE);
	ti->f_is_active = &is_tile_active_plate;
	ti->f_handle_physics = &handle_physics_tile_plate;
}
