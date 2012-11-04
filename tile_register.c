#include "common.h"
#include "tile.h"

void register_tiles()
{
	tileinfo_new(TILE_DUMMY,"Dummy");
	tileinfo_new(TILE_FLOOR,"Floor");
	tileinfo_new(TILE_WALL,"Wall");
	tileinfo_new(TILE_ROOF,"Roof");
	tileinfo_new(TILE_WIRE,"Wire");
	tileinfo_new(TILE_PNAND,"P-NAND Gate");
	tileinfo_new(TILE_CROSSER,"Wire Crosser");
	tileinfo_new(TILE_PLATE,"Plate");

	tileinfo_set_stackable_va(TILE_FLOOR, 1, TILE_DUMMY);
	tileinfo_set_stackable_va(TILE_WALL, 2, TILE_DUMMY, TILE_FLOOR);

	tileinfo_set_stackable_va(TILE_ROOF, 7, TILE_DUMMY, TILE_FLOOR, TILE_WALL, TILE_WIRE, TILE_PNAND,
		TILE_CROSSER, TILE_PLATE);

	tileinfo_set_stackable_va(TILE_WIRE, 3, TILE_DUMMY, TILE_FLOOR, TILE_WALL);
	tileinfo_set_stackable_va(TILE_PNAND, 2, TILE_DUMMY, TILE_FLOOR);
	tileinfo_set_stackable_va(TILE_CROSSER, 3, TILE_DUMMY, TILE_FLOOR, TILE_WALL);
	tileinfo_set_stackable_va(TILE_PLATE, 2, TILE_DUMMY, TILE_FLOOR);

	tileinfo_set_flag(TILE_DUMMY, TILE_WALKABLE);
	tileinfo_set_flag(TILE_FLOOR, TILE_WALKABLE);
	tileinfo_set_flag(TILE_ROOF, TILE_WALKABLE | TILE_TRANSPARENT | TILE_OVERLAY);
	tileinfo_set_flag(TILE_WIRE, TILE_ACTIVE | TILE_TRANSPARENT | TILE_WALKABLE);
	tileinfo_set_flag(TILE_PNAND, TILE_ACTIVE);
	tileinfo_set_flag(TILE_CROSSER, TILE_ACTIVE | TILE_TRANSPARENT);
	tileinfo_set_flag(TILE_PLATE, TILE_ACTIVE | TILE_TRANSPARENT | TILE_WALKABLE);

	tileinfo_set_preview_data(TILE_FLOOR, 176, 8);
	tileinfo_set_preview_data(TILE_WALL, 178, 7+(8*16));
	tileinfo_set_preview_data(TILE_ROOF, 177, 15+(7*16));
	tileinfo_set_preview_data(TILE_WIRE, 197, 12);
	tileinfo_set_preview_data(TILE_PNAND, 25, 12);
	tileinfo_set_preview_data(TILE_CROSSER, 206, 11);
	tileinfo_set_preview_data(TILE_PLATE, 22, 7);

	tileinfo_set_allowed_colors(TILE_WIRE,0x000100FE);
	tileinfo_set_allowed_colors(TILE_PNAND, 0x0001FFFE);

	// TODO: consider using C99 compound literals
	const u16 wire_ac[1] = {197};
	const u16 crosser_ac[1] = {206};
	const u16 pnand_ac[4] = {24,25,26,27};
	const u16 plate_ac[18] = {254,7,8,9,10,15,16,17,22,30,31,174,175,240,244,245,247};
	tileinfo_set_allowed_chars(TILE_WIRE,1,wire_ac);
	tileinfo_set_allowed_chars(TILE_CROSSER,1,crosser_ac);
	tileinfo_set_allowed_chars(TILE_PNAND,4,pnand_ac);
	tileinfo_set_allowed_chars(TILE_PLATE,18,plate_ac);
}
