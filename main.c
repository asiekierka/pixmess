/*
split this into several .c files as required.
yeah, welcome to bottom-up development.
hope you enjoy the refactoring.
remember to do it often, or everything will turn to crap!
    --GM
*/

#include "client.h"
#include "common.h"
#include "event.h"
#include "interface.h"
#include "lua.h"
#include "map.h"
#include "misc.h"
#include "network.h"
#include "player.h"
#include "render.h"
#include "tile.h"

player_t *player;
u8 movement_wait;
u64 frame_counter;

void player_move(s8 dx, s8 dy)
{
	s32 newx = player->x+dx;
	s32 newy = player->y+dy;
	if(tile_walkable(client_get_tile(newx,newy)))
	{
		player->x=newx;
		player->y=newy;
	}
	movement_wait = 2;
}

s32 get_rootx()
{
	return player->x-(SFP_FIELD_WIDTH/2);
}
s32 get_rooty()
{
	return player->y-(SFP_FIELD_HEIGHT/2);
}

void display (player_t *p)
{
	int i, j;
	s32 rx = p->x-(SFP_FIELD_WIDTH)/2;
	s32 orx = rx;
	s32 ry = p->y-(SFP_FIELD_HEIGHT)/2;
	s32 ory = ry;
	tile_t tile;
	map_layer_set_used_rendered(&client_map, rx, ry);
	for(j=0;j<SFP_FIELD_HEIGHT;j++)
	{
		for(i=0;i<SFP_FIELD_WIDTH;i++)
		{
			tile = client_get_tile(rx,ry);
			sfp_putc_block_2x(i,j,(tile.col>>4),(tile.col&15),tile.chr);
			rx++;
		}
		rx = orx;
		ry++;
	}
	// Early player code.
	u32 px = p->x-orx;
	u32 py = p->y-ory;
	if(!tile_overlay(client_get_tile(p->x,p->y))) sfp_putc_block_2x(px,py,(p->col>>4),(p->col&15),p->chr);
	char* name = "Gamemaster";

	u32 pnamex = (px*16)-((strlen(name))*4)+8;
	u32 pnamey = (py*16)-10;
	sfp_printf_1x(pnamex,pnamey,0x0F,0,"%s",name);
}

void mouse_placement()
{
	static int pressed_0 = 0;
	static int pressed_1 = 0;
	static int pressed_2 = 0;
	
	static int lastx = -10000;
	static int lasty = -10000;
	
	int pressing_0 = sfp_event_mouse_button(0);
	int pressing_1 = sfp_event_mouse_button(1);
	int pressing_2 = sfp_event_mouse_button(2);
	
	if(ui_is_occupied(sfp_event_mouse_x(),sfp_event_mouse_y())) return;
	
	sfp_draw_rect(sfp_event_mouse_x()&~15,sfp_event_mouse_y()&~15,16,16,0xAAAAAA);
	s32 bx = get_rootx()+((sfp_event_mouse_x())/16);
	s32 by = get_rooty()+((sfp_event_mouse_y())/16);
	
	if(!ui_can_mouse_button()) return;
	
	if(pressing_0 && (lastx != bx || lasty != by || !pressed_0))
	{
		client_push_tile(bx,by,*ui_get_tile());
	}
	else if(pressing_1 && (lastx != bx || lasty != by || !pressed_1))
	{
		tile_t *tile = ui_get_tile();
		tile_t map_tile = client_get_tile(bx,by);
		memcpy(tile,&map_tile,sizeof(tile_t));
	}
	else if(pressing_2 && (lastx != bx || lasty != by || !pressed_2))
	{
		client_pop_tile(bx,by);
	}
	
	pressed_0 = pressing_0;
	pressed_1 = pressing_1;
	pressed_2 = pressing_2;
	lastx = bx;
	lasty = by;
}

int main(int argc, char *argv[])
{
	int i;
	
	char *net_addr = NULL;
	int net_port = 0;
	
	// Parse arguments
	// Currently just doing it this way:
	// - if none, PKTCOPY mode.
	// - if one, that's a port and it's a multiplayer server/local client.
	// - if two, address and port, and it's a multiplayer client.
	// - else, print use
	switch(argc-1)
	{
		case 0:
			break;
		case 1:
			net_port = atoi(argv[1]);
			break;
		case 2:
			net_addr = argv[1];
			net_port = atoi(argv[2]);
			break;
		default:
			printf("usage:\n");
			printf("%s\n\t- PKTCOPY singleplayer mode\n\n", argv[0]);
			printf("%s port\n\t- server\n\n", argv[0]);
			printf("%s hostname port\n\t- client\n\n", argv[0]);
			return 99;
	}
	
	// Initialise stuff.
	if(sfp_init_render())
		return 1;
	
	if(net_init(net_addr, net_port))
		return 1;
	
	if(sfp_lua_init())
		return 1;
	
	map_init();
	init_ui();
	
	movement_wait = 0;
	
	player = player_get(PLAYER_SELF);
	
	while(!(sfp_event_key(SFP_KEY_APP_QUIT) || sfp_event_key(SFP_KEY_ESC)))
	{
		sfp_render_begin();
		display(player);
		if(frame_counter<90) sfp_printf_2x(1*8,2*8,0x1F,0,"Hello %s! You are player %i.", "Gamemaster", PLAYER_SELF);
		mouse_placement();
		render_ui();

		if(movement_wait==0)
		{
			if(sfp_event_key(SFP_KEY_W))
				player_move(0,-1);
			if(sfp_event_key(SFP_KEY_S))
				player_move(0,1);
			if(sfp_event_key(SFP_KEY_A))
				player_move(-1,0);
			if(sfp_event_key(SFP_KEY_D))
				player_move(1,0);
		}
		
		
		if(movement_wait>0) movement_wait--;
		frame_counter++;
		
		sfp_render_end();
		
		// constant ~30FPS, rule from old 64pixels
		// TODO: get this to 33ms rather than 30ms
		// -- SDL_Delay only goes up in 10ms increments --GM
		// TODO: throttle it properly
		for(i = 0; i < 3; i++)
		{
			// network!
			net_update();
			server_update();
			
			// sleepage
			sfp_delay(10);
		}
		
		sfp_event_poll();
		sfp_event_tick();
	}
	
	net_map_save();
	
	return 0;
}
