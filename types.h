#ifndef _SFP_TYPES_H_
#define _SFP_TYPES_H_

#include <stdint.h>

#define LAYER_SIZE_CLIENT 16
#define LAYER_SIZE_SERVER 128

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
	u8 col;
	u16 chr;
	char *name;
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

#define NPF_LOGGEDIN 0x00000001
#define NPF_NPC      0x00000002
#define NPF_KILLME   0x00000004
typedef struct netplayer
{
	int sockfd;
	player_t *player;
	u32 flags;
	netpacket_t *pkt_in_head, *pkt_in_tail;
	netpacket_t *pkt_out_head, *pkt_out_tail;
	
	u8 pkt_buf[NET_MTU];
	int pkt_buf_pos;
} netplayer_t;

typedef struct layerinfo
{
	u32 refcount;
	s32 x,y;
	layer_t *data;
} layerinfo_t;

typedef struct map
{
	char *fpath;
	
	u8 *layer_cmpbuf;
	u32 layer_cmplen;
	u32 layer_cmppos;
	u32 layer_rawlen;
	s32 layer_cmpx;
	s32 layer_cmpy;
	
	int layer_count;
	layerinfo_t layers[];
} map_t;

enum
{
	LAYER_UNALLOC = 0,
	LAYER_UNUSED,
	LAYER_USED,
	LAYER_REQUESTED,
	LAYER_LOADING,
};

#define MAP_FLAG_U16CHAR 1
#define MAP_FLAG_UNDER 2
#define MAP_FLAG_DATA 4
#define MAP_FLAG_EXT_DATALEN 8

#endif /* _SFP_TYPES_H_ */
