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
#include "context/luautils.h"
#include "context/luaiterate.h"

namespace lldebug {

const int LuaAddressForInternalTable = 0;

/// Call the function of the metatable.
static int call_metafunc(lua_State *L, int idx, const std::string &name) {
	if (lua_getmetatable(L, idx) != 0) {
		lua_pushlstring(L, name.c_str(), name.length());
		lua_rawget(L, -2);
		lua_remove(L, -2);

		if (lua_isfunction(L, -1)) {
			lua_pushvalue(L, 1);
			lua_pcall(L, 1, 1, 0);
			return 0;
		}

		lua_pop(L, 1);
	}

	return -1;
}

/// Call the function of 'lldebug'.
static int push_lldebugfunc(lua_State *L, const std::string &name) {
	lua_pushliteral(L, "lldebug");
	lua_rawget(L, LUA_GLOBALSINDEX);
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return -1;
	}

	lua_pushlstring(L, name.c_str(), name.length());
	lua_rawget(L, -2);
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, 2);
		return -1;
	}

	lua_remove(L, -2); // eliminate 'lldebug'.
	return 0;
}

std::string LuaToStringFast(lua_State *L, int idx) {
	scoped_lua scoped(L, 0);
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
	case LUA_TNUMBER:
		lua_pushvalue(L, idx);
		str = lua_tostring(L, -1);
		lua_pop(L, 1);
		break;
	case LUA_TSTRING:
		str = lua_tostring(L, idx);
		break;
	default:
		snprintf(buffer, sizeof(buffer), "%s: %p", lua_typename(L, type), lua_topointer(L, idx));
		str = buffer;
		break;
	}

	return str;
}

std::string LuaToStringForVarValue(lua_State *L, int idx) {
	scoped_lua scoped(L, 0);

	if (push_lldebugfunc(L, "tostring_for_varvalue") != 0) {
		return "Not found 'lldebug.tostring_for_varvalue'";
	}

	// Call 'lldebug.tostring_for_varvalue'
	lua_pushvalue(L, idx);
	lua_pcall(L, 1, 1, 0);

	const char *cstr = lua_tostring(L, -1);
	std::string str = (cstr != NULL ? cstr : "");
	lua_pop(L, 1);
	return str;
}

std::string LuaToString(lua_State *L, int idx) {
	scoped_lua scoped(L, 0);

	if (push_lldebugfunc(L, "tostring") != 0) {
		return "Not found 'lldebug.tostring'";
	}

	// Call the function.
	lua_pushvalue(L, idx);
	lua_pcall(L, 1, 1, 0);

	const char *cstr = lua_tostring(L, -1);
	std::string str = (cstr != NULL ? cstr : "");
	lua_pop(L, 1);
	return str;
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
static int llutils_tostring_fast(lua_State *L) {
	std::string str = LuaToStringFast(L, -1);
	lua_pushlstring(L, str.c_str(), str.length());
	return 1;
}

static int llutils_tostring(lua_State *L) {
	int top = lua_gettop(L);

	lua_pushliteral(L, "tostring");
	lua_rawget(L, LUA_GLOBALSINDEX);
	lua_pushvalue(L, 1);
	lua_pcall(L, 1, 1, 0);
	return (lua_gettop(L) - top);
}

/*static void list_local_funcname(lua_State *L) {
	lua_Debug ar;

	for (int level = 0; lua_getstack(L, level, &ar) != 0; ++level) {
		lua_getinfo(L, "Snl", &ar);
		std::string fname = LuaMakeFuncName(&ar);
		printf("level %d: %s\n", level, fname.c_str());
	}
	}*/


static int llutils_tostring_for_varvalue_default(lua_State *L) {
	scoped_lua scoped(L, 1);
	int type = lua_type(L, 1);

	if (call_metafunc(L, 1, "__tostring") == 0) {
		return 1;
	}

	switch (type) {
	case LUA_TNONE:
	case LUA_TNIL:
		lua_pushliteral(L, "nil");
		break;
	case LUA_TBOOLEAN:
		lua_pushstring(L, (lua_toboolean(L, 1) ? "true" : "false"));
		break;
	case LUA_TNUMBER:
		lua_pushvalue(L, 1);
		lua_tostring(L, -1);
		break;
	case LUA_TSTRING:
		lua_pushvalue(L, 1);
		break;
	default:
		lua_pushfstring(L, "%p", lua_topointer(L, 1));
		break;
	}

	return 1;
}

static int llutils_tostring_detail_default(lua_State *L) {
	scoped_lua scoped(L, 1);
	int top = lua_gettop(L);

	for (int i = 1; i <= top; ++i) {
		if (i > 1) {
			lua_pushliteral(L, ",\n");
		}

		if (call_metafunc(L, i, "__tostring") == 0) {
			continue;
		}

		std::string str = LuaToString(L, i);
		lua_pushlstring(L, str.c_str(), str.length());

		if (lua_type(L, i) == LUA_TTABLE) {
			lua_pushnil(L);
			while (lua_next(L, i) != 0) {
				int valueIdx = lua_gettop(L);
				int keyIdx = valueIdx - 1;

				// Make fields string of this table.
				std::string keyStr = LuaToString(L, keyIdx);
				std::string valueStr = LuaToString(L, valueIdx);

				lua_pushliteral(L, ",\n");
				lua_pushliteral(L, "    ");
				lua_pushlstring(L, keyStr.c_str(), keyStr.length());
				lua_pushliteral(L, " = ");
				lua_pushlstring(L, valueStr.c_str(), valueStr.length());

				// Move key idx to the top and eliminate value idx.
				lua_pushvalue(L, keyIdx);
				lua_remove(L, valueIdx); // We must eliminate the value idx first.
				lua_remove(L, keyIdx);
			}
		}
	}

	// Concat the all strings.
	if (top == lua_gettop(L)) {
		lua_pushliteral(L, "");
	}
	else {
		lua_concat(L, lua_gettop(L) - top);
	}

	return 1;
}

static int llutils_get_luavar_table(lua_State *L) {
	lua_pushlightuserdata(L, (void *)&LuaAddressForInternalTable);
	lua_rawget(L, LUA_REGISTRYINDEX);
	return 1;
}

struct variable_table {
	lua_State *m_L;
	int m_table;
	explicit variable_table(lua_State *L, int table)
		: m_L(L), m_table(table) {
	}
	int operator()(lua_State *L, const std::string &name, int valueIdx) {
		lua_pushlstring(L, name.c_str(), name.length());
		lua_pushvalue(L, valueIdx);
		lua_rawset(L, m_table);
		return 0;
	}
	};

static int llutils_get_locals(lua_State *L) {
	lua_newtable(L);
	int table = lua_gettop(L);

	variable_table callback(L, table);
	if (iterate_locals(callback, L, 3, true, true, false) != 0) {
		lua_remove(L, table);
		return 0;
	}

	return 1;
}

static const luaL_Reg llutils_regs[] = {
	{"tostring", llutils_tostring},
	{"tostring_for_varvalue", llutils_tostring_for_varvalue_default},
	{"tostring_detail", llutils_tostring_detail_default},
	{"get_luavar_table", llutils_get_luavar_table},
	{"getlocals", llutils_get_locals},
//	{"toluavar", llutils_toluavar},
	{NULL, NULL}
};

int luaopen_lldebug(lua_State *L) {
	luaL_register(L, "lldebug", llutils_regs);
	return 1;
}

int LuaInitialize(lua_State *L) {
	luaL_register(L, "lldebug", llutils_regs);
	return 0;
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
