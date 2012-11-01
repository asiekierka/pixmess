#ifndef _SFP_LUA_H_
#define _SFP_LUA_H_

#ifndef CONFIG_LUA_STUB
 #include <lua.h>
 #include <lauxlib.h>
#endif

int sfp_lua_init();

#endif /* _SFP_LUA_H_ */
