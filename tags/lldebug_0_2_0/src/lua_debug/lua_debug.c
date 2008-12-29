/*
 * Test program.
 */

#include "lldebug.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	lua_State *L;

	if (argc < 2) {
		printf("Usage: test filename [hostname [servicename]]\n");
		return -1;
	}

	// Set the ip address and service name, if need.
	if (argc > 2) {
		lldebug_setremoteaddress(argv[2], 0);
	}
	if (argc > 3) {
		lldebug_setremoteaddress(NULL, atoi(argv[3]));
	}

	printf("TRACE: begin lldebug_open...\n");
	L = lldebug_open();
	if (L == NULL) {
		printf("Couldn't open the lua_State.\n");
		return -1;
	}

	// Set encoding type. (default is utf8)
#if 1 //defined(WIN32) || defined(_WIN32) || defined(__WIN32)
	lldebug_setencoding(L, LLDEBUG_ENCODING_SJIS);
#else
	lldebug_setencoding(L, LLDEBUG_ENCODING_EUC);
#endif

	// Open standard libs.
	printf("TRACE: begin lldebug_openlibs...\n");
	lldebug_openlibs(L);
	
	// Load the lua script.
	printf("TRACE: begin lldebug_loadfile...\n");
	if (lldebug_loadfile(L, argv[1]) != 0) {
		printf("%s\n", lua_tostring(L, -1));
		lldebug_close(L);
		return -1;
	}

	// Execute !
	printf("TRACE: begin lldebug_pcall...\n");
	if (lldebug_pcall(L, 0, 0, 0) != 0) {
		printf("%s\n", lua_tostring(L, -1));
		lldebug_close(L);
		return -1;
	}

	// Call the lua program without protection.
	//lua_call(L, 0, 0);

	lldebug_close(L);
	return 0;
}
