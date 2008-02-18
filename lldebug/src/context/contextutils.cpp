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

#include "precomp.h"
#include "context/context.h"
#include "context/contextutils.h"

namespace lldebug {

const int LuaOriginalObject = 0;

std::string LuaToString(lua_State *L, int idx) {
	int type = lua_type(L, idx);
	std::string str;
	char buffer[512];

	switch (type) {
	case LUA_TNONE:
		str = "none";
		break;
	case LUA_TNIL:
		str = "nil";
		break;
	case LUA_TBOOLEAN:
		str = (lua_toboolean(L, idx) ? "true" : "false");
		break;
	case LUA_TNUMBER: {
		lua_Number d = lua_tonumber(L, idx);
		lua_Integer i;
		lua_number2int(i, d);
		if ((lua_Number)i == d) {
			snprintf(buffer, sizeof(buffer), "%d", i);
		}
		else {
			snprintf(buffer, sizeof(buffer), "%f", d);
		}
		str = buffer;
		}
		break;
	case LUA_TSTRING:
		str = lua_tostring(L, idx);
		break;
	case LUA_TFUNCTION:
	case LUA_TTHREAD:
	case LUA_TTABLE:
	case LUA_TUSERDATA:
	case LUA_TLIGHTUSERDATA:
		snprintf(buffer, sizeof(buffer), "%s: %p", lua_typename(L, type), lua_topointer(L, idx));
		str = buffer;
		break;
	default:
		return std::string("");
	}

	return str;
}

std::string LuaConvertString(lua_State *L, int idx) {
	int top = lua_gettop(L);
	const char *cstr;
	std::string str;

	lua_pushliteral(L, "lldebug");
	lua_gettable(L, LUA_GLOBALSINDEX);
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		goto on_error;
	}

	lua_pushliteral(L, "tostring");
	lua_gettable(L, -2);
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, 2);
		goto on_error;
	}
	lua_remove(L, -2); // eliminate 'lldebug'.

	lua_pushvalue(L, idx);
	if (lua_pcall(L, 1, 1, 0) != 0) {
		lua_pop(L, 1); // error string
		goto on_error;
	}

	cstr = lua_tostring(L, -1);
	str = (cstr != NULL ? cstr : "");
	lua_pop(L, 1);

	assert(top == lua_gettop(L));
	return str;

on_error:;
	assert(top == lua_gettop(L));
	return std::string("error on lldebug.tostring");
}

std::string LuaMakeFuncName(lua_Debug *ar) {
	std::string name;

	if (*ar->namewhat != '\0') { /* is there a name? */
		name = ar->name;
	}
	else {
		if (*ar->what == 'm') { /* main? */
			name = "main_chunk";
		}
		else if (*ar->what == 'C' || *ar->what == 't') {
			name = std::string("?");  /* C function or tail call */
		}
		else {
			std::stringstream stream;
			stream << "no name [defined <" << ar->short_src << ":" << ar->linedefined << ">]";
			stream.flush();
			name = stream.str();
		}
	}

	return name;
}


/*-----------------------------------------------------------------*/
scoped_lua::scoped_lua(lua_State *L) {
	init(NULL, L, -1, 0);
	m_top = -1; // suppress the stack check
}

scoped_lua::scoped_lua(lua_State *L, int n, int npop) {
	init(NULL, L, n, npop);
}

scoped_lua::scoped_lua(Context *ctx, lua_State *L) {
	init(ctx, L, -1, 0);
	m_top = -1; // suppress the stack check
}

scoped_lua::scoped_lua(Context *ctx, lua_State *L, int n, int npop) {
	init(ctx, L, n, npop);
}

void scoped_lua::init(Context *ctx, lua_State *L, int n, int npop) {
	m_L = L;
	m_n = n;
	m_npop = npop;
	m_ctx = ctx;
	m_top = lua_gettop(L);
	
	if (ctx == NULL) {
		ctx = Context::Find(L);
		if (ctx != NULL) {
			m_isOldEnabled = ctx->IsDebugEnabled();
			ctx->SetDebugEnable(false);
			
			m_ctx = ctx;
		}
	}
}

scoped_lua::~scoped_lua() {
	if (m_top >= 0) {
		assert(m_top + m_n == lua_gettop(m_L));
	}
	lua_pop(m_L, m_npop);

	// Restore the debug state.
	if (m_ctx != NULL) {
		m_ctx->SetDebugEnable(m_isOldEnabled);
	}
}

} // end of namespace lldebug
