#include "misc.h"
#include <unistd.h>

u64 get_current_time()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (((u64)tv.tv_sec)*(u64)1000000) + (u64)tv.tv_usec;
}

void sfp_delay(u32 ms)
{
	// IEEE Std defines that usleep's ms < 1000000.
	u32 ms2 = ms*1000;
	while(ms2>=1000000)
	{
		usleep(999999);
		ms2-=999999;
	}
	usleep(ms2);
}

