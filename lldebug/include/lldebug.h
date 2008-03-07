/*
 * Copyright (c) 2005-2008  cielacanth <cielacanth AT s60.xrea.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef __LLDEBUG_H__
#define __LLDEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#ifdef LLDEBUG_BUILD_DLL
	#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
		#define LLDEBUG_API extern __declspec(dllexport)
	#else
		#define LLDEBUG_API extern
	#endif
#else
	#define LLDEBUG_API extern
#endif

LLDEBUG_API lua_State *lldebug_open(void);
LLDEBUG_API void lldebug_close(lua_State *L);
LLDEBUG_API int lldebug_loadfile(lua_State *L, const char *filename);
LLDEBUG_API int lldebug_loadstring(lua_State *L, const char *str);

LLDEBUG_API void lldebug_call(lua_State *L, int nargs, int nresults);
LLDEBUG_API int lldebug_pcall(lua_State *L, int nargs, int nresults, int errfunc);
LLDEBUG_API int lldebug_resume(lua_State *L, int nargs);

/// The substitute function for luaopen_base overriding coroutine.create
/// and coroutine.resume.
LLDEBUG_API int lldebug_openbase(lua_State *L);
/// The substitute function for luaL_openlibs overriding 'luaopen_base'.
LLDEBUG_API void lldebug_openlibs(lua_State *L);


/// Set the host address and service name if you want to debug remotely.
/**
 * @param hostname    Host name default value is 'localhost'.
 * @param servicename Service name default value is '51123' (decided randomly).
 */
LLDEBUG_API void lldebug_setremoteaddress(const char *hostname,
										  const char *servicename);
/// Get the host address and service name. (Set the static value)
LLDEBUG_API void lldebug_getremoteaddress(const char **hostname,
										  const char **servicename);

/**
 * @brief The identifier of the encoding types.
 */
enum lldebug_Encoding {
	LLDEBUG_ENCODING_UTF8, /**< default encoding */

	// Japanese
	LLDEBUG_ENCODING_SJIS,
	LLDEBUG_ENCODING_EUC,
	LLDEBUG_ENCODING_ISO2022JP,
};

/// Set the encoding type for displaying on debugger.
LLDEBUG_API int lldebug_setencoding(lldebug_Encoding encoding);
/// Get the encoding type for displaying on debugger.
LLDEBUG_API lldebug_Encoding lldebug_getencoding();

#if !defined(LLDEBUG_CONTEXT) && !defined(LLDEBUG_VISUAL)
#undef lua_open
#define lua_open lldebug_open
#undef lua_close
#define lua_close lldebug_close
#undef luaL_loadfile
#define luaL_loadfile lldebug_loadfile
#undef luaL_loadstring
#define luaL_loadstring lldebug_loadstring
/*#undef lua_call
#define lua_call lldebug_call
#undef lua_pcall
#define lua_pcall lldebug_pcall
#undef lua_resume
#define lua_resume lldebug_resume*/
#undef luaopen_base
#define luaopen_base lldebug_openbase
#undef luaL_openlibs
#define luaL_openlibs lldebug_openlibs
#endif

#ifdef __cplusplus
}
#endif

#endif
