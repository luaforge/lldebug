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

#ifndef __LLDEBUG_LUAUTILS_H__
#define __LLDEBUG_LUAUTILS_H__

namespace lldebug {
namespace context {

class Context;

/**
 * @brief Guarantee the consistency of lua_State.
 */
class scoped_lua {
public:
	explicit scoped_lua(lua_State *L);
	explicit scoped_lua(lua_State *L, int n);
	explicit scoped_lua(Context *ctx, lua_State *L, bool withDebug=false);
	explicit scoped_lua(Context *ctx, lua_State *L, int n, bool withDebug=false);
	~scoped_lua();
	
private:
	void init(Context *ctx, lua_State *L, int n, bool withDebug);
	Context *m_ctx;
	lua_State *m_L;
	int m_top, m_n;
	bool m_isOldEnabled;
};

/// A dummy object that offers original address for lua.
extern const int llutil_address_for_internal_table;

/// Get the original name of the lua function.
std::string llutil_make_funcname(lua_Debug *ar);

/// Clone the table(idx).
int llutil_clone_table(lua_State *L, int idx);

/// Get the environ table binded the specified stack level.
int llutil_getfenv(lua_State *L, int level);


/// Convert to the string.
/** It doesn't use any lua functions
 */
std::string llutil_tostring_default(lua_State *L, int idx);

/// Convert to the string.
/** It calls 'lldebug.tostring' first,
 * if failed it calls the default function.
 */
std::string llutil_tostring(lua_State *L, int idx);

/// Convert to the string.
/** It is a lua function.
 */
int llutil_lua_tostring(lua_State *L);

/// Make a string of 'LuaVar' value.
/** It calls 'lldebug.tostring_for_varvalue' first,
 * if failed it calls the default function.
 */
std::string llutil_tostring_for_varvalue(lua_State *L, int idx);

/// Make a detail string of the lua object.
int llutil_lua_tostring_detail(lua_State *L);

/// Open the lldebug library.
int luaopen_lldebug(lua_State *L);


} // end of namespace context
} // end of namespace lldebug

#endif
