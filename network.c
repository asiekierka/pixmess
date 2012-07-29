#include "common.h"
#include "map.h"
#include "network.h"

// some delicious strings
char *net_pktstr_c2s[128] = {
	NULL, "22111", "22111", "22", NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	"1", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	"441", NULL, NULL, NULL, "441", NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	"44", "2", NULL, NULL, NULL, NULL, NULL, NULL,
	"s", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, "222s", "1", "1", "", "s",
};

char *net_pktstr_s2c[128] = {
	NULL, "22111", "22111", "22", NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	"211", "24422s", NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	NULL, "44144", "S", "", "441", NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	"244", "2", NULL, NULL, NULL, NULL, NULL, NULL,
	"s", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, "1", "1", "2", "s",
};

// client stuff
int net_initialised = 0;
netplayer_t net_player;

// server stuff
int server_sockfd = -1;
netplayer_t *server_players[65536];

int net_init()
{
	if(net_initialised)
		return 0;
	
	// Prepare the client stuff
	net_player.id = 0;
	net_player.sockfd = -1;
	net_player.pkt_in_head = NULL;
	net_player.pkt_in_tail = NULL;
	net_player.pkt_out_head = NULL;
	net_player.pkt_out_tail = NULL;
	
	// Prepare the server stuff
	server_sockfd = -1;
	
	// TODO: special treatment for Windows
	// required like everything else Microsoft ever made
	
	net_initialised = 255;
	return 0;
}

/*

Format char specification:
	1 = int8
	2 = int16
	4 = int32
	s = pstr8
	S = pstr16

For wraparound types, just use the unsigned variant.

*/

// NOTE: Set np to NULL for C->S stuff!
// Otherwise it will assume S->C stuff!
netpacket_t *net_pack(netplayer_t *np, u8 cmd, ...)
{
	int i, j;
	char *v;
	
	char *fmt = (cmd >= 128
		? NULL
		: (np == NULL ? net_pktstr_c2s : net_pktstr_s2c)[cmd]
	);
	
	// If command not defined, complain and return NULL.
	if(fmt == NULL)
	{
		fprintf(stderr, "EDOOFUS: command %02X not defined for %s\n",
			cmd, (np == NULL ? "C->S" : "S->C"));
		return NULL;
	}
	
	// If C->S, set np NOW.
	if(np == NULL)
		np = &net_player;
	
	va_list args;
	
	// Calculate size
	int size = 0;
	va_start(args, cmd);
	for(i = 0; fmt[i] != '\0'; i++)
	switch(fmt[i])
	{
		case '1':
			size += 1-0;
		case '2':
			size += 2-1;
		case '4':
			size += 4-2;
			va_arg(args, int);
			break;
		case 's':
			j = va_arg(args, int) & 0xFF;
			size += 1+j;
			break;
		case 'S':
			j = va_arg(args, int) & 0xFFFF;
			size += 2+j;
			break;
	}
	va_end(args);
	
	// Allocate packet (if possible)
	netpacket_t *pkt = malloc(sizeof(netpacket_t)+size);
	
	// Failed to allocate? DIE HORRIBLY.
	if(pkt == NULL)
	{
		fprintf(stderr, "FATAL: COULD NOT ALLOCATE PACKET OF SIZE %i\n", size);
		perror("net_pack");
		abort();
	}
	
	// Fill in the fields
	pkt->length = size+1;
	pkt->cmd = cmd;
	pkt->next = NULL;
	
	// Write data
	u8 *data = pkt->data;
	va_start(args, cmd);
	for(i = 0; fmt[i] != '\0'; i++)
	switch(fmt[i])
	{
		// yeah, let's face it. i'm insane.
		// i look at this and think "hmm, STOSB/W/D would be useful here".
		//     --GM
		case '1':
			*(data++) = va_arg(args, int);
			break;
		case '2':
			*((u16 *)data) = va_arg(args, int);
			data += 2;
			break;
		case '4':
			*((u32 *)data) = va_arg(args, int);
			data += 4;
			break;
		case 's':
			v = va_arg(args, char *);
			j = strlen(v);
			if(j > 0xFF) j = 0xFF;
			
			*data++ = j;
			memcpy(data, v, j);
			data += j;
			break;
		case 'S':
			v = va_arg(args, char *);
			j = strlen(v);
			if(j > 0xFFFF) j = 0xFFFF;
			
			*((u16 *)data) = va_arg(args, int);
			data += 2;
			memcpy(data, v, j);
			data += j;
			break;
	}
	va_end(args);
	
	// Append to the end of the list
	if(np->pkt_out_tail == NULL)
	{
		np->pkt_out_head = np->pkt_out_tail = pkt;
	} else {
		np->pkt_out_tail->next = pkt;
		np->pkt_out_tail = pkt;
	}
	
	return pkt;
}

layer_t *net_layer_request(s32 x, s32 y, u8 position)
{
	printf("net_layer_request: %d,%d, pos %d\n",x,y,position);
	
	if(net_player.sockfd != -1)
	{
		net_pack(NULL, PKT_LAYER_REQUEST,
			x, y, position);
		return NULL;
	} else {
		// TODO: load from disk if possible
		printf("TESTING: serialise / unserialise\n");
		
		//return layer_dummy_request(x, y);
		layer_t *layer = layer_dummy_request(x, y);
		int rawlen, cmplen;
		
		u8 *buffer = layer_serialise(layer, &rawlen, &cmplen);
		if(buffer == NULL)
		{
			printf("DEBUG: buffer is NULL, returning layer\n");
			return layer;
		}
		
		printf("layer size %i -> %i\n", rawlen, cmplen);
		
		layer_t *nlayer = layer_unserialise(buffer, rawlen, cmplen);
		free(buffer);
		if(nlayer == NULL)
		{
			printf("DEBUG: nlayer is NULL, returning layer\n");
			return layer;
		}
		
		// TODO: confirm layer integrity
		
		nlayer->x = layer->x;
		nlayer->y = layer->y;
		
		u16 i;
		for(i=0;i<4096;i++)
		{
			if(!((nlayer->tiles[i].type == layer->tiles[i].type)
				|| (nlayer->tiles[i].chr == layer->tiles[i].chr)
				|| (nlayer->tiles[i].col == layer->tiles[i].col)
				|| (nlayer->tiles[i].datalen == layer->tiles[i].datalen)
				  ))
				printf("DEBUG: Tile %d is incorrect!",i);
		}
		layer_free(layer);
		return nlayer;
	}
	
}

void net_layer_release(s32 x, s32 y, u8 position)
{
	printf("net_layer_release: %d,%d, pos %d\n",x,y,position);
	if(net_player.sockfd != -1)
	{
		net_pack(NULL, PKT_LAYER_RELEASE,
			x, y, position);
	}
}

void net_handle_s2c(netpacket_t *pkt)
{
	switch(pkt->cmd)
	{
		case PKT_BLOCK_SET:
			break;
		case PKT_BLOCK_PUSH:
			break;
		case PKT_BLOCK_POP:
			break;
		
		case PKT_ENTITY_MOVEMENT:
			break;
		case PKT_ENTITY_CREATION:
			break;
		
		case PKT_LAYER_REQUEST:
			break;
		case PKT_LAYER_START:
			break;
		case PKT_LAYER_DATA:
			break;
		case PKT_LAYER_END:
			break;
		case PKT_LAYER_RELEASE:
			break;
		
		case PKT_ENTITY_POSITION:
			break;
		case PKT_ENTITY_DESTRUCTION:
			break;
		
		case PKT_CHAT:
			break;
		
		case PKT_LOGIN:
			break;
		case PKT_PING:
			break;
		case PKT_PONG:
			break;
		case PKT_PLAYER_ID:
			break;
		case PKT_KICK:
			break;
	}
}

void net_handle_c2s(netpacket_t *pkt)
{
	switch(pkt->cmd)
	{
		case PKT_BLOCK_SET:
			break;
		case PKT_BLOCK_PUSH:
			break;
		case PKT_BLOCK_POP:
			break;
		
		case PKT_ENTITY_MOVEMENT:
			break;
		case PKT_ENTITY_CREATION:
			break;
		
		case PKT_LAYER_REQUEST:
			break;
		case PKT_LAYER_START:
			break;
		case PKT_LAYER_DATA:
			break;
		case PKT_LAYER_END:
			break;
		case PKT_LAYER_RELEASE:
			break;
		
		case PKT_ENTITY_POSITION:
			break;
		case PKT_ENTITY_DESTRUCTION:
			break;
		
		case PKT_CHAT:
			break;
		
		case PKT_LOGIN:
			break;
		case PKT_PING:
			break;
		case PKT_PONG:
			break;
		case PKT_PLAYER_ID:
			break;
		case PKT_KICK:
			break;
	}
}

void net_recv()
{
	// Parse any received packets.
	// TODO!
}

void net_update()
{
	// Check: are we actually networked?
	if(net_player.sockfd == -1)
		return;
	
	// Check: do we have any packets in the receive queue?
	netpacket_t *pkt, *npkt;
	
	for(pkt = net_player.pkt_in_head; pkt != NULL; pkt = npkt)
	{
		// Handle + free packet.
		net_handle_s2c(pkt);
		npkt = pkt->next;
		free(pkt);
	}
	
	// Assemble packets for the send queue.
	// TODO!
}

void server_update()
{
	// Check: are we actually running a server?
	if(server_sockfd == -1)
		return;
	
	// TODO!
}

