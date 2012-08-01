#ifndef _MISC_H_
#define _MISC_H_

#include "types.h"

#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))

#define divneg(x,y) ((x)<0 ? (x)/(y)-1 : (x)/(y))
#define absmod(x,y) ((x)<0 ? (((y)-1)-(-(x)-1)%(y)) : (x)%(y))

#define inside_rect(x,y,rx,ry,rw,rh) ((x)>=(rx) && (y)>=(ry) \
	&& (x)<((rx)+(rw)) && (y)<((ry)+(rh)))

void sfp_delay(u32 ms);

#endif /* _MISC_H_ */
