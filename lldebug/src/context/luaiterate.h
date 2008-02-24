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

#ifndef __LLDEBUG_LUAITERATE_H__
#define __LLDEBUG_LUAITERATE_H__

#include "sysinfo.h"
#include "luainfo.h"

namespace lldebug {

/// Find the local value, and push it if find.
int find_localvalue(lua_State *L, int level, const std::string &target,
					bool checkLocal, bool checkUpvalue, bool checkEnv);

/// Set the new value to the local variable.
int set_localvalue(lua_State *L, int level, const std::string &target,
				   int valueIdx, bool checkLocal, bool checkUpvalue,
				   bool checkEnv, bool forceCreate);

/**
 * @brief Make a LuaVarList object.
 */
struct varlist_maker {
	int operator()(lua_State *L, const std::string &name, int valueIdx) {
		m_result.push_back(LuaVar(LuaHandle(L), name, valueIdx));
		return 0;
	}

	/// Get the result.
	LuaVarList &get_result() {
		return m_result;
	}

private:
	LuaVarList m_result;
};


/// Iterate the all fields of idx object.
template<class Fn>
int iterate_fields(Fn &callback, lua_State *L, int idx) {
	scoped_lua scoped(L, 0);

	// check metatable
	if (lua_getmetatable(L, idx)) {
		int top = lua_gettop(L);
		// name of metatable is "(*metatable)"
		int ret = callback(L, std::string("(*metatable)"), top);
		lua_pop(L, 1);
		if (ret != 0) {
			return ret;
		}
	}
		
	if (lua_type(L, idx) != LUA_TTABLE) {
		return 0;
	}

	lua_pushnil(L);  // first key
	for (int i = 0; lua_next(L, idx) != 0; ++i) {
		// key index: top - 1, value index: top
		int top = lua_gettop(L);
		if (idx == LUA_REGISTRYINDEX && lua_islightuserdata(L, -2)
			&& lua_topointer(L, -2) == &LuaAddressForInternalTable) {
		}
		else {
			int ret = callback(L, LuaToStringFast(L, top - 1), top);
			if (ret != 0) {
				lua_pop(L, 2);
				return ret;
			}
		}

		// eliminate the value index and pushed value and key index
		lua_pop(L, 1);
	}

	return 0;
}

/// Iterate the all fields of var.
template<class Fn>
int iterate_var(Fn &callback, const LuaVar &var) {
	lua_State *L = var.GetLua().GetState();

	if (var.PushTable(L) != 0) {
		return -1;
	}

	scoped_lua scoped(L, 0, 1);
	return iterate_fields(callback, L, lua_gettop(L));
}

/// Iterate the stacks.
template<class Fn>
int iterate_stacks(Fn &callback, lua_State *L) {
	scoped_lua scoped(L, 0);
	int top = lua_gettop(L);

	for (int idx = 1; idx <= top; ++idx) {
		char buffer[32];
		snprintf(buffer, sizeof(buffer), "stack:%d", idx);
		int ret = callback(L, buffer, idx);
		if (ret != 0) {
			return ret;
		}
	}

	return 0;
}

/// Iterates the local fields.
template<class Fn>
int iterate_locals(Fn &callback, lua_State *L, int level,
						  bool checkLocal, bool checkUpvalue,
						  bool checkEnviron) {
	scoped_lua scoped(L, 0);
	lua_Debug ar;
	const char *name;

	if (lua_getstack(L, level, &ar) == 0) {
		// The stack level don't exist.
		return -1;
	}

	// Check the local value. (1 is the initial value)
	if (checkLocal) {
		for (int i = 1; (name = lua_getlocal(L, &ar, i)) != NULL; ++i) {
			if (strcmp(name, "(*temporary)") != 0) {
				int ret = callback(L, name, lua_gettop(L));
				if (ret != 0) {
					lua_pop(L, 1);
					return ret;
				}
			}
			lua_pop(L, 1); // Eliminate the local value.
		}
	}
	
	// Get the local function.
	if (lua_getinfo(L, "f", &ar) != 0) {
		// Check the upvalue.
		if (checkUpvalue) {
			for (int i = 1; (name = lua_getupvalue(L, -1, i)) != NULL; ++i) {
				if (strcmp(name, "(*temporary)") != 0) {
					int ret = callback(L, name, lua_gettop(L));
					if (ret != 0) {
						lua_pop(L, 2);
						return ret;
					}
				}
				lua_pop(L, 1); // eliminate the local value.
			}
		}

		// Check the environ table.
		if (checkEnviron) {
			lua_getfenv(L, -1);
			int tableIdx = lua_gettop(L);

			lua_pushnil(L);  // first key
			for (int i = 0; lua_next(L, tableIdx) != 0; ++i) {
				// key index: top - 1, value index: top
				int top = lua_gettop(L);
				int ret = callback(L, LuaToStringFast(L, top - 1), top);
				if (ret != 0) {
					lua_pop(L, 4);
					return ret;
				}

				// eliminate the value index.
				lua_pop(L, 1);
			}

			lua_pop(L, 1); // eliminate the environ table.
		}

		lua_pop(L, 1); // eliminate the local function.
	}

	return 0;
}

} // end of namespace lldebug

#endif
