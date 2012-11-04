/*
split this into several .c files as required.
yeah, welcome to bottom-up development.
hope you enjoy the refactoring.
remember to do it often, or everything will turn to crap!
    --GM
*/

#include "audio.h"
#include "client.h"
#include "chat.h"
#include "common.h"
#include "event.h"
#include "interface.h"
#include "lua.h"
#include "map.h"
#include "misc.h"
#include "network.h"
#include "physics.h"
#include "player.h"
#include "render.h"
#include "tile.h"

player_t *player = NULL;

u8 movement_wait;
u64 frame_counter;

#define MODE_LOADING 0
#define MODE_UI 1
#define MODE_CHAT 2
#define MODE_MENU 3
u8 mode = MODE_LOADING;

void player_move(s8 dx, s8 dy)
{
	s32 newx = player->x+dx;
	s32 newy = player->y+dy;
	
	int ldx = -player->x;
	int ldy = -player->y;
	
	if(tile_walkable(client_map->f_get_tile(client_map,newx,newy)))
	{
		player->x=newx;
		player->y=newy;
	}
	
	ldx += player->x;
	ldy += player->y;
	
	if(ldx != 0 || ldy != 0)
		net_entity_movement(ldx, ldy);
	
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

void display(player_t *p)
{
	if(p == NULL)
	{
		sfp_printf_2x((SFP_SCREEN_REAL_WIDTH)/2-8*10,(SFP_SCREEN_REAL_HEIGHT)/2-8,
			0x08, 0, "Loading...");
		return;
	}
	
	int i, j;
	s32 rx = p->x-(SFP_FIELD_WIDTH)/2;
	s32 orx = rx;
	s32 ry = p->y-(SFP_FIELD_HEIGHT)/2;
	s32 ory = ry;
	tile_t tile;
	tile_t* tilestack[64];
	s8 tilestack_pos;
	map_layer_set_used_rendered(client_map, rx, ry);
	for(j=0;j<SFP_FIELD_HEIGHT;j++)
	{
		for(i=0;i<SFP_FIELD_WIDTH;i++)
		{
			tile = client_map->f_get_tile(client_map,rx,ry);
			tilestack[0] = &tile;
			tilestack_pos = 0;
			while(tile_transparent(*tilestack[tilestack_pos]) && tilestack[tilestack_pos]->under != NULL)
			{
				tilestack[tilestack_pos+1] = tilestack[tilestack_pos]->under;
				tilestack_pos++;
			}
			tile_t* current_tile;
			while(tilestack_pos>=0)
			{
				current_tile = tilestack[tilestack_pos];
				sfp_putc_block_2x(i,j,(current_tile->col>>4),(current_tile->col&15),
						current_tile->chr | (tile_transparent(*current_tile)?32768:0));
				tilestack_pos--;
			}
			rx++;
		}
		rx = orx;
		ry++;
	}

	// Some better player code.
	for(i = 0; i < player_top; i++)
	{
		player_t *xp = players[i];
		if(xp == NULL)
			continue;
		
		u32 px = xp->x-orx;
		u32 py = xp->y-ory;
		char* name = xp->name;
		if(!tile_overlay(client_map->f_get_tile(client_map,xp->x,xp->y)))
			sfp_putc_block_2x(px,py,(xp->col>>4),(xp->col&15),xp->chr);
		
		u32 pnamex = (px*16)-((strlen(name))*4)+8;
		u32 pnamey = (py*16)-10;
		sfp_printf_1x(pnamex,pnamey,0x0F,0,"%s",name);
	}
	
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
	
	s32 bx = get_rootx()+((sfp_event_mouse_x())/16);
	s32 by = get_rooty()+((sfp_event_mouse_y())/16);
	
	if(!ui_can_mouse_button()) return;
	
	if(pressing_0 && (lastx != bx || lasty != by || !pressed_0))
	{
		client_map->f_push_tile(client_map,bx,by,*ui_get_tile());
		client_map->f_set_update_n(client_map,bx,by,0);
	}
	else if(pressing_1 && (lastx != bx || lasty != by || !pressed_1))
	{
		tile_t *tile = ui_get_tile();
		tile_t map_tile = client_map->f_get_tile(client_map, bx, by);
		memcpy(tile,&map_tile,sizeof(tile_t));
		client_map->f_set_update_n(client_map,bx,by,0);
	}
	else if(pressing_2 && (lastx != bx || lasty != by || !pressed_2))
	{
		client_map->f_pop_tile(client_map,bx,by);
		client_map->f_set_update_n(client_map,bx,by,0);
	}

	tile_t test = client_map->f_get_tile(client_map,bx,bx);
	if(test.type == TILE_WIRE && test.datalen == 2)
	{
		sfp_printf_1x(sfp_event_mouse_x(),sfp_event_mouse_y(),0x1f,0,"%d %d",test.data[0],test.data[1]);
	}
	pressed_0 = pressing_0;
	pressed_1 = pressing_1;
	pressed_2 = pressing_2;
	lastx = bx;
	lasty = by;
}

int termbysig = 0;
void sighdl_closenicely(int signum)
{
	printf("SIGNAL %i RECEIVED, TERMINATING NICELY\n", signum);
	termbysig = 1;
}

int main(int argc, char *argv[])
{
	int i;
	
	char *net_addr = NULL;
	int net_port = 0;

	int no_display = 0;
	int no_self_player = 0;
	
	// Parse arguments
	// Currently just doing it this way:
	// - if none, PKTCOPY mode.
	// - if one, that's a port and it's a multiplayer server/local client.
	// - if two, "headless" and port, that's a headless multiplayer server.
	// - if two, address and port, and it's a multiplayer client.
	// - else, print use
	switch(argc-1)
	{
		case 0:
			is_server = 1;
			break;
		case 1:
			net_port = atoi(argv[1]);
			is_server = 1;
			break;
		case 2:
			net_port = atoi(argv[2]);
			if(strcmp(argv[1],"headless") != 0)
			{
				net_addr = argv[1];
			}
			else
			{
				is_server = 1;
				no_display = 1;
				no_self_player = 1;
			}
			break;
		default:
			printf("usage:\n");
			printf("%s\n\t- PKTCOPY singleplayer mode\n\n", argv[0]);
			printf("%s <port>\n\t- server\n\n", argv[0]);
			printf("%s <hostname> <port>\n\t- client\n\n", argv[0]);
			printf("%s headless <port> - headless server\n\n", argv[0]);
			return 99;
	}
	
	// Initialise stuff.
	if((!no_display) && sfp_init_render())
		return 1;
	
	if((!no_display) && sfp_audio_init())
		return 1;
	
	if(net_init(net_addr, net_port))
		return 1;
	
	if(sfp_lua_init())
		return 1;

	register_tiles();
	register_tiles_physics();
	map_init();
	init_ui();
	
	movement_wait = 0;
	
#ifndef WIN32
	signal(SIGINT, sighdl_closenicely);
#endif
	
	if(!no_self_player) net_login(0x1F, 0x0002, NICKNAME);

	chat_add_msg("DEBUG: Hello!");

	mode = MODE_UI;	
	while(!(sfp_event_key(SFP_KEY_APP_QUIT) || sfp_event_key(SFP_KEY_ESC) || termbysig))
	{
		player = net_player.player;
	
		if(!no_display)
		{
			sfp_render_begin();
			if(mode==MODE_UI || mode==MODE_CHAT)
				display(player);
			if(mode==MODE_UI)
			{
				if(player != NULL)
				{
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
					else movement_wait--;
				}
				
				if(!ui_is_occupied(sfp_event_mouse_x(),sfp_event_mouse_y()))
				{
					sfp_draw_rect(sfp_event_mouse_x()&~15,sfp_event_mouse_y()&~15,16,16,0xAAAAAA);
					#ifdef DEBUG
						s32 bx = get_rootx()+((sfp_event_mouse_x())/16);
						s32 by = get_rooty()+((sfp_event_mouse_y())/16);
						tile_t tile = server_map->f_get_tile(server_map,bx,by);
						u8 ha = -1;
						u8 ha2 = -1;
						if(tile.datalen>0) ha=tile.data[0];
						if(tile.datalen>1) ha2=tile.data[1];
						sfp_printf_2x(0,0,0x1F,0,"%d, %d, %d",tile.type,ha,ha2);
					#endif
					// That code prints out some wire data for debugging.
				}
			}
			sfp_render_end();
		}
			
		frame_counter++;

		if(!is_server && ((frame_counter%WIRIUM_DIVISOR) == 0)) handle_physics(client_map);

		// constant ~30FPS, rule from old 64pixels
		// TODO: get this to 33ms rather than 30ms
		// -- SDL_Delay only goes up in 10ms increments --GM
		// TODO: throttle it properly
		for(i = 0; i < 3; i++)
		{
			// placement
			sfp_event_poll();
			if(player!=NULL) mouse_placement();

			// network!
			net_update();
			server_update();
			
			// sleepage
			sfp_delay(10);
		}

		if(is_server && ((frame_counter%WIRIUM_DIVISOR) == 0)) handle_physics(server_map);
		
		sfp_event_tick();
	}
	printf("Cleaning up!\n");
	
	net_map_save();
	
	// TODO: free stuff w/o having to rely on the OS!
	// NOTABLE THINGS are maps, layers, players and netplayers.
	net_free();
	
	printf("DONE.\n");
	
#ifdef WIN32
	// DEBUG: attempt to clean up in a rather nasty way
	// as i symlink stderr.txt and stdout.txt to /dev/stderr --GM
	abort();
#endif
	
	return 0;
}
