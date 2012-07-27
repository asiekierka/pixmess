#include "common.h"
#include "network.h"

int network_initialised = 0;

int network_init()
{
	if(network_initialised)
		return 0;
	
	// Only Windows requires special initialisation. --GM
	
	network_initialised = 255;
	return 0;
}

int server_init()
{
	if(network_init())
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

