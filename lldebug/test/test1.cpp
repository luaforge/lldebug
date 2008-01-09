/*
 */

#include "lldebug.h"
#include <stdio.h>

#if defined(_MSC_VER)
#define snprintf _snprintf
#endif

const char *c_filename = "fft.lua";

int init_state(lua_State *L) {
	const char *s_dirlist[] = {
		"./"
		"../",
		"./test/",
		"../test/",
		"../../test/",
		NULL
	};
	
	luaL_openlibs(L);

	for (int i = 0; s_dirlist[i] != NULL; ++i) {
		char buffer[128];

		snprintf(buffer, sizeof(buffer), "%s%s", s_dirlist[i], c_filename);
		if (luaL_loadfile(L, buffer) == 0) {
			return 0;
		}
	}

	printf("%s\n", lua_tostring(L, -1));
	lldebug_close(L);
	return -1;
}

int main(int argc, char **argv) {
	lldebug_setinitstate(init_state);
	lua_State *L = lua_open();
	if (L == NULL) {
		return -1;
	}

	init_state(L);

	if (lua_pcall(L, 0, 0, 0) != 0) {
		printf("%s\n", lua_tostring(L, -1));
		lldebug_close(L);
		return -1;
	}

	lua_close(L);
	return 0;
}
