#include "common.h"
#include "lua.h"
#include "types.h"

lua_State *LS = NULL;
lua_State *LCg = NULL;
lua_State *LCs = NULL;

// NOTE: WE WILL NOT PROVIDE STANDARD Lua LIBRARIES.
// We can provide subsets, but that's it - no string.dump for you!

int sfp_lua_init()
{
	LS = luaL_newstate();
	LCg = luaL_newstate();
	LCs = luaL_newstate();
	
	if(LS == NULL || LCg == NULL || LCs == NULL)
	{
		fprintf(stderr, "ERROR: Could not allocate Lua states!\n");
		return 1;
	}
	
	return 0;
}
