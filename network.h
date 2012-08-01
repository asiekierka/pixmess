#ifndef _NETWORK_H_
#define _NETWORK_H_

// TODO: winsock support
#ifdef WIN32
#include <winsock.h>
#else
#include <sys/time.h>
#include <sys/types.h>

#include <netdb.h>
#include <poll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <signal.h>
#endif

// IMMEDIATE: everything returns instantly, no server running.
// note, this requires effort to keep it in sync
// with the multiplayer code.
#define FD_LOCAL_IMMEDIATE -1

// PKTCOPY: copies the packet structures across.
#define FD_LOCAL_PKTCOPY -2

// SOCKETPAIR: actual one-computer networking.
// hypothetically, the most accurate singleplayer mode.
#define FD_LOCAL_SOCKETPAIR -3

extern int net_id;
extern netplayer_t net_player;
extern int player_top;
extern player_t *players[];

extern netplayer_t *server_players[];
extern int server_player_top;

#define SFP_PROTO_VERSION 0x0001
int net_init();

netpacket_t *net_pack(netplayer_t *np, u8 cmd, ...);

void net_login(u8 col, u16 chr, char *name);
layer_t *net_layer_request(s32 x, s32 y);
int net_player_id();
void net_layer_release(s32 x, s32 y);
void net_entity_movement(s32 dx, s32 dy);

void net_map_save();

void net_update();
void server_update();

enum
{
	PKT_BLOCK_SET = 0x01,
	PKT_BLOCK_PUSH,
	PKT_BLOCK_POP,
	PKT_BLOCK_SET_EXT,
	PKT_BLOCK_ALLOC_DATA,
	PKT_BLOCK_SET_DATA,
	
	PKT_ENTITY_MOVEMENT = 0x10,
	PKT_ENTITY_CREATION,
	
	PKT_LAYER_REQUEST = 0x40,
	PKT_LAYER_START,
	PKT_LAYER_DATA,
	PKT_LAYER_END,
	PKT_LAYER_RELEASE,
	
	PKT_ENTITY_POSITION = 0x50,
	PKT_ENTITY_DESTRUCTION,
	
	PKT_CHAT = 0x58,
	
	PKT_LOGIN = 0x7B,
	PKT_PING,
	PKT_PONG,
	PKT_PLAYER_ID,
	PKT_KICK,
};

#endif /* _NETWORK_H_ */

