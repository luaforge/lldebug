//---------------------------------------------------------------------------
//
// Name:        Project1App.h
// Author:      ‰ë”Ž
// Created:     2007/11/23 0:05:32
// Description: 
//
//---------------------------------------------------------------------------

#ifndef __LLDEBUG_H__
#define __LLDEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

typedef int (*lldebug_InitState)(lua_State *L);
LUA_API lldebug_InitState lldebug_setinitstate(lldebug_InitState fn);
LUA_API lldebug_InitState lldebug_getinitstate(void);

LUA_API lua_State *lldebug_open(void);
LUA_API void lldebug_close(lua_State *L);
LUA_API int lldebug_openbase(lua_State *L);
LUA_API void lldebug_openlibs(lua_State *L);

LUA_API void lldebug_call(lua_State *L, int argn, int resultn);
LUA_API int lldebug_pcall(lua_State *L, int argn, int resultn, int errfunc);
LUA_API int lldebug_resume(lua_State *L, int argn);
LUA_API int lldebug_yield(lua_State *L, int resultn);

#ifndef LLDEBUG_BUILD_DLL
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
