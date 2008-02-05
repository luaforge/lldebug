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

#ifndef __LLDEBUG_CONTEXT_H__
#define __LLDEBUG_CONTEXT_H__

#include "lldebug_sysinfo.h"
#include "lldebug_luainfo.h"
#include "lldebug_remoteengine.h"

namespace lldebug {

class MainFrame;

enum TableType {
	TABLETYPE_GLOBAL,
	TABLETYPE_REGISTRY,
	TABLETYPE_ENVIRON,
};

class Context {
public:
	enum State {
		STATE_INITIAL,
		STATE_NORMAL,
		STATE_STEPOVER,
		STATE_STEPINTO,
		STATE_STEPRETURN,
		STATE_BREAK,
		STATE_QUIT,
	};

	class scoped_lua;
	friend class scoped_lua;

public:
	static Context *Create();
	virtual void Delete();

	static Context *Find(lua_State *L);
	virtual void Quit();

	/// 文字列をウィンドウに出力します。
	void OutputLuaError(const char *str);
	void OutputError(const std::string &str);
	void OutputLog(const std::string &str);

	int LoadFile(const char *filename);
	int LoadString(const char *str);

	int LuaOpenBase(lua_State *L);
	void LuaOpenLibs(lua_State *L);

	LuaVarList LuaGetLocals(lua_State *L, int level);
	LuaVarList LuaGetFields(TableType type);
	LuaVarList LuaGetFields(const LuaVar &var);
	LuaStackList LuaGetStack();
	LuaBacktraceList LuaGetBacktrace();
	int LuaEval(const std::string &str, lua_State *L = NULL);

	/// コンテキストのＩＤを取得します。
	int GetId() const {
		return m_id;
	}

	/// luaオブジェクトを取得します。
	lua_State *GetLua() {
		return m_coroutines.back().L;
	}

	/// 一番最初に作成されたluaオブジェクトを取得します。
	lua_State *GetMainLua() {
		return m_lua;
	}

	/// Get the source contents.
	const Source *GetSource(const std::string &key) {
		scoped_lock lock(m_mutex);
		return m_sourceManager.Get(key);
	}

	/// Save the source.
	int SaveSource(const std::string &key, const string_array &source) {
		scoped_lock lock(m_mutex);
		return m_sourceManager.Save(key, source);
	}

	/// Find the breakpoint.
	Breakpoint FindBreakpoint(const std::string &key, int line) {
		scoped_lock lock(m_mutex);
		return m_breakpoints.Find(key, line);
	}

	/// Find the next breakpoint.
	Breakpoint NextBreakpoint(const Breakpoint &bp) {
		scoped_lock lock(m_mutex);
		return m_breakpoints.Next(bp);
	}

	/// Set the breakpoint.
	void SetBreakpoint(const Breakpoint &bp) {
		scoped_lock lock(m_mutex);
		m_breakpoints.Set(bp);
	}

	/// Toggle on/off of the breakpoint.
	void ToggleBreakpoint(const std::string &key, int line) {
		scoped_lock lock(m_mutex);
		m_breakpoints.Toggle(key, line);
	}

	/// Toggle on/off of the breakpoint.
	void ChangedBreakpointList(const BreakpointList &bps) {
		scoped_lock lock(m_mutex);
		m_breakpoints = bps;
	}

private:
	explicit Context();
	virtual ~Context();
	virtual int Initialize();
	virtual int CreateDebuggerFrame();
	virtual int LoadConfig();
	virtual int SaveConfig();
	virtual void SetState(State state);
	virtual void CommandCallback(const Command &command);
	virtual int HandleCommand();

	static void SetHook(lua_State *L, bool enable);
	virtual void HookCallback(lua_State *L, lua_Debug *ar);
	static void s_HookCallback(lua_State *L, lua_Debug *ar);

	class LuaImpl;
	friend class LuaImpl;
	int LuaInitialize(lua_State *L);
	void BeginCoroutine(lua_State *L);
	void EndCoroutine(lua_State *L);

private:
	class ContextManager;
	static shared_ptr<ContextManager> ms_manager;
	static int ms_idCounter;

	mutex m_mutex;
	int m_id;
	lua_State *m_lua;
	State m_state;
	int m_updateSourceCount;

	/// lua_State *ごとの関数呼び出し回数を記録することで
	/// ステップオーバーを安全に実装します。
	struct CoroutineInfo {
		CoroutineInfo(lua_State *L_ = NULL, int call_ = 0)
			: L(L_), call(call_) {
		}
		lua_State *L;
		int call;
	};
	typedef std::vector<CoroutineInfo> CoroutineList;
	CoroutineList m_coroutines;
	CoroutineInfo m_stepinfo;

	shared_ptr<RemoteEngine> m_engine;
	SourceManager m_sourceManager;
	BreakpointList m_breakpoints;
	std::string m_rootFileKey;

	typedef std::queue<Command> CommandQueue;
	CommandQueue m_readCommandQueue;
	boost::condition m_readCommandQueueCond;
};

/**
 * @brief 特定のスコープでluaを使うためのクラスです。
 */
class Context::scoped_lua {
public:
	explicit scoped_lua(lua_State *L, int n, int npop = 0)
		: m_L(L), m_npop(npop) {
		m_pos = lua_gettop(L) + n;
		m_oldhook = (lua_gethook(L) != NULL);
		Context::SetHook(L, false);
	}

	~scoped_lua() {
		Context::SetHook(m_L, m_oldhook);
		//wxASSERT(m_pos == lua_gettop(m_L));
		lua_pop(m_L, m_npop);
	}

private:
	lua_State *m_L;
	int m_pos;
	int m_npop;
	bool m_oldhook;
};

}

#endif

