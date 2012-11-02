#include "misc.h"
#include <unistd.h>

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

