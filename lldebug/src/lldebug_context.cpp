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

#include "lldebug_prec.h"
#include "lldebug_codeconv.h"
#include "lldebug_serialization.h"
#include "lldebug_context.h"
#include "lldebug.h"

#include <fstream>

#include <shellapi.h>
#pragma comment(lib, "shell32")

#define DUMMY_FUNCNAME "__LLDEBUG_DUMMY_TEMPORARY_FUNCTION_NAME__"

namespace lldebug {

/**
 * @brief lua_State*からコンテキストを取得できるようにするクラスです。
 */
class Context::ContextManager {
public:
	explicit ContextManager() {
	}

	~ContextManager() {
		// 'lock' must be unlocked
		// at the end of this function.
		{
			scoped_lock lock(m_mutex);

			while (!m_map.empty()) {
				Context *ctx = (*m_map.begin()).second;
				ctx->Delete();
			}
		}
	}

	/// Get weather this object is empty.
	bool IsEmpty() {
		scoped_lock lock(m_mutex);

		return m_map.empty();
	}

	/// Add the pair of (L, ctx).
	void Add(Context *ctx, lua_State *L) {
		scoped_lock lock(m_mutex);

		if (ctx == NULL || L == NULL) {
			return;
		}

		m_map.insert(std::make_pair(L, ctx));
	}

	/// Erase the 'ctx' and corresponding lua_State objects.
	void Erase(Context *ctx) {
		scoped_lock lock(m_mutex);

		if (ctx == NULL) {
			return;
		}

		for (Map::iterator it = m_map.begin(); it != m_map.end(); ) {
			Context *p = (*it).second;
 
			if (p == ctx) {
				m_map.erase(it++);
			}
			else {
				++it;
			}
		}
	}

	/// Find the Context object from a lua_State object.
	Context *Find(lua_State *L) {
		scoped_lock lock(m_mutex);

		if (L == NULL) {
			return NULL;
		}

		Map::iterator it = m_map.find(L);
		if (it == m_map.end()) {
			return NULL;
		}

		return (*it).second;
	}

private:
	typedef std::map<lua_State *, Context *> Map;
	Map m_map;
	mutex m_mutex;
};


/*-----------------------------------------------------------------*/
shared_ptr<Context::ContextManager> Context::ms_manager;
int Context::ms_idCounter = 0;

Context *Context::Create() {
	// It's impossible to use 'xxx_ptr' classes,
	// because ctx don't have public delete.
	Context *ctx = new Context;

	try {
		if (ctx->Initialize() != 0) {
			ctx->Delete();
			return NULL;
		}

		// After the all initialization was done,
		// we create a new frame for this context.
		if (ctx->CreateDebuggerFrame() != 0) {
			ctx->Delete();
			return NULL;
		}

		if (ms_manager == NULL) {
			ms_manager.reset(new ContextManager);
		}

		ms_manager->Add(ctx, ctx->GetLua());
	}
	catch (...) {
		ctx->Delete();
		return NULL;
	}

	return ctx;
}

Context::Context()
	: m_id(0), m_lua(NULL)
	, m_state(STATE_INITIAL), m_updateSourceCount(0)
	, m_isMustUpdate(false), m_engine(new RemoteEngine)
	, m_sourceManager(&*m_engine), m_breakpoints(&*m_engine) {

	m_engine->SetReadCommandCallback(
		boost::bind1st(boost::mem_fn(&Context::CommandCallback), this));
	m_id = ms_idCounter++;
}

/// Create the new debug frame.
int Context::CreateDebuggerFrame() {
	scoped_lock lock(m_mutex);

	/*HINSTANCE inst = ::ShellExecuteA(
		NULL, NULL,
		"..\\debug\\lldebug_frame.exe",
		"localhost 51123",
		"",
		SW_SHOWNORMAL);
	if ((unsigned long int)inst <= 32) {
		return -1;
	}*/

	if (m_engine->StartContext(51123, m_id, 20) != 0) {
		return -1;
	}

	return 0;
}

int Context::Initialize() {
	scoped_lock lock(m_mutex);

	// dllからの呼び出しなので普通に失敗する可能性があります。
	lua_State *L = lua_open();
	if (L == NULL) {
		return -1;
	}

	if (LuaInitialize(L) != 0) {
		lua_close(L);
		return -1;
	}

	/*lldebug_InitState init = lldebug_getinitstate();
	if (init != NULL && init(L) != 0) {
		lua_close(L);
		return -1;
	}*/

	SetHook(L, true);
	m_lua = L;
	m_state = STATE_STEPINTO; //NORMAL;
	m_coroutines.push_back(CoroutineInfo(L));
	return 0;
}

Context::~Context() {
	SaveConfig();
	m_engine.reset();

	scoped_lock lock(m_mutex);

	if (m_lua != NULL) {
		lua_close(m_lua);
		m_lua = NULL;
	}

	// 最初のコンテキストの作成に失敗している可能性があります。
	if (ms_manager != NULL) {
		ms_manager->Erase(this);

		if (ms_manager->IsEmpty()) {
			ms_manager.reset();
		}
	}
}

void Context::Delete() {
	delete this;
}

class scoped_locale {
public:
	explicit scoped_locale() {
		m_prev = std::locale::global(std::locale(""));
	}

	~scoped_locale() {
		std::locale::global(m_prev);
	}

private:
	std::locale m_prev;
};

/// Transform key string that may contain all characters such as "!"#$%&'()"
/// to string that can use a filename.
static std::string TransformKey(const std::string &key) {
	const char * s_lookupTable = 
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789"
		"_-";
	std::string result;
	unsigned int c = 0;
	int bcount = 0;

	for (std::string::size_type i = 0; i <= key.size(); ++i) {
		c = (c << 8) | (i == key.size() ? 0 : key[i]);
		bcount += 8;

		while (bcount >= 6) {
			const unsigned int shiftcount = bcount - 6;
			const unsigned int mask = ((1 << 6) - 1);
			result += s_lookupTable[(c >> shiftcount) & mask];
			c &= (1 << shiftcount) - 1;
			bcount -= 6;
		}
	}

	return result;
}

int Context::LoadConfig() {
	scoped_lock lock(m_mutex);
	scoped_locale scoped;

	try {
		std::string filename = TransformKey(m_rootFileKey);
		std::string path = GetConfigFileName(filename + ".xml");
		std::ifstream ifs;
		ifs.open(path.c_str(), std::ios::in);
		if (!ifs.is_open()) {
			return -1;
		}

		boost::archive::xml_iarchive ar(ifs);
		BreakpointList bps(&*m_engine);
		ar & BOOST_SERIALIZATION_NVP(bps);

		// set values
		m_breakpoints = bps;
		m_engine->ChangedBreakpointList(m_breakpoints);
	}
	catch (std::exception &) {
		OutputError("Couldn't open the config file.");
	}

	return 0;
}

int Context::SaveConfig() {
	scoped_lock lock(m_mutex);
	scoped_locale scoped;

	try {
		std::string filename = TransformKey(m_rootFileKey);
		std::string path = GetConfigFileName(filename + ".xml");
		std::ofstream ofs;
		ofs.open(path.c_str(), std::ios::out);
		if (!ofs.is_open()) {
			return -1;
		}

		boost::archive::xml_oarchive ar(ofs);
		ar & LLDEBUG_MEMBER_NVP(breakpoints);
	}
	catch (std::exception &) {
		OutputError("Couldn't save the config file.");
	}

	return 0;
}

Context *Context::Find(lua_State *L) {
	if (ms_manager == NULL) {
		return NULL;
	}

	return ms_manager->Find(L);
}

void Context::Quit() {
	scoped_lock lock(m_mutex);

	SetState(STATE_QUIT);
}

void Context::OutputLuaError(const char *cstr) {
	scoped_lock lock(m_mutex);

	if (cstr == NULL) {
		return;
	}

	// FILENAME:LINE:str...
	std::string str = cstr;
	bool found = false;
	int line;
	std::string::size_type pos, prevPos = 0;
	while ((prevPos = str.find(':', prevPos)) != str.npos) {
		pos = prevPos;

		// parse line number
		line = 0;
		while (++pos < str.length()) {
			char c = str[pos];

			if (c < '0' || '9' < c) {
				break;
			}
			line = line * 10 + (c - '0');
		}

		// If there is ':', parse is success.
		if (pos < str.length() && str[pos] == ':') {
			found = true;
			break;
		}

		// str[prevPos] is ':', so advance prevPos.
		++prevPos;
	}

	// source is file, key:line: str...
	// source is string, [string "***"]:line: str...
	//               ,or [string "***..."]:line: str...
	// If found is true, str[prevPos] is first ':', str[pos] is second ':'.
	std::string key;
	if (found) {
		std::string filename = str.substr(0, prevPos);
		const char *beginStr = "[string \"";
		const char *endStr = "\"]";

		// If source is string, key is manipulated and may be shorten.
		if (filename.find(beginStr) == 0 &&
			filename.rfind(endStr) == filename.length() - strlen(endStr)) {
			filename = filename.substr(
				strlen(beginStr),
				filename.length() - strlen(beginStr) - strlen(endStr));

			// If key is "***...", "..." must be removed.
			if (filename.rfind("...") == filename.length() - 3) {
				filename = filename.substr(0, filename.length() - 3);
			}
 
			// Replace key if any.
			const Source *source = m_sourceManager.GetString(filename);
			if (source != NULL) {
				key = source->GetKey();
				str = str.substr(std::min(pos + 2, str.length())); // skip ": "
			}
			else if (filename == DUMMY_FUNCNAME) {
				str = str.substr(std::min(pos + 2, str.length())); // skip ": "
				m_engine->OutputLog(LOGTYPE_INTERACTIVE, str, key, line);
				return;
			}
		}
		else {
			key = std::string("@") + filename;
			str = str.substr(std::min(pos + 2, str.length())); // skip ": "
		}
	}

	// If parse is success, str, filename, and line are modified correctly.
	m_engine->OutputLog(LOGTYPE_ERROR, str, key, line);
}

void Context::OutputLog(LogType type, const std::string &str) {
	scoped_lock lock(m_mutex);

	m_engine->OutputLog(type, str, "", -1);
}

void Context::OutputLog(const std::string &str) {
	scoped_lock lock(m_mutex);

	m_engine->OutputLog(LOGTYPE_MESSAGE, str, "", -1);
}

void Context::OutputError(const std::string &str) {
	scoped_lock lock(m_mutex);

	m_engine->OutputLog(LOGTYPE_ERROR, str, "", -1);
}

void Context::BeginCoroutine(lua_State *L) {
	scoped_lock lock(m_mutex);

	CoroutineInfo info(L);
	m_coroutines.push_back(info);
}

void Context::EndCoroutine(lua_State *L) {
	scoped_lock lock(m_mutex);

	if (m_coroutines.empty() || m_coroutines.back().L != L) {
		assert(0 && "Couldn't end coroutine.");
		return;
	}

	// ステップオーバーが設定されたコルーチンを抜けるときは
	// 強制的にブレイクします。
	if (m_state == STATE_STEPOVER || m_state == STATE_STEPRETURN) {
		if (m_stepinfo.L == L) {
			SetState(STATE_BREAK);
		}
	}

	m_coroutines.pop_back();
}

void Context::SetHook(lua_State *L, bool enable) {
	if (enable) {
		int mask = LUA_MASKLINE | LUA_MASKCALL | LUA_MASKRET;
		lua_sethook(L, Context::s_HookCallback, mask, 0);
	}
	else {
		lua_sethook(L, NULL, 0, 0);
	}
}

void Context::s_HookCallback(lua_State *L, lua_Debug *ar) {
	Context *ctx = Context::Find(L);

	if (ctx != NULL) {
		ctx->HookCallback(L, ar);
	}
}

void Context::SetState(State state) {
	assert(state != STATE_INITIAL);
	scoped_lock lock(m_mutex);

	if (state == m_state) {
		return;
	}

	if (state == STATE_QUIT) {
		m_state = state;
		m_readCommandQueueCond.notify_all();
		return;
	}

	// 状態遷移を行います。
	switch (m_state) {
	case STATE_INITIAL:
		m_state = state;
		break;
	case STATE_NORMAL: // running or stable
		switch (state) {
		case STATE_BREAK:
			m_state = state;
			m_engine->ChangedState(true);
			break;
		case STATE_STEPOVER:
		case STATE_STEPINTO:
		case STATE_STEPRETURN:
			/* ignore */
			break;
		default:
			/* error */
			assert(false && "Value of 'state' is illegal.");
			return;
		}
		break;
	case STATE_BREAK:
		switch (state) {
		case STATE_NORMAL:
			m_state = state;
			m_engine->ChangedState(false);
			break;
		case STATE_STEPOVER:
		case STATE_STEPINTO:
		case STATE_STEPRETURN:
			m_state = state;
			if (state == STATE_STEPOVER || m_state == STATE_STEPRETURN) {
				//m_engine.ChangedState(false);
			}
			break;
		default:
			/* error */
			assert(false && "Value of 'state' is illegal.");
			return;
		}
		break;
	case STATE_STEPOVER:
	case STATE_STEPINTO:
	case STATE_STEPRETURN:
		switch (state) {
		case STATE_NORMAL:
		case STATE_STEPOVER:
		case STATE_STEPINTO:
		case STATE_STEPRETURN:
			/* ignore */
			break;
		case STATE_BREAK:
			if (m_state == STATE_STEPOVER || m_state == STATE_STEPRETURN) {
				//m_engine.ChangedState(true);
			}
			m_state = state;
			break;
		default:
			/* error */
			assert(false && "Value of 'state' is illegal.");
			return;
		}
		break;
	case STATE_QUIT:
		/* ignore */
		break;
	default:
		/* error */
		assert(false && "Value of 'm_state' is illegal.");
		return;
	}

	// ステップオーバーなら情報を設定します。
	if (m_state == STATE_STEPOVER || m_state == STATE_STEPRETURN) {
		m_stepinfo = m_coroutines.back();
	}
}

void Context::CommandCallback(const Command &command) {
	scoped_lock lock(m_mutex);

	m_readCommandQueue.push(command);
	m_readCommandQueueCond.notify_all();
}

int Context::HandleCommand() {
	scoped_lock lock(m_mutex);

	while (!m_readCommandQueue.empty()) {
		Command command = m_readCommandQueue.front();
		m_readCommandQueue.pop();

		switch (command.GetType()) {
		case REMOTECOMMANDTYPE_END_CONNECTION:
			Quit();
			return -1;
		case REMOTECOMMANDTYPE_BREAK:
			SetState(STATE_BREAK);
			break;
		case REMOTECOMMANDTYPE_RESUME:
			SetState(STATE_NORMAL);
			break;
		case REMOTECOMMANDTYPE_STEPINTO:
			SetState(STATE_STEPINTO);
			break;
		case REMOTECOMMANDTYPE_STEPOVER:
			SetState(STATE_STEPOVER);
			break;
		case REMOTECOMMANDTYPE_STEPRETURN:
			SetState(STATE_STEPRETURN);
			break;

		case REMOTECOMMANDTYPE_EVAL:
			{
				std::string str;
				command.GetData().Get_Eval(str);
				LuaEval(str);
			}
			break;

		case REMOTECOMMANDTYPE_SET_BREAKPOINT:
			{
				Breakpoint bp;
				command.GetData().Get_SetBreakpoint(bp);
				m_breakpoints.Set(bp);
			}
			break;
		case REMOTECOMMANDTYPE_REMOVE_BREAKPOINT:
			{
				Breakpoint bp;
				command.GetData().Get_RemoveBreakpoint(bp);
				m_breakpoints.Remove(bp);
			}
			break;

		case REMOTECOMMANDTYPE_REQUEST_FIELDSVARLIST:
			{
				LuaVar var;
				command.GetData().Get_RequestFieldVarList(var);
				m_engine->ResponseVarList(command, LuaGetFields(var));
			}
			break;
		case REMOTECOMMANDTYPE_REQUEST_LOCALVARLIST:
			{
				LuaStackFrame stackFrame;
				command.GetData().Get_RequestLocalVarList(stackFrame);
				m_engine->ResponseVarList(command,
					LuaGetLocals(stackFrame.GetLua().GetState(), stackFrame.GetLevel()));
			}
			break;
		case REMOTECOMMANDTYPE_REQUEST_ENVIRONVARLIST:
			{
				LuaStackFrame stackFrame;
				command.GetData().Get_RequestEnvironVarList(stackFrame);
				m_engine->ResponseVarList(command,
					LuaGetEnviron(stackFrame.GetLua().GetState(), stackFrame.GetLevel()));
			}
			break;
		case REMOTECOMMANDTYPE_REQUEST_GLOBALVARLIST:
			m_engine->ResponseVarList(command, LuaGetFields(TABLETYPE_GLOBAL));
			break;
		case REMOTECOMMANDTYPE_REQUEST_REGISTRYVARLIST:
			m_engine->ResponseVarList(command, LuaGetFields(TABLETYPE_REGISTRY));
			break;
		case REMOTECOMMANDTYPE_REQUEST_STACKLIST:
			m_engine->ResponseVarList(command, LuaGetStack());
			break;
		case REMOTECOMMANDTYPE_REQUEST_BACKTRACE:
			//m_engine->ResponseBacktrace(command, LuaGetBacktrace());
			break;
		}
	}

	return 0;
}

void Context::HookCallback(lua_State *L, lua_Debug *ar) {
	scoped_lock lock(m_mutex);

	assert(m_state != STATE_INITIAL && "Not initialized !!!");

	switch (ar->event) {
	case LUA_HOOKCALL:
		++m_coroutines.back().call;
		return;
	case LUA_HOOKRET:
	case LUA_HOOKTAILRET:
		if (m_state == STATE_STEPRETURN) {
			const CoroutineInfo &info = m_coroutines.back();
			if (m_stepinfo.L == info.L  && info.call <= m_stepinfo.call) {
				SetState(STATE_BREAK);
			}
		}
		--m_coroutines.back().call;
		return;
	default:
		break;
	}

	// Stop running if need.
	switch (m_state) {
	case STATE_QUIT:
		luaL_error(L, "quited");
		return;
	case STATE_STEPOVER: {
		const CoroutineInfo &info = m_coroutines.back();
		if (m_stepinfo.L == info.L  && info.call <= m_stepinfo.call) {
			SetState(STATE_BREAK);
		}
		}
		break;
	case STATE_STEPINTO:
		SetState(STATE_BREAK);
		break;
	case STATE_NORMAL:
	case STATE_STEPRETURN:
	case STATE_BREAK:
		break;
	case STATE_INITIAL:
		/* error */
		assert(false && "Value of 'state' is illegal.");
		break;
	}

	// Get the infomation of the current function.
	lua_getinfo(L, "nSl", ar);

	// Break and stop program, if any.
	Breakpoint bp = FindBreakpoint(ar->source, ar->currentline - 1);
	if (bp.IsOk()) {
		SetState(STATE_BREAK);
	}

	// ブレイクにつき一回フレームを更新するために必要です。
	State prevState = STATE_NORMAL;
	for (;;) {
		// handle event and message queue
		if (HandleCommand() != 0 || !m_engine->IsConnected()) {
			luaL_error(L, "quited");
			return;
		}

		// Break this loop if the state isn't STATE_BREAK.
		if (m_state != STATE_BREAK) {
			break;
		}
		else {
			if (m_isMustUpdate || prevState != STATE_BREAK) {
				if (m_sourceManager.Get(ar->source) == NULL) {
					m_sourceManager.Add(ar->source);
				}
				m_isMustUpdate = false;

				BooleanCallbackWaiter waiter;
				m_engine->UpdateSource(
					ar->source, ar->currentline,
					++m_updateSourceCount, waiter);
				lock.unlock();
				waiter.Wait();
				lock.lock();
			}
			prevState = m_state;

			if (m_readCommandQueue.empty()) {
				boost::xtime xt;
				boost::xtime_get(&xt, boost::TIME_UTC);
				xt.sec += 1;
				m_readCommandQueueCond.timed_wait(lock, xt);
			}
		}
	}
}

/**
 * @brief lua用の関数を実装しています。
 */
class Context::LuaImpl {
public:
	static int atpanic(lua_State *L) {
		printf("%s\n", lua_tostring(L, -1));
		return -1;
	}

	static int cocreate(lua_State *L) {
		lua_State *NL = lua_newthread(L);
		luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1), 1,
			"Lua function expected");
		lua_pushvalue(L, 1);  /* move function to top */
		lua_xmove(L, NL, 1);  /* move function from L to NL */

		lua_atpanic(NL, atpanic);
		Context::SetHook(NL, true);
		Context::ms_manager->Add(Context::Find(L), NL);
		return 1;
	}

	static int auxresume (lua_State *L, lua_State *co, int narg) {
		Context *ctx = Context::Find(L);
		int status;
		if (!lua_checkstack(co, narg))
			luaL_error(L, "too many arguments to resume");
		lua_xmove(L, co, narg);

		ctx->BeginCoroutine(co);
		status = lua_resume(co, narg);
		ctx->EndCoroutine(co);

		if (status == 0 || status == LUA_YIELD) {
			int nres = lua_gettop(co);
			if (!lua_checkstack(L, nres))
				luaL_error(L, "too many results to resume");
			lua_xmove(co, L, nres);  /* move yielded values */
			return nres;
		}
		else {
			lua_xmove(co, L, 1);  /* move error message */
			return -1;  /* error flag */
		}
	}

	static int coresume(lua_State *L) {
		lua_State *co = lua_tothread(L, 1);
		int r;
		luaL_argcheck(L, co, 1, "coroutine expected");
		r = auxresume(L, co, lua_gettop(L) - 1);
		if (r < 0) {
			printf("%s\n", lua_tostring(L, -1));
			lua_pushboolean(L, 0);
			lua_insert(L, -2);
			return 2;  /* return false + error message */
		}
		else {
			lua_pushboolean(L, 1);
			lua_insert(L, -(r + 1));
			return r + 1;  /* return true + `resume' returns */
		}
	}

	static void override_baselib(lua_State *L) {
		const luaL_reg s_coregs[] = {
			{"create", cocreate},
			{"resume", coresume},
			{NULL, NULL}
		};

		luaL_openlib(L, LUA_COLIBNAME, s_coregs, 0);
	}

	/*---------------------------------------*/
	static int index_for_eval(lua_State *L) {
		Context *ctx = Context::Find(L);
		if (ctx == NULL) {
			return 0;
		}

		return ctx->LuaIndexForEval(L);
	}

	static int newindex_for_eval(lua_State *L) {
		Context *ctx = Context::Find(L);
		if (ctx == NULL) {
			return 0;
		}

		return ctx->LuaNewindexForEval(L);
	}

	static int output_interactive(lua_State *L) {
		Context *ctx = Context::Find(L);
		if (ctx == NULL) {
			return 0;
		}

		std::string str = LuaToString(L, -1);
		str += ", [";
		str += lua_typename(L, lua_type(L, -1));
		str += "]";

		// Output log to InteractiveView.
		ctx->OutputLog(LOGTYPE_INTERACTIVE, str);
		return 0;
	}
};

int Context::LuaInitialize(lua_State *L) {
	scoped_lock lock(m_mutex);

	// lua_register macro may be changed a little on the specifiy lua version.
#define lldebug_register(L, name, func) \
	lua_pushliteral(L, name); \
	lua_pushcclosure(L, func, 0); \
	lua_settable(L, LUA_GLOBALSINDEX);

	lldebug_register(L,
		"lldebug_output_interactive",
		LuaImpl::output_interactive);

	lldebug_register(L,
		"lldebug_index_for_eval",
		LuaImpl::index_for_eval);

	lldebug_register(L,
		"lldebug_newindex_for_eval",
		LuaImpl::newindex_for_eval);

#undef lldebug_register

	lua_atpanic(L, LuaImpl::atpanic);
	return 0;
}

int Context::LoadFile(const char *filename) {
	scoped_lock lock(m_mutex);

	if (filename == NULL) {
		return -1;
	}

	if (luaL_loadfile(m_lua, filename) != 0) {
		m_sourceManager.Add(std::string("@") + filename);
		OutputLuaError(lua_tostring(m_lua, -1));
		return -1;
	}

	// Save the first key.
	if (m_rootFileKey.empty()) {
		m_rootFileKey = "@";
		m_rootFileKey += filename;
		LoadConfig();
	}

	m_sourceManager.Add(std::string("@") + filename);
	return 0;
}

int Context::LoadString(const char *str) {
	scoped_lock lock(m_mutex);
	
	if (str == NULL) {
		return -1;
	}

	if (luaL_loadstring(m_lua, str) != 0) {
		m_sourceManager.Add(str);
		OutputLuaError(lua_tostring(m_lua, -1));
		return -1;
	}

	// Save the first key.
	if (m_rootFileKey.empty()) {
		m_rootFileKey = str;
		LoadConfig();
	}

	m_sourceManager.Add(str);
	return 0;
}

int Context::LuaOpenBase(lua_State *L) {
	scoped_lock lock(m_mutex);

	if (luaopen_base(L) == 0) {
		return 0;
	}

	LuaImpl::override_baselib(L);
	return 2;
}

void Context::LuaOpenLibs(lua_State *L) {
	scoped_lock lock(m_mutex);

	luaL_openlibs(L);
	LuaImpl::override_baselib(L);
}


/*-----------------------------------------------------------------*/
/**
 * @brief Iteration callback.
 *
 * To use template causes less portability.
 */
struct iiterate_callback {
	explicit iiterate_callback() {}
	virtual ~iiterate_callback() {}

	virtual int on_callback(lua_State *L, const std::string &name,
							int valueIndex) = 0;
};

/**
 * @brief The fields iterator.
 */
class field_iterator {
public:
	explicit field_iterator(iiterate_callback *callback)
		: m_callback(callback), m_result(-1) {
		m_curpos = m_parentList.end();
	}

	explicit field_iterator(shared_ptr<LuaVar> var, iiterate_callback *callback)
		: m_var(var), m_callback(callback), m_result(-1) {

		// restructure parents tree to list
		for (shared_ptr<LuaVar> tmp = var;
			tmp != NULL;
			tmp = tmp->GetParent()) {
			m_parentList.push_front(tmp);
		}

		m_curpos = m_parentList.begin();
	}

	/// Iterate the all fields of var.
	int iterate() {
		lua_State *L = m_var->GetLua().GetState();
		Context::scoped_lua scoped(L, 0);

		m_curpos = m_parentList.begin();
		if (m_curpos != m_parentList.end()) {
			++m_curpos; // root table has pushed
		}

		// push root table value
		switch (m_var->GetRootType()) {
		case VARROOT_GLOBAL:
			lua_pushvalue(L, LUA_GLOBALSINDEX);
			iterate_fields(L, lua_gettop(L));
			lua_pop(L, 1);
			break;
		case VARROOT_REGISTRY:
			lua_pushvalue(L, LUA_REGISTRYINDEX);
			iterate_fields(L, lua_gettop(L));
			lua_pop(L, 1);
			break;
		case VARROOT_LOCAL:
			iterate_locals(L, m_var->GetLevel(), true, true, false);
			break;
		case VARROOT_ENVIRON:
			iterate_locals(L, m_var->GetLevel(), false, false, true);
			break;
		case VARROOT_STACK:
			iterate_stacks(L);
			break;
		default:
			return -1;
		}

		return m_result;
	}

	/// Iterate the all fields of idx object.
	int iterate_fields(lua_State *L, int idx) {
		Context::scoped_lua scoped(L, 0);

		// check metatable
		if (lua_getmetatable(L, idx)) {
			int top = lua_gettop(L);
			// name of metatable is "(*metatable)"
			int ret = on_listup(L, std::string("(*metatable)"), top);
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
			int ret = on_listup(L, LuaToString(L, top - 1), top);
			if (ret != 0) {
				lua_pop(L, 2);
				return ret;
			}

			// eliminate the value index and pushed value and key index
			lua_pop(L, 1);
		}

		return 0;
	}

	/// Iterates the local fields.
	int iterate_locals(lua_State *L, int level,
					   bool checkLocal, bool checkUpvalue,
					   bool checkEnviron) {
		Context::scoped_lua scoped(L, 0);
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
					int ret = on_listup(L, ConvToUTF8(name), lua_gettop(L));
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
						int ret = on_listup(L, ConvToUTF8(name), lua_gettop(L));
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
					int ret = on_listup(L, LuaToString(L, top - 1), top);
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

	/// Iterate the stacks.
	int iterate_stacks(lua_State *L) {
		Context::scoped_lua scoped(L, 0);
		int top = lua_gettop(L);

		for (int idx = 1; idx <= top; ++idx) {
			char buffer[32];
			snprintf(buffer, sizeof(buffer), "stack:%d", idx);
			int ret = on_listup(L, buffer, idx);
			if (ret != 0) {
				return ret;
			}
		}

		return 0;
	}

private:
	int on_listup(lua_State *L, const std::string &name, int valueIdx) {
		if (m_curpos != m_parentList.end()) {
			// もしまだ親がいたら、さらにvarの親テーブルをたどります。
			if ((*m_curpos)->GetName() == name) {
				++m_curpos;
				iterate_fields(L, valueIdx);
				return 2; // return non zero value
			}

			// continue to iterate
			return 0;
		}

		return (m_result = m_callback->on_callback(L, name, valueIdx));
	}

private:
	shared_ptr<LuaVar> m_var;
	iiterate_callback *m_callback;
	int m_result;

	typedef std::list<shared_ptr<LuaVar> > LuaVarParentList;
	LuaVarParentList m_parentList; ///< parent list of var
	LuaVarParentList::iterator m_curpos;
};

static int LuaIterateFieldsOfVar(shared_ptr<LuaVar> var,
								 iiterate_callback *callback) {
	field_iterator it(var, callback);
	return it.iterate();
}

struct get_fields_callback : iiterate_callback
	{
	shared_ptr<LuaVar> var;
	LuaVarList result;
	explicit get_fields_callback(shared_ptr<LuaVar> var_)
		: var(var_) {
	}

	struct inner_callback : iiterate_callback {
		virtual int on_callback(lua_State *, const std::string &, int) {
			// To come here tells that parent object has some fields.
			return 0xffff;
		}
	};

	virtual int on_callback(lua_State *L, const std::string &name, int valueIdx) {
		shared_ptr<LuaVar> newVar(new LuaVar(var, name, valueIdx));
		// Check weather newVar have fields (newVar is table).
		inner_callback callback;
		int ret = LuaIterateFieldsOfVar(newVar, &callback);
		newVar->SetHasFields(ret == 0xffff);
		result.push_back(*newVar);
		return 0;
	}
	};

/// Iterate the all fields of var.
static LuaVarList LuaGetFieldsOfVar(shared_ptr<LuaVar> var) {
	get_fields_callback callback(var);
	if (LuaIterateFieldsOfVar(var, &callback) != 0) {
		return LuaVarList();
	}

	return callback.result;
}

LuaVarList Context::LuaGetFields(TableType type) {
	scoped_lock lock(m_mutex);
	VarRootType rootType;

	// Detect table type.
	switch (type) {
	case TABLETYPE_GLOBAL:
		rootType = VARROOT_GLOBAL;
		break;
	case TABLETYPE_REGISTRY:
		rootType = VARROOT_REGISTRY;
		break;
	default:
		assert(0 && "type [:TableType] is invalid");
		return LuaVarList();
	}

	// Get the fields of the table.
	shared_ptr<LuaVar> var(new LuaVar(GetLua(), rootType));
	return LuaGetFieldsOfVar(var);
}

LuaVarList Context::LuaGetFields(const LuaVar &var) {
	scoped_lock lock(m_mutex);

	// Get the fields of var.
	shared_ptr<LuaVar> newVar(new LuaVar(var));
	return LuaGetFieldsOfVar(newVar);
}

LuaVarList Context::LuaGetLocals(lua_State *L, int level) {
	scoped_lock lock(m_mutex);
	
	shared_ptr<LuaVar> var(
		new LuaVar((L != NULL ? L : GetLua()), VARROOT_LOCAL, level));
	return LuaGetFieldsOfVar(var);
}

LuaVarList Context::LuaGetEnviron(lua_State *L, int level) {
	scoped_lock lock(m_mutex);
	
	shared_ptr<LuaVar> var(
		new LuaVar((L != NULL ? L : GetLua()), VARROOT_ENVIRON, level));
	return LuaGetFieldsOfVar(var);
}

LuaStackList Context::LuaGetStack() {
	scoped_lock lock(m_mutex);
	shared_ptr<LuaVar> var(new LuaVar(GetLua(), VARROOT_STACK));
	return LuaGetFieldsOfVar(var);
}

/// Get the value of the local variable.
static int LuaGetValueLocal(lua_State *L, int level,
							const std::string &target,
							bool checkEnv) {
	Context::scoped_lua scoped(L, 1);
	const char *cname;
	lua_Debug ar;

	if (lua_getstack(L, level, &ar) == 0) {
		// The stack level don't exist.
		scoped.reset_stackn(0);
		return -1;
	}

	// Check the local value. (1 is the initial value)
	for (int i = 1; (cname = lua_getlocal(L, &ar, i)) != NULL; ++i) {
		if (target == cname) {
			return 0;
		}
		lua_pop(L, 1); // Eliminate the local value.
	}

	// Get the local function.
	if (lua_getinfo(L, "f", &ar) != 0) {
		int funcpos = lua_gettop(L);

		// Check the upvalue.
		for (int i = 1; (cname = lua_getupvalue(L, -1, i)) != NULL; ++i) {
			if (target == cname) {
				lua_remove(L, funcpos); // Eliminate the local function.
				return 0; // find !
			}
			lua_pop(L, 1); // Eliminate the upvalue.
		}

		// Check the environment of the function.
		if (checkEnv) {
			lua_getfenv(L, funcpos); // Push the environment table.
			int tableIdx = lua_gettop(L);

			lua_pushnil(L); // first key
			for (int i = 0; lua_next(L, tableIdx) != 0; ++i) {
				// key index: -2, value index: -1
				std::string name = LuaToString(L, -2);
				if (target == name) {
					lua_replace(L, funcpos); // Exchange funcpos and value.
					lua_settop(L, funcpos);
					return 0;
				}

				// eliminate the value index and pushed value and key index
				lua_pop(L, 1);
			}
			lua_pop(L, 1); // Eliminate the environment table.local function.
		}

		lua_pop(L, 1); // Eliminate the local function.
	}

	scoped.reset_stackn(0);
	return -1;
}

int Context::LuaIndexForEval(lua_State *L) {
	scoped_lock lock(m_mutex);
	scoped_lua scoped(L, 1);

	if (!lua_isstring(L, 2)) {
		lua_pushnil(L);
		return 1;
	}

	std::string target(lua_tostring(L, 2));

	/*lua_getstack(L, 1, &ar);
	lua_getinfo(L, "Snl", &ar);
	std::string fname = LuaMakeFuncName(&ar);*/

	// level 0: this C function
	// level 1: __lldebug_dummy__ function
	// level 2: loaded string in LuaEval (includes __lldebug_dummy__)
	// level 3: current running function
	if (LuaGetValueLocal(L, 1, target, false) == 0) {
		return 1;
	}

	if (LuaGetValueLocal(L, 3, target, true) == 0) {
		return 1;
	}

	lua_pushnil(L);
	return 1; // return nil
}

/// Set the new value to the local variable.
static int LuaSetValueLocal(lua_State *L, int level,
							const std::string &target,
							int valueIdx, bool checkEnv) {
	Context::scoped_lua scoped(L, 0);
	const char *cname;
	lua_Debug ar;

	if (lua_getstack(L, level, &ar) == 0) {
		// The stack level don't exist.
		return -1;
	}

	// Check the local value. (1 is the initial value)
	for (int i = 1; (cname = lua_getlocal(L, &ar, i)) != NULL; ++i) {
		if (target == cname) {
			lua_pushvalue(L, valueIdx); // new value
			lua_setlocal(L, &ar, i);
			lua_pop(L, 1);
			return 0;
		}
		lua_pop(L, 1); // Eliminate the local value.
	}

	// Get the local function.
	if (lua_getinfo(L, "f", &ar) != 0) {
		// Check the upvalue.
		for (int i = 1; (cname = lua_getupvalue(L, -1, i)) != NULL; ++i) {
			if (target == cname) {
				lua_pushvalue(L, 3); // new value
				lua_setupvalue(L, -1, i);
				lua_pop(L, 2); // Eliminate the upvalue and the local function.
				return 0;
			}
			lua_pop(L, 1); // Eliminate the upvalue.
		}

		// Check the environment of the function.
		if (checkEnv) {
			lua_getfenv(L, -1); // Push the environment table.
			int tableIdx = lua_gettop(L);

			lua_pushnil(L); // first key
			for (int i = 0; lua_next(L, tableIdx) != 0; ++i) {
				// key index: -2, value index: -1
				std::string name = LuaToString(L, -2);
				if (target == name) {
					lua_pop(L, 1); // Eliminate the old value.
					lua_pushvalue(L, valueIdx);
					lua_settable(L, -3);
					lua_pop(L, 2);
					return 0;
				}

				// eliminate the value index and pushed value and key index
				lua_pop(L, 1);
			}

			// Force create
			if (true) {
				lua_pushlstring(L, target.c_str(), target.length());
				lua_pushvalue(L, valueIdx);
				lua_settable(L, tableIdx);
				lua_pop(L, 2);
				return 0;
			}

			lua_pop(L, 1); // Eliminate the environment table.
		}

		lua_pop(L, 1); // Eliminate the local function.
	}

	return -1;
}

int Context::LuaNewindexForEval(lua_State *L) {
	scoped_lock lock(m_mutex);
	scoped_lua scoped(L, 0);

	if (!lua_isstring(L, 2)) {
		return 0;
	}
	std::string target(lua_tostring(L, 2));
	
	// level 0: this C function
	// level 1: __lldebug_dummy__ function
	// level 2: loaded string in LuaEval (includes __lldebug_dummy__)
	// level 3: current running function
	if (LuaSetValueLocal(L, 1, target, 3, false) == 0) {
		return 0;
	}

	if (LuaSetValueLocal(L, 3, target, 3, true) == 0) {
		return 0;
	}

	// Create the new variable on global.
	/*lua_pushvalue(L, LUA_GLOBALSINDEX);
	lua_pushvalue(L, 2);
	lua_pushvalue(L, 3);
	lua_settable(L, -3);
	lua_pop(L, 1);*/

	std::string msg;
	msg = "The variable of '";
	msg += target;
	msg += "' was created on global scope.";
	OutputLog(LOGTYPE_INTERACTIVE, msg);
	return 0;
}

/**
 * @brief string reader for LuaEval.
 */
struct string_reader
	{
	std::string str;
	int state;

	explicit string_reader(const std::string &s)
		: str(s), state(0) {
	}

	static const char *exec(lua_State *L, void *data, size_t *size) {
		string_reader *reader = (string_reader *)data;
		static const char beginning[] =
			"local function __lldebug_dummy__()\n";
		static const char ending[] =
			"\nend\n"
			"setfenv(__lldebug_dummy__, setmetatable({}, {\n"
			"    __index = lldebug_index_for_eval,\n"
			"    __newindex = lldebug_newindex_for_eval\n"
			"}))\n"
			"__lldebug_dummy__()\n";

		switch (reader->state) {
		case 0: // beginning function part
			++reader->state;
			*size = (sizeof(beginning) / sizeof(char)) - 1;
			return beginning;

		case 1: // content part
			++reader->state;
			*size = reader->str.length();
			return reader->str.c_str();

		case 2: // ending function part
			++reader->state;
			*size = (sizeof(ending) / sizeof(char)) - 1;
			return ending;
		}

		return NULL;
	}
	};

int Context::LuaEval(const std::string &str, lua_State *L) {
	L = (L == NULL ? GetLua() : L);
	scoped_lock lock(m_mutex);
	scoped_lua scoped(L, 0);

	// Load string using lua_load.
	// (existence of luaL_loadstring depends on the lua version)
	string_reader reader(str);
	if (lua_load(L, string_reader::exec, &reader, DUMMY_FUNCNAME) != 0) {
		OutputLuaError(lua_tostring(L, -1));
		lua_pop(L, 1);
		return -1;
	}

	// Do execute !
	if (lua_pcall(L, 0, 0, 0) != 0) {
		OutputLuaError(lua_tostring(L, -1));
		lua_pop(L, 1);
		return -1;
	}

	m_isMustUpdate = true;
	return 0;
}

LuaBacktraceList Context::LuaGetBacktrace() {
	const int LEVEL_MAX = 1024;	/* maximum size of the stack */
	scoped_lock lock(m_mutex);
	LuaBacktraceList array;
	
	for (CoroutineList::reverse_iterator it = m_coroutines.rbegin();
		it != m_coroutines.rend();
		++it) {
		lua_State *L1 = it->L;
		scoped_lua scoped(L1, 0);
		lua_Debug ar;

		/* level 0 may be this own function */
		for (int level = 0; lua_getstack(L1, level, &ar); ++level) {
			if (level > LEVEL_MAX) {
				assert(false && "stack size is too many");
				return array;
			}

			lua_getinfo(L1, "Snl", &ar);

			// ソースタイトルはバックトレースを表示するときに絶対使われるので
			// ここで値を設定してしまいます。
			std::string sourceTitle;
			const Source *source = GetSource(ar.source);
			if (source != NULL) {
				sourceTitle = source->GetTitle();
			}

			std::string name = LuaMakeFuncName(&ar);
			array.push_back(LuaBacktrace(L1, name, ar.source, sourceTitle, ar.currentline, level));
		}
	}

	return array;
}

}

