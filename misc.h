#ifndef _MISC_H_
#define _MISC_H_

#include "types.h"

#define absmod(x,y) ((x)<0 ? 63-(abs((x)+1)%(y)) : (x)%(y))

#define inside_rect(x,y,rx,ry,rw,rh) ((x)>=(rx) && (y)>=(ry) \
	&& (x)<((rx)+(rw)) && (y)<((ry)+(rh)))

void sfp_delay(u32 ms);

#endif /* _MISC_H_ */
