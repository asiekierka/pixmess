#include "common.h"
#include "event.h"

// Some abstractions.

int sfp_event_keywithwait(int key) {
	if(sfp_event_getkeywait(key)==0)
		return sfp_event_key(key);
	return 0;
}

int sfp_event_getkeywithwait() {
	int i = sfp_event_getkey();
	if(sfp_event_getkeywait(i)==0)
		return i;
	return 0;
}
