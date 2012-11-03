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
	u64 t;
	
	// TODO win32-appropriate version (once again, windows sucks)
	struct timeval tv;
	gettimeofday(&tv, NULL);
	t = (((u64)tv.tv_sec)*(u64)1000000) + (u64)tv.tv_usec;
	
	// use t just to shut the compiler up
	printf("time: %lli\n", (long long int)t);
	if(messages[MAX_CHAT_MSG-1] != NULL)
	{
		for(i=1;i<MAX_CHAT_MSG;i++)
			messages[i-1] = messages[i];
		messages[MAX_CHAT_MSG-1] = malloc(sizeof(chatmsg_t));
		messages[MAX_CHAT_MSG-1]->msg = msg;
		messages[MAX_CHAT_MSG-1]->time = t;
		return;
	}
	else
	{
		for(i=0;i<MAX_CHAT_MSG;i++)
			if(messages[i]==NULL)
			{
				messages[i] = malloc(sizeof(chatmsg_t));
				messages[i]->msg = msg;
				messages[i]->time = t;
				return;
			}
	}
}
