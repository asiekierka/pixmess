#include "common.h"
#include "misc.h"
#include <SDL.h>

u64 get_current_time()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (((u64)tv.tv_sec)*(u64)1000000) + (u64)tv.tv_usec;
}

void sfp_delay(u32 ms)
{
	SDL_Delay(ms);
}

