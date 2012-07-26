#include "common.h"

#define LAYER_SIZE 64 // 32-255, no promises for lower, will crash on higher

// TODO these defintions should have a file later on, maybe
layer_t *layer_request(s32 x, s32 y);
void net_report_layer(s32 x, s32 y, u8 position);

layer_t *layers[LAYER_SIZE];
u8 layer_set[LAYER_SIZE];

layer_t *layer_get(s32 x, s32 y)
{
	u8 i;
	// Finding existing layer
	for(i=0;i<LAYER_SIZE;i++) {
		if(layer_set[i]==LAYER_USED && layers[i]->x==x && layers[i]->y==y)
			return layers[i];
	}
	// We didn't find that layer
	for(i=0;i<LAYER_SIZE;i++) {
		if(layer_set[i]==LAYER_UNALLOC)
		{
			layers[i] = (layer_t*)malloc(sizeof(layer_t));
			layer_set[i] = LAYER_UNUSED;
		}
		if(layer_set[i]==LAYER_UNUSED)
			if(layers[i] = layer_request(x,y))
			{
				if(layers[i]->x != x || layers[i]->y != y)
					return NULL;
				layer_set[i] = LAYER_USED;
				net_report_layer(x,y,i);
				return layers[i];
			}
	}
	// We really didn't
	return NULL;
}

void layer_set_unused(s32 x, s32 y)
{
	u8 i;
	for(i=0;i<LAYER_SIZE;i++) {
		if(layer_set[i]==LAYER_USED && layers[i]->x==x && layers[i]->y==y)
		{
			layer_set[i] = LAYER_UNUSED;
			return;
		}
	}
}
