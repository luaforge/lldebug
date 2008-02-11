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

#ifndef __LLDEBUG_EVENT_H__
#define __LLDEBUG_EVENT_H__

#include "lldebug_sysinfo.h"
#include "lldebug_luainfo.h"

namespace lldebug {

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_CHANGED_STATE, 2652)
DECLARE_EVENT_TYPE(wxEVT_UPDATE_SOURCE, 2653)
DECLARE_EVENT_TYPE(wxEVT_ADDED_SOURCE, 2654)
DECLARE_EVENT_TYPE(wxEVT_CHANGED_BREAKPOINTS, 2655)
DECLARE_EVENT_TYPE(wxEVT_OUTPUT_LOG, 2656)
DECLARE_EVENT_TYPE(wxEVT_OUTPUT_INTERACTIVEVIEW, 2657)
DECLARE_EVENT_TYPE(wxEVT_FOCUS_ERRORLINE, 2658)
DECLARE_EVENT_TYPE(wxEVT_FOCUS_BACKTRACELINE, 2659)
END_DECLARE_EVENT_TYPES()

class wxDebugEvent : public wxEvent {
public:
	/// ChangedBreakpointList event
	explicit wxDebugEvent(wxEventType type, int winid)
		: wxEvent(winid, type) {
		wxASSERT(type == wxEVT_CHANGED_BREAKPOINTS);
	}

	/// ChangedState event
	explicit wxDebugEvent(wxEventType type, int winid,
						  bool isBreak)
		: wxEvent(winid, type), m_isBreak(isBreak) {
		wxASSERT(type == wxEVT_CHANGED_STATE);
	}

	/// UpdateSource event
	explicit wxDebugEvent(wxEventType type, int winid,
						  const std::string &key, int line,
						  int updateCount)
		: wxEvent(winid, type), m_key(key), m_line(line)
		, m_updateCount(m_updateCount) {
		wxASSERT(type == wxEVT_UPDATE_SOURCE);
	}

	/// FocusErrorLine event
	explicit wxDebugEvent(wxEventType type, int winid,
						  const std::string &key, int line)
		: wxEvent(winid, type), m_key(key), m_line(line) {
		wxASSERT(type == wxEVT_FOCUS_ERRORLINE);
	}

	/// FocusBacktraceLine event
	explicit wxDebugEvent(wxEventType type, int winid,
						  const LuaBacktrace &bt)
		: wxEvent(winid, type), m_backtrace(bt) {
		wxASSERT(type == wxEVT_FOCUS_BACKTRACELINE);
	}

	/// AddedSource event
	explicit wxDebugEvent(wxEventType type, int winid,
						  const Source &source)
		: wxEvent(winid, type), m_source(source) {
		wxASSERT(type == wxEVT_ADDED_SOURCE);
	}

	/// OutputLog event
	explicit wxDebugEvent(wxEventType type, int winid,
						  LogType logType, const wxString &str,
						  const std::string &key, int line)
		: wxEvent(winid, type), m_str(str)
		, m_key(key), m_line(line), m_logType(logType) {
		wxASSERT(type == wxEVT_OUTPUT_LOG);
	}

	/// OutputInteractiveView event
	explicit wxDebugEvent(wxEventType type, int winid,
						 const wxString &str)
		: wxEvent(winid, type), m_str(str) {
		wxASSERT(type == wxEVT_OUTPUT_INTERACTIVEVIEW);
	}

	virtual ~wxDebugEvent() {
	}

	virtual wxEvent *Clone() const {
		return new wxDebugEvent(*this);
	}

	/// Get source object.
	const Source &GetSource() const {
		return m_source;
	}

	/// Get the string object.
	const wxString &GetStr() const {
		return m_str;
	}

	/// Get the number of line.
	int GetLine() const {
		return m_line;
	}

	/// Get identifier key of the source file.
	const std::string &GetKey() const {
		return m_key;
	}

	/// Get the log type.
	LogType GetLogType() const {
		return m_logType;
	}

	/// Get the count of 'update source'.
	int GetUpdateCount() const {
		return m_updateCount;
	}

	/// Is state breaking (stop running) ?
	bool IsBreak() const {
		return m_isBreak;
	}

private:
	Source m_source;
	LuaBacktrace m_backtrace;
	wxString m_str;
	std::string m_key;
	int m_line;
	LogType m_logType;
	int m_updateCount;
	bool m_isBreak;
};

typedef void (wxEvtHandler::*wxDebugEventFunction)(wxDebugEvent &);

#if !wxCHECK_VERSION(2, 5, 0)
#define EVT_LLDEBUG_CHANGED_STATE(id, fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_CHANGED_STATE, id, wxCONCAT(id, _END), (wxObjectEventFunction)(wxEventFunction)(wxChangedStateEventFunction)&fn, (wxObject *)NULL),
#define EVT_LLDEBUG_UPDATE_SOURCE(id, fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_UPDATE_SOURCE, id, wxCONCAT(id, _END), (wxObjectEventFunction)(wxEventFunction)(wxSourceLineEventFunction)&fn, (wxObject *)NULL),
#else
#define EVT_LLDEBUG_CHANGED_STATE(id, fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_CHANGED_STATE, id, wxID_ANY, (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxDebugEventFunction, &fn), (wxObject *)NULL),
#define EVT_LLDEBUG_UPDATE_SOURCE(id, fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_UPDATE_SOURCE, id, wxID_ANY, (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxDebugEventFunction, &fn), (wxObject *)NULL),
#define EVT_LLDEBUG_ADDED_SOURCE(id, fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_ADDED_SOURCE, id, wxID_ANY, (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxDebugEventFunction, &fn), (wxObject *)NULL),
#define EVT_LLDEBUG_CHANGED_BREAKPOINTS(id, fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_CHANGED_BREAKPOINTS, id, wxID_ANY, (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxDebugEventFunction, &fn), (wxObject *)NULL),
#define EVT_LLDEBUG_OUTPUT_LOG(id, fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_OUTPUT_LOG, id, wxID_ANY, (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxDebugEventFunction, &fn), (wxObject *)NULL),
#define EVT_LLDEBUG_OUTPUT_INTERACTIVEVIEW(id, fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_OUTPUT_INTERACTIVEVIEW, id, wxID_ANY, (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxDebugEventFunction, &fn), (wxObject *)NULL),
#define EVT_LLDEBUG_FOCUS_ERRORLINE(id, fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_FOCUS_ERRORLINE, id, wxID_ANY, (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxDebugEventFunction, &fn), (wxObject *)NULL),
#define EVT_LLDEBUG_FOCUS_BACKTRACELINE(id, fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_FOCUS_BACKTRACELINE, id, wxID_ANY, (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxDebugEventFunction, &fn), (wxObject *)NULL),
#endif

}

#endif
