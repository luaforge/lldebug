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
#include "lldebug_encoding.h"

typedef int (*lldebug_InitState)(lua_State *L);
LUA_API lldebug_InitState lldebug_setinitstate(lldebug_InitState fn);
LUA_API lldebug_InitState lldebug_getinitstate(void);

LUA_API lua_State *lldebug_open(void);
LUA_API void lldebug_close(lua_State *L);
LUA_API int lldebug_openbase(lua_State *L);
LUA_API void lldebug_openlibs(lua_State *L);

LUA_API int lldebug_loadfile(lua_State *L, const char *filename);
LUA_API int lldebug_loadstring(lua_State *L, const char *str);

LUA_API void lldebug_call(lua_State *L, int narg, int nresult);
LUA_API int lldebug_pcall(lua_State *L, int narg, int nresult, int errfunc);
LUA_API int lldebug_resume(lua_State *L, int narg);

LUA_API int lldebug_setencoding(lldebug_Encoding encoding);
LUA_API lldebug_Encoding lldebug_getencoding();

#if !defined(LLDEBUG_CONTEXT) && !defined(LLDEBUG_FRAME)
#undef lua_open
#define lua_open() lldebug_open()
#undef lua_close
#define lua_close(L) lldebug_close(L)
#undef luaopen_base
#define luaopen_base(L) lldebug_openbase(L)
#undef luaL_openlibs
#define luaL_openlibs(L) lldebug_openlibs(L)
#endif

#ifdef __cplusplus
}
#endif

#endif
