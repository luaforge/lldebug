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
namespace context {

const int llutil_address_for_internal_table = 0;

/// Get field from the 'lldebug' table.
int llutil_rawget(lua_State *L, const char *name) {
	lua_pushliteral(L, "lldebug");
	lua_rawget(L, LUA_GLOBALSINDEX);
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return 0;
	}

	lua_pushstring(L, name);
	lua_rawget(L, -2);
	lua_remove(L, -2); // eliminate 'lldebug'.
	return 1;
}

std::string llutil_makefuncname(lua_Debug *ar) {
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

int llutil_listlocalfuncs(lua_State *L) {
	lua_Debug ar;

	for (int level = 0; lua_getstack(L, level, &ar) != 0; ++level) {
		lua_getinfo(L, "Snl", &ar);
		std::string name = llutil_makefuncname(&ar);
		printf("level %d: %s\n", level, name.c_str());
	}

	return 0;
}

int llutil_clonetable(lua_State *L, int idx) {
	scoped_lua scoped(L);

	if (!lua_istable(L, idx)) {
		return 0;
	}

	// Create new table.
	lua_newtable(L);
	int table = lua_gettop(L);

	// Clone table.
	lua_pushnil(L);
	for (int i = 0; lua_next(L, idx) != 0; ++i) {
		lua_pushvalue(L, lua_gettop(L) - 1); // Push key index.
		lua_insert(L, -2);
		lua_rawset(L, table);
	}

	return scoped.check(1);
}

int llutil_getfenv(lua_State *L, int level) {
	lua_Debug ar;
	
	if (lua_getstack(L, level, &ar) == 0) {
		return 0;
	}

	if (lua_getinfo(L, "f", &ar) == 0) {
		return 0;
	}

	lua_getfenv(L, -1);
	lua_remove(L, -2); // eliminate the local function.
	return 1;
}

int llutil_setfenv(lua_State *L, int level, int idx) {
	lua_Debug ar;

	if (!lua_istable(L, idx)) {
		return -1;
	}

	// Get the stack frame.
	if (lua_getstack(L, level, &ar) == 0) {
		return -1;
	}

	// Push idx now, because idx may move after lua_getinfo.
	lua_pushvalue(L, idx);
	if (lua_getinfo(L, "f", &ar) == 0) {
		lua_pop(L, 1);
		return -1;
	}

	lua_insert(L, -2);
	lua_setfenv(L, -2);
	lua_remove(L, -1); // eliminate the local function.
	return 0;
}

/**
 * @brief Make the table that has all local variables.
 */
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

int llutil_getlocals(lua_State *L, int level, bool checkLocal,
					  bool checkUpvalue, bool checkEnv) {
	scoped_lua scoped(L);

	// Push result table.
	lua_newtable(L);
	int table = lua_gettop(L);

	variable_table callback(L, table);
	if (iterate_locals(
		callback, L, level, checkLocal, checkUpvalue, checkEnv) != 0) {
		lua_remove(L, table);
		return scoped.check(0);
	}

	return scoped.check(1);
}


std::string llutil_tostring_fast(lua_State *L, int idx) {
	scoped_lua scoped(L);
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

	scoped.check(0);
	return str;
}

static int llutil_lua_tostring_default(lua_State *L) {
	std::string str = llutil_tostring_fast(L, 1);
	lua_pushlstring(L, str.c_str(), str.length());
	return 1;
}

std::string llutil_tostring(lua_State *L, int idx) {
	scoped_lua scoped(L);

	if (llutil_rawget(L, "tostring") == 0) {
		scoped.check(0);
		return llutil_tostring_fast(L, idx);
	}

	// Call the function.
	lua_pushvalue(L, idx);
	if (lua_pcall(L, 1, 1, 0) != 0) {
		lua_pop(L, 1); // eliminate error message
		scoped.check(0);
		return llutil_tostring_fast(L, idx);
	}

	const char *cstr = lua_tostring(L, -1);
	std::string str = (cstr != NULL ? cstr : "");
	lua_pop(L, 1);
	scoped.check(0);
	return str;
}

int llutil_lua_tostring(lua_State *L) {
	std::string str = llutil_tostring(L, 1);
	lua_pushlstring(L, str.c_str(), str.length());
	return 1;
}


/// Make a string of 'LuaVar' value.
/** It doesn't use any lua functions.
 */
static std::string llutil_tostring_for_varvalue_default(lua_State *L, int idx) {
	scoped_lua scoped(L);
	std::string result;

	if (luaL_callmeta(L, idx, "__tostring") != 0) {
		const char *cstr = lua_tostring(L, -1);
		result = (cstr != NULL ? cstr : "");
		lua_pop(L, 1);
		scoped.check(0);
		return result;
	}

	switch (lua_type(L, idx)) {
	case LUA_TNONE:
	case LUA_TNIL:
		result = "nil";
		break;
	case LUA_TBOOLEAN:
		result = (lua_toboolean(L, idx) ? "true" : "false");
		break;
	case LUA_TNUMBER:
		lua_pushvalue(L, idx);
		result = lua_tostring(L, -1);
		lua_pop(L, 1);
		break;
	case LUA_TSTRING:
		result = lua_tostring(L, idx);
		break;
	default:
		lua_pushfstring(L, "%p", lua_topointer(L, idx));
		result = lua_tostring(L, -1);
		lua_pop(L, 1);
		break;
	}

	scoped.check(0);
	return result;
}

/// Make a string of 'LuaVar' value.
/** It calls 'llutil_tostring_for_varvalue_default' function.
 */
static int llutil_lua_tostring_for_varvalue_default(lua_State *L) {
	std::string str = llutil_tostring_for_varvalue_default(L, 1);
	lua_pushlstring(L, str.c_str(), str.length());
	return 1;
}

std::string llutil_tostring_for_varvalue(lua_State *L, int idx) {
	scoped_lua scoped(L);

	if (llutil_rawget(L, "tostring_for_varvalue") == 0) {
		scoped.check(0);
		return llutil_tostring_for_varvalue_default(L, idx);
	}

	// Call 'lldebug.tostring_for_varvalue'
	lua_pushvalue(L, idx);
	if (lua_pcall(L, 1, 1, 0) != 0) {
		lua_pop(L, 1);
		scoped.check(0);
		return llutil_tostring_for_varvalue_default(L, idx);
	}

	const char *cstr = lua_tostring(L, -1);
	std::string str = (cstr != NULL ? cstr : "");
	lua_pop(L, 1);
	scoped.check(0);
	return str;
}


/**
 * @brief 
 */
struct table_string_maker {
	explicit table_string_maker(lua_State *L)
		: m_L(L), m_successed(true) {
		lua_pushliteral(L, "");
		m_strpos = lua_gettop(L);
	}
	~table_string_maker() {
		if (!m_successed) {
			lua_remove(m_L, m_strpos);
		}
	}
	void failed() {
		m_successed = false;
	}
	int operator()(lua_State *, const std::string &name, int valueIdx) {
		// Convert a field to string.
		std::string valueStr = llutil_tostring(m_L, valueIdx);

		// Concat strings.
		lua_pushvalue(m_L, m_strpos);
		lua_pushliteral(m_L, ",\n    ");
		lua_pushlstring(m_L, name.c_str(), name.length());
		lua_pushliteral(m_L, " = ");
		lua_pushlstring(m_L, valueStr.c_str(), valueStr.length());
		lua_concat(m_L, 5);

		// Set the new string to m_strpos.
		lua_replace(m_L, m_strpos);
		return 0;
	}
private:
	lua_State *m_L;
	int m_strpos;
	bool m_successed;
	};

static std::string llutil_tostring_detail_default(lua_State *L, int first, int last) {
	scoped_lua scoped(L);
	int top = lua_gettop(L);

	for (int i = first; i <= last; ++i) {
		if (i > first) {
			lua_pushliteral(L, ",\n");
		}

		// Try to call metatable.__tostring.
		if (luaL_callmeta(L, i, "__tostring") != 0) {
			continue;
		}

		std::string str = llutil_tostring(L, i);
		lua_pushlstring(L, str.c_str(), str.length());

		// Make string and push it, if successed.
		table_string_maker maker(L);
		if (iterate_fields(maker, L, i) != 0) {
			maker.failed();
		}
	}

	// Does any strings exist ?
	if (top == lua_gettop(L)) {
		scoped.check(0);
		return std::string("");
	}

	// Concat all the strings.
	lua_concat(L, lua_gettop(L) - top);
	const char *cstr = lua_tostring(L, -1);
	lua_pop(L, 1);
	scoped.check(0);
	return std::string(cstr != NULL ? cstr : "");
}

static int llutil_lua_tostring_detail_default(lua_State *L) {
	std::string str = llutil_tostring_detail_default(L, 1, lua_gettop(L));
	lua_pushlstring(L, str.c_str(), str.length());
	return 1;
}

int llutil_lua_tostring_detail(lua_State *L) {
	scoped_lua scoped(L);
	int top = lua_gettop(L);

	if (llutil_rawget(L, "tostring_detail") == 0) {
		scoped.check(0);
		return llutil_lua_tostring_detail_default(L);
	}

	// Push all arguments.
	for (int i = 1; i <= top; ++i) {
		lua_pushvalue(L, i);
	}
	
	// Call 'lldebug.tostring_detail'
	if (lua_pcall(L, top, 1, 0) != 0) {
		scoped.check(1);
		return 1; // Return the error message.
	}

	scoped.check(1);
	return 1;
}


/*-----------------------------------------------------------------*/
static int llutil_get_luavar_table(lua_State *L) {
	lua_pushlightuserdata(L, (void *)&llutil_address_for_internal_table);
	lua_rawget(L, LUA_REGISTRYINDEX);
	return 1;
}

static const luaL_reg llutils_regs[] = {
	{"tostring", llutil_lua_tostring_default},
	{"tostring_for_varvalue", llutil_lua_tostring_for_varvalue_default},
	{"tostring_detail", llutil_lua_tostring_detail_default},
	{"get_luavar_table", llutil_get_luavar_table},
	{NULL, NULL}
};

int luaopen_lldebug(lua_State *L) {
	luaL_openlib(L, LUA_LLDEBUGLIBNAME, llutils_regs, 0);
	return 1;
}


/*-----------------------------------------------------------------*/
scoped_lua::scoped_lua(lua_State *L) {
	init(NULL, L, false);
}

scoped_lua::scoped_lua(Context *ctx, lua_State *L, bool withDebug) {
	init(ctx, L, withDebug);
}

void scoped_lua::init(Context *ctx, lua_State *L, bool withDebug) {
	if (ctx != NULL) {
		m_isOldEnabled = ctx->IsDebugEnabled();
		ctx->SetDebugEnable(withDebug);
	}

	m_L = L;
	m_ctx = ctx;
	m_top = (L != NULL ? lua_gettop(L) : -1);
}

scoped_lua::~scoped_lua() {
	if (m_ctx != NULL) {
		m_ctx->SetDebugEnable(m_isOldEnabled);
	}
}

int scoped_lua::check(int n) {
	if (m_top >= 0) {
		assert(m_top + n == lua_gettop(m_L));
	}

	return n;
}

} // end of namespace context
} // end of namespace lldebug
