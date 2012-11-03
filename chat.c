#include "chat.h"
#include "common.h"

chatmsg_t *messages[MAX_CHAT_MSG];

char* chat_get_msg(int index)
{
	if(index>=MAX_CHAT_MSG) return NULL;
	if(messages[index] == NULL) return NULL;
	return messages[index]->msg;
}

u8 chat_is_visible(int index)
{
	if(index>=MAX_CHAT_MSG) return 0;
	if(messages[index] == NULL) return 0;
	if((get_current_time() - messages[index]->time)<=CHAT_TIME_VISIBLE) return 1;
	return 0;
}

void chat_add_msg(char* msg)
{
	u8 i;
	u64 t = get_current_time();
	
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
