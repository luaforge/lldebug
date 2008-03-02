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
	explicit scoped_lua(lua_State *L, int n, int npop = 0);
	explicit scoped_lua(Context *ctx, lua_State *L);
	explicit scoped_lua(Context *ctx, lua_State *L, int n, int npop = 0);
	~scoped_lua();
	
private:
	void init(Context *ctx, lua_State *L, int n, int npop);
	Context *m_ctx;
	lua_State *m_L;
	int m_top, m_n;
	int m_npop;
	bool m_isOldEnabled;
};

/// A dummy object that offers original address for lua.
extern const int LuaAddressForInternalTable;

/// Convert the lua object placed on idx to string.
/// It doesn't use any lua functions.
std::string LuaToStringFast(lua_State *L, int idx);

/// Convert the lua object placed on idx to string for the value of LuaVar.
/// It uses lua script functions.
std::string LuaToStringForVarValue(lua_State *L, int idx);

/// Get the original name of the lua function.
std::string LuaMakeFuncName(lua_Debug *ar);

int luaopen_lldebug(lua_State *L);

} // end of namespace context
} // end of namespace lldebug

#endif
