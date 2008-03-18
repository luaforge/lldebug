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
#include "context/luaiterate.h"
#include "context/context.h"
#include "context/luautils.h"

namespace lldebug {
namespace context {

/**
 * @brief variable finder
 */
struct variable_finder {
	explicit variable_finder(lua_State *L, const std::string &target)
		: m_L(L), m_target(target), m_replaced(false) {
		lua_pushnil(L); // for replace
		m_nilpos = lua_gettop(L);
	}

	~variable_finder() {
		if (!m_replaced) {
			lua_remove(m_L, m_nilpos);
		}
	}

	int operator()(lua_State *L, const std::string &name, int valueIdx) {
		if (m_target == name) {
			lua_pushvalue(L, valueIdx);
			lua_replace(L, m_nilpos); // replace valueIdx with nil
			m_replaced = true;
			return 0xffff;
		}

		return 0;
	}
	
private:
	lua_State *m_L;
	std::string m_target;
	int m_nilpos;
	bool m_replaced;
};

int find_fieldvalue(lua_State *L, int idx, const std::string &target) {
	variable_finder finder(L, target);

	int ret = iterate_fields(finder, L, idx);
	if (ret == 0xffff) {
		return 0;
	}

	return -1;
}

int set_fieldvalue(lua_State *L, int idx, const std::string &target,
				   int valueIdx, bool forceCreate) {
	scoped_lua scoped(L);

	if (lua_type(L, idx) != LUA_TTABLE) {
		return -1;
	}

	lua_pushnil(L);  // first key
	for (int i = 0; lua_next(L, idx) != 0; ++i) {
		lua_pop(L, 1); // eliminate the value index

		int key = lua_gettop(L);
		if (llutil_tostring_fast(L, key) == target) {
			lua_pushvalue(L, valueIdx);
			lua_settable(L, idx);
			scoped.check(0);
			return 0;
		}
	}

	// If you want to force to create new field...
	if (forceCreate) {
		lua_pushlstring(L, target.c_str(), target.length());
		lua_pushvalue(L, valueIdx);
		lua_settable(L, idx);
		scoped.check(0);
		return 0;
	}

	scoped.check(0);
	return -1;
}

int find_localvalue(lua_State *L, int level, const std::string &target,
					bool checkLocal, bool checkUpvalue, bool checkEnv) {
	variable_finder finder(L, target);

	int ret = iterate_locals(
		finder, L, level,
		checkLocal, checkUpvalue, checkEnv);
	if (ret == 0xffff) {
		return 0;
	}

	return -1;
}

int set_localvalue(lua_State *L, int level, const std::string &target,
				   int valueIdx, bool checkLocal, bool checkUpvalue,
				   bool checkEnv, bool forceCreate) {
	scoped_lua scoped(L);
	const char *cname;
	lua_Debug ar;

	if (lua_getstack(L, level, &ar) == 0) {
		// The stack level don't exist.
		return -1;
	}

	// Check the local value. (1 is the initial value)
	if (checkLocal) {
		for (int i = 1; (cname = lua_getlocal(L, &ar, i)) != NULL; ++i) {
			if (target == cname) {
				lua_pushvalue(L, valueIdx); // new value
				lua_setlocal(L, &ar, i);
				lua_pop(L, 1);
				scoped.check(0);
				return 0;
			}
			lua_pop(L, 1); // Eliminate the local value.
		}
	}

	// Get the local function.
	if (lua_getinfo(L, "f", &ar) != 0) {
		// Check the upvalue.
		if (checkUpvalue) {
			for (int i = 1; (cname = lua_getupvalue(L, -1, i)) != NULL; ++i) {
				if (target == cname) {
					lua_pushvalue(L, 3); // new value
					lua_setupvalue(L, -1, i);
					lua_pop(L, 2); // Eliminate the upvalue and the local function.
					scoped.check(0);
					return 0;
				}
				lua_pop(L, 1); // Eliminate the upvalue.
			}
		}

		if (checkEnv) {
			// Check the environment of the function.
			lua_getfenv(L, -1); // Push the environment table.
			int tableIdx = lua_gettop(L);
			
			lua_pushnil(L); // first key
			for (int i = 0; lua_next(L, tableIdx) != 0; ++i) {
				// key index: -2, value index: -1
				std::string name = llutil_tostring_fast(L, -2);
				if (target == name) {
					lua_pop(L, 1); // Eliminate the old value.
					lua_pushvalue(L, valueIdx);
					lua_settable(L, -3);
					lua_pop(L, 2);
					scoped.check(0);
					return 0;
				}
				
				// eliminate the value index and pushed value and key index
				lua_pop(L, 1);
			}
			
			lua_pop(L, 1); // Eliminate the environment table.
		}
		
		lua_pop(L, 1); // Eliminate the local function.
	}

	// Force create
	if (forceCreate) {
		lua_pushlstring(L, target.c_str(), target.length());
		lua_pushvalue(L, valueIdx);
		lua_settable(L, LUA_GLOBALSINDEX);
		scoped.check(0);
		return 0;
	}

	scoped.check(0);
	return -1;
}

} // end of namespace context
} // end of namespace lldebug
