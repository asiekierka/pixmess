/*
split this into several .c files as required.
yeah, welcome to bottom-up development.
hope you enjoy the refactoring.
remember to do it often, or everything will turn to crap!
    --GM
*/

#include "common.h"
#include "render.h"

int main(int argc, char *argv[])
{
	sfp_init_render();

	sfp_render_begin();
	sfp_printf_2x(1*8,2*8,0x1F,0,"Hello %s! You are player %i.", "Neo", 42);
	sfp_printf_1x(3*8,7*8,0x07,0,"(You also suck at this game.)");
	sfp_render_end();

	sfp_delay(2000);
	
	return 0;
}

