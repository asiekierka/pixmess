#ifndef _SFP_COMMON_H_
#define _SFP_COMMON_H_

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include <sys/time.h>

#ifndef WIN32
#include <signal.h>
#endif

#include <zlib.h>

#include "types.h"

#include "misc.h"
#include "tile.h"

#define WIRIUM_DIVISOR 2
#define NICKNAME "Gamemaster"

// TODO shift into the right location
void write16le(u16 data, u8 *v);
void write32le(u32 data, u8 *v);
u16 read16le(u8 *v);
u32 read32le(u8 *v);

#endif /* _SFP_COMMON_H_ */
