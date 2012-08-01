#include "client.h"
#include "common.h"
#include "map.h"
#include "network.h"
#include "physics.h"
#include "player.h"
#include "server.h"

#ifdef WIN32
#define close closesocket
#define MSG_DONTWAIT 0
#define SHUT_RDWR SD_BOTH
#define socklen_t int
WSADATA wsaihateyou;
#endif

// some delicious strings
char *net_pktstr_c2s[128] = {
	NULL, "44112", "44112", "44", "441112", "4412", "4412s", NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	"1", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	"44", NULL, NULL, NULL, "44", NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	"44", "2", NULL, NULL, NULL, NULL, NULL, NULL,
	"s", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, "212s", "1", "1", "", "s",
};

char *net_pktstr_s2c[128] = {
	NULL, "44112", "44112", "44", "441112", "4412", "4412s", NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	"211", "24412s", NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	NULL, "4444", "S", "", "44", NULL, NULL, NULL,
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
int net_id = PLAYER_NONE;
netplayer_t net_player;
player_t *players[65536];
int player_top = 0;

// server stuff
int server_sockfd = -1;
int server_sockfd_socketpair = -1;
netplayer_t *server_players[65536];
int server_player_top = 0;

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
			size += 1;
			va_arg(args, int);
			break;
		case '2':
			size += 2;
			va_arg(args, int);
			break;
		case '4':
			size += 4;
			va_arg(args, int);
			break;
		case 's':
			j = va_arg(args, int) & 0xFF;
			va_arg(args, char *);
			size += 1+j;
			break;
		case 'S':
			j = va_arg(args, int) & 0xFFFF;
			va_arg(args, char *);
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
			j = va_arg(args, int);
			if(j > 0xFF) j = 0xFF;
			
			v = va_arg(args, char *);
			*data++ = j;
			memcpy(data, v, j);
			data += j;
			break;
		case 'S':
			j = va_arg(args, int);
			if(j > 0xFFFF) j = 0xFFFF;
			
			v = va_arg(args, char *);
			*((u16 *)data) = j;
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

int net_sum_size(u8 cmd, int is_server)
{
	char *fmt = (cmd >= 128
		? NULL
		: (is_server ? net_pktstr_c2s : net_pktstr_s2c)[cmd]
	);
	
	if(fmt == NULL)
	{
		fprintf(stderr, "EDOOFUS: command %02X [sum] not defined for %s\n",
			cmd, (is_server ? "C->S" : "S->C"));
		return ((NET_MTU*4)<<8)|0;
	}
	
	int i;
	
	int sum = 1; // include cmd byte
	int flags = 0;
	
	for(i = 0; fmt[i] != '\0'; i++)
	switch(fmt[i])
	{
		case '1':
			sum += 1;
			break;
		case '2':
			sum += 2;
			break;
		case '4':
			sum += 4;
			break;
		case 's':
			sum += 1;
			flags |= 1;
			break;
		case 'S':
			sum += 2;
			flags |= 2;
			break;
	}
	
	return (sum<<8)|flags;
}

layer_t *net_layer_request(s32 x, s32 y)
{
	printf("net_layer_request: %d,%d\n",x,y);
	
	if(net_player.sockfd == FD_LOCAL_IMMEDIATE)
	{
		// We are a server running on the localhosts.
		layer_t *layer = map_get_file_layer(client_map, x, y);
		return layer;
	} else {
		net_pack(NULL, PKT_LAYER_REQUEST,
			x, y);
	}
	return NULL;
}

void net_login(u8 col, u16 chr, char *name)
{
	printf("net_login: chr=%04X, col=%02X, name=\"%s\"\n", chr, col, name);
	
	if(net_player.sockfd != FD_LOCAL_IMMEDIATE)
	{
		net_pack(NULL, PKT_LOGIN, SFP_PROTO_VERSION, col, chr, strlen(name), name);
	} else {
		// TODO: FD_LOCAL_IMMEDIATE mode
	}
}

int net_player_id()
{
	if(net_player.sockfd != FD_LOCAL_IMMEDIATE)
	{
		net_pack(NULL, PKT_PLAYER_ID);
		return -1;
	} else {
		return PLAYER_SELF;
	}
}

void net_layer_release(s32 x, s32 y)
{
	printf("net_layer_release: %d,%d\n",x,y);
	
	if(net_player.sockfd != FD_LOCAL_IMMEDIATE)
	{
		net_pack(NULL, PKT_LAYER_RELEASE,
			x, y);
	}
}

void net_entity_movement(s32 dx, s32 dy)
{
	//printf("net_entity_movement: %d,%d\n", dx, dy);
	
	if(net_player.sockfd != FD_LOCAL_IMMEDIATE)
	{
		u8 dir;
		if (dx < -7 || dx > 7 || dy < -7 || dy > 7)
		{
			net_pack(NULL, PKT_ENTITY_POSITION,
				net_player.player->x, net_player.player->y);
		} else {
			dir = (dx&15)|(dy<<4);
			net_pack(NULL, PKT_ENTITY_MOVEMENT, dir);
		}
	}
}

netplayer_t *net_player_new(int sockfd)
{
	netplayer_t *np = malloc(sizeof(netplayer_t));
	
	if(np == NULL)
	{
		fprintf(stderr, "FATAL: COULD NOT ALLOCATE NETPLAYER\n");
		perror("net_player_new");
		abort();
	}
	
	np->player = malloc(sizeof(player_t));
	
	if(np->player == NULL)
	{
		fprintf(stderr, "FATAL: COULD NOT ALLOCATE NETPLAYER PLAYER\n");
		perror("net_player_new");
		abort();
	}
	
	np->sockfd = sockfd;
	np->player->x = 0;
	np->player->y = 0;
	np->player->col = 0x00;
	np->player->chr = 0x0000;
	np->player->name = NULL;
	np->flags = 0;
	np->pkt_in_head = NULL;
	np->pkt_in_tail = NULL;
	np->pkt_out_head = NULL;
	np->pkt_out_tail = NULL;
	np->pkt_buf_pos = 0;
	
	return np;
}

void net_server_kick(u16 id, char *msg)
{
	netplayer_t *np = server_players[id];
	if(np == NULL)
	{
		fprintf(stderr, "ERROR: netplayer %i does not exist!\n", id);
		return;
	}
	
	net_pack(np, PKT_KICK, strlen(msg), msg);
	np->flags |= NPF_KILLME;
}

void net_player_destroy_server(u16 id)
{
	netplayer_t *np = server_players[id];
	
	if(np == NULL)
		return;
	
	// SET ME FREEEEEEE
	if(np->player != NULL)
	{
		if(np->player->name != NULL)
			free(np->player->name);
		free(np->player);
	}
	free(np);
	server_players[id] = NULL;
	
	// fix the top index
	while(server_player_top > 0)
	{
		if(server_players[server_player_top-1] != NULL)
			break;
		
		server_player_top--;
	}
}

void net_player_destroy_client(u16 id)
{
	if(id == net_id)
		return;
	
	player_t *p = players[id];
	
	if(p == NULL)
		return;
	
	// SET ME FREEEEEEE
	if(p->name != NULL)
		free(p->name);
	free(p);
	players[id] = NULL;
	
	// fix the top index
	while(player_top > 0)
	{
		if(players[player_top-1] != NULL)
			break;
		
		player_top--;
	}
}

netplayer_t *net_player_allocnew_server(int sockfd)
{
	int i;
	
	// TODO: set some "max players" variable
	for(i = 0; i < 65536; i++)
	{
		if(server_players[i] == NULL)
		{
			server_players[i] = net_player_new(sockfd);
			
			if(server_players[i] != NULL && i >= server_player_top)
				server_player_top = i+1;
			
			return server_players[i];
		}
	}
	
	printf("Uh oh, out of player slots!\n");
	return NULL;
}

player_t *net_player_allocnew_client(int id)
{
	// TODO: set some "max players" variable
	if(players[id] == NULL)
	{
		players[id] = malloc(sizeof(player_t));
		if(players[id] == NULL)
		{
			fprintf(stderr, "FATAL: COULD NOT ALLOCATE PLAYER\n");
			perror("net_player_allocnew_client");
			abort();
		}
		
		players[id]->x = 0;
		players[id]->y = 0;
		players[id]->chr = 0;
		players[id]->col = 0;
		players[id]->name = NULL;
		
		if(players[id] != NULL && id >= player_top)
			player_top = id+1;
	}
	
	return players[id];
}

void net_handle_s2c(netpacket_t *pkt)
{
	int i;
	
	printf("CLIENT: packet %02X\n", pkt->cmd);
	
	switch(pkt->cmd)
	{
		case PKT_BLOCK_SET: {
			s32 x = *(s32 *)(&pkt->data[0]);
			s32 y = *(s32 *)(&pkt->data[4]);
			u8 type = *(u8 *)(&pkt->data[8]);
			u8 col = *(u8 *)(&pkt->data[9]);
			u16 chr = *(u16 *)(&pkt->data[10]);
			
			tile_t t;
			
			t.type = type;
			t.col = col;
			t.chr = chr;
			t.under = NULL;
			t.datalen = 0;
			t.data = NULL;
			
			// DANGER: DON'T YOU DARE FEED THIS INTO CLIENT_SET_TILE
			// IT WILL SEND THE SET MESSAGE STRAIGHT BACK TO THE SERVER!!!
			//     --GM
			map_set_tile(client_map, x, y, t);
			client_map->f_set_update_n(client_map,x,y,0);
		} break;
		case PKT_BLOCK_PUSH: {
			s32 x = *(s32 *)(&pkt->data[0]);
			s32 y = *(s32 *)(&pkt->data[4]);
			u8 type = *(u8 *)(&pkt->data[8]);
			u8 col = *(u8 *)(&pkt->data[9]);
			u16 chr = *(u16 *)(&pkt->data[10]);
			
			tile_t t;
			
			t.type = type;
			t.col = col;
			t.chr = chr;
			t.under = NULL;
			t.datalen = 0;
			t.data = NULL;
			
			map_push_tile(client_map, x, y, t);
			client_map->f_set_update_n(client_map,x,y,0);
		} break;
		case PKT_BLOCK_POP: {
			s32 x = *(s32 *)(&pkt->data[0]);
			s32 y = *(s32 *)(&pkt->data[4]);
			
			map_pop_tile(client_map, x, y);
			client_map->f_set_update_n(client_map,x,y,0);
		} break;
		case PKT_BLOCK_SET_EXT: {
			s32 x = *(s32 *)(&pkt->data[0]);
			s32 y = *(s32 *)(&pkt->data[4]);
			u8 uidx = *(u8 *)(&pkt->data[8]);
			u8 type = *(u8 *)(&pkt->data[9]);
			u8 col = *(u8 *)(&pkt->data[10]);
			u16 chr = *(u16 *)(&pkt->data[11]);
			
			tile_t t;
			
			t.type = type;
			t.col = col;
			t.chr = chr;
			t.under = NULL;
			t.datalen = 0;
			t.data = NULL;
			
			map_set_tile_ext(client_map, x, y, uidx, t);
			client_map->f_set_update_n(client_map,x,y,0);
		} break;
		case PKT_BLOCK_ALLOC_DATA: {
			s32 x = *(s32 *)(&pkt->data[0]);
			s32 y = *(s32 *)(&pkt->data[4]);
			u8 uidx = *(u8 *)(&pkt->data[8]);
			u16 datalen = *(u16 *)(&pkt->data[9]);
			
			map_alloc_tile_data(client_map, x, y, uidx, datalen);
			client_map->f_set_update_n(client_map,x,y,0);
		} break;
		case PKT_BLOCK_SET_DATA: {
			s32 x = *(s32 *)(&pkt->data[0]);
			s32 y = *(s32 *)(&pkt->data[4]);
			u8 uidx = *(u8 *)(&pkt->data[8]);
			u16 datapos = *(u16 *)(&pkt->data[9]);
			u8 datalen = *(u16 *)(&pkt->data[11]);
			
			map_set_tile_data(client_map, x, y, uidx, datalen, datapos, &pkt->data[12]);
			client_map->f_set_update_n(client_map,x,y,0);
		} break;
		
		case PKT_ENTITY_MOVEMENT: {
			int id = *(u16 *)&pkt->data[0];
			int dx = *(s8 *)&pkt->data[2];
			int dy = *(s8 *)&pkt->data[3];
			
			player_t *p = players[id];
			if(p == NULL)
			{
				printf("ERROR: entity %i does not exist!\n", id);
				break;
			}
			
			p->x += dx;
			p->y += dy;
		} break;
		case PKT_ENTITY_CREATION: {
			u16 id = *(u16 *)(&pkt->data[0]);
			
			player_t *p = net_player_allocnew_client(id);
			
			p->x = *(s32 *)(&pkt->data[2]);
			p->y = *(s32 *)(&pkt->data[6]);
			p->col = *(u16 *)(&pkt->data[10]);
			p->chr = *(u16 *)(&pkt->data[11]);
			int namelen = pkt->data[13];
			if(p->name != NULL)
				free(p->name);
			p->name = malloc(namelen+1);
			if(p->name == NULL)
			{
				fprintf(stderr, "FATAL: COULD NOT ALLOCATE NAME\n");
				perror("net_handle_s2c");
				abort();
			}
			p->name[namelen] = '\x00';
			memcpy(p->name, &pkt->data[14], namelen);
			
			printf("%04X %02X \"%s\"\n",p->chr,p->col,p->name);
			
			if(id == net_id)
				net_player.player = p;
		} break;
		
		case PKT_LAYER_REQUEST:
			break;
		case PKT_LAYER_START: {
			s32 x, y;
			x = *(s32 *)(&pkt->data[0]);
			y = *(s32 *)(&pkt->data[4]);
			int rawlen = (int)*(u32 *)(&pkt->data[8]);
			int cmplen = (int)*(u32 *)(&pkt->data[12]);
			printf("layer start %i,%i: %i -> %i\n", x,y,rawlen,cmplen);
			
			// don't accept anything above 2MB compressed OR uncompressed
			if(cmplen > 2*1024*1024 || rawlen > 2*1024*1024)
			{
				printf("LAYER REJECTED - SIZE TOO LARGE\n");
				break;
			}
			
			// nuke old buffer if necessary
			if(client_map->layer_cmpbuf != NULL)
			{
				free(client_map->layer_cmpbuf);
				client_map->layer_cmpbuf = NULL;
			}
			
			// set layer properly
			client_map->layer_cmpx = x;
			client_map->layer_cmpy = y;
			
			// allocate buffer
			client_map->layer_cmpbuf = malloc(cmplen);
			if(client_map->layer_cmpbuf == NULL)
			{
				fprintf(stderr, "FATAL: COULD NOT ALLOCATE CMPBUF");
				perror("net_handle_s2c");
				abort();
			}
			client_map->layer_rawlen = rawlen;
			client_map->layer_cmplen = cmplen;
			client_map->layer_cmppos = 0;
		} break;
		case PKT_LAYER_DATA: {
			int slen = *(u16 *)(&pkt->data[0]);
			u8 *inbuf = (u8 *)(&pkt->data[2]);
			
			printf("layer data: %i+%i -> %i\n",
				client_map->layer_cmppos,
				slen,
				client_map->layer_cmplen);
			
			if(client_map->layer_cmpbuf == NULL)
			{
				fprintf(stderr, "ERROR: Layer not allocated for data!\n");
				break;
			}
			
			if(client_map->layer_cmppos+slen
					> client_map->layer_cmplen)
			{
				fprintf(stderr, "ERROR: Compression buffer overflow in layer!\n");
				free(client_map->layer_cmpbuf);
				client_map->layer_cmpbuf = NULL;
				break;
			}
			
			if(slen > 0)
			{
				memcpy(&(client_map->layer_cmpbuf[
						client_map->layer_cmppos]),
					inbuf, slen);
			}
			
			client_map->layer_cmppos += slen;
		} break;
		case PKT_LAYER_END: {
			printf("layer end\n");
			
			if(client_map->layer_cmpbuf == NULL)
			{
				fprintf(stderr, "ERROR: Layer not allocated for data!\n");
				break;
			}
			
			if(client_map->layer_cmppos != client_map->layer_cmplen)
			{
				fprintf(stderr, "ERROR: Compression buffer size mismatch in layer!\n");
				free(client_map->layer_cmpbuf);
				client_map->layer_cmpbuf = NULL;
				break;
			}
			
			layer_t *layer = layer_unserialise(client_map->layer_cmpbuf,
				client_map->layer_rawlen,
				client_map->layer_cmplen);
			
			if(layer == NULL)
			{
				fprintf(stderr, "ERROR: Layer failed to decompress!\n");
				free(client_map->layer_cmpbuf);
				client_map->layer_cmpbuf = NULL;
				break;
			}
			
			int pos = map_find_good_layer(client_map,
				client_map->layer_cmpx,
				client_map->layer_cmpy);
			
			if(pos == -1)
			{
				fprintf(stderr, "ERROR: too many layers allocated!\n");
				fprintf(stderr, "TODO! DEAL TO THIS GRACEFULLY - currently CRASHING\n");
				abort();
			}
			
			// nuke old layer if necessary
			if(client_map->layers[pos].data != NULL)
			{
				layer_free(client_map->layers[pos].data);
				client_map->layers[pos].data = NULL;
			}
			
			layer->x = client_map->layer_cmpx;
			layer->y = client_map->layer_cmpy;
			client_map->layers[pos].data = layer;
			client_map->layers[pos].refcount = 1;
			
			free(client_map->layer_cmpbuf);
			client_map->layer_cmpbuf = NULL;
		} break;
		case PKT_LAYER_RELEASE: {
			// TODO: deal with this correctly
			s32 x, y;
			u8 pos;
			x = *(s32 *)(&pkt->data[0]);
			y = *(s32 *)(&pkt->data[4]);
			printf("layer release %i,%i\n", x,y);
		} break;
		
		case PKT_ENTITY_POSITION: {
			u16 id = *(u16 *)(&pkt->data[0]);
			player_t *p = players[id];
			if(p == NULL)
			{
				printf("ERROR: entity %i does not exist!\n", id);
				break;
			}
			p->x = *(s32 *)(&pkt->data[2]);
			p->y = *(s32 *)(&pkt->data[6]);
		} break;
		case PKT_ENTITY_DESTRUCTION: {
			u16 id = *(u16 *)(&pkt->data[0]);
			player_t *p = players[id];
			if(p == NULL)
			{
				printf("ERROR: entity %i does not exist!\n", id);
				break;
			}
			net_player_destroy_client(id);
		} break;
		
		case PKT_CHAT:
			break;
		
		case PKT_LOGIN:
			break;
		case PKT_PING: {
			net_pack(NULL, PKT_PONG, pkt->data[0]);
		} break;
		case PKT_PONG:
			break;
		case PKT_PLAYER_ID: {
			net_id = *(u16 *)&pkt->data[0];
			net_player.flags |= NPF_LOGGEDIN;
			net_player.player = players[net_id];
		} break;
		case PKT_KICK: {
			char msg[256];
			int msglen = pkt->data[0];
			msg[msglen] = '\x00';
			memcpy(msg, &pkt->data[1], msglen);
			printf("*** WE HAVE BEEN KICKED ***\n");
			printf("Reason: \"%s\"\n", msg);
			net_player.flags |= NPF_KILLME;
		} break;
	}
}


void net_handle_c2s(int id, netplayer_t *np, netpacket_t *pkt)
{
#define C2S_NEED_LOGIN if(!(np->flags & NPF_LOGGEDIN)) break;
#define C2S_NEED_NOLOGIN if(np->flags & NPF_LOGGEDIN) break;
	int i;
	
	printf("SERVER: %04X: packet %02X\n", id, pkt->cmd);
	
	switch(pkt->cmd)
	{
		case PKT_BLOCK_SET: {
			C2S_NEED_LOGIN;
			s32 x = *(s32 *)(&pkt->data[0]);
			s32 y = *(s32 *)(&pkt->data[4]);
			u8 type = *(u8 *)(&pkt->data[8]);
			u8 col = *(u8 *)(&pkt->data[9]);
			u16 chr = *(u16 *)(&pkt->data[10]);
			
			tile_t t;
			
			t.type = type;
			t.col = col;
			t.chr = chr;
			t.under = NULL;
			t.datalen = 0;
			t.data = NULL;
			
			server_map->f_set_tile(server_map, x, y, t);
			for(i = 0; i < server_player_top; i++)
				if(server_players[i] != NULL)
					net_pack(server_players[i], PKT_BLOCK_SET,
						x, y, t.type, t.col, t.chr);
			server_map->f_set_update_n(server_map, x, y, 0);
		} break;
		case PKT_BLOCK_PUSH: {
			C2S_NEED_LOGIN;
			s32 x = *(s32 *)(&pkt->data[0]);
			s32 y = *(s32 *)(&pkt->data[4]);
			u8 type = *(u8 *)(&pkt->data[8]);
			u8 col = *(u8 *)(&pkt->data[9]);
			u16 chr = *(u16 *)(&pkt->data[10]);
			
			tile_t t;
			
			t.type = type;
			t.col = col;
			t.chr = chr;
			t.under = NULL;
			t.datalen = 0;
			t.data = NULL;
			
			server_map->f_push_tile(server_map, x, y, t);
			for(i = 0; i < server_player_top; i++)
				if(server_players[i] != NULL)
					net_pack(server_players[i], PKT_BLOCK_PUSH,
						x, y, t.type, t.col, t.chr);
			server_map->f_set_update_n(server_map, x, y, 0);
		} break;
		case PKT_BLOCK_POP: {
			C2S_NEED_LOGIN;
			s32 x = *(s32 *)(&pkt->data[0]);
			s32 y = *(s32 *)(&pkt->data[4]);
			
			server_map->f_pop_tile(server_map, x, y);
			
			for(i = 0; i < server_player_top; i++)
				if(server_players[i] != NULL)
					net_pack(server_players[i], PKT_BLOCK_POP,
						x, y);
			server_map->f_set_update_n(server_map, x, y, 0);
		} break;
		case PKT_BLOCK_SET_EXT:
			break;
		case PKT_BLOCK_ALLOC_DATA:
			break;
		case PKT_BLOCK_SET_DATA:
			break;
		
		case PKT_ENTITY_MOVEMENT:
			C2S_NEED_LOGIN;
			
			u8 dir = pkt->data[0];
			int x = (dir&15);
			int y = (dir>>4);
			
			if(x >= 8) x -= 16;
			if(y >= 8) y -= 16;
			
			np->player->x += x;
			np->player->y += y;
			
			for(i = 0; i < server_player_top; i++)
				if(i != id && server_players[i] != NULL)
					net_pack(server_players[i], PKT_ENTITY_MOVEMENT,
						id, x, y);
			
			break;
		case PKT_ENTITY_CREATION:
			break;
		
		case PKT_LAYER_REQUEST: {
			C2S_NEED_LOGIN;
			
			s32 x, y;
			u8 pos;
			x = *(s32 *)(&pkt->data[0]);
			y = *(s32 *)(&pkt->data[4]);
			pos = *(u8 *)(&pkt->data[8]);
			printf("layer request %i,%i [%i]\n", x,y,pos);
			
			// serialise layer
			int rawlen;
			int cmplen;
			
			layer_t *layer;
			
			layer = map_get_existing_layer(server_map, x, y);
			if(layer == NULL)
				layer = map_get_file_layer(server_map, x, y);
			if(layer == NULL)
				layer = map_get_new_layer(server_map, x, y);
			if(layer == NULL)
			{
				fprintf(stderr, "EDOOFUS: could not load a new layer for the client\n");
				net_pack(np, PKT_LAYER_RELEASE, x, y, pos);
				break;
			}
			
			layer->x = x;
			layer->y = y;
			
			u8 *buffer = layer_serialise(layer, &rawlen, &cmplen);
			
			if(buffer == NULL)
			{
				fprintf(stderr, "ERROR: cannot serialise layer!\n");
				net_pack(np, PKT_LAYER_RELEASE, x, y, pos);
				break;
			}
			
			// send data
			net_pack(np, PKT_LAYER_START,
				x, y, rawlen, cmplen);
			
			for(i = 0; i < cmplen; i += 512)
				net_pack(np, PKT_LAYER_DATA, (cmplen-i > 512 ? 512 : cmplen-i), &buffer[i]);
			
			net_pack(np, PKT_LAYER_END);
			printf("sent: %i -> %i\n", rawlen, cmplen);
		} break;
		case PKT_LAYER_START:
			break;
		case PKT_LAYER_DATA:
			break;
		case PKT_LAYER_END:
			break;
		case PKT_LAYER_RELEASE: {
			C2S_NEED_LOGIN;
			
			// TODO: deal with this correctly
			s32 x, y;
			u8 pos;
			x = *(s32 *)(&pkt->data[0]);
			y = *(s32 *)(&pkt->data[4]);
			pos = *(u8 *)(&pkt->data[8]);
			printf("layer release %i,%i [%i]\n", x,y,pos);
		} break;
		
		case PKT_ENTITY_POSITION:
			break;
		case PKT_ENTITY_DESTRUCTION:
			break;
		
		case PKT_CHAT:
			break;
		
		case PKT_LOGIN: {
			C2S_NEED_NOLOGIN;
			
			// TODO: set stuff properly
			int version = *(u16 *)&pkt->data[0];
			printf("Login: version %i\n", version);
			
			if(version != SFP_PROTO_VERSION)
			{
				printf("VERSION MISMATCH!\n");
				const char *msg = "Version mismatch!";
				net_pack(np, PKT_KICK, strlen(msg), msg);
				break;
			}
			np->player->x = 0;
			np->player->y = 0;
			np->player->col = pkt->data[2];
			np->player->chr = *(u16 *)&pkt->data[3];
			int namelen = pkt->data[5];
			np->player->name = malloc(namelen+1);
			np->player->name[namelen] = '\x00';
			memcpy(np->player->name, &pkt->data[6], namelen);
			if(np->player->name == NULL)
			{
				fprintf(stderr, "FATAL: COULD NOT ALLOCATE PLAYER NAME\n");
				perror("net_handle_c2s");
				abort();
			}
			np->flags |= NPF_LOGGEDIN;
			
			net_pack(np, PKT_ENTITY_CREATION,
				id,
				np->player->x, np->player->y,
				np->player->col, np->player->chr,
				strlen(np->player->name), np->player->name);
			
			// exchange player spawn info
			for(i = 0; i < server_player_top; i++)
			{
				// don't send this to yourself!
				if(i == id)
					continue;
				
				// there MUST be a player in this slot (duh)
				netplayer_t *xnp = server_players[i];
				if(xnp == NULL)
					continue;
				
				// cannot send to dead or not logged in connections!
				if((xnp->flags & NPF_KILLME) || !(xnp->flags & NPF_LOGGEDIN))
					continue;
				
				printf("exch %i %i\n", id, i);
				// you to them
				net_pack(xnp, PKT_ENTITY_CREATION,
					id,
					np->player->x, np->player->y,
					np->player->col, np->player->chr,
					strlen(np->player->name), np->player->name);
				
				// them to you
				net_pack(np, PKT_ENTITY_CREATION,
					i,
					xnp->player->x, xnp->player->y,
					xnp->player->col, xnp->player->chr,
					strlen(xnp->player->name), xnp->player->name);
			}
			net_pack(np, PKT_PLAYER_ID, id);
		} break;
		case PKT_PING: {
			net_pack(np, PKT_PONG, pkt->data[0]);
		} break;
		case PKT_PONG: {
			
		} break;
		case PKT_PLAYER_ID: {
			C2S_NEED_LOGIN;
			
			net_pack(np, PKT_PLAYER_ID, id);
		} break;
		case PKT_KICK:
			break;
	}
}

void net_map_save()
{
	if(net_player.sockfd == FD_LOCAL_IMMEDIATE)
		map_save(client_map);
	else if(server_sockfd != FD_LOCAL_IMMEDIATE)
		map_save(server_map);
}

void net_recv(netplayer_t *np)
{
	// Parse any received packets.
	int is_server = (np != NULL);
	
	if(np == NULL)
		np = &net_player;
	
	// Check: are these certain local connection types?
	if(np->sockfd == FD_LOCAL_IMMEDIATE)
		return;
	if(np->sockfd == FD_LOCAL_PKTCOPY)
		return;
	
	// receive data
	if(np->sockfd >= 0)
	for(;;)
	{
		int rcount = recv(np->sockfd, (char *)&np->pkt_buf[np->pkt_buf_pos],
			NET_MTU-np->pkt_buf_pos,
			MSG_DONTWAIT);
		
		if(rcount <= -1)
		{
#ifdef WIN32
			if(rcount != WSAEWOULDBLOCK)
#else
			int err = errno;
			if(err != EAGAIN)
#endif
			{
				perror("net_recv");
				// TODO: disconnect gracefully!
				break;
			}
			break;
		} else if(rcount == 0) {
			// TODO: disconnect gracefully!
			break;
		} else {
			np->pkt_buf_pos += rcount;
		}
		
		
		// parse the data we have
		while(np->pkt_buf_pos != 0)
		{
			int size = net_sum_size(np->pkt_buf[0], is_server);
			int flags = size&0xFF;
			size >>= 8;
			
			// check size first
			if(np->pkt_buf_pos < size)
				break;
			
			if(flags & 1)
			{
				size += np->pkt_buf[size-1];
			} else if(flags & 2) {
				size += *(u16 *)&np->pkt_buf[size-2];
			}
			
			// check size again
			// if excessive, drop the packet
			if(size > NET_MTU)
			{
				fprintf(stderr, "ERROR: calculated packet size %i too large! DROPPED.\n"
					,size);
				np->pkt_buf_pos = 0;
				break;
			}
			
			if(np->pkt_buf_pos < size)
				break;
			
			// create packet
			netpacket_t *pkt = malloc(sizeof(netpacket_t)+size-1);
			
			if(pkt == NULL)
			{
				fprintf(stderr, "FATAL: COULD NOT ALLOCATE PACKET OF SIZE %i\n", size);
				perror("net_recv");
				abort();
			}
			
			// Fill in the fields
			pkt->length = size;
			pkt->cmd = np->pkt_buf[0];
			pkt->next = NULL;
			
			// copy data
			memcpy(pkt->data, &np->pkt_buf[1], size-1);
			
			// add to list
			if(np->pkt_in_tail == NULL)
			{
				np->pkt_in_tail = np->pkt_in_head = pkt;
			} else {
				np->pkt_in_tail->next = pkt;
				np->pkt_in_tail = pkt;
			}
			
			// copy memory back
			memmove(np->pkt_buf, &np->pkt_buf[size], np->pkt_buf_pos-size);
			np->pkt_buf_pos -= size;
		}
	}
}

void net_send(netplayer_t *np_to, netplayer_t *np_from, int is_server)
{
	netpacket_t *pkt, *npkt;
	
	// Assemble packets for the send queue.
	if(np_to != NULL && np_from != NULL && np_to->sockfd == FD_LOCAL_PKTCOPY)
	{
		// PKTCOPY: Just copy the packets across.
		for(pkt = np_from->pkt_out_head; pkt != NULL; pkt = npkt)
		{
			if(np_to->pkt_in_tail == NULL)
			{
				np_to->pkt_in_tail = np_to->pkt_in_head = pkt;
			} else {
				np_to->pkt_in_tail->next = pkt;
				np_to->pkt_in_tail = pkt;
			}
			npkt = pkt->next;
		}
		
		if(np_to->pkt_in_tail != NULL)
			np_to->pkt_in_tail->next = NULL;
		
		np_from->pkt_out_head = np_from->pkt_out_tail = NULL;
	} else {
		// PKTPARSE: Actually buffer the packets.
		u8 buf[NET_MTU];
		
		netplayer_t *np = np_from;
		int sockfd = np->sockfd;
		
		int buf_pos = 0;
		
		for(pkt = np->pkt_out_head; np->pkt_out_head != NULL; pkt = npkt)
		{
			npkt = pkt->next;
			
			// Drop oversize packets automatically.
			if(pkt->length > NET_MTU)
			{
				fprintf(stderr, "ERROR: oversize packet %i! DROPPED.\n", pkt->length);
				free(pkt);
				continue;
			}
			
			// Stop if we've hit our MTU.
			if(buf_pos + pkt->length > NET_MTU)
				break;
			
			// Copy packet data.
			buf[buf_pos] = pkt->cmd;
			memcpy(&buf[buf_pos+1], pkt->data, pkt->length-1);
			buf_pos += pkt->length;
			
			// Free packet.
			free(pkt);
			
			// Change head and, if empty, tail.
			np->pkt_out_head = npkt;
			if(npkt == NULL)
				np->pkt_out_tail = NULL;
		}
		
		if(buf_pos != 0)
		{
			int amt = send(sockfd, (char *)buf, buf_pos, 0);
			if(amt != buf_pos)
			{
				if(amt == -1)
				{
					perror("net_send");
				} else {
					fprintf(stderr, "ERROR: expected %i bytes, sent %i!\n",
						buf_pos, amt);
				}
				
				if(is_server)
					np->flags |= NPF_KILLME;
				else
					net_player.flags |= NPF_KILLME;
			}
		}
	}
}

void net_update()
{
	// Check: are we actually networked?
	if(net_player.sockfd == FD_LOCAL_IMMEDIATE)
		return;
	
	// Attempt to receive packets.
	if(net_player.sockfd != FD_LOCAL_PKTCOPY)
		net_recv(NULL);
	
	// Check: do we have any packets in the receive queue?
	netpacket_t *pkt, *npkt;
	
	for(pkt = net_player.pkt_in_head; pkt != NULL; pkt = npkt)
	{
		// Handle + free packet.
		net_handle_s2c(pkt);
		npkt = pkt->next;
		free(pkt);
	}
	net_player.pkt_in_head = net_player.pkt_in_tail = NULL;
	
	// If networked, sockfd will be >= 0.
	// Check if local player has been accepted.
	if((server_sockfd == FD_LOCAL_SOCKETPAIR || net_player.sockfd < -1)
		&& net_id == PLAYER_NONE)
	{
		net_id = 0;
		server_players[net_id] = net_player_allocnew_server(
			(server_sockfd == FD_LOCAL_SOCKETPAIR
				? server_sockfd_socketpair
				: net_player.sockfd));
	}
	
	netplayer_t *np = (net_player.sockfd < -1
		? server_players[net_id]
		: NULL);
	
	// Assemble packets for the send queue.
	net_send(np, &net_player, 0);
	
	// Update some stuff!
	handle_physics(client_map);
}

void server_update()
{
	int i;
	
	// Check: are we actually running a server?
	if(server_sockfd == FD_LOCAL_IMMEDIATE)
		return;
	
	// TODO: select() like a boss
	
	// Poll for new connection.
	// TODO: remove old connections eventually!
	if(server_sockfd >= 0)
	{
#ifdef WIN32
		// WINSOCK SUCKS
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(server_sockfd, &fds);
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		select(server_sockfd+1, &fds, NULL, NULL, &timeout);
		if(FD_ISSET(server_sockfd, &fds))
#else
		struct pollfd pfd;
		pfd.fd = server_sockfd;
		pfd.events = POLLIN;
		pfd.revents = 0;
		
		poll(&pfd, 1, 0);
		if(pfd.revents & POLLIN)
#endif
		{
			struct sockaddr_in sa;
			socklen_t sa_len = sizeof(sa);
			int csockfd = accept(server_sockfd, (struct sockaddr *)&sa, &sa_len);
			if(csockfd == -1)
			{
				fprintf(stderr, "ERROR: accept failed\n");
				perror("server_update");
			} else {
#ifdef WIN32
				u_long mode = 1;
				ioctlsocket(csockfd, FIONBIO, &mode);
#endif
				// TODO: use this info!
				printf("%08X -> port %i connected!\n",
					sa.sin_addr.s_addr, sa.sin_port);
				// TODO: give a decent spawn location!
				// TODO: actually save this for login!
				netplayer_t *npx = net_player_allocnew_server(csockfd);
			}
		}
	}
	
	netpacket_t *pkt, *npkt;
	
	// Go through all the players.
	for(i = 0; i < server_player_top; i++)
	{
		netplayer_t *np = server_players[i];
		
		// Skip unallocated players.
		if(np == NULL)
			continue;
		
		// Any connections we need to kill?
		if(np->sockfd == -1 || (np->flags & NPF_KILLME))
		{
			// Close.
			if(np->sockfd >= 0)
				close(np->sockfd);
			
			// Destroy.
			net_player_destroy_server(i);
			
			// Tell everyone else.
			int j;
			for(j = 0; j < server_player_top; j++)
			{
				netplayer_t *xnp = server_players[j];
				
				if(xnp == NULL)
					continue;
				
				net_pack(xnp, PKT_ENTITY_DESTRUCTION, i);
			}
			
			// Continue.
			continue;
		}
		
		// Attempt to receive packets.
		if(np->sockfd >= 0)
			net_recv(np);
		
		// Parse ALL the packets!
		for(pkt = np->pkt_in_head; pkt != NULL; pkt = npkt)
		{
			// Handle + free packet.
			net_handle_c2s(i, np, pkt);
			npkt = pkt->next;
			free(pkt);
		}
		np->pkt_in_head = np->pkt_in_tail = NULL;
		
		// Assemble packets for the send queue.
		net_send((np->sockfd >= -1 ? NULL : &net_player), np, 1);
	}
	
	// Update some stuff!
	handle_physics(server_map);
}

int net_init(char *addr, int port)
{
	int i;
	
	if(net_initialised)
		return 0;
	
#ifdef WIN32
	// WINSOCK SUCKS, ONCE AGAIN
	// NO I AM NOT CLEANING UP
	if(WSAStartup(MAKEWORD(1,1), &wsaihateyou))
	{
		fprintf(stderr, "ERROR: Winsock decided to suck\n");
		// Let's divide by zero and then if that fails attempt to segfault
		int qp = 0;
		int qc = qp++;
		int *z = (void *)qc;
		int f = qc / qp;
		int g = *z;
		printf("YOU SHOULD NOT SEE THIS: %i %i\n", f, g);
		abort();
	}
#endif
	
	// Prepare the client stuff
	net_id = PLAYER_NONE;
	net_player.flags = 0;
	net_player.player = NULL;
	net_player.pkt_in_head = NULL;
	net_player.pkt_in_tail = NULL;
	net_player.pkt_out_head = NULL;
	net_player.pkt_out_tail = NULL;
	net_player.pkt_buf_pos = 0;
	
	// Prepare the server stuff
	
	// Initiate appropriate connections
	/*
	int sv[2] = {FD_LOCAL_PKTCOPY, FD_LOCAL_PKTCOPY};
	socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
	printf("%i %i\n", sv[0], sv[1]);
	net_player.sockfd = sv[0];
	server_sockfd_socketpair = sv[1];
	server_sockfd = FD_LOCAL_SOCKETPAIR;
	*/
	net_player.sockfd = FD_LOCAL_PKTCOPY;
	server_sockfd = -1;
	//net_player.sockfd = server_sockfd = FD_LOCAL_IMMEDIATE;
	if(port == 0)
	{
		// PKTCOPY.
		server_sockfd = FD_LOCAL_PKTCOPY;
	} else {
		struct sockaddr_in sa;
		
		sa.sin_family = AF_INET;
		sa.sin_port = htons(port);
		
		if(addr == NULL) {
			// host server!
			// FIXME: use PKTCOPY for the local user!
			// TODO: IPv6 support
			
			// server
			sa.sin_addr.s_addr = 0;
			
			server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if(server_sockfd == -1)
			{
				fprintf(stderr, "ERROR: could not create server socket!\n");
				perror("net_init");
				return 1;
			}
			
			if(bind(server_sockfd, (struct sockaddr *)&sa, sizeof(sa)))
			{
				fprintf(stderr, "ERROR: could not bind server socket!\n");
				perror("net_init");
				shutdown(server_sockfd, SHUT_RDWR);
				close(server_sockfd);
				return 1;
			}
			
			if(listen(server_sockfd, 1))
			{
				fprintf(stderr, "ERROR: could not tell server socket to listen!\n");
				// Naughty kids!
				perror("net_init");
				shutdown(server_sockfd, SHUT_RDWR);
				close(server_sockfd);
				return 1;
			}
		}
		
		// client connect
		struct hostent *he;
		if(addr == NULL) he = gethostbyname("127.0.0.1");
		else he = gethostbyname(addr);
		if(he == NULL || he->h_addr_list[0] == NULL)
		{
			fprintf(stderr, "ERROR: gethostbyname failed!\n");
			
			if(server_sockfd != -1)
			{
				shutdown(server_sockfd, SHUT_RDWR);
				close(server_sockfd);
			}
			
			//if(he != NULL)
			//	free(he);
			
			return 1;
		}
		
		memcpy(&sa.sin_addr.s_addr,
			(struct in_addr *)(he->h_addr_list[0]),
			sizeof(struct in_addr));
		
		//if(he != NULL)
		//	free(he);
		
		net_player.sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if(net_player.sockfd == -1)
		{
			fprintf(stderr, "ERROR: could not create client socket!\n");
			perror("net_init");
			
			if(server_sockfd != -1)
			{
				shutdown(server_sockfd, SHUT_RDWR);
				close(server_sockfd);
			}
			
			return 1;
		}
		
		if(connect(net_player.sockfd, (struct sockaddr *)&sa, sizeof(sa)))
		{
			fprintf(stderr, "ERROR: could not connect to server!\n");
			perror("net_init");
			
			close(net_player.sockfd);
			
			if(server_sockfd != -1)
			{
				shutdown(server_sockfd, SHUT_RDWR);
				close(server_sockfd);
			}
			
			return 1;
		}
		
#ifdef WIN32
		u_long mode = 1;
		ioctlsocket(net_player.sockfd, FIONBIO, &mode);
#endif
	}
	
	// STOP IT FROM KILLING THE PROGRAM ON A BROKEN PIPE.
	// Broken pipes happen! LOTS!
#ifndef WIN32
	signal(SIGPIPE, SIG_IGN);
#endif
	
	for(i = 0; i < 65536; i++)
		server_players[i] = NULL;
	server_player_top = 0;
	
	// TODO: special treatment for Windows
	// required like everything else Microsoft ever made
	
	net_initialised = 255;
	return 0;
}
