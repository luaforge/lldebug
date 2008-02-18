/*
 */

#undef LLDEBUG_CONTEXT
#include "lldebug.h"
/*extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}*/
#include <stdio.h>

#if defined(_MSC_VER)
#define snprintf _snprintf
#endif

const char *c_filename = "fft.lua";

int init_state(lua_State *L) {
	const char *s_dirlist[] = {
		"./",
		"../",
		"./test/",
		"../test/",
		"../../test/",
		NULL
	};
	
	luaL_openlibs(L);

	lua_getfield(L, LUA_GLOBALSINDEX, "package");
	int package = lua_gettop(L);

	lua_getfield(L, package, "path");
	lua_pushliteral(L, ";e:\\programs\\cpp\\library\\external\\bin\\lua\\?.lua");
	lua_concat(L, 2);
	lua_setfield(L, package, "path");

	lua_getfield(L, package, "cpath");
	lua_pushliteral(L, ";e:\\programs\\cpp\\library\\external\\bin\\lua\\?\\core.dll");
	lua_concat(L, 2);
	lua_setfield(L, package, "cpath");

	lua_pop(L, 1);

	for (int i = 0; s_dirlist[i] != NULL; ++i) {
		char buffer[128];

		snprintf(buffer, sizeof(buffer), "%s%s", s_dirlist[i], c_filename);
		if (lldebug_loadfile(L, buffer) == 0) {
			return 0;
		}
	}

	if (lua_isstring(L, -1)) {
		printf("%s\n", lua_tostring(L, -1));
	}
	return -1;
}

int main(int argc, char **argv) {
	lldebug_setinitstate(init_state);
	lua_State *L = lua_open();
	if (L == NULL) {
		return -1;
	}

	init_state(L);

	/*if (lldebug_loadstring(L,
		"function dtest()\n"
		"end\n"
		"function test2()\n"
		"end\n"
		"while 1 do\n"
		"  print(\"test\")\n"
		"end\n"
		"return 1 + 2 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4"
		) != 0) {
		//return 0;
	}*/

	/*if (lldebug_loadstring(L,
		"return"
		) != 0) {
		//return 0;
	}*/
//	while (1) ;

	if (lua_pcall(L, 0, 0, 0) != 0) {
		printf("%s\n", lua_tostring(L, -1));
		lua_close(L);
		return -1;
	}

	lua_close(L);
	return 0;
}
