#ifndef _NETWORK_H_
#define _NETWORK_H_

// TODO: winsock support
#ifdef WIN32
#warning "Winsock not supported yet!"
#else
#include <sys/time.h>
#include <sys/types.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

int net_init();

layer_t *net_layer_request(s32 x, s32 y, u8 position);
void net_layer_release(s32 x, s32 y, u8 position);

enum
{
	PKT_BLOCK_SET = 0x01,
	PKT_BLOCK_PUSH,
	PKT_BLOCK_POP,
	
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

