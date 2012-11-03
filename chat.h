#ifndef _CHAT_H_
#define _CHAT_H_

#include "common.h"

#define MAX_CHAT_MSG 20
#define CHAT_TIME_VISIBLE (20*1000000)

void chat_add_msg(char* msg);
char* chat_get_msg(int count);

#endif /* _CHAT_H */
