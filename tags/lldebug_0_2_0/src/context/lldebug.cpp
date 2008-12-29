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
#include "lldebug.h"
#include "context/context.h"

using namespace lldebug;
using context::Context;

struct LogWrapper {
	lldebug_Logger m_logger;
	void *m_data;
	explicit LogWrapper(lldebug_Logger logger, void *data)
		: m_logger(logger), m_data(data) {
	}
	void operator()(shared_ptr<Context> ctx, const LogData &logData) {
		if (m_logger == NULL) {
			return;
		}

		m_logger(
			ctx->GetLua(), m_data, logData.GetLog().c_str(),
			logData.GetKey().c_str(), logData.GetLine(),
			(logData.IsRemote() ? 1 : 0));
	}
	};

void lldebug_setlogger(lua_State *L, lldebug_Logger logger, void *data) {
	shared_ptr<Context> ctx = Context::Find(L);
	if (ctx == NULL) {
		return;
	}

	ctx->SetLogger(LogWrapper(logger, data));
}

lua_State *lldebug_open() {
	shared_ptr<Context> ctx(new Context);
	if (ctx == NULL) {
		return NULL;
	}

	if (ctx->Initialize() != 0) {
		return NULL;
	}

	return ctx->GetLua();
}

void lldebug_close(lua_State *L) {
	shared_ptr<Context> ctx = Context::Find(L);

	if (ctx != NULL && ctx->GetMainLua() == L) {
		ctx->Delete();
	}
}

enum lldebug_Start {
	LLDEBUG_START,
	LLDEBUG_START_STEPIN,
	LLDEBUG_START_EDIT,
	LLDEBUG_START_WITHOUT_DEBUG,
};

int lldebug_startfile(lua_State *L, const char *filename, lldebug_Start start) {
	shared_ptr<Context> ctx = Context::Find(L);
	if (ctx == NULL || ctx->GetMainLua() != L) {
		return -1;
	}

	// return ctx->DebugFile(L, filename);
	// loadfile(L, filename);
	// main: pcall(L, 0, 0, 0);
	// return 0;
	// 
	// on_error:;
	//   while (isIdling) {
	//   }
	return 0;
}

int lldebug_loadfile(lua_State *L, const char *filename) {
	shared_ptr<Context> ctx = Context::Find(L);
	if (ctx == NULL) {
		lua_pushliteral(L, "Couldn't find the Context object.");
		return -1;
	}

	return ctx->LoadFile(L, filename);
}

int lldebug_loadstring(lua_State *L, const char *str) {
	shared_ptr<Context> ctx = Context::Find(L);
	if (ctx == NULL) {
		lua_pushliteral(L, "Couldn't find the Context object.");
		return -1;
	}

	return ctx->LoadString(L, str);
}

void lldebug_call(lua_State *L, int nargs, int nresults) {
	shared_ptr<Context> ctx = Context::Find(L);
	if (ctx == NULL) {
		lua_pushliteral(L, "Couldn't find the Context object.");
		return;
	}

	// Sorry. lua_call can't use now.
	// Please use 'lua_pcall' alternatively.
	*(unsigned long *)NULL = 0xcdcdcdcd;
	// ctx->Call(L, nargs, nresults);
}

int lldebug_pcall(lua_State *L, int nargs, int nresults, int errfunc) {
	shared_ptr<Context> ctx = Context::Find(L);
	if (ctx == NULL) {
		lua_pushliteral(L, "Conldn't find the Context object.");
		return -1;
	}

	return ctx->PCall(L, nargs, nresults, errfunc);
}

int lldebug_resume(lua_State *L, int nargs) {
	shared_ptr<Context> ctx = Context::Find(L);
	if (ctx == NULL) {
		lua_pushliteral(L, "Conldn't find the Context object.");
		return -1;
	}

	return ctx->Resume(L, nargs);
}

int lldebug_openbase(lua_State *L) {
	shared_ptr<Context> ctx = Context::Find(L);
	if (ctx == NULL || ctx->GetMainLua() != L) {
		return -1;
	}

	return ctx->LuaOpenBase(L);
}

void lldebug_openlibs(lua_State *L) {
	shared_ptr<Context> ctx = Context::Find(L);
	if (ctx == NULL || ctx->GetMainLua() != L) {
		return;
	}

	ctx->LuaOpenLibs(L);
}

int lldebug_setencoding(lua_State *L, lldebug_Encoding encoding) {
	shared_ptr<Context> ctx = Context::Find(L);
	if (ctx == NULL) {
		return -1;
	}

	ctx->SetEncoding(encoding);
	return 0;
}

lldebug_Encoding lldebug_getencoding(lua_State *L) {
	shared_ptr<Context> ctx = Context::Find(L);
	if (ctx == NULL) {
		return LLDEBUG_ENCODING_UTF8;
	}

	return ctx->GetEncoding();
}


static std::string s_hostname = "localhost";
static unsigned short s_port = 24752;

void lldebug_setremoteaddress(const char *hostname,
							  unsigned short port) {
	if (hostname != NULL) {
		s_hostname = hostname;
	}
	if (port != 0) {
		s_port = port;
	}
}

void lldebug_getremoteaddress(const char **hostname,
							  unsigned short *port) {
	if (hostname != NULL) {
		*hostname = s_hostname.c_str();
	}
	if (port != NULL) {
		*port = s_port;
	}
}
