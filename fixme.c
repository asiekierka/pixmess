#include "common.h"

layer_t *layer_request(s32 x, s32 y)
{
	layer_t *a = (layer_t *)malloc(sizeof(layer_t));
	a->x = x; a->y = y;
	return a;
};
void net_report_layer(s32 x, s32 y, u8 position) { printf("net_report_layer: %d,%d, pos %d",x,y,position); };
