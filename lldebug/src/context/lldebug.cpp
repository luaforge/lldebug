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
#include "context/codeconv.h"

static lldebug_InitState s_callbackInitState = NULL;

using namespace lldebug;
using context::Context;

lldebug_InitState lldebug_setinitstate(lldebug_InitState fn) {
	lldebug_InitState old = s_callbackInitState;
	s_callbackInitState = fn;
	return old;
}

lldebug_InitState lldebug_getinitstate(void) {
	return s_callbackInitState;
}

lua_State *lldebug_open() {
	Context *ctx = Context::Create();
	if (ctx == NULL) {
		return NULL;
	}
	
	return ctx->GetLua();
}

void lldebug_close(lua_State *L) {
	Context *ctx = Context::Find(L);

	if (ctx != NULL && ctx->GetMainLua() == L) {
		ctx->Delete();
	}
}

int lldebug_loadfile(lua_State *L, const char *filename) {
	Context *ctx = Context::Find(L);
	if (ctx == NULL || ctx->GetMainLua() != L) {
		return -1;
	}

	return ctx->LoadFile(filename);
}

int lldebug_loadstring(lua_State *L, const char *str) {
	Context *ctx = Context::Find(L);
	if (ctx == NULL || ctx->GetMainLua() != L) {
		return -1;
	}

	return ctx->LoadString(str);
}

void lldebug_call(lua_State *L, int narg, int nresult) {
	Context *ctx = Context::Find(L);
	if (ctx == NULL) {
		lua_pushliteral(L, "Couldn't find the Context object.");
		return;
	}

//	ctx->Call();
}

int lldebug_pcall(lua_State *L, int narg, int nresult, int errfunc) {
	Context *ctx = Context::Find(L);
	if (ctx == NULL) {
		lua_pushliteral(L, "Conldn't find the Context object.");
		return -1;
	}

//	return ctx->PCall();
	return 0;
}

int lldebug_resume(lua_State *L, int narg) {
	Context *ctx = Context::Find(L);
	if (ctx == NULL) {
		lua_pushliteral(L, "Conldn't find the Context object.");
		return -1;
	}

	return 0;
}

int lldebug_openbase(lua_State *L) {
	Context *ctx = Context::Find(L);
	if (ctx == NULL || ctx->GetMainLua() != L) {
		return -1;
	}

	return ctx->LuaOpenBase(L);
}

void lldebug_openlibs(lua_State *L) {
	Context *ctx = Context::Find(L);
	if (ctx == NULL || ctx->GetMainLua() != L) {
		return;
	}

	ctx->LuaOpenLibs(L);
}

int lldebug_setencoding(lldebug_Encoding encoding) {
	return lldebug::context::SetEncoding(encoding);
}

lldebug_Encoding lldebug_getencoding() {
	return lldebug::context::GetEncoding();
}
