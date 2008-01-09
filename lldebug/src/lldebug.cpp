//---------------------------------------------------------------------------
//
// Name:        Project1App.cpp
// Author:      ‰ë”Ž
// Created:     2007/11/23 0:05:32
// Description: 
//
//---------------------------------------------------------------------------

#include "lldebug_prec.h"
#include "lldebug.h"
#include "lldebug_bootstrap.h"
#include "lldebug_context.h"

static lldebug::shared_ptr<lldebug::Bootstrap> s_boot;
static int s_count = 0;
static lldebug_InitState s_callbackInitState = NULL;

using namespace lldebug;

int lldebug_initialize() {
	if (s_boot != NULL) {
		++s_count;
		return 0;
	}

	shared_ptr<Bootstrap> boot(new Bootstrap);
	if (boot->Initialize() != 0) {
		return -1;
	}

	s_boot = boot;
	s_count = 1;
	return 0;
}

void lldebug_finalize() {
	if (s_boot == NULL || s_count <= 0) {
		return;
	}

	if (--s_count == 0) {
		s_boot->Exit();
		s_boot->WaitExit();
		s_boot.reset();
	}
}

lldebug_InitState lldebug_setinitstate(lldebug_InitState fn) {
	lldebug_InitState old = s_callbackInitState;
	s_callbackInitState = fn;
	return old;
}

lldebug_InitState lldebug_getinitstate(void) {
	return s_callbackInitState;
}

lua_State *lldebug_open() {
	if (lldebug_initialize() != 0) {
		return NULL;
	}

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
	
	lldebug_finalize();
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
