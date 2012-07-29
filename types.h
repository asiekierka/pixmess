#ifndef _SFP_TYPES_H_
#define _SFP_TYPES_H_

#include <stdint.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

// Mental note: chr, not char
typedef struct player
{
	s32 x,y;
	u16 id;
	u8 col;
	u16 chr;
} player_t;

typedef struct tile tile_t;
struct tile
{
	u8 type;
	u16 chr;
	u8 col;
	u16 datalen;
	
	tile_t *under;
	u8 *data;
};

typedef struct layer
{
	s32 x,y;
	u16 w,h;
	tile_t *tiles;
} layer_t;

typedef struct netpacket netpacket_t;
struct netpacket
{
	u32 length;
	netpacket_t *next;
	u8 cmd;
	u8 data[];
};

#define NET_MTU 1024
#define NET_BUF_MAX 4096
typedef struct netplayer
{
	u16 id;
	int sockfd;
	netpacket_t *pkt_in_head, *pkt_in_tail;
	netpacket_t *pkt_out_head, *pkt_out_tail;
	int pkt_out_pos;
	int pkt_in_pos;
} netplayer_t;

enum
{
	LAYER_UNALLOC = 0,
	LAYER_UNUSED,
	LAYER_USED,
	LAYER_REQUESTED,
};

enum
{
	MAP_FLAG_U16CHAR = 1,
	MAP_FLAG_UNDER = 2,
	MAP_FLAG_DATA = 4,
	MAP_FLAG_EXT_DATALEN = 8,
};

#endif /* _SFP_TYPES_H_ */
