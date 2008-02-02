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

namespace lldebug {

class wxChangedStateEvent : public wxEvent {
public:
	explicit wxChangedStateEvent(wxEventType type, int winid,
								 bool isBreak)
		: wxEvent(winid, type), m_isBreak(isBreak) {
	}

	virtual ~wxChangedStateEvent() {
	}

	/// Is state breaking (stop running) ?
	bool IsBreak() const {
		return m_isBreak;
	}

	virtual wxEvent *Clone() const {
		return new wxChangedStateEvent(*this);
	}

private:
	bool m_isBreak;
};


class wxSourceLineEvent : public wxEvent {
public:
	explicit wxSourceLineEvent(wxEventType type, int winid,
							   const std::string &key, int line,
							   int updateSourceCount)
		: wxEvent(winid, type), m_key(key), m_line(line)
		, m_updateSourceCount(m_updateSourceCount) {
	}

	virtual ~wxSourceLineEvent() {
	}

	/// Get identifier key of the source file.
	const std::string &GetKey() const {
		return m_key;
	}

	/// Get the number of line.
	int GetLine() const {
		return m_line;
	}

	/// Get the count of 'update source'.
	int GetUpdateSourceCount() const {
		return m_updateSourceCount;
	}

	virtual wxEvent *Clone() const {
		return new wxSourceLineEvent(*this);
	}

private:
	std::string m_key;
	int m_line;
	int m_updateSourceCount;
};

class wxSourceEvent : public wxEvent {
public:
	explicit wxSourceEvent(wxEventType type, int winid,
						   const Source &source)
		: wxEvent(winid, type), m_source(source) {
	}

	virtual ~wxSourceEvent() {
	}

	/// Get source object.
	const Source &GetSource() const {
		return m_source;
	}

	virtual wxEvent *Clone() const {
		return new wxSourceEvent(*this);
	}

private:
	Source m_source;
};

class wxBreakpointEvent : public wxEvent {
public:
	explicit wxBreakpointEvent(wxEventType type, int winid,
							   const BreakpointList &breakpoints)
		: wxEvent(winid, type), m_breakpoints(breakpoints) {
	}

	virtual ~wxBreakpointEvent() {
	}

	/// Get breakpoint object.
	BreakpointList &GetBreakpoints() {
		return m_breakpoints;
	}

	virtual wxEvent *Clone() const {
		return new wxBreakpointEvent(*this);
	}

private:
	BreakpointList m_breakpoints;
};


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_CHANGED_STATE, 2652)
DECLARE_EVENT_TYPE(wxEVT_UPDATE_SOURCE, 2653)
DECLARE_EVENT_TYPE(wxEVT_ADDED_SOURCE, 2654)
DECLARE_EVENT_TYPE(wxEVT_CHANGED_BREAKPOINTS, 2655)
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*wxChangedStateEventFunction)(wxChangedStateEvent &);
typedef void (wxEvtHandler::*wxSourceLineEventFunction)(wxSourceLineEvent &);
typedef void (wxEvtHandler::*wxSourceEventFunction)(wxSourceEvent &);
typedef void (wxEvtHandler::*wxBreakpointEventFunction)(wxBreakpointEvent &);

#if !wxCHECK_VERSION(2, 5, 0)
#define EVT_LLDEBUG_CHANGED_STATE(id, fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_CHANGED_STATE, id, wxCONCAT(id, _END), (wxObjectEventFunction)(wxEventFunction)(wxChangedStateEventFunction)&fn, (wxObject *)NULL),
#define EVT_LLDEBUG_UPDATE_SOURCE(id, fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_UPDATE_SOURCE, id, wxCONCAT(id, _END), (wxObjectEventFunction)(wxEventFunction)(wxSourceLineEventFunction)&fn, (wxObject *)NULL),
#else
#define EVT_LLDEBUG_CHANGED_STATE(id, fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_CHANGED_STATE, id, wxID_ANY, (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxChangedStateEventFunction, &fn), (wxObject *)NULL),
#define EVT_LLDEBUG_UPDATE_SOURCE(id, fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_UPDATE_SOURCE, id, wxID_ANY, (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxSourceLineEventFunction, &fn), (wxObject *)NULL),
#define EVT_LLDEBUG_ADDED_SOURCE(id, fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_ADDED_SOURCE, id, wxID_ANY, (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxSourceEventFunction, &fn), (wxObject *)NULL),
#define EVT_LLDEBUG_CHANGED_BREAKPOINTS(id, fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_CHANGED_BREAKPOINTS, id, wxID_ANY, (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxBreakpointEventFunction, &fn), (wxObject *)NULL),
#endif

}

#endif
