#include "common.h"
#include "network.h"

int net_sockfd = 0;
int server_sockfd = 0;

int net_initialised = 0;

int net_init()
{
	if(net_initialised)
		return 0;
	
	// Only Windows requires special initialisation. --GM
	
	net_initialised = 255;
	return 0;
}

void net_report_layer(s32 x, s32 y, u8 position)
{
	if(net_init())
		return;
	
	printf("net_report_layer: %d,%d, pos %d\n",x,y,position);
}

void net_report_unlayer(s32 x, s32 y, u8 position)
{
	if(net_init())
		return;
	
	printf("net_report_unlayer: %d,%d, pos %d\n",x,y,position);
}

int server_init()
{
	if(net_init())
		return 1;
}

int server_run_dedicated()
{
	printf("Hi, I'm a server that's *dedicated* to giving YOU a return code of 0.\n");
	
	if(server_init())
	{
		printf("UNFORTUNATELY, something went wrong, so have a 1 instead.\n");
		return 1;
	}
	
	return 0;
}

