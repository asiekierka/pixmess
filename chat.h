#ifndef _CHAT_H_
#define _CHAT_H_

#include "common.h"

#define MAX_CHAT_MSG 20

void add_chat_msg(char* msg);
char* get_chat_msg(int count);

#endif /* _CHAT_H */
