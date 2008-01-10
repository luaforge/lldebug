//---------------------------------------------------------------------------
//
// Name:        Project1App.cpp
// Author:      雅博
// Created:     2007/11/23 0:05:32
// Description: 
//
//---------------------------------------------------------------------------

#include "lldebug_prec.h"
#include "lldebug_codeconv.h"
#include "lldebug_mainframe.h"
#include "lldebug_context.h"
#include "lldebug.h"

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <fstream>

namespace lldebug {

/**
 * @brief lua_State*からコンテキストを取得できるようにするクラスです。
 */
class Context::ContextManager {
public:
	explicit ContextManager();
	~ContextManager();

	Context *Find(lua_State *L);
	void Add(Context *ctx, lua_State *L);
	void Erase(Context *ctx);

private:
	typedef std::map<lua_State *, Context *> Map;
	Map m_map;
	mutex m_mutex;
};

Context::ContextManager::ContextManager() {
}

Context::ContextManager::~ContextManager() {
	scoped_lock lock(m_mutex);

	while (!m_map.empty()) {
		Context *ctx = (*m_map.begin()).second;
		ctx->Delete();
	}
}

void Context::ContextManager::Add(Context *ctx, lua_State *L) {
	if (ctx == NULL || L == NULL) {
		return;
	}

	scoped_lock lock(m_mutex);
	m_map.insert(std::make_pair(L, ctx));
}

void Context::ContextManager::Erase(Context *ctx) {
	if (ctx == NULL) {
		return;
	}

	scoped_lock lock(m_mutex);
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

Context *Context::ContextManager::Find(lua_State *L) {
	if (L == NULL) {
		return NULL;
	}

	scoped_lock lock(m_mutex);
	for (Map::iterator it = m_map.begin(); it != m_map.end(); ++it) {
		Map::value_type v = *it;

		if (v.first == L) {
			return v.second;
		}
	}

	return NULL;
}


/*-----------------------------------------------------------------*/
shared_ptr<Context::ContextManager> Context::ms_manager;
int Context::ms_idCounter = 0;

Context *Context::Create() {
	// deleteできないので、xxx_ptr関係のクラスは使えません。
	Context *ctx = new Context;

	try {
		if (ctx->Initialize() != 0) {
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
	, m_state(STATE_INITIAL)
	, m_currentSourceKey(NULL), m_currentLine(-1)
	, m_frame(NULL) {

	scoped_lock lock(m_mutex);
	m_id = ms_idCounter++;
}

int Context::Initialize() {
	scoped_lock lock(m_mutex);
	int result = 0;

	// このコンテキスト用のフレーム作成指令を出します。
	SendCreateFrameEvent(this);

	// dllからの呼び出しなので普通に失敗する可能性があります。
	lua_State *L = lua_open();
	if (L == NULL) {
		result = -1;
		goto on_end;
	}

	if (LuaInitialize(L) != 0) {
		lua_close(L);
		result = -1;
		goto on_end;
	}

	/*lldebug_InitState init = lldebug_getinitstate();
	if (init != NULL && init(L) != 0) {
		lua_close(L);
		result = -1;
		goto on_end;
	}*/

	SetHook(L, true);
	m_lua = L;
	m_state = STATE_STEPINTO; //NORMAL;
	m_coroutines.push_back(CoroutineInfo(L));

on_end:;
	// ウィンドウが作成されるのを待つ必要があります。
	while (m_frame == NULL) {
		boost::xtime xt;
		boost::xtime_get(&xt, boost::TIME_UTC);
		xt.sec += 30;
		// ３０秒待ってだめならあきらめます。
		if (!m_condFrame.timed_wait(lock, xt)) {
			result = -1;
			break;
		}
	}
	
	return result;
}

Context::~Context() {
	scoped_lock lock(m_mutex);

	SaveConfig();
	
	while (m_frame != NULL) {
		// イベントでウィンドウのクローズを通知します。
		wxCloseEvent event(wxEVT_CLOSE_WINDOW, m_frame->GetId());
		event.SetCanVeto(false);
		event.SetEventObject(m_frame);
		m_frame->AddPendingEvent(event);

		boost::xtime xt;
		boost::xtime_get(&xt, boost::TIME_UTC);
		xt.sec += 30;
		// ３０秒待ってだめならあきらめます。
		if (!m_condFrame.timed_wait(lock, xt)) {
			break;
		}
	}

	if (m_lua != NULL) {
		lua_close(m_lua);
		m_lua = NULL;
	}

	// 最初のコンテキストの作成に失敗している可能性があります。
	if (ms_manager != NULL) {
		ms_manager->Erase(this);
	}
}

void Context::Delete() {
	// delete後はthisが持ついかなるオブジェクトも使えなくなります。
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

	std::string filename = TransformKey(m_rootFileKey);
	std::string path = GetConfigFileName(filename + ".xml");
	std::ifstream ifs;

	ifs.open(path.c_str(), std::ios::in);
	if (!ifs.is_open()) {
		return -1;
	}

	boost::archive::xml_iarchive ar(ifs);
	ar & LLDEBUG_MEMBER_NVP(breakPoints);
	return 0;
}

int Context::SaveConfig() {
	scoped_lock lock(m_mutex);
	scoped_locale scoped;

	std::string filename = TransformKey(m_rootFileKey);
	std::string path = GetConfigFileName(filename + ".xml");
	std::ofstream ofs(path.c_str());
	if (!ofs.is_open()) {
		return -1;
	}

	boost::archive::xml_oarchive ar(ofs);
	ar & LLDEBUG_MEMBER_NVP(breakPoints);
	return 0;
}

Context *Context::Find(lua_State *L) {
	if (ms_manager == NULL) {
		return NULL;
	}

	return ms_manager->Find(L);
}

void Context::SetFrame(MainFrame *frame) {
	scoped_lock lock(m_mutex);

	if (m_frame == frame) {
		return;
	}

	m_frame = frame;
	if (m_frame == NULL) Quit();
	m_condFrame.notify_all();
}

void Context::Quit() {
	scoped_lock lock(m_mutex);

	m_cmdQueue.PushCommand(Command::TYPE_QUIT);
}

void Context::Output(const std::string &str) {
	scoped_lock lock(m_mutex);

	if (m_frame == NULL) {
		return;
	}

	//m_frame->Output(str);
}

void Context::OutputF(const char *fmt, ...) {
	char buffer[512];
	va_list vlist;

	va_start(vlist, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, vlist);
	va_end(vlist);

	Output(buffer);
}

void Context::BeginCoroutine(lua_State *L) {
	scoped_lock lock(m_mutex);

	CoroutineInfo info(L);
	m_coroutines.push_back(info);
}

void Context::EndCoroutine(lua_State *L) {
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
		SetFrame(NULL);
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
			m_frame->ChangedState(true);
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
			m_frame->ChangedState(false);
			break;
		case STATE_STEPOVER:
		case STATE_STEPINTO:
		case STATE_STEPRETURN:
			m_state = state;
			if (state == STATE_STEPOVER || m_state == STATE_STEPRETURN) {
				m_frame->ChangedState(false);
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
				m_frame->ChangedState(true);
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

class Context::scoped_current_source {
public:
	explicit scoped_current_source(Context *ctx, lua_Debug *ar)
		: m_ctx(ctx) {
		m_ctx->m_currentSourceKey = ar->source;
		m_ctx->m_currentLine = ar->currentline;
	}

	~scoped_current_source() {
		m_ctx->m_currentSourceKey = "";
		m_ctx->m_currentLine = -1;
	}

private:
	Context *m_ctx;
};

void Context::HookCallback(lua_State *L, lua_Debug *ar) {
	scoped_lock lock(m_mutex);

	assert(m_state != STATE_INITIAL && "Not initialized !!!");

	if (ar->event == LUA_HOOKCALL) {
		++m_coroutines.back().call;
		return;
	}
	else if (ar->event == LUA_HOOKRET || ar->event == LUA_HOOKTAILRET) {
		if (m_state == STATE_STEPRETURN) {
			const CoroutineInfo &info = m_coroutines.back();
			if (m_stepinfo.L == info.L  && info.call <= m_stepinfo.call) {
				SetState(STATE_BREAK);
			}
		}

		--m_coroutines.back().call;
		return;
	}

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

	lua_getinfo(L, "nSl", ar);
	scoped_current_source currentSource(this, ar);

	// 一番最初のファイルキーを保存します。
	if (m_rootFileKey.empty()) {
		m_rootFileKey = ar->source;
		LoadConfig();
	}

	if (FindBreakPoint(ar->source, ar->currentline - 1) != NULL) {
		SetState(STATE_BREAK);
	}

	// ブレイクにつき一回フレームを更新するために必要です。
	State prevState = STATE_NORMAL;
	do {
		// handle event and message queue
		while (!m_cmdQueue.IsEmpty()) {
			Command cmd = m_cmdQueue.Get();
			m_cmdQueue.Pop();

			switch (cmd.GetType()) {
			case Command::TYPE_NONE:
				/* ignore */
				break;
			case Command::TYPE_PAUSE:
				SetState(STATE_BREAK);
				break;
			case Command::TYPE_RESTART:
				SetState(STATE_NORMAL);
				break;
			case Command::TYPE_TOGGLE:
				//breaking = !breaking;
				break;
			case Command::TYPE_STEPOVER:
				SetState(STATE_STEPOVER);
				break;
			case Command::TYPE_STEPINTO:
				SetState(STATE_STEPINTO);
				break;
			case Command::TYPE_STEPRETURN:
				SetState(STATE_STEPRETURN);
				break;
			case Command::TYPE_QUIT:
				SetState(STATE_QUIT);
				luaL_error(L, "quited");
				return;
			}
		}

		if (prevState != STATE_BREAK && m_state == STATE_BREAK) {
			if (m_frame != NULL) {
				m_sourceManager.Add(ar->source);
				m_frame->UpdateSource(ar->source, ar->currentline);
			}
		}
		prevState = m_state;

		// デバッグコマンドを最大３０秒間待ちます。
		if (m_state == STATE_BREAK) {
			lock.unlock();
			m_cmdQueue.Wait(30);
			lock.lock();
		}
	} while (m_state == STATE_BREAK);
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

	static int callstack(lua_State *L) {
		Context *ctx = Context::Find(L);

		if (ctx == NULL) {
			return 0;
		}

		LuaBackTrace array = ctx->LuaGetBackTrace();
		for (int i = 1; i < (int)array.size(); ++i) {
			const LuaBackTraceInfo &info = array[i];
			printf("%s:%d: '%s'\n", info.GetKey().c_str(), info.GetLine(), info.GetFuncName().c_str());
		}

		return 0;
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
};

int Context::LuaInitialize(lua_State *L) {
	scoped_lock lock(m_mutex);

	// マクロを使うとLuadllのバージョンによって動作が変わる可能性があります。
	lua_pushliteral(L, "callstack");
	lua_pushcclosure(L, LuaImpl::callstack, 0);
	lua_settable(L, LUA_GLOBALSINDEX);

	lua_atpanic(L, LuaImpl::atpanic);
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
// Not to use template because of portability.
class IIterateCallback {
public:
	explicit IIterateCallback() {}
	virtual ~IIterateCallback() {}

	virtual int OnCallback(lua_State *L, const std::string &name,
							int valueIndex) = 0;
};

class LuaVarIterator {
public:
	explicit LuaVarIterator(shared_ptr<LuaVar> var, IIterateCallback *callback)
		: m_var(var), m_callback(callback), m_result(-1) {

		// restructure parents tree to list
		for (shared_ptr<LuaVar> tmp = var;
			tmp != NULL;
			tmp = tmp->GetParent()) {
			m_parentList.push_front(tmp);
		}
	}

	virtual ~LuaVarIterator() {
	}

	virtual int Iterate() {
		lua_State *L = m_var->GetLua();
		Context::scoped_lua scoped(L, 0);

		m_curpos = m_parentList.begin();
		++m_curpos; // root table has pushed

		// push root table value
		switch (m_var->GetRootType()) {
		case VARROOT_GLOBAL:
			lua_pushvalue(L, LUA_GLOBALSINDEX);
			LuaIterateFields(L, lua_gettop(L));
			lua_pop(L, 1);
			break;
		case VARROOT_REGISTRY:
			lua_pushvalue(L, LUA_REGISTRYINDEX);
			LuaIterateFields(L, lua_gettop(L));
			lua_pop(L, 1);
			break;
		case VARROOT_ENVIRON:
			lua_pushvalue(L, LUA_ENVIRONINDEX);
			LuaIterateFields(L, lua_gettop(L));
			lua_pop(L, 1);
			break;
		case VARROOT_LOCAL:
			LuaIterateLocals(L, m_var->GetLevel());
			break;
		case VARROOT_STACK:
			LuaIterateStack(L);
			break;
		default:
			return -1;
		}

		return m_result;
	}

private:
	int OnListup(lua_State *L, const std::string &name, int valueIdx) {
		if (m_curpos != m_parentList.end()) {
			// もしまだ親がいたら、さらにvarの親テーブルをたどります。
			if ((*m_curpos)->GetName() == name) {
				++m_curpos;
				LuaIterateFields(L, valueIdx);
				return 2; // return non zero value
			}

			// continue to iterate
			return 0;
		}

		return (m_result = m_callback->OnCallback(L, name, valueIdx));
	}

private:
	int LuaIterateFields(lua_State *L, int idx) {
		Context::scoped_lua scoped(L, 0);

		// check metatable
		if (lua_getmetatable(L, idx)) {
			int top = lua_gettop(L);
			// name of metatable is "__metatable"
			int ret = OnListup(L, std::string("__metatable"), top);
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
			int ret = OnListup(L, LuaToString(L, top - 1), top);
			if (ret != 0) {
				lua_pop(L, 2);
				return ret;
			}

			// eliminate the value index and pushed value and key index
			lua_pop(L, 1);
		}

		return 0;
	}

	int LuaIterateLocals(lua_State *L, int level) {
		Context::scoped_lua scoped(L, 0);
		lua_Debug ar;
		const char *name;

		if (lua_getstack(L, level, &ar) == 0) {
			// 指定されたスタックレベルが存在しません。
			return -1;
		}

		// 1が初期値です。
		for (int i = 1; (name = lua_getlocal(L, &ar, i)) != NULL; ++i) {
			if (strcmp(name, "(*temporary)") != 0) {
				int ret = OnListup(L, ConvToUTF8(name), lua_gettop(L));
				if (ret != 0) {
					lua_pop(L, 1);
					return ret;
				}
			}
			lua_pop(L, 1);  /* ローカル変数を取り除く */
		}

		lua_getinfo(L, "f", &ar);  /* 関数を問い合わせ */
		for (int i = 1; (name = lua_getupvalue(L, -1, i)) != NULL; ++i) {
			if (strcmp(name, "(*temporary)") != 0) {
				int ret = OnListup(L, ConvToUTF8(name), lua_gettop(L));
				if (ret != 0) {
					lua_pop(L, 2);
					return ret;
				}
			}
			lua_pop(L, 1);  /* 上位値を取り除く */
		}
		lua_pop(L, 1);

		return 0;
	}

	int LuaIterateStack(lua_State *L) {
		Context::scoped_lua scoped(L, 0);
		int top = lua_gettop(L);

		for (int idx = 1; idx <= top; ++idx) {
			char buffer[32];
			snprintf(buffer, sizeof(buffer), "stack:%d", idx);
			int ret = OnListup(L, buffer, idx);
			if (ret != 0) {
				return ret;
			}
		}

		return 0;
	}

private:
	shared_ptr<LuaVar> m_var;
	IIterateCallback *m_callback;
	int m_result;

	typedef std::list<shared_ptr<LuaVar> > LuaVarParentList;
	LuaVarParentList m_parentList; ///< parent list of var
	LuaVarParentList::iterator m_curpos;
};

static int LuaIterateFieldsOfVar(shared_ptr<LuaVar> var, IIterateCallback *callback) {
	LuaVarIterator it(var, callback);
	return it.Iterate();
}

struct LuaGetFieldsCallback : public IIterateCallback {
	shared_ptr<LuaVar> var;
	LuaVarList result;

	explicit LuaGetFieldsCallback(shared_ptr<LuaVar> var_)
		: var(var_) {
	}

private:
	struct CallbackInner : public IIterateCallback {
		virtual int OnCallback(lua_State *L, const std::string &name, int valueIdx) {
			return 0xFFFF;
		}
	};

	virtual int OnCallback(lua_State *L, const std::string &name, int valueIdx) {
		shared_ptr<LuaVar> newVar(new LuaVar(var, name, valueIdx));

		// Check weather newVar have fields (newVar is table).
		CallbackInner callback;
		int ret = LuaIterateFieldsOfVar(newVar, &callback);
		newVar->SetHasFields(ret == 0xFFFF);
		result.push_back(*newVar);
		return 0;
	}
};

static LuaVarList LuaGetFieldsOfVar(shared_ptr<LuaVar> var) {
	LuaGetFieldsCallback callback(var);
	if (LuaIterateFieldsOfVar(var, &callback) != 0) {
		return LuaVarList();
	}

	return callback.result;
}

LuaVarList Context::LuaGetFields(TableType type) {
	scoped_lock lock(m_mutex);
	VarRootType rootType;

	// テーブルをスタック上に積みます。
	switch (type) {
	case TABLETYPE_GLOBAL:
		rootType = VARROOT_GLOBAL;
		break;
	case TABLETYPE_REGISTRY:
		rootType = VARROOT_REGISTRY;
		break;
	case TABLETYPE_ENVIRON:
		rootType = VARROOT_ENVIRON;
		break;
	default:
		assert(0 && "type [:TableType] is invalid");
		return LuaVarList();
	}

	// テーブルに登録された変数群を取り出します。
	shared_ptr<LuaVar> var(new LuaVar(GetLua(), rootType));
	return LuaGetFieldsOfVar(var);
}

LuaVarList Context::LuaGetFields(const LuaVar &var) {
	scoped_lock lock(m_mutex);

	// テーブルに登録された変数群を取り出します。
	shared_ptr<LuaVar> newVar(new LuaVar(var));
	return LuaGetFieldsOfVar(newVar);
}

LuaVarList Context::LuaGetLocals(lua_State *L, int level) {
	scoped_lock lock(m_mutex);
	shared_ptr<LuaVar> var(
		new LuaVar((L != NULL ? L : GetLua()), VARROOT_LOCAL, level));
	return LuaGetFieldsOfVar(var);
}

LuaStackList Context::LuaGetStack() {
	scoped_lock lock(m_mutex);
	shared_ptr<LuaVar> var(new LuaVar(GetLua(), VARROOT_STACK));
	return LuaGetFieldsOfVar(var);
}

int Context::LuaEval(const std::string &str, lua_State *L) {
	scoped_lock lock(m_mutex);
	L = GetLua();
	scoped_lua scoped(L, 0);

	struct string_reader {
		explicit string_reader(const std::string &s)
			: str(s), firstpart(true) {
		}
		static const char *exec(lua_State *L, void *dt, size_t *size) {
			string_reader *reader = (string_reader *)dt;
			if (reader->firstpart) {
				reader->firstpart = false;
				*size = reader->str.length();
				return reader->str.c_str();
			}
			return NULL;
		}
		std::string str;
		bool firstpart;
	};

	// 文字列を読み込みます。
	// luaL_loadstringはLuaのバージョンによってあったり無かったりするため
	// lua_loadを使います。
	string_reader reader(str);
	if (lua_load(L, string_reader::exec, &reader, "") != 0) {
		printf("%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
		return -1;
	}

	// それを実行します。
	if (lua_pcall(L, 0, 0, 0) != 0) {
		printf("%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
		return -1;
	}

	return 0;
}

#define LEVEL_MAX	1024	/* maximum size of the stack */

LuaBackTrace Context::LuaGetBackTrace() {
	scoped_lock lock(m_mutex);
	LuaBackTrace array;
	
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

			array.push_back(LuaBackTraceInfo(L1, level, &ar, sourceTitle));
		}
	}

	return array;
}

}

