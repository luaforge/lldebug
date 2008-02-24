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
#include "luainfo.h"

#ifdef LLDEBUG_CONTEXT
#include "context/luautils.h"
#endif

namespace lldebug {

// #define LUA_TNONE		(-1)
// #define LUA_TNIL		0
// #define LUA_TBOOLEAN		1
// #define LUA_TLIGHTUSERDATA	2
// #define LUA_TNUMBER		3
// #define LUA_TSTRING		4
// #define LUA_TTABLE		5
// #define LUA_TFUNCTION		6
// #define LUA_TUSERDATA		7
// #define LUA_TTHREAD		8
std::string LuaGetTypeName(int type) {
	const static std::string s_typenames[] = {
		"nil",
		"boolean",
		"lightuserdata",
		"number",
		"string",
		"table",
		"function",
		"userdata",
		"thread",
	};
	const static int size =
		(sizeof(s_typenames) / sizeof(s_typenames[0]));
	
	if (type < 0 || size <= type) {
		return std::string("");
	}

	return s_typenames[type];
}


/*-----------------------------------------------------------------*/
LuaStackFrame::LuaStackFrame(const LuaHandle &lua, int level)
	: m_lua(lua), m_level(level) {
}

LuaStackFrame::~LuaStackFrame() {
}


/*-----------------------------------------------------------------*/
#ifdef LLDEBUG_CONTEXT
LuaVar::LuaVar(const LuaHandle &lua, const std::string &name, int valueIdx)
	: m_lua(lua), m_name(name) {
	lua_State *L = lua.GetState();
	m_value = LuaToStringForVarValue(L, valueIdx);
	m_valueType = lua_type(L, valueIdx);
	m_tableIdx = RegisterTable(L, valueIdx);
	m_hasFields = CheckHasFields(L, valueIdx);
}

LuaVar::LuaVar(const LuaHandle &lua, const std::string &name,
			   const std::string &error)
	: m_lua(lua), m_name(name), m_value(error), m_valueType(LUA_TNONE)
	, m_tableIdx(-1), m_hasFields(false) {
}
#endif

LuaVar::LuaVar()
	: m_valueType(-1), m_tableIdx(-1), m_hasFields(false) {
}

LuaVar::~LuaVar() {
}

#ifdef LLDEBUG_CONTEXT
bool LuaVar::CheckHasFields(lua_State *L, int valueIdx) const {
	// Check whether 'valueIdx' has metatable.
	if (lua_getmetatable(L, valueIdx) != 0) {
		lua_pop(L, 1);
		return true;
	}

	if (!lua_istable(L, valueIdx)) {
		return false;
	}

	// Check whether 'valueIdx' has fields.
	lua_pushnil(L);
	if (lua_next(L, valueIdx) != 0) {
		lua_pop(L, 2);
		return true;
	}

	return false;
}

int LuaVar::RegisterTable(lua_State *L, int valueIdx) {
	if (!lua_istable(L, valueIdx)) {
		if (lua_getmetatable(L, valueIdx) == 0) {
			return -1;
		}
		lua_pop(L, 1);
	}

	// OriginalObj couldn't be handled correctly.
	if (lua_islightuserdata(L, valueIdx)
		&& lua_topointer(L, valueIdx) == &LuaAddressForInternalTable) {
		return -1;
	}

	int top = lua_gettop(L);

	// Try to do "table = registry[&OriginalObj]"
	lua_pushlightuserdata(L, (void *)&LuaAddressForInternalTable);
	lua_rawget(L, LUA_REGISTRYINDEX);
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);

		// local newtable = {}
		lua_newtable(L);

		// setmetatable(newtable, {__mode="vk"})
		lua_newtable(L);
		lua_pushliteral(L, "__mode");
		lua_pushliteral(L, "vk");
		lua_rawset(L, -3);
		lua_setmetatable(L, -2);

		// newtable[0] = 1 -- initial index
		lua_pushinteger(L, 1);
		lua_rawseti(L, -2, 0);

		// registry[&OriginalObj] = newtable
		lua_pushlightuserdata(L, (void *)&LuaAddressForInternalTable);
		lua_pushvalue(L, -2);
		lua_rawset(L, LUA_REGISTRYINDEX);
	}

	// table is registry[&OriginalObj]
	int table = lua_gettop(L);
	int n = 0; // use after

	// Check table[valueIdx] is number.
	lua_pushvalue(L, valueIdx);
	lua_rawget(L, table);
	if (lua_isnumber(L, -1)) {
		n = (int)lua_tointeger(L, -1);
		lua_pop(L, 1);

		// Check table[n] is table.
		lua_rawgeti(L, table, n);
		if (lua_istable(L, -1)) {
			// No problems.
			lua_pop(L, 1);
		}
		else {
			lua_pop(L, 1);

			// table[n] = valueIdx, again
			lua_pushvalue(L, valueIdx);
			lua_rawseti(L, table, n);
		}
	}
	else {
		lua_pop(L, 1);

		// n = table[0]
		lua_rawgeti(L, table, 0);
		n = (int)lua_tointeger(L, -1);
		lua_pop(L, 1);

		// table[0] = n + 1
		lua_pushinteger(L, n + 1);
		lua_rawseti(L, table, 0);

		// table[valueIdx] = n
		lua_pushvalue(L, valueIdx);
		lua_pushinteger(L, n);
		lua_rawset(L, table);

		// table[n] = valueIdx
		lua_pushvalue(L, valueIdx);
		lua_rawseti(L, table, n);
	}
	
	lua_pop(L, 1);
	assert(top == lua_gettop(L));
	return n;
}

int LuaVar::PushTable(lua_State *L) const {
	if (m_tableIdx < 0) {
		return -1;
	}

	// push registry[&OriginalObj][m_tableIdx]
	lua_pushlightuserdata(L, (void *)&LuaAddressForInternalTable);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_rawgeti(L, -1, m_tableIdx);
	lua_remove(L, -2);
	return 0;
}
#endif


/*-----------------------------------------------------------------*/
#ifdef LLDEBUG_CONTEXT
LuaBacktrace::LuaBacktrace(const LuaHandle &lua,
						   const std::string &name,
						   const std::string &sourceKey,
						   const std::string &sourceTitle,
						   int line, int level)
	: m_lua(lua), m_funcName(name)
	, m_key(sourceKey), m_sourceTitle(sourceTitle)
	, m_line(line), m_level(level) {
}
#endif

LuaBacktrace::LuaBacktrace() {
}

LuaBacktrace::~LuaBacktrace() {
}

}
