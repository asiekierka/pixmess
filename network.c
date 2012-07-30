#include "common.h"
#include "map.h"
#include "network.h"
#include "player.h"
#include "server.h"

// some delicious strings
char *net_pktstr_c2s[128] = {
	NULL, "44112", "44112", "44", NULL, NULL, NULL, NULL,
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
	NULL, "44112", "44112", "44", NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	"211", "24422s", NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	
	NULL, "44144", "1S", "1", "441", NULL, NULL, NULL,
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

layer_t *net_layer_request(s32 x, s32 y, u8 position)
{
	printf("net_layer_request: %d,%d, pos %d\n",x,y,position);
	
	if(net_player.sockfd == FD_LOCAL_IMMEDIATE)
	{
		// We are a server running on the localhosts.
		layer_t* layer = layer_load(&client_map,x,y);
		if(layer == NULL)
		{
			layer = layer_new(LAYER_WIDTH, LAYER_HEIGHT, LAYER_TEMPLATE_CLASSIC);
			layer->x = x;
			layer->y = y;
		}
		return layer;
	} else {
		net_pack(NULL, PKT_LAYER_REQUEST,
			x, y, position);
	}
	return NULL;
}

void net_layer_release(s32 x, s32 y, u8 position)
{
	printf("net_layer_release: %d,%d, pos %d\n",x,y,position);
	
	if(net_player.sockfd != FD_LOCAL_IMMEDIATE)
	{
		net_pack(NULL, PKT_LAYER_RELEASE,
			x, y, position);
	}
}

void net_block_set(s32 x, s32 y, tile_t tile)
{
	printf("net_block_set: %d,%d\n", x, y);
	
	if(net_player.sockfd != FD_LOCAL_IMMEDIATE)
	{
		net_pack(NULL, PKT_BLOCK_SET,
			x, y,
			tile.type, tile.col, tile.chr);
	} else {
		map_set_tile(&client_map, x, y, tile);
	}
}

void net_block_push(s32 x, s32 y, tile_t tile)
{
	printf("net_block_push: %d,%d\n", x, y);
	
	if(net_player.sockfd != FD_LOCAL_IMMEDIATE)
	{
		net_pack(NULL, PKT_BLOCK_PUSH,
			x, y,
			tile.type, tile.col, tile.chr);
	} else {
		map_push_tile(&client_map, x, y, tile);
	}
}

void net_block_pop(s32 x, s32 y)
{
	printf("net_block_pop: %d,%d\n", x, y);
	
	if(net_player.sockfd != FD_LOCAL_IMMEDIATE)
	{
		net_pack(NULL, PKT_BLOCK_POP,
			x, y);
	} else {
		map_pop_tile(&client_map, x, y);
	}
}

netplayer_t *net_player_new(u16 id, int sockfd, s32 x, s32 y, u8 col, u16 chr)
{
	netplayer_t *np = malloc(sizeof(netplayer_t));
	
	if(np == NULL)
	{
		fprintf(stderr, "FATAL: COULD NOT ALLOCATE NETPLAYER\n");
		perror("net_player_new");
		abort();
	}
	
	np->id = id;
	np->sockfd = sockfd;
	np->player.x = x;
	np->player.y = y;
	np->player.id = id;
	np->player.col = col;
	np->player.chr = chr;
	np->pkt_in_head = NULL;
	np->pkt_in_tail = NULL;
	np->pkt_out_head = NULL;
	np->pkt_out_tail = NULL;
	np->pkt_buf_pos = 0;
	
	if(id >= server_player_top)
		server_player_top = id+1;
	
	return np;
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
			map_set_tile(&client_map, x, y, t);
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
			
			map_push_tile(&client_map, x, y, t);
		} break;
		case PKT_BLOCK_POP: {
			s32 x = *(s32 *)(&pkt->data[0]);
			s32 y = *(s32 *)(&pkt->data[4]);
			
			map_pop_tile(&client_map, x, y);
		} break;
		
		case PKT_ENTITY_MOVEMENT:
			break;
		case PKT_ENTITY_CREATION:
			break;
		
		case PKT_LAYER_REQUEST:
			break;
		case PKT_LAYER_START: {
			s32 x, y;
			u8 pos;
			x = *(s32 *)(&pkt->data[0]);
			y = *(s32 *)(&pkt->data[4]);
			pos = *(u8 *)(&pkt->data[8]);
			int rawlen = (int)*(u32 *)(&pkt->data[9]);
			int cmplen = (int)*(u32 *)(&pkt->data[13]);
			printf("layer start %i,%i [%i]: %i -> %i\n", x,y,pos,rawlen,cmplen);
			
			// don't accept anything above 2MB compressed OR uncompressed
			if(cmplen > 2*1024*1024 || rawlen > 2*1024*1024)
			{
				printf("LAYER REJECTED - SIZE TOO LARGE\n");
				break;
			}
			
			// nuke old layer if necessary
			if(client_map.layers[pos] != NULL)
			{
				layer_free(client_map.layers[pos]);
				client_map.layers[pos] = NULL;
			}
			
			// nuke old buffer if necessary
			if(client_map.layer_cmpbuf[pos] != NULL)
			{
				free(client_map.layer_cmpbuf[pos]);
				client_map.layer_cmpbuf[pos] = NULL;
			}
			
			// set layer properly
			client_map.layer_set[pos] = LAYER_LOADING;
			client_map.layer_x[pos] = x;
			client_map.layer_y[pos] = y;
			
			// allocate buffer
			client_map.layer_cmpbuf[pos] = malloc(cmplen);
			if(client_map.layer_cmpbuf[pos] == NULL)
			{
				fprintf(stderr, "FATAL: COULD NOT ALLOCATE CMPBUF");
				perror("net_handle_s2c");
				abort();
			}
			client_map.layer_rawlen[pos] = rawlen;
			client_map.layer_cmplen[pos] = cmplen;
			client_map.layer_cmppos[pos] = 0;
		} break;
		case PKT_LAYER_DATA: {
			u8 pos = *(u8 *)(&pkt->data[0]);
			int slen = *(u16 *)(&pkt->data[1]);
			u8 *inbuf = (u8 *)(&pkt->data[3]);
			
			printf("layer data %i: %i+%i -> %i\n", pos,
				client_map.layer_cmppos[pos],
				slen,
				client_map.layer_cmplen[pos]);
			
			if(client_map.layer_cmpbuf[pos] == NULL)
			{
				fprintf(stderr, "ERROR: Layer %i not allocated for data!\n"
					,pos);
				break;
			}
			
			if(client_map.layer_cmppos[pos]+slen
					> client_map.layer_cmplen[pos])
			{
				fprintf(stderr, "ERROR: Compression buffer overflow in layer %i!\n"
					,pos);
				free(client_map.layer_cmpbuf[pos]);
				client_map.layer_cmpbuf[pos] = NULL;
				break;
			}
			
			if(slen > 0)
			{
				memcpy(&(client_map.layer_cmpbuf[pos][
						client_map.layer_cmppos[pos]]),
					inbuf, slen);
			}
			
			client_map.layer_cmppos[pos] += slen;
		} break;
		case PKT_LAYER_END: {
			u8 pos = *(u8 *)(&pkt->data[0]);
			printf("layer end %i\n", pos);
			
			if(client_map.layer_cmpbuf[pos] == NULL)
			{
				fprintf(stderr, "ERROR: Layer %i not allocated for data!\n"
					,pos);
				break;
			}
			
			if(client_map.layer_cmppos[pos] != client_map.layer_cmplen[pos])
			{
				fprintf(stderr, "ERROR: Compression buffer size mismatch in layer %i!\n"
					,pos);
				free(client_map.layer_cmpbuf[pos]);
				client_map.layer_cmpbuf[pos] = NULL;
				break;
			}
			
			layer_t *layer = layer_unserialise(client_map.layer_cmpbuf[pos],
				client_map.layer_rawlen[pos],
				client_map.layer_cmplen[pos]);
			
			if(layer == NULL)
			{
				fprintf(stderr, "ERROR: Layer failed to decompress!\n");
				free(client_map.layer_cmpbuf[pos]);
				client_map.layer_cmpbuf[pos] = NULL;
				break;
			}
			
			layer->x = client_map.layer_x[pos];
			layer->y = client_map.layer_y[pos];
			client_map.layers[pos] = layer;
			client_map.layer_set[pos] = LAYER_USED;
			
			free(client_map.layer_cmpbuf[pos]);
			client_map.layer_cmpbuf[pos] = NULL;
		} break;
		case PKT_LAYER_RELEASE: {
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

void net_handle_c2s(netplayer_t *np, netpacket_t *pkt)
{
	int i;
	
	printf("SERVER: %04X: packet %02X\n", np->id, pkt->cmd);
	
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
			
			server_set_tile(x, y, t);
			for(i = 0; i < server_player_top; i++)
				if(server_players[i] != NULL)
					net_pack(server_players[i], PKT_BLOCK_SET,
						x, y, t.type, t.col, t.chr);
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
			
			server_push_tile(x, y, t);
			for(i = 0; i < server_player_top; i++)
				if(server_players[i] != NULL)
					net_pack(server_players[i], PKT_BLOCK_PUSH,
						x, y, t.type, t.col, t.chr);
		} break;
		case PKT_BLOCK_POP: {
			s32 x = *(s32 *)(&pkt->data[0]);
			s32 y = *(s32 *)(&pkt->data[4]);
			
			server_pop_tile(x, y);
			
			for(i = 0; i < server_player_top; i++)
				if(server_players[i] != NULL)
					net_pack(server_players[i], PKT_BLOCK_POP,
						x, y);
		} break;
		
		case PKT_ENTITY_MOVEMENT:
			break;
		case PKT_ENTITY_CREATION:
			break;
		
		case PKT_LAYER_REQUEST: {
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
			
			layer = map_get_existing_layer(&server_map, x, y);
			if(layer == NULL)
				layer = map_get_file_layer(&server_map, x, y);
			if(layer == NULL)
				layer = map_get_new_layer(&server_map, x, y);
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
				x, y, pos, rawlen, cmplen);
			
			for(i = 0; i < cmplen; i += 512)
				net_pack(np, PKT_LAYER_DATA, pos, (cmplen-i > 512 ? 512 : cmplen-i), &buffer[i]);
			
			net_pack(np, PKT_LAYER_END, pos);
			printf("sent: %i -> %i\n", rawlen, cmplen);
		} break;
		case PKT_LAYER_START:
			break;
		case PKT_LAYER_DATA:
			break;
		case PKT_LAYER_END:
			break;
		case PKT_LAYER_RELEASE: {
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

void net_map_save()
{
	if(net_player.sockfd == FD_LOCAL_IMMEDIATE)
		map_save(&client_map);
	else if(server_sockfd != FD_LOCAL_IMMEDIATE)
		map_save(&server_map);
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
		int rcount = recv(np->sockfd, np->pkt_buf,
			NET_MTU-np->pkt_buf_pos,
			MSG_DONTWAIT);
		
		if(rcount == -1)
		{
			int err = errno;
			if(err != EAGAIN)
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
			int amt = send(sockfd, buf, buf_pos, 0);
			if(amt != buf_pos)
			{
				// TODO: deal with these cases correctly
				if(amt == -1)
					perror("net_send");
				else
					fprintf(stderr, "ERROR: expected %i bytes, sent %i!\n",
						buf_pos, amt);
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
		&& server_players[net_player.id] == NULL)
	{
		server_players[net_player.id] = net_player_new(
			net_player.id, 
			(server_sockfd == FD_LOCAL_SOCKETPAIR
				? server_sockfd_socketpair
				: net_player.sockfd),
			player->x,
			player->y,
			player->col,
			player->chr);
	}
	
	netplayer_t *np = (net_player.sockfd < -1
		? server_players[net_player.id]
		: NULL);
	
	// Assemble packets for the send queue.
	net_send(np, &net_player, 0);
}

void server_update()
{
	int i;
	
	// Check: are we actually running a server?
	if(server_sockfd == FD_LOCAL_IMMEDIATE)
		return;
	
	
	netpacket_t *pkt, *npkt;
	
	// Go through all the players.
	for(i = 0; i < server_player_top; i++)
	{
		netplayer_t *np = server_players[i];
		
		// Skip unallocated players.
		if(np == NULL)
			continue;
		
		// Skip unallocated sockets (TODO: clean these up)
		if(np->sockfd == -1)
			continue;
		
		// Attempt to receive packets.
		if(np->sockfd >= 0)
			net_recv(np);
		
		// Parse ALL the packets!
		for(pkt = np->pkt_in_head; pkt != NULL; pkt = npkt)
		{
			// Handle + free packet.
			net_handle_c2s(np, pkt);
			npkt = pkt->next;
			free(pkt);
		}
		np->pkt_in_head = np->pkt_in_tail = NULL;
		
		// Assemble packets for the send queue.
		net_send((np->sockfd >= -1 ? NULL : &net_player), np, 1);
	}
}

int net_init()
{
	int i;
	
	if(net_initialised)
		return 0;
	
	// Prepare the client stuff
	net_player.id = PLAYER_SELF;
	net_player.pkt_in_head = NULL;
	net_player.pkt_in_tail = NULL;
	net_player.pkt_out_head = NULL;
	net_player.pkt_out_tail = NULL;
	net_player.pkt_buf_pos = 0;
	
	// Prepare the server stuff
	
	// SINGLEPLAYER, TODO: MULTIPLAYER
	{
		int sv[2] = {FD_LOCAL_PKTCOPY, FD_LOCAL_PKTCOPY};
		socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
		printf("%i %i\n", sv[0], sv[1]);
		net_player.sockfd = sv[0];
		server_sockfd_socketpair = sv[1];
		server_sockfd = FD_LOCAL_SOCKETPAIR;
	}
	//net_player.sockfd = server_sockfd = FD_LOCAL_PKTCOPY;
	//net_player.sockfd = server_sockfd = FD_LOCAL_IMMEDIATE;
	
	for(i = 0; i < 65536; i++)
		server_players[i] = NULL;
	server_player_top = 0;
	
	// TODO: special treatment for Windows
	// required like everything else Microsoft ever made
	
	net_initialised = 255;
	return 0;
}
