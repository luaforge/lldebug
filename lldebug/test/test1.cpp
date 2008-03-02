/*
 * Test program.
 */

#include "lldebug.h"
#include <stdio.h>
#include <fstream>

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Usage: test filename\n");
		return -1;
	}

	// Set encoding type.
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
	lldebug_setencoding(LLDEBUG_ENCODING_SJIS);
#else
	lldebug_setencoding(LLDEBUG_ENCODING_EUC);
#endif

	lua_State *L = lldebug_open();
	if (L == NULL) {
		printf("Couldn't open the lua_State.\n");
		return -1;
	}

	lldebug_openlibs(L);

	// Load the lua script.
	if (lldebug_loadfile(L, argv[1]) != 0) {
		printf("%s\n", lua_tostring(L, -1));
		lua_close(L);
		return -1;
	}

	// Execute !
	if (lua_pcall(L, 0, 0, 0) != 0) {
		printf("%s\n", lua_tostring(L, -1));
		lua_close(L);
		return -1;
	}

	lua_close(L);
	return 0;
}
