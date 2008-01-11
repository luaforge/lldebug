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
#include "lldebug_event.h"
#include "lldebug_controls.h"

namespace lldebug {

DEFINE_EVENT_TYPE(wxEVT_CREATE_FRAME)
DEFINE_EVENT_TYPE(wxEVT_DESTROY_FRAME)
DEFINE_EVENT_TYPE(wxEVT_CHANGED_STATE)
DEFINE_EVENT_TYPE(wxEVT_UPDATE_SOURCE)

wxContextEvent::wxContextEvent(wxEventType type, int winid, Context *ctx)
	: wxEvent(winid, type), m_ctx(ctx) {
}

wxContextEvent::~wxContextEvent() {
}

wxChangedStateEvent::wxChangedStateEvent(wxEventType type, int winid, bool value)
	: wxEvent(winid, type), m_value(value) {
}

wxChangedStateEvent::~wxChangedStateEvent() {
}

wxSourceLineEvent::wxSourceLineEvent(wxEventType type, int winid,
									 Context *ctx, lua_State *L, bool isCurrentRunning,
									 const std::string &key, const std::string &title,
									 int line, int level)
	: wxContextEvent(type, winid, ctx)
	, m_lua(L), m_key(key), m_title(title), m_line(line)
	, m_isCurrentRunning(isCurrentRunning), m_level(level) {
}

wxSourceLineEvent::~wxSourceLineEvent() {
}

}
