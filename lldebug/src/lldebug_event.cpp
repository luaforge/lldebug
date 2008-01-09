/*
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
