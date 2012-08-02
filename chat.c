#include "chat.h"
#include "common.h"

chatmsg_t *messages[MAX_CHAT_MSG];

char* get_chat_msg(int index)
{
	if(index>=MAX_CHAT_MSG) return NULL;
	if(messages[index] == NULL) return NULL;
	return messages[index]->msg;
}

void add_chat_msg(char* msg)
{
	u8 i;
	u64 add_time = time();
}
